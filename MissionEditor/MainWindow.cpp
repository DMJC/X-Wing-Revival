#include "MainWindow.h"
#include "Dialogs.h"
#include <gtkmm.h>
#include <cstdio>
#include <stdexcept>

// ── construction ──────────────────────────────────────────────────────────────

MainWindow::MainWindow() {
    set_title("X-Wing Revival Mission Editor");
    set_default_size(1200, 720);

    // File toolbar at top
    mainVBox.pack_start(fileBox, false, false, 4);
    fileBox.set_margin_start(8); fileBox.set_margin_end(8);
    fileBox.set_margin_top(4);
    fileBox.pack_start(btnNew,    false, false, 0);
    fileBox.pack_start(btnOpen,   false, false, 0);
    fileBox.pack_start(btnSave,   false, false, 0);
    fileBox.pack_end  (btnExport, false, false, 0);

    mainVBox.pack_start(hPaned, true, true, 0);
    add(mainVBox);

    // ── LEFT PANEL ────────────────────────────────────────────────────────────
    leftBox.set_margin_start(6); leftBox.set_margin_end(4);
    leftBox.set_margin_top(6);   leftBox.set_margin_bottom(6);
    leftBox.set_size_request(180, -1);

    lblSystems.set_markup("<b>Star Systems</b>");
    lblSystems.set_halign(Gtk::ALIGN_START);
    leftBox.pack_start(lblSystems, false, false, 2);

    sysScroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    sysScroll.add(sysList);
    leftBox.pack_start(sysScroll, true, true, 0);

    sysButtons.set_homogeneous(true);
    sysButtons.pack_start(btnAddSystem,  true, true, 0);
    sysButtons.pack_start(btnDelSystem,  true, true, 0);
    leftBox.pack_start(sysButtons, false, false, 0);
    leftBox.pack_start(btnEditSystem, false, false, 0);

    hPaned.pack1(leftBox, false, false);

    // ── CANVAS (centre) ───────────────────────────────────────────────────────
    canvas.set_size_request(600, -1);
    canvas.setMission(&mission);
    canvas.setActiveSystem(0);

    // ── RIGHT PANEL ───────────────────────────────────────────────────────────
    rightBox.set_margin_start(4); rightBox.set_margin_end(8);
    rightBox.set_margin_top(6);   rightBox.set_margin_bottom(6);
    rightBox.set_size_request(220, -1);

    // Add-object frame
    addBox.set_margin_start(6); addBox.set_margin_end(6);
    addBox.set_margin_top(4);   addBox.set_margin_bottom(4);

    // Object type selector
    cmbObjType.append("Ship",          "Ship");
    cmbObjType.append("NavPoint",      "NavPoint");
    cmbObjType.append("AsteroidField", "Asteroid Field");
    cmbObjType.set_active(0);
    addBox.pack_start(cmbObjType, false, false, 0);

    // Ship type
    for (const auto& st : SHIP_TYPES)
        cmbShipType.append(st.shortName, st.shortName + "  " + st.displayName);
    cmbShipType.set_active(0);
    addBox.pack_start(cmbShipType, false, false, 0);

    // Faction
    for (int i = 0; i < (int)FACTION_NAMES.size(); i++)
        cmbFaction.append(std::to_string(i), FACTION_NAMES[i]);
    cmbFaction.set_active(0);
    addBox.pack_start(cmbFaction, false, false, 0);

    addBox.pack_start(btnAdd, false, false, 0);
    frmAdd.add(addBox);
    rightBox.pack_start(frmAdd, false, false, 0);

    // Detail frame
    detailGrid.set_column_spacing(6); detailGrid.set_row_spacing(4);
    detailGrid.set_margin_start(6);   detailGrid.set_margin_end(6);
    detailGrid.set_margin_top(4);     detailGrid.set_margin_bottom(4);

    int dr = 0;
    auto addDRow = [&](Gtk::Label& lbl, Gtk::Widget& w) {
        lbl.set_halign(Gtk::ALIGN_END);
        detailGrid.attach(lbl, 0, dr,   1, 1);
        detailGrid.attach(w,   1, dr++, 1, 1);
        w.set_hexpand(true);
    };
    addDRow(lblName, entName);
    addDRow(lblX,    entX);
    addDRow(lblY,    entY);
    addDRow(lblZ,    entZ);
    detailGrid.attach(chkLanding,   0, dr++, 2, 1);

    // Asteroid count (hidden by default)
    adjAstCount = Gtk::Adjustment::create(16, 1, 512, 1, 8);
    spnAstCount.set_adjustment(adjAstCount);
    addDRow(lblAstCount, spnAstCount);
    lblAstCount.set_visible(false);
    spnAstCount.set_visible(false);

    // Buttons row
    detailButtons.set_homogeneous(true);
    detailButtons.pack_start(btnDelete,   true, true, 0);
    detailButtons.pack_start(btnEditNavPt,true, true, 0);
    detailGrid.attach(detailButtons, 0, dr++, 2, 1);

    frmDetails.add(detailGrid);
    rightBox.pack_start(frmDetails, false, false, 0);

    // Mission buttons frame
    missionBtnBox.set_margin_start(6); missionBtnBox.set_margin_end(6);
    missionBtnBox.set_margin_top(4);   missionBtnBox.set_margin_bottom(4);
    missionBtnBox.pack_start(btnProperties, false, false, 0);
    missionBtnBox.pack_start(btnBriefing,   false, false, 0);
    missionBtnBox.pack_start(btnWinLoss,    false, false, 0);
    frmMission.add(missionBtnBox);
    rightBox.pack_start(frmMission, false, false, 0);

    // Assemble: hPaned = leftBox | innerPaned(canvas | rightBox)
    Gtk::HPaned* innerPaned = Gtk::manage(new Gtk::HPaned());
    innerPaned->pack1(canvas,   true,  true);
    innerPaned->pack2(rightBox, false, false);
    hPaned.pack2(*innerPaned, true, true);

    // Widgets hidden at startup: use set_no_show_all so win.show_all() in main() skips them
    btnEditNavPt.set_no_show_all(true);
    btnEditNavPt.set_visible(false);
    lblAstCount.set_no_show_all(true);
    lblAstCount.set_visible(false);
    spnAstCount.set_no_show_all(true);
    spnAstCount.set_visible(false);
    frmDetails.set_sensitive(false);

    // ── signal connections ────────────────────────────────────────────────────
    btnAddSystem.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onAddSystem));
    btnDelSystem.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onDeleteSystem));
    btnEditSystem.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onEditSystem));
    sysList.signal_row_selected().connect(sigc::mem_fun(*this, &MainWindow::onSystemRowSelected));

    btnAdd.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onAddObject));
    btnDelete.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onDeleteObject));
    btnEditNavPt.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onEditNavPoint));

    canvas.signalObjectSelected.connect(sigc::mem_fun(*this, &MainWindow::onObjectSelected));
    canvas.signalObjectMoved.connect(sigc::mem_fun(*this, &MainWindow::onObjectMoved));

    cmbObjType.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onObjTypeChanged));

    // Detail panel live-update
    entName.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onDetailChanged));
    entX.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onDetailChanged));
    entY.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onDetailChanged));
    entZ.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onDetailChanged));
    chkLanding.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onDetailChanged));
    spnAstCount.signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::onDetailChanged));

    btnProperties.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onProperties));
    btnBriefing.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onBriefing));
    btnWinLoss.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onWinLoss));

    btnNew.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onNew));
    btnOpen.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onOpen));
    btnSave.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onSave));
    btnExport.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onExport));

    rebuildSystemList();
    setTitle();

    // Pre-show all children so app->run(win) only needs to call win.show().
    // Widgets marked set_no_show_all(true) above are skipped by this call.
    show_all_children();
}

// ── system list management ────────────────────────────────────────────────────

void MainWindow::rebuildSystemList() {
    // Remove all rows
    auto rows = sysList.get_children();
    for (auto* w : rows) sysList.remove(*w);

    for (int i = 0; i < (int)mission.systems.size(); i++) {
        auto* row = Gtk::manage(new Gtk::ListBoxRow());
        auto* lbl = Gtk::manage(new Gtk::Label(
            "System " + std::to_string(i + 1) +
            "  [" + mission.systems[i].background + "]"));
        lbl->set_halign(Gtk::ALIGN_START);
        lbl->set_margin_start(6); lbl->set_margin_end(6);
        lbl->set_margin_top(4);   lbl->set_margin_bottom(4);
        row->add(*lbl);
        sysList.append(*row);
    }
    sysList.show_all();
    if (activeSystemIdx < (int)mission.systems.size())
        sysList.select_row(*sysList.get_row_at_index(activeSystemIdx));
}

void MainWindow::selectSystem(int idx) {
    if (idx < 0 || idx >= (int)mission.systems.size()) return;
    activeSystemIdx = idx;
    canvas.setActiveSystem(idx);
    updateDetailPanel(-1);
}

// ── detail panel ─────────────────────────────────────────────────────────────

void MainWindow::updateObjectTypeControls(ObjectType t) {
    bool isShip     = (t == ObjectType::Ship);
    bool isNav      = (t == ObjectType::NavPoint);
    bool isAsteroid = (t == ObjectType::AsteroidField);

    cmbShipType.set_visible(isShip);
    cmbFaction.set_visible(isShip);
    chkLanding.set_visible(isShip);

    // Clear no_show_all before making visible so set_visible(true) takes effect
    btnEditNavPt.set_no_show_all(false);
    btnEditNavPt.set_visible(isNav);
    btnEditNavPt.set_no_show_all(!isNav);

    lblAstCount.set_no_show_all(false);
    lblAstCount.set_visible(isAsteroid);
    lblAstCount.set_no_show_all(!isAsteroid);

    spnAstCount.set_no_show_all(false);
    spnAstCount.set_visible(isAsteroid);
    spnAstCount.set_no_show_all(!isAsteroid);
}

void MainWindow::updateDetailPanel(int objIdx) {
    updatingDetails = true;

    bool haveObj = (objIdx >= 0 &&
                    activeSystemIdx < (int)mission.systems.size() &&
                    objIdx < (int)mission.systems[activeSystemIdx].objects.size());

    frmDetails.set_sensitive(haveObj);

    if (haveObj) {
        const auto& obj = mission.systems[activeSystemIdx].objects[objIdx];
        entName.set_text(obj.name);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", obj.x); entX.set_text(buf);
        snprintf(buf, sizeof(buf), "%.1f", obj.y); entY.set_text(buf);
        snprintf(buf, sizeof(buf), "%.1f", obj.z); entZ.set_text(buf);
        chkLanding.set_active(obj.isLandingShip);
        spnAstCount.set_value(obj.asteroidCount);
        updateObjectTypeControls(obj.objType);
    } else {
        entName.set_text("");
        entX.set_text("0"); entY.set_text("0"); entZ.set_text("0");
        chkLanding.set_active(false);
        spnAstCount.set_value(16);
        updateObjectTypeControls(ObjectType::Ship);
    }

    updatingDetails = false;
}

void MainWindow::applyDetailPanel(int objIdx) {
    if (objIdx < 0 || activeSystemIdx >= (int)mission.systems.size()) return;
    if (objIdx >= (int)mission.systems[activeSystemIdx].objects.size()) return;

    auto& obj = mission.systems[activeSystemIdx].objects[objIdx];
    obj.name = entName.get_text();
    try { obj.x = std::stod(entX.get_text()); } catch (...) {}
    try { obj.y = std::stod(entY.get_text()); } catch (...) {}
    try { obj.z = std::stod(entZ.get_text()); } catch (...) {}
    obj.isLandingShip = chkLanding.get_active();
    obj.asteroidCount = (int)spnAstCount.get_value();
}

// ── signal handlers ───────────────────────────────────────────────────────────

void MainWindow::onAddSystem() {
    MissionSystem sys;
    sys.number = (int)mission.systems.size() + 1;
    // Offset from last system so they don't overlap
    if (!mission.systems.empty()) {
        const auto& prev = mission.systems.back();
        sys.spawnX = prev.spawnX + 10000000.0;
        sys.spawnY = prev.spawnY;
        sys.spawnZ = 0;
        sys.background = "nebula";
    }
    mission.systems.push_back(sys);
    rebuildSystemList();
    selectSystem((int)mission.systems.size() - 1);
}

void MainWindow::onDeleteSystem() {
    if (mission.systems.size() <= 1) {
        Gtk::MessageDialog d(*this, "Cannot remove the last system.", false,
                             Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        d.run();
        return;
    }
    mission.systems.erase(mission.systems.begin() + activeSystemIdx);
    // Renumber
    for (int i = 0; i < (int)mission.systems.size(); i++)
        mission.systems[i].number = i + 1;
    activeSystemIdx = std::max(0, activeSystemIdx - 1);
    rebuildSystemList();
    selectSystem(activeSystemIdx);
}

void MainWindow::onEditSystem() {
    if (activeSystemIdx >= (int)mission.systems.size()) return;
    SystemDialog dlg(*this, mission.systems[activeSystemIdx]);
    if (dlg.run() == Gtk::RESPONSE_OK) {
        dlg.apply();
        rebuildSystemList();
        canvas.refresh();
    }
}

void MainWindow::onSystemRowSelected(Gtk::ListBoxRow* row) {
    if (!row) return;
    selectSystem(row->get_index());
}

void MainWindow::onAddObject() {
    if (activeSystemIdx >= (int)mission.systems.size()) return;

    std::string typeId = cmbObjType.get_active_id();
    MissionObject obj;
    // Place at view centre (world 0,0 initially, or could read canvas view)
    obj.x = 0; obj.y = 0; obj.z = 0;

    if (typeId == "NavPoint") {
        obj.objType = ObjectType::NavPoint;
        obj.name    = "NavPoint";
        // Default target: next system, or system 1 if we're on the last
        int numSys = (int)mission.systems.size();
        obj.navTargetSystem = (activeSystemIdx + 1 < numSys)
                              ? activeSystemIdx + 2  // 1-based
                              : 1;
        obj.navVisible = true;
    } else if (typeId == "AsteroidField") {
        obj.objType = ObjectType::AsteroidField;
        obj.name    = "Asteroids";
        obj.asteroidCount = 16;
    } else {
        obj.objType   = ObjectType::Ship;
        obj.shipClass = cmbShipType.get_active_id();
        if (obj.shipClass.empty()) obj.shipClass = "X/W";
        obj.size      = shipSizeFor(obj.shipClass);
        int fi        = cmbFaction.get_active_row_number();
        obj.faction   = (fi >= 0) ? static_cast<Faction>(fi) : Faction::Rebel;
    }

    auto& sys = mission.systems[activeSystemIdx];
    sys.objects.push_back(obj);
    int newIdx = (int)sys.objects.size() - 1;

    // Select the new object
    for (auto& o : sys.objects) o.selected = false;
    sys.objects[newIdx].selected = true;
    canvas.refresh();
    updateDetailPanel(newIdx);
    onObjectSelected(newIdx);

    // For navpoints, open their dialog immediately
    if (obj.objType == ObjectType::NavPoint)
        onEditNavPoint();
}

void MainWindow::onDeleteObject() {
    if (activeSystemIdx >= (int)mission.systems.size()) return;
    auto& objs = mission.systems[activeSystemIdx].objects;
    int idx = canvas.selectedIndex();
    if (idx < 0 || idx >= (int)objs.size()) return;
    objs.erase(objs.begin() + idx);
    canvas.refresh();
    updateDetailPanel(-1);
    canvas.signalObjectSelected.emit(-1);
}

void MainWindow::onEditNavPoint() {
    if (activeSystemIdx >= (int)mission.systems.size()) return;
    int idx = canvas.selectedIndex();
    if (idx < 0) return;
    auto& objs = mission.systems[activeSystemIdx].objects;
    if (idx >= (int)objs.size() || objs[idx].objType != ObjectType::NavPoint) return;

    NavPointDialog dlg(*this, objs[idx], (int)mission.systems.size());
    if (dlg.run() == Gtk::RESPONSE_OK) {
        dlg.apply();
        canvas.refresh();
    }
}

void MainWindow::onObjectSelected(int idx) {
    updateDetailPanel(idx);
}

void MainWindow::onObjectMoved(int idx) {
    // Update coordinate entries when user drags an object
    if (idx < 0 || activeSystemIdx >= (int)mission.systems.size()) return;
    if (idx >= (int)mission.systems[activeSystemIdx].objects.size()) return;

    updatingDetails = true;
    const auto& obj = mission.systems[activeSystemIdx].objects[idx];
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f", obj.x); entX.set_text(buf);
    snprintf(buf, sizeof(buf), "%.1f", obj.y); entY.set_text(buf);
    snprintf(buf, sizeof(buf), "%.1f", obj.z); entZ.set_text(buf);
    updatingDetails = false;
}

void MainWindow::onDetailChanged() {
    if (updatingDetails) return;
    int idx = canvas.selectedIndex();
    if (idx >= 0) {
        applyDetailPanel(idx);
        canvas.refresh();
    }
}

void MainWindow::onObjTypeChanged() {
    std::string id = cmbObjType.get_active_id();
    ObjectType t = ObjectType::Ship;
    if (id == "NavPoint")      t = ObjectType::NavPoint;
    if (id == "AsteroidField") t = ObjectType::AsteroidField;
    // Update the Add controls
    cmbShipType.set_visible(t == ObjectType::Ship);
    cmbFaction.set_visible(t == ObjectType::Ship);
}

void MainWindow::onProperties() {
    PropertiesDialog dlg(*this, mission);
    if (dlg.run() == Gtk::RESPONSE_OK) {
        dlg.apply();
        setTitle();
    }
}

void MainWindow::onBriefing() {
    BriefingDialog dlg(*this, mission);
    if (dlg.run() == Gtk::RESPONSE_OK)
        dlg.apply();
}

void MainWindow::onWinLoss() {
    WinLossDialog dlg(*this, mission);
    if (dlg.run() == Gtk::RESPONSE_OK)
        dlg.apply();
}

void MainWindow::onNew() {
    Gtk::MessageDialog d(*this, "Create a new mission? Unsaved changes will be lost.",
                         false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
    if (d.run() != Gtk::RESPONSE_YES) return;
    mission = MissionData();
    activeSystemIdx = 0;
    canvas.setMission(&mission);
    canvas.setActiveSystem(0);
    rebuildSystemList();
    updateDetailPanel(-1);
    setTitle();
}

void MainWindow::onOpen() {
    Gtk::FileChooserDialog dlg(*this, "Open Mission File", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dlg.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dlg.add_button("Open",   Gtk::RESPONSE_OK);
    auto filter = Gtk::FileFilter::create();
    filter->set_name("Mission files (*.def)");
    filter->add_pattern("*.def");
    dlg.add_filter(filter);
    // Default directory
    dlg.set_current_folder(
        Glib::get_home_dir() + "/development/X-Wing-Revival/Data/Missions");

    if (dlg.run() == Gtk::RESPONSE_OK) {
        // Basic import: read the file and show it as text for now
        // Full parsing is out of scope; we just display a notice
        Gtk::MessageDialog info(*this,
            "Open: file parsing is not yet implemented.\n"
            "Use 'Save As' to export new missions, and edit existing files by hand.",
            false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        info.run();
    }
}

void MainWindow::onSave() {
    Gtk::FileChooserDialog dlg(*this, "Save Mission File", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dlg.add_button("Cancel",  Gtk::RESPONSE_CANCEL);
    dlg.add_button("Save",    Gtk::RESPONSE_OK);
    dlg.set_do_overwrite_confirmation(true);
    auto filter = Gtk::FileFilter::create();
    filter->set_name("Mission files (*.def)");
    filter->add_pattern("*.def");
    dlg.add_filter(filter);
    dlg.set_current_folder(
        Glib::get_home_dir() + "/development/X-Wing-Revival/Data/Missions");
    dlg.set_current_name(mission.missionName + ".def");

    if (dlg.run() == Gtk::RESPONSE_OK) {
        std::string path = dlg.get_filename();
        if (mission.saveToFile(path)) {
            Gtk::MessageDialog ok(*this, "Saved to:\n" + path,
                                  false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            ok.run();
        } else {
            Gtk::MessageDialog err(*this, "Failed to write file.",
                                   false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run();
        }
    }
}

void MainWindow::onExport() {
    ExportDialog dlg(*this, mission.exportText());
    dlg.run();
}

void MainWindow::setTitle() {
    set_title("X-Wing Revival Mission Editor  —  " + mission.missionName);
}
