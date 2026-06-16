/*
 * List Manager for Modern Java
 * version 1.1
 * https://github.com/MrR736/FetchISO
 *
 * SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
 * SPDX-License-Identifier: MIT
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

	private static final String FETCHISO_CONFIG_FILE =
		"https://raw.githubusercontent.com/MrR736/FetchISO/main/tools/List.json";

	private static final Path TMP_DIR = Paths.get(System.getProperty("java.io.tmpdir"));
	private static final int CACHE_MINUTES = 60;



	public static class ListSource {
		public boolean isUrl;
		public String value;
		public ListSource(boolean isUrl,String value) {
			this.isUrl = isUrl;
			this.value = value;
		}
	}



	public static class JsonFiles {
		public List<String> files = new ArrayList<>();
		public List<String> tempFiles = new ArrayList<>();
	}



	/* CONFIG*/

	private static void ensureConfigExists() {
		Path path = Paths.get(CONFIG_FILE);

		if (!Files.exists(path)) {
			try (BufferedWriter writer =
				Files.newBufferedWriter(path)) {
				writer.write("# FetchISO List\n\n");
				writer.write("FIL " + FETCHISO_CONFIG_FILE + "\n");
				System.out.println("Created default config: " + CONFIG_FILE);
			} catch (IOException e) {
				System.err.println("Failed creating config: " + e.getMessage());
			}
		}
	}

	public static Optional<List<ListSource>> readConfigFile() {
		ensureConfigExists();
		List<ListSource> result = new ArrayList<>();
		try (BufferedReader reader = Files.newBufferedReader(Paths.get(CONFIG_FILE))) {
			String line;
			while ((line = reader.readLine()) != null) {
				if (line.isEmpty() || line.startsWith("#"))
					continue;

				if (!line.startsWith("FIL "))
					continue;

				String value = line.substring(4).trim();
				boolean url = value.startsWith("http://") || value.startsWith("https://");
				result.add(new ListSource(url,value));
			}
		} catch (IOException e) {
			System.err.println("Critical Error reading config: " + e.getMessage());
			return Optional.empty();
		}

		if (result.isEmpty()) {
			System.err.println("No valid entries in config");
			return Optional.empty();
		}
		return Optional.of(result);
	}





	/* DOWNLOAD*/

	public static JsonFiles getJsonFiles(List<ListSource> sources) {
		JsonFiles result = new JsonFiles();

		ExecutorService executor =
			Executors.newFixedThreadPool(Math.max(1,Math.min(sources.size(),Runtime.getRuntime().availableProcessors())));

		List<Future<?>> tasks = new ArrayList<>();
		Object lock = new Object();

		for (int i = 0; i < sources.size(); i++) {
			int index = i;
			ListSource src = sources.get(i);
			if (!src.isUrl) {
				if (Files.exists(Paths.get(src.value))) {
					result.files.add(src.value);
				} else {
					System.err.println("Missing file: " + src.value);
				}
				continue;
			}

			tasks.add(executor.submit(() -> {
					Downloader downloader = new Downloader();
					Path file = TMP_DIR.resolve("List_" + index + ".json");
					try {
						if (Files.exists(file)) {
							Duration age = Duration.between(Files.getLastModifiedTime(file).toInstant(),Instant.now());
							if (age.toMinutes() < CACHE_MINUTES) {
								synchronized(lock) {
									result.files.add(file.toString());
									result.tempFiles.add(file.toString());
								}
								return;
							}
						}

						if (downloader.downloadFile(src.value,file.toString())) {
							synchronized(lock) {
								result.files.add(file.toString());
								result.tempFiles.add(file.toString());
							}
						} else {
							System.err.println("Download failed: " + src.value + " (" + downloader.getLastError() + ")");
						}
					} catch (Exception e) {
						System.err.println("Download error: " + e.getMessage());
					}
				}));
		}

		for (Future<?> task : tasks) {
			try {
				task.get();
			} catch (Exception e) {
				System.err.println("Worker error: " + e.getMessage());
			}
		}
		executor.shutdown();
		return result;
	}

	/* JSON LOAD */
	public static JSONArray loadJsonData(List<String> files) {
		JSONArray all = new JSONArray();
		for (String file : files) {
			try {
				String content = Files.readString(Paths.get(file));
				Object parsed = new JSONTokener(content).nextValue();
				if (parsed instanceof JSONArray json) {
					for (Object obj : json) {
						all.put(obj);
					}
				} else {
					System.err.println("Invalid JSON format: " + file);
				}
			} catch (Exception e) {
				System.err.println("JSON error in " + file + ": " + e.getMessage());
			}
		}

		return all;
	}

	/* CLEANUP */
	public static void cleanupFiles(List<String> files) {
		for (String file : files) {
			try {
				Files.deleteIfExists(Paths.get(file));
			} catch (IOException ignored) { }
		}
	}

	/* MAIN ENTRY */
	public static JSONArray refreshDownloadList() {
		Optional<List<ListSource>> opt = readConfigFile();
		if (opt.isEmpty())
			return new JSONArray();
		JsonFiles files = getJsonFiles(opt.get());
		JSONArray releases = loadJsonData(files.files);
		cleanupFiles(files.tempFiles);
		return releases;
	}

	/* ISO LIST */
	public static List<ISOEntry> getISOList() {
		JSONArray releases = refreshDownloadList();
		List<ISOEntry> result = new ArrayList<>();

		for (int i = 0; i < releases.length(); i++) {
			JSONObject obj = releases.getJSONObject(i);
			result.add(new ISOEntry(obj.optString("name","all"),obj.optString("url",""),
					   obj.optString("arch",""),obj.optString("file","")));
		}
		return result;
	}

	public static class ISOEntry {
		public String name;
		public String url;
		public String arch;
		public String file;

		public ISOEntry(String name,String url,String arch,String file) {
			this.name = name;
			this.url = url;
			this.arch = arch;
			this.file = file;
		}
	}
}
