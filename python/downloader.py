"""/*
// Downloader for Modern Python
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/"""

import os
import subprocess
from typing import Optional


class Downloader:
    def __init__(self):
        self.last_error: Optional[str] = None

    def download_file(self, url: str, file: str) -> bool:
        """Downloads a file using wget or curl."""
        if self.is_command_available("wget"):
            command = f"wget -q --show-progress -O \"{file}\" \"{url}\""
        elif self.is_command_available("curl"):
            command = f"curl -L --progress-bar -o \"{file}\" \"{url}\""
        else:
            self.last_error = "No suitable download tool found (wget or curl)"
            return False

        # Execute command and capture output
        ret_code, output = self.execute_command(command)

        if ret_code != 0:
            self.last_error = f"Download failed with exit code: {ret_code}\n{output}"
            return False

        # Ensure the file actually exists after download
        if not os.path.exists(file):
            self.last_error = f"Download completed but file does not exist: {file}"
            return False

        return True

    def get_last_error(self) -> Optional[str]:
        """Returns the last error message."""
        return self.last_error

    @staticmethod
    def is_command_available(cmd: str) -> bool:
        """Checks if a command is available on the system."""
        check_cmd = "where" if os.name == "nt" else "command -v"
        return subprocess.call(f"{check_cmd} {cmd}", shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) == 0

    @staticmethod
    def execute_command(command: str):
        """Executes a shell command and returns (exit_code, output)."""
        try:
            result = subprocess.run(command, shell=True, check=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            return result.returncode, result.stdout.strip()
        except Exception as e:
            return -1, str(e)
