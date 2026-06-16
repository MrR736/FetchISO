#
# List Manager for Modern Python
# version 1.1
# https://github.com/MrR736/FetchISO
#
# SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
# SPDX-License-Identifier: MIT
#

from __future__ import annotations

import json
import tempfile
from pathlib import Path
from datetime import datetime, timedelta
from dataclasses import dataclass
from typing import Optional

from downloader import Downloader

FETCHISO_CONFIG_FILE = (
	"https://raw.githubusercontent.com/MrR736/FetchISO/main/tools/List.json"
)

CONFIG_FILE = "FetchISO.list"
CACHE_EXPIRY = timedelta(hours=1)


@dataclass
class ListSource:
	is_url: bool
	value: str


@dataclass
class JsonFiles:
	files: list[str]
	temp_files: list[str]


# --------------------------------------------------
# CONFIG
# --------------------------------------------------

def ensure_config_exists() -> None:
	if not Path(CONFIG_FILE).exists():
		with open(CONFIG_FILE, "w", encoding="utf-8") as f:
			f.write("# FetchISO List\n\n")
			f.write(f"FIL {FETCHISO_CONFIG_FILE}\n")

		print(f"Created default config: {CONFIG_FILE}")


def read_config_file() -> Optional[list[ListSource]]:
	ensure_config_exists()

	try:
		with open(CONFIG_FILE, "r", encoding="utf-8") as f:
			sources: list[ListSource] = []

			for line in f:
				line = line.strip()

				if not line or line.startswith("#"):
					continue

				if not line.startswith("FIL "):
					continue

				value = line[4:].strip()

				is_url = (
					value.startswith("http://")
					or value.startswith("https://")
				)

				sources.append(ListSource(is_url, value))

		if not sources:
			print("No valid entries in config")
			return None

		return sources

	except OSError as e:
		print(f"Critical Error: Failed to read config: {e}")
		return None


# --------------------------------------------------
# DOWNLOAD / FILE HANDLING
# --------------------------------------------------

def get_json_files(sources: list[ListSource]) -> JsonFiles:
	downloader = Downloader()

	result = JsonFiles(
		files=[],
		temp_files=[]
	)

	tmpdir = Path(tempfile.gettempdir())

	for index, source in enumerate(sources):

		if not source.is_url:
			if Path(source.value).exists():
				result.files.append(source.value)
			else:
				print(f"Missing file: {source.value}")

			continue

		local_file = tmpdir / f"List_{index}.json"

		if local_file.exists():
			age = (
				datetime.now()
				- datetime.fromtimestamp(local_file.stat().st_mtime)
			)

			if age < CACHE_EXPIRY:
				result.files.append(str(local_file))
				result.temp_files.append(str(local_file))
				continue

		if downloader.download_file(
			source.value,
			str(local_file)
		):
			result.files.append(str(local_file))
			result.temp_files.append(str(local_file))
		else:
			print(
				f"Download failed: "
				f"{source.value} "
				f"({downloader.get_last_error()})"
			)

	return result


# --------------------------------------------------
# JSON LOADING
# --------------------------------------------------

def load_json_data(files: list[str]) -> list:
	all_data: list = []

	for file in files:
		try:
			with open(file, "r", encoding="utf-8") as f:
				data = json.load(f)

			if isinstance(data, list):
				all_data.extend(data)
			else:
				print(f"Invalid JSON format: {file}")

		except (OSError, json.JSONDecodeError) as e:
			print(f"JSON error in {file}: {e}")

	return all_data


# --------------------------------------------------
# CLEANUP
# --------------------------------------------------

def cleanup_files(files: list[str]) -> None:
	for file in files:
		try:
			Path(file).unlink(missing_ok=True)
		except OSError:
			pass


# --------------------------------------------------
# MAIN ENTRY
# --------------------------------------------------

def refresh_download_list() -> list:
	sources = read_config_file()

	if not sources:
		return []

	files = get_json_files(sources)

	releases = load_json_data(files.files)

	cleanup_files(files.temp_files)

	return releases
