"""/*
// FetchISO Qt6 for Modern Python
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/"""

import sys
import json
from pathlib import Path
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, QWidget,
    QLineEdit, QComboBox, QListWidget, QPushButton, QMessageBox,
    QDialog, QTextEdit
)

from downloader import Downloader
from list1_0 import refresh_download_list

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

class FetchISOApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("FetchISO - Quickly Fetch ISOs")
        self.setGeometry(100, 100, 500, 500)

        self.iso_list = []
        self.filtered_list = []

        self.initUI()
        self.fetch_iso_list()

    def initUI(self):
        container = QWidget()
        self.setCentralWidget(container)
        layout = QVBoxLayout()
        container.setLayout(layout)

        # Search Bar & Architecture Selector
        search_layout = QHBoxLayout()
        self.search_entry = QLineEdit()
        self.search_entry.setPlaceholderText("Search")
        self.search_entry.textChanged.connect(self.update_search_results)
        search_layout.addWidget(self.search_entry)

        self.arch_selector = QComboBox()
        self.arch_selector.addItem("All Architectures")
        for arch in ARCH_MAP.values():
            if arch != "All Architectures":
                self.arch_selector.addItem(arch)
        self.arch_selector.currentTextChanged.connect(self.update_search_results)
        search_layout.addWidget(self.arch_selector)

        layout.addLayout(search_layout)

        # ISO List
        self.list_widget = QListWidget()
        layout.addWidget(self.list_widget)

    def fetch_iso_list(self):
        self.iso_list.clear()
        releases = refresh_download_list()
        for entry in releases:
            if all(key in entry for key in ["name", "url", "arch", "file"]):
                self.iso_list.append(entry)
        self.update_search_results()

    def update_search_results(self):
        search_query = self.search_entry.text().lower()
        selected_arch = self.arch_selector.currentText()

        self.list_widget.clear()
        self.filtered_list.clear()

        for iso in self.iso_list:
            iso_name_lower = iso["name"].lower()
            matches_search = search_query in iso_name_lower if search_query else True
            matches_arch = (
                selected_arch == "All Architectures" or iso["arch"] == "all" or ARCH_MAP.get(iso["arch"], "") == selected_arch
            )

            if matches_search and matches_arch:
                self.filtered_list.append(iso)
                self.list_widget.addItem(iso["name"])

        self.list_widget.itemClicked.connect(self.on_item_clicked)

    def on_item_clicked(self, item):
        for iso in self.filtered_list:
            if iso["name"] == item.text():
                self.start_download(iso)
                break

    def start_download(self, iso):
        """Show confirmation dialog before starting download."""
        reply = QMessageBox.question(
            self,
            "Download Confirmation",
            f"Do you want to download: {iso['name']}?\nFrom: {iso['url']}?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )

        if reply == QMessageBox.StandardButton.Yes:
            self.perform_download(iso)

    def perform_download(self, iso):
        """Perform the actual file download."""
        downloader = Downloader()
        success = downloader.download_file(iso["url"], iso["file"])

        if success:
            message = f"File saved at: {iso['file']}"
        else:
            message = f"Download failed: {downloader.get_last_error()}"

        self.show_message_dialog("Download Status", message)

    def show_message_dialog(self, title, message):
        """Display a message dialog with a scrollable text view."""
        dialog = QDialog(self)
        dialog.setWindowTitle(title)
        dialog.setMinimumSize(500, 300)

        layout = QVBoxLayout()
        text_view = QTextEdit()
        text_view.setReadOnly(True)
        text_view.setPlainText(message)
        layout.addWidget(text_view)

        ok_button = QPushButton("OK")
        ok_button.clicked.connect(dialog.accept)
        layout.addWidget(ok_button)

        dialog.setLayout(layout)
        dialog.exec()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = FetchISOApp()
    window.show()
    sys.exit(app.exec())
