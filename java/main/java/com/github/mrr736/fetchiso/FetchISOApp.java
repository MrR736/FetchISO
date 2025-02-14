/*
// FetchISO Gui for Modern Java
// version 1.0
// https://github.com/MrR736/FetchISO
//
// SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
// SPDX-License-Identifier: MIT
*/

package com.github.mrr736.fetchiso;

import com.github.mrr736.fetchiso.Downloader;
import com.github.mrr736.fetchiso.FetchISOListManager;

import javax.swing.*;
import java.awt.*;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

public class FetchISOApp {
    private JFrame frame;
    private JTextField searchField;
    private JComboBox<String> archSelector;
    private DefaultListModel<String> listModel;
    private JList<String> isoList;
    private List<FetchISOListManager.ISOEntry> allEntries;

    private static final Map<String, String> ARCH_MAP = Map.ofEntries(
        Map.entry("all", "All Architectures"),
        Map.entry("amd64", "x86_64 (64-bit)"),
        Map.entry("i386", "x86 (32-bit)"),
        Map.entry("aarch64", "ARM64 (AArch64)"),
        Map.entry("armhf", "ARMv7 Hard Float"),
        Map.entry("armel", "ARMEL"),
        Map.entry("mips64el", "MIPS64EL"),
        Map.entry("mipsel", "MIPSEL"),
        Map.entry("ppc64el", "PowerPC64LE"),
        Map.entry("ppc64", "PowerPC (64-bit)"),
        Map.entry("ppc", "PowerPC (32-bit)"),
        Map.entry("ppcspe", "PowerPC SPE"),
        Map.entry("s390x", "IBM Z (s390x)"),
        Map.entry("riscv64", "RISC-V 64-bit")
    );

    public FetchISOApp() {
        frame = new JFrame("FetchISO - Quickly Fetch ISOs");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(500, 500);
        frame.setLayout(new BorderLayout());

        // Top Panel (Search & Architecture Filter)
        JPanel searchPanel = new JPanel(new FlowLayout());
        searchField = new JTextField(20);
        archSelector = new JComboBox<>(ARCH_MAP.values().toArray(new String[0]));
        archSelector.setSelectedItem(ARCH_MAP.get("all"));

        // Event listeners
        searchField.addKeyListener(new KeyAdapter() {
            @Override
            public void keyReleased(KeyEvent e) {
                updateSearchResults();
            }
        });

        archSelector.addActionListener(e -> updateSearchResults());
        searchPanel.add(searchField);
        searchPanel.add(archSelector);

        // List Model & ISO List
        listModel = new DefaultListModel<>();
        isoList = new JList<>(listModel);
        isoList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        JScrollPane scrollPane = new JScrollPane(isoList);

        // Handle selection and initiate download
        isoList.addListSelectionListener(e -> {
            if (!e.getValueIsAdjusting() && isoList.getSelectedIndex() != -1) {
                startDownload();
            }
        });

        // Add components to frame
        frame.add(searchPanel, BorderLayout.NORTH);
        frame.add(scrollPane, BorderLayout.CENTER);

        // Load ISO list and update UI
        fetchISOList();
        updateSearchResults();

        frame.setVisible(true);
    }

    private void fetchISOList() {
        List<FetchISOListManager.ISOEntry> fetchedList = FetchISOListManager.getISOList();
        allEntries = (fetchedList != null) ? fetchedList : List.of(); // Prevents null issues
    }

    private void updateSearchResults() {
        if (allEntries.isEmpty()) {
            listModel.clear();
            return;
        }

        String query = searchField.getText().trim().toLowerCase();
        String selectedArch = (String) archSelector.getSelectedItem();
        String selectedArchKey = ARCH_MAP.entrySet().stream()
            .filter(entry -> entry.getValue().equals(selectedArch))
            .map(Map.Entry::getKey)
            .findFirst()
            .orElse("all");

        // Filter ISOs based on search query and architecture
        List<String> filteredNames = allEntries.stream()
            .filter(e -> e.name.toLowerCase().contains(query) &&
                         ("all".equals(selectedArchKey) || selectedArchKey.equalsIgnoreCase(e.arch)))
            .map(e -> e.name)  // Extract names directly
            .collect(Collectors.toList());

        // Update UI only if necessary
        if (!listModel.equals(filteredNames)) {
            listModel.clear();
            filteredNames.forEach(listModel::addElement);
        }
    }

    private void startDownload() {
        int selectedIndex = isoList.getSelectedIndex();
        if (selectedIndex == -1) return;

        String selectedISOName = isoList.getSelectedValue();
        FetchISOListManager.ISOEntry iso = allEntries.stream()
            .filter(e -> e.name.equals(selectedISOName))
            .findFirst()
            .orElse(null);

        if (iso != null) {
            int confirm = JOptionPane.showConfirmDialog(
                frame, "Download " + iso.name + "?", "Confirm", JOptionPane.YES_NO_OPTION
            );
            if (confirm == JOptionPane.YES_OPTION) {
                new Downloader().downloadFile(iso.url, iso.file);
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(FetchISOApp::new);
    }
}
