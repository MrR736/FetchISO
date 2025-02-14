/*
// FetchISO GTK 3.0 or GTK 4.0 for Modern C++
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

#include <gtkmm.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <MrR736/downloader.h>
#include <MrR736/list1.0.h>

using json = nlohmann::json;

struct ISOEntry {
    std::string name;
    std::string url;
    std::string arch;
    std::string file;
};

class FetchISOApp : public Gtk::Window {
public:
    FetchISOApp() {
    set_title("FetchISO - Quickly Fetch ISOs");
    set_default_size(500, 500);

#if GTK_CHECK_VERSION(4, 0, 0)
    set_child(main_vbox);
#else
    add(main_vbox);
#endif

    search_entry.set_placeholder_text("Search");
    search_entry.signal_changed().connect(sigc::mem_fun(*this, &FetchISOApp::update_search_results));

    arch_selector.append("All Architectures");
    for (const auto& [key, value] : arch_map) {
        if (key != "all") arch_selector.append(value);
    }
    arch_selector.set_active(0);
    arch_selector.signal_changed().connect(sigc::mem_fun(*this, &FetchISOApp::update_search_results));

#if GTK_CHECK_VERSION(4, 0, 0)
    search_hbox.append(search_entry);
    search_hbox.append(arch_selector);
    main_vbox.append(search_hbox);
    main_vbox.append(scrolled_window);
    scrolled_window.set_child(list_box);

    scrolled_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

    search_hbox.set_homogeneous(true);

    list_box.set_expand(true);
    scrolled_window.set_expand(true);

    search_hbox.set_hexpand(true);

    list_box.set_visible(true);
    scrolled_window.set_visible(true);
#else
    search_hbox.pack_start(search_entry, Gtk::PACK_EXPAND_WIDGET);
    search_hbox.pack_start(arch_selector, Gtk::PACK_EXPAND_WIDGET);
    search_hbox.set_homogeneous(true);
    main_vbox.pack_start(search_hbox, Gtk::PACK_SHRINK);
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window.add(list_box);
    main_vbox.pack_start(scrolled_window, Gtk::PACK_EXPAND_WIDGET);
    list_box.set_visible(true);
    scrolled_window.set_visible(true);
    show_all_children();
#endif

    fetch_iso_list();
    update_search_results();
}

protected:
#if GTK_CHECK_VERSION(4, 0, 0)
    Gtk::Box main_vbox{Gtk::Orientation::VERTICAL, 5};
    Gtk::Box search_hbox{Gtk::Orientation::HORIZONTAL, 5};
#else
    Gtk::Box main_vbox{Gtk::ORIENTATION_VERTICAL, 5};
    Gtk::Box search_hbox{Gtk::ORIENTATION_HORIZONTAL, 5};
#endif

    Gtk::Entry search_entry;
    Gtk::ComboBoxText arch_selector;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::ListBox list_box;

    std::vector<ISOEntry> iso_list;
    std::vector<ISOEntry> filtered_list;

    std::unordered_map<std::string, std::string> arch_map = {
        {"all", "All Architectures"},
        {"amd64", "x86_64 (64-bit)"},
        {"i386", "x86 (32-bit)"},
        {"aarch64", "ARM64 (AArch64)"},
        {"armhf", "ARMv7 Hard Float"},
        {"armel", "ARMEL"},
        {"mips64el", "MIPS64EL"},
        {"mipsel", "MIPSEL"},
        {"ppc64el", "PowerPC64LE"},
        {"ppc64", "PowerPC (64-bit)"},
        {"ppc", "PowerPC (32-bit)"},
        {"ppcspe", "PowerPC SPE"},
        {"s390x", "IBM Z (s390x)"},
        {"riscv64", "RISC-V 64-bit"}
    };

    void fetch_iso_list() {
        json releases;
        refresh_download_list(releases);

        iso_list.clear();
        for (const auto& entry : releases) {
            if (entry.contains("name") && entry.contains("url") && entry.contains("arch") && entry.contains("file")) {
                iso_list.push_back({entry["name"], entry["url"], entry["arch"], entry["file"]});
            }
        }
    }

    void update_search_results() {
        std::string search_query = search_entry.get_text();
        std::string selected_arch = arch_selector.get_active_text();

        // Convert search_query to lowercase for case-insensitive matching
        std::transform(search_query.begin(), search_query.end(), search_query.begin(), ::tolower);

        // Clear existing entries from the list
#if GTK_CHECK_VERSION(4, 0, 0)
        for (auto* child = list_box.get_first_child(); child != nullptr; ) {
            auto* next = child->get_next_sibling();
            list_box.remove(*child);
            child = next;
        }
#else
        std::vector<Gtk::Widget*> children = list_box.get_children();
        for (auto* child : children) {
            list_box.remove(*child);
            delete child; // Necessary for memory management in GTK3
        }
#endif

        filtered_list.clear();

        for (const auto& iso : iso_list) {
            std::string iso_name_lower = iso.name;
            std::transform(iso_name_lower.begin(), iso_name_lower.end(), iso_name_lower.begin(), ::tolower);

            bool matches_search = search_query.empty() || iso_name_lower.find(search_query) != std::string::npos;
            bool matches_arch = (selected_arch == "All Architectures" || iso.arch == "all" || (arch_map.find(iso.arch) != arch_map.end() && arch_map[iso.arch] == selected_arch));

            if (matches_search && matches_arch) {
                filtered_list.push_back(iso);

                auto* btn = Gtk::make_managed<Gtk::Button>(iso.name);
                btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &FetchISOApp::start_download), iso));

#if GTK_CHECK_VERSION(4, 0, 0)
                list_box.append(*btn);
                btn->set_visible(true);
#else
                list_box.add(*btn);
#endif
            }
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        list_box.show_all_children(); // Necessary in GTK3
#endif
    }

    void start_download(const ISOEntry& iso) {
#if GTK_CHECK_VERSION(4, 0, 0)
        auto dialog = std::make_shared<Gtk::MessageDialog>(*this, "Download Confirmation", false);
        dialog->set_secondary_text("Do you want to download: " + iso.name + "?\nFrom: " + iso.url);
        dialog->add_button("Cancel", Gtk::ResponseType::CANCEL);
        dialog->set_modal(true);

        dialog->signal_response().connect([this, dialog, iso](int response) {
            dialog->close();  // Close dialog first
            if (response == Gtk::ResponseType::OK) {
                perform_download(iso);  // Start download only if OK is clicked
            }
        });

        dialog->present();
#else
        Gtk::MessageDialog dialog(*this, "Download Confirmation", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);
        dialog.set_secondary_text("Do you want to download: " + iso.name + "?\nFrom: " + iso.url);
        dialog.set_default_size(500, 250);
        dialog.add_button("OK", Gtk::RESPONSE_OK);
        dialog.add_button("Close", Gtk::RESPONSE_CLOSE);

        int response = dialog.run();
        dialog.hide();  // Always hide the dialog after response

        if (response == Gtk::RESPONSE_OK) {
            perform_download(iso);  // Start download only if OK is clicked
        }
#endif
    }

    void perform_download(const ISOEntry& iso) {
        Downloader dl;
        if (dl.downloadFile(iso.url, iso.file)) {
#if GTK_CHECK_VERSION(4, 0, 0)
            auto success = std::make_shared<Gtk::MessageDialog>(*this, "Download Complete!", false);
            success->set_default_size(500, 250);
            success->set_secondary_text("File saved at: " + iso.file);
            success->add_button("OK", Gtk::ResponseType::OK);
            success->add_button("Close", Gtk::ResponseType::CLOSE);

            auto text_view = Gtk::make_managed<Gtk::TextView>();
            auto text_buffer = text_view->get_buffer();
            text_buffer->set_text("File saved at: " + iso.file);
            text_view->set_editable(false);
            text_view->set_wrap_mode(Gtk::WrapMode::WORD);
            Gtk::ScrolledWindow scrolled_window;
            scrolled_window.set_child(*text_view);
            success->set_child(scrolled_window);

            success->signal_response().connect([success](int response) {
                success->close();  // Close only the success dialog
            });

            success->present();
#else
            Gtk::MessageDialog success(*this, "Download Complete!", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE, true);
            success.set_default_size(500, 250);
            success.set_secondary_text("File saved at: " + iso.file);
            success.add_button("OK", Gtk::RESPONSE_OK);
            success.add_button("Close", Gtk::RESPONSE_CLOSE);

            Gtk::TextView text_view;
            Gtk::ScrolledWindow scrolled_window;
            text_view.get_buffer()->set_text("File saved at: " + iso.file);
            text_view.set_editable(false);
            text_view.set_wrap_mode(Gtk::WRAP_WORD);
            scrolled_window.add(text_view);
            success.get_vbox()->pack_start(scrolled_window, Gtk::PACK_EXPAND_WIDGET);
            scrolled_window.show_all();

            int response = success.run();
            success.hide();
#endif
        } else {
#if GTK_CHECK_VERSION(4, 0, 0)
            auto failure = std::make_shared<Gtk::MessageDialog>(*this, "Download Failed!", false);
            failure->set_default_size(500, 250);
            failure->set_secondary_text(dl.getLastError());
            failure->add_button("OK", Gtk::ResponseType::OK);
            failure->add_button("Close", Gtk::ResponseType::CLOSE);

            auto text_view = Gtk::make_managed<Gtk::TextView>();
            auto text_buffer = text_view->get_buffer();
            text_buffer->set_text(dl.getLastError());
            text_view->set_editable(false);
            text_view->set_wrap_mode(Gtk::WrapMode::WORD);
            Gtk::ScrolledWindow scrolled_window;
            scrolled_window.set_child(*text_view);
            failure->set_child(scrolled_window);

            failure->signal_response().connect([failure](int response) {
                failure->close();
            });

            failure->present();
#else
            Gtk::MessageDialog failure(*this, "Download Failed!", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, true);
            failure.set_default_size(500, 250);
            failure.set_secondary_text(dl.getLastError());
            failure.add_button("OK", Gtk::RESPONSE_OK);
            failure.add_button("Close", Gtk::RESPONSE_CLOSE);

            Gtk::TextView text_view;
            Gtk::ScrolledWindow scrolled_window;
            text_view.get_buffer()->set_text(dl.getLastError());
            text_view.set_editable(false);
            text_view.set_wrap_mode(Gtk::WRAP_WORD);
            scrolled_window.add(text_view);
            failure.get_vbox()->pack_start(scrolled_window, Gtk::PACK_EXPAND_WIDGET);
            scrolled_window.show_all();

            int response = failure.run();
            failure.hide();
#endif
        }
    }
};

int main(int argc, char* argv[]) {
#if GTK_CHECK_VERSION(4, 0, 0)
    auto app = Gtk::Application::create("com.github.mrr736.fetchiso");
    return app->make_window_and_run<FetchISOApp>(argc, argv);
#else
    auto app = Gtk::Application::create(argc, argv, "com.github.mrr736.fetchiso");
    FetchISOApp window;
    return app->run(window);
#endif
}
