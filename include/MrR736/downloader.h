/*
// Downloader for Modern C++
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <array>
#include <memory>
#include <filesystem>

class Downloader {
public:
	Downloader() = default;

	bool downloadFile(const std::string& url, const std::string& file) {
		std::string command;

		if (isCommandAvailable("wget")) {
			command = "wget -q --show-progress -O \"" + file + "\" \"" + url + "\" 2>&1";
		} else if (isCommandAvailable("curl")) {
			command = "curl -L --progress-bar -o \"" + file + "\" \"" + url + "\" 2>&1";
		} else {
			last_error = "No suitable download tool found (wget or curl)";
			return false;
		}

		// Execute command and capture output
		std::string output;
		int ret_code = executeCommand(command, output);

		if (ret_code != 0) {
			last_error = "Download failed with exit code: " + std::to_string(ret_code) + "\n" + output;
			return false;
		}

		// Ensure the file actually exists after download
		if (!std::filesystem::exists(file)) {
			last_error = "Download completed but file does not exist: " + file;
			return false;
		}

		return true;
	}

	std::string getLastError() const {
		return last_error;
	}

private:
	std::string last_error;

	bool isCommandAvailable(const std::string& cmd) {
#ifdef _WIN32
		std::string check_cmd = "where " + cmd + " >nul 2>nul";
#else
		std::string check_cmd = "command -v " + cmd + " >/dev/null 2>&1";
#endif
		return (std::system(check_cmd.c_str()) == 0);
	}

	int executeCommand(const std::string& command, std::string& output) {
		std::array<char, 128> buffer;
		output.clear();
		std::ostringstream result;

		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
		if (!pipe) {
			last_error = "Failed to run command: " + command;
			return -1;
		}

		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			result << buffer.data();
		}

		output = result.str();
		int exit_code = pclose(pipe.release());
		return WEXITSTATUS(exit_code);
	}
};

#endif // DOWNLOADER_H
