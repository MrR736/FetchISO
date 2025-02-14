//
// List Manager for Modern C++
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT

#ifndef LIST1_0_H
#define LIST1_0_H

#include <future>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <optional>
#include <nlohmann/json.hpp>
#include <MrR736/downloader.h>

using json = nlohmann::json;
static const std::string CONFIG_FILE = "FetchISO.list";

/**
 * Downloads JSON files from the provided URLs.
 * @param urls List of URLs to download JSON files from.
 * @return Vector of file paths where JSONs are saved.
 */
std::vector<std::string> get_json_files(const std::vector<std::string> &urls) {
    Downloader dl;
    std::vector<std::string> local_files(urls.size());
    std::vector<std::future<bool>> futures;

    for (size_t i = 0; i < urls.size(); i++) {
        futures.emplace_back(std::async(std::launch::async, [&, i]() -> bool {
            std::ostringstream local_file_path;
            local_file_path << "/tmp/List_" << i << ".json";
            std::string local_file = local_file_path.str();

            if (std::filesystem::exists(local_file)) {
                auto last_write = std::filesystem::last_write_time(local_file);
                auto now = std::filesystem::file_time_type::clock::now();
                if (std::chrono::duration_cast<std::chrono::minutes>(now - last_write).count() < 60) {
                    std::cout << "Skipping fresh file: " << local_file << std::endl;
                    local_files[i] = local_file;
                    return true;
                }
            }

            if (dl.downloadFile(urls[i], local_file)) {
                local_files[i] = local_file;
                return true;
            } else {
                std::cerr << "Error downloading JSON: " << urls[i] << " (" << dl.getLastError() << ")\n";
                return false;
            }
        }));
    }

    for (size_t i = 0; i < futures.size(); i++) {
        if (!futures[i].get()) {
            local_files[i].clear(); // Mark failed downloads
        }
    }

    // Remove empty paths from failed downloads
    local_files.erase(std::remove(local_files.begin(), local_files.end(), ""), local_files.end());
    return local_files;
}

/**
 * Removes temporary JSON files.
 * @param files List of file paths to remove.
 */
void cleanup_files(const std::vector<std::string> &files) {
    for (const auto &file : files) {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
}

/**
 * Loads JSON data from the given list of local JSON files.
 * @param files List of local JSON file paths.
 * @return Merged JSON array containing all releases.
 */
json load_json_data(const std::vector<std::string> &files) {
    json all_releases = json::array();

    for (const auto &file : files) {
        std::ifstream input_file(file);
        if (!input_file) {
            std::cerr << "Error opening file: " << file << std::endl;
            continue;
        }

        json releases;
        try {
            input_file >> releases;
            if (releases.is_array()) {
                all_releases.insert(all_releases.end(), releases.begin(), releases.end());
            } else {
                std::cerr << "Warning: JSON in " << file << " is not an array.\n";
            }
        } catch (const json::exception &e) {
            std::cerr << "JSON Parsing error in " << file << ": " << e.what() << std::endl;
        }
    }

    return all_releases;
}

/**
 * Ensures the configuration file exists. If not, creates a default one.
 */
void ensure_config_exists() {
    if (!std::filesystem::exists(CONFIG_FILE)) {
        std::ofstream default_config(CONFIG_FILE);
        if (default_config) {
            default_config << "# FetchISO List\n\n";
            default_config << "FIL https://raw.githubusercontent.com/MrR736/FetchISO/main/tools/List.json\n";
            default_config.flush();
            std::cout << "Created default config: " << CONFIG_FILE << std::endl;
        } else {
            std::cerr << "Error creating " << CONFIG_FILE << std::endl;
        }
    }
}

/**
 * Reads the configuration file to extract JSON list URLs.
 * @return Vector of URLs found in the config file.
 */
std::optional<std::vector<std::string>> read_config_file() {
    ensure_config_exists();

    std::ifstream file(CONFIG_FILE);
    if (!file) {
        std::cerr << "Critical Error: Failed to read '" << CONFIG_FILE << "'.\n";
        return std::nullopt; // Indicate failure
    }

    std::vector<std::string> urls;
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("FIL ", 0) == 0) {
            urls.emplace_back(line.substr(4));
        }
    }

    if (urls.empty()) {
        std::cerr << "Error: No valid URLs in '" << CONFIG_FILE << "'\n";
        return std::nullopt;
    }

    return urls;
}

/**
 * Refreshes the download list by fetching JSON data from configured URLs.
 * @param releases Reference to a JSON object where the updated release data will be stored.
 */
void refresh_download_list(json &releases) {
    auto urls_opt = read_config_file();
    if (!urls_opt) return;

    std::vector<std::string> json_files = get_json_files(urls_opt.value());
    releases = load_json_data(json_files);

    cleanup_files(json_files); // Cleanup after parsing
}

#endif // LIST1_0_H
