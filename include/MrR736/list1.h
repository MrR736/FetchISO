//
// List Manager for Modern C++
// version 1.1
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT

#ifndef LIST1_H
#define LIST1_H

#include <future>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <optional>
#include <mutex>

#include <nlohmann/json.hpp>
#include <MrR736/downloader.h>

#define FETCHISO_CONFIG_FILE "https://raw.githubusercontent.com/MrR736/FetchISO/main/tools/List.json"

using json = nlohmann::json;

static const std::string CONFIG_FILE = "FetchISO.list";

struct ListSource {
	bool is_url;
	std::string value;
};

struct JsonFiles {
	std::vector<std::string> files;       // all usable json files
	std::vector<std::string> temp_files;  // only downloaded temp files
};

/* ---------------- CONFIG ---------------- */

static inline void ensure_config_exists() {
	if (!std::filesystem::exists(CONFIG_FILE)) {
		std::ofstream f(CONFIG_FILE);
		if (f) {
			f << "# FetchISO List\n\n";
			f << "FIL " FETCHISO_CONFIG_FILE "\n";
			std::cout << "Created default config: " << CONFIG_FILE << std::endl;
		}
	}
}

static inline std::optional<std::vector<ListSource>> read_config_file() {
	ensure_config_exists();

	std::ifstream file(CONFIG_FILE);
	if (!file) {
		std::cerr << "Critical Error: Failed to read config\n";
		return std::nullopt;
	}

	std::vector<ListSource> out;
	std::string line;

	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#')
			continue;

		if (line.rfind("FIL ", 0) != 0)
			continue;

		std::string value = line.substr(4);

		bool is_url =
		value.rfind("http://", 0) == 0 ||
		value.rfind("https://", 0) == 0;

		out.push_back({is_url, std::move(value)});
	}

	if (out.empty()) {
		std::cerr << "No valid entries in config\n";
		return std::nullopt;
	}

	return out;
}

/* ---------------- DOWNLOAD / FILE HANDLING ---------------- */

static inline JsonFiles get_json_files(const std::vector<ListSource>& sources) {
	Downloader dl;
	auto tmpdir = std::filesystem::temp_directory_path();

	JsonFiles result;

	std::mutex mtx;
	std::vector<std::future<void>> futures;

	for (size_t i = 0; i < sources.size(); ++i) {

		if (!sources[i].is_url) {
			if (std::filesystem::exists(sources[i].value)) {
				result.files.push_back(sources[i].value);
			} else {
				std::cerr << "Missing file: " << sources[i].value << '\n';
			}
			continue;
		}

		futures.emplace_back(std::async(std::launch::async, [&, i]() {
			std::filesystem::path path =
			tmpdir / ("List_" + std::to_string(i) + ".json");

			std::string local_file = path.string();

			// reuse if fresh (<60 min)
			if (std::filesystem::exists(local_file)) {
				auto last_write = std::filesystem::last_write_time(local_file);
				auto now = std::filesystem::file_time_type::clock::now();

				if (std::chrono::duration_cast<std::chrono::minutes>(now - last_write).count() < 60) {
					std::lock_guard<std::mutex> lock(mtx);
					result.files.push_back(local_file);
					result.temp_files.push_back(local_file);
					return;
				}
			}

			if (dl.downloadFile(sources[i].value, local_file)) {
				std::lock_guard<std::mutex> lock(mtx);
				result.files.push_back(local_file);
				result.temp_files.push_back(local_file);
			} else {
				std::cerr << "Download failed: "
				<< sources[i].value
				<< " (" << dl.getLastError() << ")\n";
			}
		}));
	}

	for (auto& f : futures)
		f.get();

	return result;
}

/* ---------------- JSON LOADING ---------------- */

static inline json load_json_data(const std::vector<std::string>& files) {
	json all = json::array();

	for (const auto& file : files) {
		std::ifstream in(file);
		if (!in) {
			std::cerr << "Cannot open: " << file << '\n';
			continue;
		}

		try {
			json j;
			in >> j;

			if (j.is_array()) all.insert(all.end(), j.begin(), j.end());
			else std::cerr << "Invalid JSON format: " << file << '\n';
		} catch (const json::exception& e) {
			std::cerr << "JSON error in " << file << ": " << e.what() << '\n';
		}
	}

	return all;
}

/* ---------------- CLEANUP ---------------- */

static inline void cleanup_files(const std::vector<std::string>& files) {
	for (const auto& f : files) {
		std::error_code ec;
		std::filesystem::remove(f, ec);
	}
}

/* ---------------- MAIN ENTRY ---------------- */

static inline void refresh_download_list(json& releases) {
	auto sources_opt = read_config_file();
	if (!sources_opt) return;

	JsonFiles files = get_json_files(*sources_opt);

	releases = load_json_data(files.files);

	cleanup_files(files.temp_files);
}

#endif // LIST1_H
