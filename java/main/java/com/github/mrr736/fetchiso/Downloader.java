/*
 * Downloader for Modern Java
 * version 1.1
 * https://github.com/MrR736/FetchISO
 *
 * SPDX-FileCopyrightText: 2026 MrR736
 * SPDX-License-Identifier: MIT
 */

package com.github.mrr736.fetchiso;

import java.io.*;
import java.nio.file.*;

public class Downloader {
	private volatile String lastError = "";

	public boolean downloadFile(String url,String file) {
		ProcessBuilder builder;
		try {
			if (isCommandAvailable("wget")) {
				builder = new ProcessBuilder("wget","-q","--show-progress","-O",file,url);
			} else if (isCommandAvailable("curl")) {
				builder = new ProcessBuilder("curl","-L","--progress-bar","-o",file,url);
			} else if (isWindows() && isCommandAvailable("powershell")) {
				builder = new ProcessBuilder(
					"powershell", "-Command",
					"Invoke-WebRequest -Uri \"" + url +
					"\" -OutFile \"" + file + "\""
				);

			} else {
				lastError = "No downloader found (wget, curl, powershell)";
				return false;
			}

			Process process = builder.redirectErrorStream(true).start();
			String output = readOutput(process);

			int exit = process.waitFor();
			if (exit != 0) {
				lastError = "Downloader failed: " + output;
				return false;
			}

			if (!Files.exists(Paths.get(file))) {
				lastError = "File not created: " + file;
				return false;
			}
			return true;
		} catch (Exception e) {
			lastError = "Download error: " + e.getMessage();
			return false;
		}
	}

	public String getLastError() {
		return lastError;
	}

	private boolean isCommandAvailable(String command) {
		try {
			Process process;

			if (isWindows()) {
				process = new ProcessBuilder("where",command).start();
			} else {
				process = new ProcessBuilder("sh","-c","command -v " + command).start();
			}

			return process.waitFor() == 0;
		} catch (Exception e) {
			return false;
		}
	}

	private String readOutput(Process process) {
		StringBuilder out = new StringBuilder();

		try (
			BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))
		) {

			String line;
			while ((line = reader.readLine()) != null) {
				out.append(line).append('\n');
			}
		} catch (IOException e) {
			out.append(e.getMessage());
		}

		return out.toString();
	}

	private boolean isWindows() {
		return System.getProperty("os.name").toLowerCase().contains("win");
	}
}
