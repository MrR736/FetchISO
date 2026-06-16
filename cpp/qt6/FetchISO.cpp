/*
// FetchISO Qt5 or Qt6 for Modern C++
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>
#include <vector>
#include <string>
#include <iostream>

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

class FetchISO : public QWidget {
	Q_OBJECT

public:
	FetchISO(QWidget *parent = nullptr) : QWidget(parent) {
		setWindowTitle("FetchISO - Quickly Fetch ISOs");
		resize(FETCHISO_WIDTH, FETCHISO_HEIGHT);

		QVBoxLayout *mainLayout = new QVBoxLayout(this);
		QHBoxLayout *searchLayout = new QHBoxLayout();

		searchEntry = new QLineEdit(this);
		searchEntry->setPlaceholderText("Search");
		connect(searchEntry, &QLineEdit::textChanged, this, &FetchISO::updateSearchResults);

		archSelector = new QComboBox(this);
		archSelector->addItem("All Architectures");
		for (const auto& [key, value] : arch_map) {
			if (key != "all") archSelector->addItem(QString::fromStdString(value));
		}
		connect(archSelector, &QComboBox::currentTextChanged, this, &FetchISO::updateSearchResults);

		searchLayout->addWidget(searchEntry);
		searchLayout->addWidget(archSelector);
		mainLayout->addLayout(searchLayout);

		listWidget = new QListWidget(this);
		connect(listWidget, &QListWidget::itemClicked, this, &FetchISO::startDownload);
		mainLayout->addWidget(listWidget);

		fetchISOList();
		updateSearchResults();
	}

private slots:
	void updateSearchResults() {
		QString searchQuery = searchEntry->text().toLower();
		QString selectedArch = archSelector->currentText();

		listWidget->clear();

		for (const auto &iso : isoList) {
			QString isoName = QString::fromStdString(iso.name);
			QString isoFile = QString::fromStdString(iso.file);
			QString isoArch = QString::fromStdString(iso.arch);
			QString isoUrl = QString::fromStdString(iso.url);

			bool matchesSearch = searchQuery.isEmpty() || isoName.toLower().contains(searchQuery);
			bool matchesArch = (selectedArch == "All Architectures" ||
								(arch_map.count(iso.arch) && arch_map[iso.arch] == selectedArch.toStdString()));

			if (matchesSearch && matchesArch) {
				QListWidgetItem *item = new QListWidgetItem(isoName, listWidget);
				item->setData(Qt::UserRole, isoFile);  // Store file name
				item->setData(Qt::UserRole + 1, isoUrl); // Store URL with a different role
			}
		}
	}

	void startDownload(QListWidgetItem *item) {
		QString isoName = item->text();
		QString isoFile = item->data(Qt::UserRole).toString();
		QString isoUrl = item->data(Qt::UserRole + 1).toString();  // Retrieve URL correctly

		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "Download Confirmation", "Do you want to download: " + isoName +
									  "\nFrom: " + isoUrl + "\n\nFile: " + isoFile, QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::Yes) {
			performDownload(isoUrl, isoFile);
		}
	}


	void performDownload(const QString &url, const QString &file) {
		Downloader dl;
		if (dl.downloadFile(url.toStdString(), file.toStdString())) {
			QMessageBox::information(this, "Download Complete", "File saved as: " + file);
		} else {
			QMessageBox::critical(this, "Download Failed", QString::fromStdString(dl.getLastError()));
		}
	}

private:
	QLineEdit *searchEntry;
	QComboBox *archSelector;
	QListWidget *listWidget;
	std::vector<ISOEntry> isoList;

	void fetchISOList() {
		json releases;
		try {
			refresh_download_list(releases);
		} catch (const std::exception &e) {
			QMessageBox::critical(this, "Error", "Failed to fetch ISO list: " + QString::fromStdString(e.what()));
			return;
		}

		isoList.clear();
		for (const auto &entry : releases) {
			try {
				isoList.push_back({
					entry.at("name").get<std::string>(), entry.at("url").get<std::string>(),
					entry.at("arch").get<std::string>(), entry.at("file").get<std::string>()
				});
			} catch (const std::exception &e) { std::cerr << "Skipping invalid entry: " << e.what() << std::endl; }
		}
	}
};

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	FetchISO window;
	window.show();
	return app.exec();
}
