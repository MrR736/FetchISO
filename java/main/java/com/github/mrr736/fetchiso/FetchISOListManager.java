/*
// List Manager for Modern Java
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

package com.github.mrr736.fetchiso;

import com.github.mrr736.fetchiso.Downloader;

import org.json.*;
import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.util.*;
import java.util.concurrent.*;

public class FetchISOListManager {
    private static final String CONFIG_FILE = "FetchISO.list";
    private static final Path TMP_DIR = Paths.get(System.getProperty("java.io.tmpdir"));
    private static final boolean DEBUG = true;
    private static final int CACHE_DURATION_MINUTES = 60;
    private static final int THREAD_POOL_SIZE = Runtime.getRuntime().availableProcessors();

    public static List<String> getJsonFiles(List<String> urls) {
        ExecutorService executor = Executors.newFixedThreadPool(Math.min(urls.size(), THREAD_POOL_SIZE));
        List<Future<Optional<String>>> futures = new ArrayList<>();
        List<String> localFiles = new ArrayList<>();
        Downloader downloader = new Downloader();

        for (int i = 0; i < urls.size(); i++) {
            final int index = i;
            futures.add(executor.submit(() -> {
                Path localFile = TMP_DIR.resolve("List_" + index + ".json");

                if (Files.exists(localFile) &&
                    Duration.between(Files.getLastModifiedTime(localFile).toInstant(), Instant.now()).toMinutes() < CACHE_DURATION_MINUTES) {
                    if (DEBUG) System.out.println("Skipping fresh file: " + localFile);
                    return Optional.of(localFile.toString());
                }

                if (downloader.downloadFile(urls.get(index), localFile.toString())) {
                    return Optional.of(localFile.toString());
                } else {
                    System.err.println("Error downloading JSON: " + urls.get(index) + " - " + downloader.getLastError());
                    return Optional.empty();
                }
            }));
        }

        for (Future<Optional<String>> future : futures) {
            try {
                future.get().ifPresent(localFiles::add);
            } catch (Exception e) {
                System.err.println("Error processing future: " + e.getMessage());
            }
        }
        executor.shutdown();
        return localFiles;
    }

    public static void cleanupFiles(List<String> files) {
        for (String file : files) {
            try {
                Files.deleteIfExists(Paths.get(file));
                if (DEBUG) System.out.println("Removed file: " + file);
            } catch (IOException e) {
                System.err.println("Error deleting file: " + file);
            }
        }
    }

    public static JSONArray loadJsonData(List<String> files) {
        JSONArray allReleases = new JSONArray();

        for (String file : files) {
            try (BufferedReader reader = Files.newBufferedReader(Paths.get(file))) {
                JSONArray releases = new JSONArray(new JSONTokener(reader));
                releases.forEach(allReleases::put);
            } catch (Exception e) {
                System.err.println("Error processing JSON in " + file + ": " + e.getMessage());
            }
        }
        return allReleases;
    }

    public static void ensureConfigExists() {
        Path configPath = Paths.get(CONFIG_FILE);
        if (!Files.exists(configPath)) {
            try (BufferedWriter writer = Files.newBufferedWriter(configPath)) {
                writer.write("# FetchISO List\n\n");
                writer.write("FIL https://raw.githubusercontent.com/MrR736/FetchISO/main/tools/List.json\n");
                if (DEBUG) System.out.println("Created default config: " + CONFIG_FILE);
            } catch (IOException e) {
                System.err.println("Error creating " + CONFIG_FILE + ": " + e.getMessage());
            }
        }
    }

    public static Optional<List<String>> readConfigFile() {
        ensureConfigExists();
        List<String> urls = new ArrayList<>();

        try (BufferedReader reader = Files.newBufferedReader(Paths.get(CONFIG_FILE))) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (line.startsWith("FIL ")) {
                    urls.add(line.substring(4));
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading " + CONFIG_FILE + ": " + e.getMessage());
            return Optional.empty();
        }

        return urls.isEmpty() ? Optional.empty() : Optional.of(urls);
    }

    public static JSONArray refreshDownloadList() {
        Optional<List<String>> urlsOpt = readConfigFile();
        if (urlsOpt.isEmpty()) return new JSONArray();

        List<String> jsonFiles = getJsonFiles(urlsOpt.get());
        JSONArray releases = loadJsonData(jsonFiles);
        cleanupFiles(jsonFiles);

        return releases;
    }

    public static List<ISOEntry> getISOList() {
        JSONArray releases = refreshDownloadList();
        List<ISOEntry> isoEntries = new ArrayList<>();

        for (int i = 0; i < releases.length(); i++) {
            JSONObject obj = releases.getJSONObject(i);
            isoEntries.add(new ISOEntry(
                obj.optString("name", "Unknown"),
                obj.optString("url", ""),
                obj.optString("arch", ""),
                obj.optString("file", "")
            ));
        }
        return isoEntries;
    }

    public static class ISOEntry {
        public String name;
        public String url;
        public String arch;
        public String file;

        public ISOEntry(String name, String url, String arch, String file) {
            this.name = name;
            this.url = url;
            this.arch = arch;
            this.file = file;
        }
    }
}
