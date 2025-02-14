"""/*
// FetchISO CLI for Modern Python
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/"""

import curses
import string
from downloader import Downloader
from list1_0 import refresh_download_list

arch_map = {
    "all": "All Architectures",
    "amd64": "x86_64 (64-bit)",
    "i386": "x86 (32-bit)",
    "aarch64": "ARM64 (AArch64)",
    "armhf": "ARMv7 Hard Float",
    "armel": "ARMEL",
    "mips64el": "MIPS64EL",
    "mipsel": "MIPSEL",
    "ppc64el": "PowerPC64LE",
    "ppc64": "PowerPC (64-bit)",
    "ppc": "PowerPC (32-bit)",
    "ppcspe": "PowerPC SPE",
    "s390x": "IBM Z (s390x)",
    "riscv64": "RISC-V 64-bit"
}

arch_keys = list(arch_map.keys())

class FetchISOApp:
    def __init__(self, stdscr):
        self.stdscr = stdscr
        self.search_query = ""
        self.selected_arch = "all"
        self.arch_index = 0
        self.highlight = 0
        self.first_visible = 0
        self.selecting_arch = False
        self.iso_list = []
        self.filtered_list = []

        curses.curs_set(1)  # Show cursor for search bar
        self.stdscr.clear()
        self.fetch_iso_list()

    def fetch_iso_list(self):
        self.iso_list = refresh_download_list()
        self.update_search_results()

    def update_search_results(self):
        query_lower = self.search_query.lower()
        self.filtered_list = [
            iso for iso in self.iso_list
            if query_lower in iso["name"].lower() and
            (iso["arch"] == "all" or self.selected_arch == "all" or iso["arch"] == self.selected_arch)
        ]

    def draw_scrollbar(self, max_rows):
        if len(self.filtered_list) <= max_rows:
            return

        scrollbar_height = max(1, (max_rows * max_rows) // len(self.filtered_list))
        scroll_pos = (self.highlight * (max_rows - scrollbar_height)) // (len(self.filtered_list) - 1)

        for i in range(max_rows):
            if scroll_pos <= i < scroll_pos + scrollbar_height:
                self.stdscr.addstr(4 + i, curses.COLS - 2, "█")
            else:
                self.stdscr.addstr(4 + i, curses.COLS - 2, "|")

    def run(self):
        while True:
            self.stdscr.clear()
            rows, cols = self.stdscr.getmaxyx()
            max_items = rows - 6

            self.stdscr.addstr(0, 2, f"Search: {self.search_query}_")
            self.stdscr.addstr(1, 2, f"Arch: {arch_map[self.selected_arch]} (Press A + Ctrl to change)")
            self.stdscr.addstr(2, 2, "Use ↑/↓ to navigate, Enter to select, Ctrl + Q to quit")

            if self.selecting_arch:
                max_rows = curses.LINES - 6
                for i, key in enumerate(arch_keys):
                    if 4 + i < max_rows:
                        self.stdscr.addstr(4 + i, 4, arch_map[key])
                    self.stdscr.attroff(curses.A_REVERSE)
                self.stdscr.refresh()
                self.handle_arch_selection()
                continue

            self.update_search_results()
            if not self.filtered_list:
                self.stdscr.addstr(4, 4, "No results found.")
            else:
                self.highlight = max(0, min(self.highlight, len(self.filtered_list) - 1))
                self.first_visible = max(0, min(self.first_visible, self.highlight))

                for i in range(min(max_items, len(self.filtered_list))):
                    item_index = self.first_visible + i
                    if item_index >= len(self.filtered_list):
                        break
                    if self.highlight == item_index:
                        self.stdscr.attron(curses.A_REVERSE)
                    self.stdscr.addstr(4 + i, 4, self.filtered_list[item_index]["name"])
                    self.stdscr.attroff(curses.A_REVERSE)

                self.draw_scrollbar(max_items)

            self.stdscr.refresh()
            key = self.stdscr.getch()
            if not self.handle_input(key):
                break

    def handle_input(self, key):
        self.stdscr.addstr(0, 0, f"Key Pressed: {key}")
        self.stdscr.refresh()
        curses.napms(500)  # Short delay for visibility

        if key == curses.KEY_UP and self.highlight > 0:
            self.highlight -= 1
            if self.highlight < self.first_visible:
                self.first_visible -= 1
        elif key == curses.KEY_DOWN and self.highlight < len(self.filtered_list) - 1:
            self.highlight += 1
            if self.highlight >= self.first_visible + (curses.LINES - 6):
                self.first_visible += 1
        elif key == 1:
            self.selecting_arch = True
        elif key == 10 and self.filtered_list:  # Enter key
            self.download_iso(self.filtered_list[self.highlight])
        elif key == 127 or key == curses.KEY_BACKSPACE:
            self.search_query = self.search_query[:-1]
        elif key in map(ord, string.printable):
            self.search_query += chr(key)
        elif key == 17:
            return False
        return True


    def handle_arch_selection(self):
        while self.selecting_arch:
            self.stdscr.clear()
            rows, cols = self.stdscr.getmaxyx()
            max_rows = min(rows - 6, len(arch_keys))

            self.stdscr.addstr(2, 2, "Select Architecture (Press Enter to confirm, Q to cancel)")

            for i, key in enumerate(arch_keys[:max_rows]):
                if i == self.arch_index:
                    self.stdscr.attron(curses.A_REVERSE)
                self.stdscr.addstr(4 + i, 4, arch_map[key][:cols - 6])  # Truncate if needed
                if i == self.arch_index:
                    self.stdscr.attroff(curses.A_REVERSE)

            self.stdscr.refresh()
            key = self.stdscr.getch()

            if key == curses.KEY_UP and self.arch_index > 0:
                self.arch_index -= 1
            elif key == curses.KEY_DOWN and self.arch_index < len(arch_keys) - 1:
                self.arch_index += 1
            elif key == 10:  # Enter key
                self.selected_arch = arch_keys[self.arch_index]
                self.update_search_results()
                self.selecting_arch = False
            elif key in (ord('q'), ord('Q')):  # Quit arch selection
                self.selecting_arch = False


    def download_iso(self, iso):
        self.stdscr.clear()
        self.stdscr.addstr(5, 5, f"Downloading: {iso['name']}")

        self.stdscr.refresh()
        downloader = Downloader()
        success = downloader.download_file(iso["url"], iso["file"])

        if success:
            self.stdscr.addstr(7, 5, f"Download complete: {iso['file']}")
        else:
            self.stdscr.addstr(7, 5, f"Download failed: {downloader.get_last_error()}")

        self.stdscr.refresh()
        self.stdscr.getch()  # Wait for key press before returning


def main(stdscr):
    app = FetchISOApp(stdscr)
    app.run()


if __name__ == "__main__":
    curses.wrapper(main)
