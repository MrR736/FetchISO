/*
// FetchISO CLI for Modern C++
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <MrR736/downloader.h>
#include <MrR736/list1.h>
#include <FetchISO_private.hh>

using json = nlohmann::json;

struct ISOEntry {
	std::string name;
	std::string url;
	std::string arch;
	std::string file;
};

std::vector<ISOEntry> iso_list;
std::vector<ISOEntry> filtered_list;
std::string search_query;
std::string selected_arch = "all";

std::vector<std::string> arch_keys = {
	"all", "amd64", "i386", "aarch64", "armhf", "armel",
	"mips64el", "mipsel", "ppc64el", "ppc64", "ppc",
	"ppcspe", "s390x", "riscv64"
};

// Fetch ISO list and store in `iso_list`
static void fetch_iso_list() {
	json releases;
	refresh_download_list(releases);

	iso_list.clear();
	for (const auto& entry : releases) {
		if (entry.contains("name") && entry.contains("url") &&
			entry.contains("arch") && entry.contains("file")) {
			iso_list.push_back({entry["name"], entry["url"], entry["arch"], entry["file"]});
		}
	}
}

// Filter ISO list based on search query and selected architecture
static void update_search_results() {
	filtered_list.clear();
	std::string query_lower = search_query;
	std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

	for (const auto& iso : iso_list) {
		std::string name_lower = iso.name;
		std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

		bool matches_search = (search_query.empty() || name_lower.find(query_lower) != std::string::npos);
		bool matches_arch = (iso.arch == "all" || selected_arch == "all" || iso.arch == selected_arch);

		if (matches_search && matches_arch) {
			filtered_list.push_back(iso);
		}
	}
}

// Draws scrollbar on the right side
static void draw_scrollbar(int start_y, int list_size, int visible_items, int first_visible) {
	if (list_size <= visible_items || visible_items <= 0)
		return;

	int scrollbar_height =
	std::max(1, visible_items * visible_items / list_size);

	int scroll_pos = 0;

	if (list_size > visible_items) {
		scroll_pos =
		first_visible * (visible_items - scrollbar_height) /
		(list_size - visible_items);
	}

	for (int i = 0; i < visible_items; ++i) {
		mvaddch(start_y + i, COLS - 2,
				(i >= scroll_pos && i < scroll_pos + scrollbar_height)
				? ACS_CKBOARD
				: ACS_VLINE);
	}
}

int main() {
	setlocale(LC_ALL, "");
	initscr();
	noecho();
	curs_set(1); // Enable cursor for typing
	keypad(stdscr, TRUE);
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	fetch_iso_list();
	if (iso_list.empty()) {
		mvprintw(5, 5, "No ISOs available. Press any key to exit.");
		getch();
		endwin();
		return 1;
	}

	int highlight = 0;
	int choice = -1;
	int ch;
	int first_visible = 0;
	int visible_items = rows - 5; // Adjust for top message & search bar
	int arch_index = 0;
	bool selecting_arch = false;

	while (choice == -1) {
		clear();
		mvprintw(0, 2, "Search: %s_", search_query.c_str()); // Search bar
		mvprintw(1, 2, "Arch: %s (Press A to change)", arch_map[selected_arch].c_str());
		mvprintw(2, 2, "Use UP/DOWN to navigate, Enter to select, Q to quit");

		if (selecting_arch) {
			for (size_t i = 0; i < arch_keys.size(); i++) {
				if ((int)i == arch_index) attron(A_REVERSE);
				mvprintw(4 + i, 4, "%s",arch_map[arch_keys[i]].c_str());
				attroff(A_REVERSE);
			}
			refresh();
			ch = getch();
			switch (ch) {
				case KEY_UP:
					if (arch_index > 0) arch_index--;
					break;
				case KEY_DOWN:
					if (arch_index < (int)arch_keys.size() - 1) arch_index++;
					break;
				case 10: // Enter key
					selected_arch = arch_keys[arch_index];
					selecting_arch = false;
					highlight = 0;
					first_visible = 0;
					break;
				case 'q': case 'Q':
					selecting_arch = false;
					break;
			}
			continue; // Skip the rest of the loop if selecting architecture
		}

		update_search_results();
		if (highlight >= (int)filtered_list.size()) highlight = (int)filtered_list.size() - 1;
		if (highlight < 0) highlight = 0;

		if (highlight < first_visible) first_visible = highlight;
		if (highlight >= first_visible + visible_items) first_visible = highlight - visible_items + 1;

		for (int i = 0; i < visible_items && first_visible + i < (int)filtered_list.size(); i++) {
			if (highlight == first_visible + i) attron(A_REVERSE);
			mvprintw(i + 4, 4, "%s",filtered_list[first_visible + i].name.c_str());
			attroff(A_REVERSE);
		}

		draw_scrollbar(4, filtered_list.size(), visible_items, highlight);

		ch = getch();
		switch (ch) {
			case KEY_UP:
				if (highlight > 0) highlight--;
				break;
			case KEY_DOWN:
				if (highlight < (int)filtered_list.size() - 1) highlight++;
				break;
			case 10: // Enter key
				if (!filtered_list.empty()) choice = highlight;
				break;
			case 'a': case 'A': // Select architecture
				selecting_arch = true;
				break;
			case 'q': case 'Q': // Quit
				endwin();
				return 0;
			case 127: case KEY_BACKSPACE: // Backspace
				if (!search_query.empty()) {
					search_query.pop_back();
				}
				highlight = 0;
				first_visible = 0;
				break;
			default: // Typing
				if (isprint(ch)) {
					search_query += ch;
				}
				highlight = 0;
				first_visible = 0;
				break;
		}
	}

	clear();
	mvaddnstr(5, 5, ("Downloading: " + filtered_list[choice].name).c_str(), COLS - 6);
	refresh();

	// Start downloading the selected ISO
	Downloader downloader;
	std::string download_path = filtered_list[choice].file;

	if (downloader.downloadFile(filtered_list[choice].url, download_path)) {
		mvaddnstr(7, 5,("Download complete: " + download_path).c_str(),COLS - 6);
	} else {
		mvprintw(7, 5, "Download failed: %s", downloader.getLastError().c_str());
	}

	refresh();
	getch();
	endwin();
	return 0;
}
