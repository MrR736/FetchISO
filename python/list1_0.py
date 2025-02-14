"""/*
// List Manager for Modern Python
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/"""

import os
import json
import requests
from pathlib import Path
from downloader import Downloader
from datetime import datetime, timedelta
from typing import List, Optional

CONFIG_FILE = "FetchISO.list"
CACHE_DIR = Path("/tmp")
CACHE_EXPIRY = timedelta(hours=1)

def download_file(downloader: Downloader, url: str, local_file: Path) -> bool:
    return downloader.download_file(url, str(local_file))

def get_json_files(urls: List[str]) -> List[Path]:
    """Downloads JSON files from provided URLs and caches them."""
    local_files = []
    downloader = Downloader()  # Create a single instance

    for i, url in enumerate(urls):
        local_file = CACHE_DIR / f"List_{i}.json"

        # Check if cached file is still valid
        if local_file.exists() and datetime.now() - datetime.fromtimestamp(local_file.stat().st_mtime) < CACHE_EXPIRY:
            print(f"Skipping fresh file: {local_file}")
            local_files.append(local_file)
            continue

        if download_file(downloader, url, local_file):
            local_files.append(local_file)

    return local_files

def cleanup_files(files: List[Path]):
    """Removes temporary JSON files."""
    for file in files:
        if file.exists():
            file.unlink()


def load_json_data(files: List[Path]) -> list:
    """Loads JSON data from downloaded files and merges them."""
    all_releases = []

    for file in files:
        try:
            with open(file, 'r', encoding='utf-8') as f:
                data = json.load(f)
                if isinstance(data, list):
                    all_releases.extend(data)
                else:
                    print(f"Warning: JSON in {file} is not a list.")
        except (json.JSONDecodeError, OSError) as e:
            print(f"Error processing {file}: {e}")

    return all_releases


def ensure_config_exists():
    """Ensures the configuration file exists."""
    if not Path(CONFIG_FILE).exists():
        with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
            f.write("# FetchISO List\n\nFIL https://raw.githubusercontent.com/MrR736/FetchISO/main/tools/List.json\n")
        print(f"Created default config: {CONFIG_FILE}")


def read_config_file() -> Optional[List[str]]:
    """Reads URLs from the configuration file."""
    ensure_config_exists()

    try:
        with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
            urls = [line.strip()[4:] for line in f if line.startswith("FIL ")]
            if not urls:
                print(f"Error: No valid URLs in '{CONFIG_FILE}'")
                return None
            return urls
    except OSError as e:
        print(f"Critical Error: Failed to read '{CONFIG_FILE}': {e}")
        return None


def refresh_download_list() -> list:
    """Fetches JSON data from configured URLs and returns merged data."""
    urls = read_config_file()
    if not urls:
        return []

    json_files = get_json_files(urls)
    releases = load_json_data(json_files)

    cleanup_files(json_files)
    return releases
