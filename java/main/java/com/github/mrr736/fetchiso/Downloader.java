/*
// Downloader for Modern Java
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

package com.github.mrr736.fetchiso;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Downloader {
    private String lastError = "";

    public boolean downloadFile(String url, String file) {
        String command = "";

        if (isCommandAvailable("wget")) {
            command = "wget -q --show-progress -O \"" + file + "\" \"" + url + "\" 2>&1";
        } else if (isCommandAvailable("curl")) {
            command = "curl -L --progress-bar -o \"" + file + "\" \"" + url + "\" 2>&1";
        } else if (isWindows() && isCommandAvailable("powershell")) {
            command = "powershell -Command \"Invoke-WebRequest -Uri '" + url + "' -OutFile '" + file + "'\"";
        } else {
            lastError = "No suitable download tool found (wget, curl, powershell)";
            return false;
        }

        String output = executeCommand(command);
        if (output == null) {
            lastError = "Download command execution failed.";
            return false;
        }

        if (!Files.exists(Paths.get(file))) {
            lastError = "Download completed but file does not exist: " + file;
            return false;
        }
        return true;
    }

    public String getLastError() {
        return lastError;
    }

    private boolean isCommandAvailable(String cmd) {
        String checkCmd = isWindows() ? "where " + cmd : "command -v " + cmd;
        return executeCommand(checkCmd) != null;
    }

    private String executeCommand(String command) {
        StringBuilder output = new StringBuilder();
        try {
            Process process = Runtime.getRuntime().exec(new String[]{"sh", "-c", command});
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }
            process.waitFor();
            if (process.exitValue() != 0) {
                return null;
            }
        } catch (Exception e) {
            lastError = "Command execution failed: " + e.getMessage();
            return null;
        }
        return output.toString();
    }

    private boolean isWindows() {
        return System.getProperty("os.name").toLowerCase().contains("win");
    }
}

