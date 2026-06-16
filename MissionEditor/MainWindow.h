#pragma once
#include <gtkmm.h>
#include "MissionData.h"
#include "MissionCanvas.h"

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow();
    ~MainWindow() override = default;

private:
    MissionData mission;
    int         activeSystemIdx = 0;

    // ── layout ────────────────────────────────────────────────────────────────
    Gtk::Box        mainVBox{Gtk::ORIENTATION_VERTICAL};
    Gtk::HPaned     hPaned;

    // Left panel
    Gtk::Box        leftBox{Gtk::ORIENTATION_VERTICAL, 4};
    Gtk::Label      lblSystems{"Star Systems"};
    Gtk::ScrolledWindow sysScroll;
    Gtk::ListBox    sysList;
    Gtk::Box        sysButtons{Gtk::ORIENTATION_HORIZONTAL, 4};
    Gtk::Button     btnAddSystem{"Add System"};
    Gtk::Button     btnDelSystem{"Delete System"};
    Gtk::Button     btnEditSystem{"Properties…"};

    // Centre
    MissionCanvas   canvas;

    // Right panel
    Gtk::Box        rightBox{Gtk::ORIENTATION_VERTICAL, 6};

    // Add-object controls
    Gtk::Frame      frmAdd{"Add Object"};
    Gtk::Box        addBox{Gtk::ORIENTATION_VERTICAL, 4};
    Gtk::Box        addTypeRow{Gtk::ORIENTATION_HORIZONTAL, 4};
    Gtk::ComboBoxText cmbObjType;
    Gtk::ComboBoxText cmbShipType;
    Gtk::ComboBoxText cmbFaction;
    Gtk::Button     btnAdd{"Add"};

    // Selected-object details
    Gtk::Frame      frmDetails{"Selected Object"};
    Gtk::Grid       detailGrid;
    Gtk::Label      lblName{"Name:"};
    Gtk::Entry      entName;
    Gtk::Label      lblX{"X:"}, lblY{"Y:"}, lblZ{"Z:"};
    Gtk::Entry      entX, entY, entZ;
    Gtk::CheckButton chkLanding{"Landing Ship"};
    Gtk::Box        detailButtons{Gtk::ORIENTATION_HORIZONTAL, 4};
    Gtk::Button     btnDelete{"Delete"};
    Gtk::Button     btnEditNavPt{"NavPoint…"};

    // Asteroid count (shown only for asteroid fields)
    Gtk::Label      lblAstCount{"Asteroids:"};
    Gtk::SpinButton spnAstCount;
    Glib::RefPtr<Gtk::Adjustment> adjAstCount;

    // Mission buttons
    Gtk::Frame      frmMission{"Mission"};
    Gtk::Box        missionBtnBox{Gtk::ORIENTATION_VERTICAL, 4};
    Gtk::Button     btnProperties{"Mission Properties…"};
    Gtk::Button     btnBriefing{"Briefing Text…"};
    Gtk::Button     btnWinLoss{"Win / Loss Text…"};

    // File ops
    Gtk::Box        fileBox{Gtk::ORIENTATION_HORIZONTAL, 4};
    Gtk::Button     btnNew{"New"};
    Gtk::Button     btnOpen{"Open…"};
    Gtk::Button     btnSave{"Save As…"};
    Gtk::Button     btnExport{"Preview Output"};

    // ── state ─────────────────────────────────────────────────────────────────
    bool updatingDetails = false;  // guard against re-entrancy

    // ── helpers ───────────────────────────────────────────────────────────────
    void rebuildSystemList();
    void selectSystem(int idx);
    void updateDetailPanel(int objIdx);
    void applyDetailPanel(int objIdx);
    void updateObjectTypeControls(ObjectType t);
    void setTitle();

    // ── signal handlers ───────────────────────────────────────────────────────
    void onAddSystem();
    void onDeleteSystem();
    void onEditSystem();
    void onSystemRowSelected(Gtk::ListBoxRow* row);
    void onAddObject();
    void onDeleteObject();
    void onEditNavPoint();
    void onObjectSelected(int idx);
    void onObjectMoved(int idx);
    void onDetailChanged();
    void onObjTypeChanged();
    void onProperties();
    void onBriefing();
    void onWinLoss();
    void onNew();
    void onOpen();
    void onSave();
    void onExport();
};
