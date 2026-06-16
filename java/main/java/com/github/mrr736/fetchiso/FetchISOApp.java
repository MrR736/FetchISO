/*
 * FetchISO Gui for Modern Java
 * version 1.1
 * https://github.com/MrR736/FetchISO
 *
 * SPDX-FileCopyrightText: 2025 MrR736 <https://github.com/MrR736>
 * SPDX-License-Identifier: MIT
 */

package com.github.mrr736.fetchiso;

import javax.swing.*;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.util.List;
import java.util.stream.Collectors;

import com.github.mrr736.fetchiso.*;

public class FetchISOApp {
	private JFrame frame;
	private JTextField searchField;
	private JComboBox<String> archSelector;
	private DefaultListModel<String> listModel;
	private JList<String> isoList;
	private List<FetchISOListManager.ISOEntry> allEntries;

	private static final int FETCHISO_WIDTH =  500;
	private static final int FETCHISO_HEIGHT = 500;

	private static final Map<String,String> ARCH_MAP = new LinkedHashMap<>();

	static {
		ARCH_MAP.put("all","All Architectures");
		ARCH_MAP.put("amd64","x86_64 (64-bit)");
		ARCH_MAP.put("i386","x86 (32-bit)");
		ARCH_MAP.put("aarch64","ARM64 (AArch64)");
		ARCH_MAP.put("armhf","ARMv7 Hard Float");
		ARCH_MAP.put("armel","ARMEL");
		ARCH_MAP.put("mips64el","MIPS64EL");
		ARCH_MAP.put("mipsel","MIPSEL");
		ARCH_MAP.put("ppc64el","PowerPC64LE");
		ARCH_MAP.put("ppc64","PowerPC (64-bit)");
		ARCH_MAP.put("ppc","PowerPC (32-bit)");
		ARCH_MAP.put("ppcspe","PowerPC SPE");
		ARCH_MAP.put("s390x","IBM Z (s390x)");
		ARCH_MAP.put("riscv64","RISC-V 64-bit");
	}

	public FetchISOApp() {
		frame = new JFrame("FetchISO - Quickly Fetch ISOs");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setSize(FETCHISO_WIDTH,FETCHISO_HEIGHT);
		frame.setLayout(new BorderLayout());
		JPanel searchPanel = new JPanel(new FlowLayout());
		searchField = new JTextField(20);
		archSelector = new JComboBox<>(ARCH_MAP.values().toArray(new String[0]));
		archSelector.setSelectedItem(ARCH_MAP.get("all"));
		searchField.addKeyListener(
			new KeyAdapter() {
				@Override
				public void keyReleased(KeyEvent e) {
					updateSearchResults();
				}
			}
		);
		archSelector.addActionListener(e->updateSearchResults());
		searchPanel.add(searchField);
		searchPanel.add(archSelector);
		listModel = new DefaultListModel<>();
		isoList = new JList<>(listModel);
		isoList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		JScrollPane scrollPane = new JScrollPane(isoList);
		isoList.addListSelectionListener(e-> {
				if (!e.getValueIsAdjusting() && isoList.getSelectedIndex() != -1) {
					startDownload();
				}
			}
		);

		frame.add(searchPanel,BorderLayout.NORTH);
		frame.add(scrollPane,BorderLayout.CENTER);
		fetchISOList();
		updateSearchResults();
		frame.setVisible(true);
	}

	private void fetchISOList() {
		List<FetchISOListManager.ISOEntry> fetchedList = FetchISOListManager.getISOList();
		allEntries = (fetchedList != null) ? fetchedList : List.of();
	}

	private void updateSearchResults() {
		if (allEntries.isEmpty()) {
			listModel.clear();
			return;
		}

		String query = searchField.getText().trim().toLowerCase();
		String selectedArch =(String)archSelector.getSelectedItem();
		String selectedArchKey =
			ARCH_MAP.entrySet().stream().filter(entry->entry.getValue().equals(selectedArch))
			.map(Map.Entry::getKey).findFirst().orElse("all");

		List<String> filteredNames =
			allEntries.stream().filter(e->e.name.toLowerCase().contains(query) && ("all".equals(selectedArchKey) ||
					selectedArchKey.equalsIgnoreCase(e.arch))
			).map(e->e.name).collect(Collectors.toList());
		listModel.clear();
		filteredNames.forEach(listModel::addElement);
	}

	private void startDownload() {
		int selectedIndex = isoList.getSelectedIndex();
		if (selectedIndex == -1)
			return;
		String selectedISOName = isoList.getSelectedValue();
		FetchISOListManager.ISOEntry iso =
			allEntries.stream().filter(e->e.name.equals(selectedISOName)).findFirst().orElse(null);
		if (iso != null) {
			int confirm =
				JOptionPane.showConfirmDialog(frame,"Download " + iso.name + "?", "Confirm", JOptionPane.YES_NO_OPTION);
			if (confirm == JOptionPane.YES_OPTION) {
				new Downloader().downloadFile(iso.url,iso.file);
			}
		}
	}

	public static void main(String[] args) {
		SwingUtilities.invokeLater(FetchISOApp::new);
	}
}
