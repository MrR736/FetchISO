"""/*
// FetchISO GTK 4.0 for Modern Python
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/"""

import gi
import json
import os
from pathlib import Path
from downloader import Downloader
from list1 import refresh_download_list

gi.require_version("Gtk", "4.0")
from gi.repository import Gtk, GLib

CONFIG_FILE = "FetchISO.list"

ARCH_MAP = {
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
    "riscv64": "RISC-V 64-bit",
}

class FetchISOApp(Gtk.ApplicationWindow):
    def __init__(self, app):
        super().__init__(application=app, title="FetchISO - Quickly Fetch ISOs")
        self.set_default_size(500, 500)

        # Main layout
        self.main_vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.set_child(self.main_vbox)

        # Search Box
        self.search_hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=5)
        self.main_vbox.append(self.search_hbox)
        self.search_hbox.set_homogeneous(True)
        self.search_hbox.set_hexpand(True)

        self.search_entry = Gtk.Entry()
        self.search_entry.set_placeholder_text("Search")
        self.search_entry.connect("changed", self.update_search_results)
        self.search_hbox.append(self.search_entry)

        # Architecture Selector
        self.arch_selector = Gtk.ComboBoxText()
        self.arch_selector.append_text("All Architectures")
        for arch in ARCH_MAP.values():
            if arch != "All Architectures":
                self.arch_selector.append_text(arch)
        self.arch_selector.set_active(0)
        self.arch_selector.connect("changed", self.update_search_results)
        self.search_hbox.append(self.arch_selector)

        # List Box with Scroll
        self.scrolled_window = Gtk.ScrolledWindow()
        self.scrolled_window.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        self.scrolled_window.set_vexpand(True)
        self.main_vbox.append(self.scrolled_window)

        self.list_box = Gtk.ListBox()
        self.scrolled_window.set_child(self.list_box)

        # Load ISO list
        self.iso_list = []
        self.filtered_list = []
        self.fetch_iso_list()
        self.update_search_results()

    def fetch_iso_list(self):
        GLib.idle_add(self._fetch_iso_list)

    def _fetch_iso_list(self):
        self.iso_list.clear()
        releases = refresh_download_list()
        for entry in releases:
            if all(key in entry for key in ["name", "url", "arch", "file"]):
                self.iso_list.append(entry)
        self.update_search_results()

    def on_dialog_response(self, dialog, response_id, iso):
        """Handles the dialog response."""
        dialog.close()  # Close the dialog

        if response_id == Gtk.ResponseType.OK:
            self.perform_download(iso)

    def update_search_results(self, widget=None):
        """Update the displayed list based on search query and architecture selection."""
        search_query = self.search_entry.get_text().lower()
        selected_arch = self.arch_selector.get_active_text()

        # Clear existing list
        child = self.list_box.get_first_child()
        while child:
            next_child = child.get_next_sibling()
            self.list_box.remove(child)
            child = next_child

        self.filtered_list.clear()

        for iso in self.iso_list:
            iso_name_lower = iso["name"].lower()
            matches_search = search_query in iso_name_lower if search_query else True
            matches_arch = (
                selected_arch == "All Architectures"
                or iso["arch"] == "all"
                or ARCH_MAP.get(iso["arch"], "") == selected_arch
            )

            if matches_search and matches_arch:
                self.filtered_list.append(iso)

                btn = Gtk.Button(label=iso["name"])
                btn.connect("clicked", self.start_download, iso)
                self.list_box.append(btn)

    def start_download(self, button, iso):
        """Show confirmation dialog before starting download."""
        dialog = Gtk.MessageDialog(
            transient_for=self,
            modal=True,
            message_type=Gtk.MessageType.QUESTION,
            buttons=Gtk.ButtonsType.OK_CANCEL,
            text=f"Do you want to download: {iso['name']}?\nFrom: {iso['url']}?",
        )
        dialog.connect("response", self.on_dialog_response, iso)
        dialog.present()

    def perform_download(self, iso):
        """Perform the actual file download."""
        downloader = Downloader()
        success = downloader.download_file(iso["url"], iso["file"])

        if success:
            message = f"File saved at: {iso['file']}"
            message_type = "info"
        else:
            message = f"Download failed: {downloader.get_last_error()}"
            message_type = "error"

        self.show_message_dialog(message, message_type)

    def show_message_dialog(self, message, message_type):
        """Display a message dialog with a scrollable text view."""
        dialog = Gtk.Dialog(title="Message", transient_for=self)
        dialog.set_default_size(500, 300)

        # Scrolled Window
        scrolled_window = Gtk.ScrolledWindow()
        scrolled_window.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scrolled_window.set_size_request(480, 200)

        # TextView (Read-Only)
        text_view = Gtk.TextView()
        text_view.set_editable(False)
        text_view.set_wrap_mode(Gtk.WrapMode.WORD)
        text_buffer = text_view.get_buffer()
        text_buffer.set_text(message)

        scrolled_window.set_child(text_view)

        # OK Button
        ok_button = Gtk.Button(label="OK")
        ok_button.connect("clicked", lambda _: dialog.close())

        # Container Box
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        box.append(scrolled_window)
        box.append(ok_button)

        dialog.set_child(box)
        dialog.show()

class FetchISOAppInstance(Gtk.Application):
    def __init__(self):
        super().__init__()

    def do_activate(self):
        win = FetchISOApp(self)
        win.present()

def main():
    print("Starting FetchISOApp...")
    app = FetchISOAppInstance()
    app.run(None)

if __name__ == "__main__":
    main()

