#pragma once
#include <gtkmm.h>
#include "MissionData.h"

// ── Mission Properties Dialog ─────────────────────────────────────────────────

class PropertiesDialog : public Gtk::Dialog {
public:
    PropertiesDialog(Gtk::Window& parent, MissionData& mission)
        : Gtk::Dialog("Mission Properties", parent, true)
        , m(mission)
    {
        set_default_size(400, 320);
        auto* area = get_content_area();

        auto* grid = Gtk::manage(new Gtk::Grid());
        grid->set_column_spacing(8);
        grid->set_row_spacing(6);
        grid->set_margin_start(12); grid->set_margin_end(12);
        grid->set_margin_top(8);   grid->set_margin_bottom(8);
        area->pack_start(*grid, true, true, 0);

        int row = 0;
        auto addRow = [&](const char* label, Gtk::Widget& w) {
            auto* lbl = Gtk::manage(new Gtk::Label(label));
            lbl->set_halign(Gtk::ALIGN_END);
            grid->attach(*lbl, 0, row, 1, 1);
            grid->attach(w,    1, row, 1, 1);
            w.set_hexpand(true);
            row++;
        };

        entName.set_text(m.missionName);
        addRow("Mission Name:", entName);

        entDesc.set_text(m.missionDesc);
        addRow("Description:", entDesc);

        for (const auto& gt : GAMETYPES)
            cmbGametype.append(gt);
        cmbGametype.set_active_text(m.gametype);
        addRow("Gametype:", cmbGametype);

        cmbPlayerTeam.append("rebel");
        cmbPlayerTeam.append("empire");
        cmbPlayerTeam.set_active_text(m.playerTeam);
        addRow("Player Team:", cmbPlayerTeam);

        entPlayerShip.set_text(m.playerShip);
        addRow("Player Ship(s):", entPlayerShip);

        chkAllowChange.set_label("Allow mid-mission ship change");
        chkAllowChange.set_active(m.allowShipChange);
        grid->attach(chkAllowChange, 0, row++, 2, 1);

        add_button("Cancel", Gtk::RESPONSE_CANCEL);
        add_button("OK",     Gtk::RESPONSE_OK);
        set_default_response(Gtk::RESPONSE_OK);

        show_all_children();
    }

    void apply() {
        m.missionName    = entName.get_text();
        m.missionDesc    = entDesc.get_text();
        m.gametype       = cmbGametype.get_active_text();
        m.playerTeam     = cmbPlayerTeam.get_active_text();
        m.playerShip     = entPlayerShip.get_text();
        m.allowShipChange= chkAllowChange.get_active();
    }

private:
    MissionData& m;
    Gtk::Entry       entName, entDesc, entPlayerShip;
    Gtk::ComboBoxText cmbGametype, cmbPlayerTeam;
    Gtk::CheckButton  chkAllowChange;
};

// ── Briefing Dialog ───────────────────────────────────────────────────────────

class BriefingDialog : public Gtk::Dialog {
public:
    BriefingDialog(Gtk::Window& parent, MissionData& mission)
        : Gtk::Dialog("Mission Briefing", parent, true)
        , m(mission)
    {
        set_default_size(500, 380);
        auto* area = get_content_area();
        area->set_margin_start(12); area->set_margin_end(12);
        area->set_margin_top(8);   area->set_margin_bottom(8);

        auto* lbl = Gtk::manage(new Gtk::Label("Briefing text shown before the mission:"));
        lbl->set_halign(Gtk::ALIGN_START);
        area->pack_start(*lbl, false, false, 4);

        textBuf = Gtk::TextBuffer::create();
        textBuf->set_text(m.briefingText);
        textView.set_buffer(textBuf);
        textView.set_wrap_mode(Gtk::WRAP_WORD);
        textView.set_monospace(true);

        auto* scroll = Gtk::manage(new Gtk::ScrolledWindow());
        scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scroll->add(textView);
        scroll->set_hexpand(true);
        scroll->set_vexpand(true);
        area->pack_start(*scroll, true, true, 0);

        add_button("Cancel", Gtk::RESPONSE_CANCEL);
        add_button("OK",     Gtk::RESPONSE_OK);
        set_default_response(Gtk::RESPONSE_OK);
        show_all_children();
    }

    void apply() {
        m.briefingText = textBuf->get_text();
    }

private:
    MissionData& m;
    Glib::RefPtr<Gtk::TextBuffer> textBuf;
    Gtk::TextView textView;
};

// ── Win/Loss Dialog ───────────────────────────────────────────────────────────

class WinLossDialog : public Gtk::Dialog {
public:
    WinLossDialog(Gtk::Window& parent, MissionData& mission)
        : Gtk::Dialog("Win / Loss Messages", parent, true)
        , m(mission)
    {
        set_default_size(420, 260);
        auto* area = get_content_area();

        auto* grid = Gtk::manage(new Gtk::Grid());
        grid->set_column_spacing(8);
        grid->set_row_spacing(6);
        grid->set_margin_start(12); grid->set_margin_end(12);
        grid->set_margin_top(8);   grid->set_margin_bottom(8);
        area->pack_start(*grid, true, true, 0);

        auto* lblW = Gtk::manage(new Gtk::Label("Victory message:"));
        lblW->set_halign(Gtk::ALIGN_START);
        grid->attach(*lblW, 0, 0, 1, 1);

        winBuf = Gtk::TextBuffer::create();
        winBuf->set_text(m.winText);
        winView.set_buffer(winBuf);
        winView.set_wrap_mode(Gtk::WRAP_WORD);
        winView.set_size_request(-1, 80);
        auto* sw = Gtk::manage(new Gtk::ScrolledWindow());
        sw->add(winView); sw->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        sw->set_hexpand(true); sw->set_vexpand(true);
        grid->attach(*sw, 0, 1, 1, 1);

        auto* lblL = Gtk::manage(new Gtk::Label("Defeat message:"));
        lblL->set_halign(Gtk::ALIGN_START);
        grid->attach(*lblL, 0, 2, 1, 1);

        lossBuf = Gtk::TextBuffer::create();
        lossBuf->set_text(m.lossText);
        lossView.set_buffer(lossBuf);
        lossView.set_wrap_mode(Gtk::WRAP_WORD);
        lossView.set_size_request(-1, 80);
        auto* sl = Gtk::manage(new Gtk::ScrolledWindow());
        sl->add(lossView); sl->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        sl->set_hexpand(true); sl->set_vexpand(true);
        grid->attach(*sl, 0, 3, 1, 1);

        add_button("Cancel", Gtk::RESPONSE_CANCEL);
        add_button("OK",     Gtk::RESPONSE_OK);
        set_default_response(Gtk::RESPONSE_OK);
        show_all_children();
    }

    void apply() {
        m.winText  = winBuf->get_text();
        m.lossText = lossBuf->get_text();
    }

private:
    MissionData& m;
    Glib::RefPtr<Gtk::TextBuffer> winBuf, lossBuf;
    Gtk::TextView winView, lossView;
};

// ── Export Preview Dialog ─────────────────────────────────────────────────────

class ExportDialog : public Gtk::Dialog {
public:
    ExportDialog(Gtk::Window& parent, const std::string& text)
        : Gtk::Dialog("Mission Output", parent, true)
    {
        set_default_size(640, 540);
        auto* area = get_content_area();
        area->set_margin_start(8); area->set_margin_end(8);
        area->set_margin_top(6);   area->set_margin_bottom(6);

        buf = Gtk::TextBuffer::create();
        buf->set_text(text);
        tv.set_buffer(buf);
        tv.set_monospace(true);
        tv.set_editable(false);

        auto* sc = Gtk::manage(new Gtk::ScrolledWindow());
        sc->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        sc->add(tv);
        sc->set_hexpand(true);
        sc->set_vexpand(true);
        area->pack_start(*sc, true, true, 0);

        add_button("Close",     Gtk::RESPONSE_CLOSE);
        add_button("Copy All",  1);
        show_all_children();

        signal_response().connect(sigc::mem_fun(*this, &ExportDialog::onResponse));
    }

    void onResponse(int id) {
        if (id == 1) {
            Gtk::Clipboard::get()->set_text(buf->get_text());
        } else {
            hide();
        }
    }

private:
    Glib::RefPtr<Gtk::TextBuffer> buf;
    Gtk::TextView tv;
};

// ── NavPoint Properties Dialog ────────────────────────────────────────────────

class NavPointDialog : public Gtk::Dialog {
public:
    NavPointDialog(Gtk::Window& parent, MissionObject& obj, int numSystems)
        : Gtk::Dialog("NavPoint Properties", parent, true)
        , o(obj)
    {
        set_default_size(360, 220);
        auto* area = get_content_area();
        auto* grid = Gtk::manage(new Gtk::Grid());
        grid->set_column_spacing(8); grid->set_row_spacing(6);
        grid->set_margin_start(12); grid->set_margin_end(12);
        grid->set_margin_top(8);   grid->set_margin_bottom(8);
        area->pack_start(*grid, true, true, 0);

        int row = 0;
        auto addRow = [&](const char* lbl, Gtk::Widget& w) {
            auto* l = Gtk::manage(new Gtk::Label(lbl));
            l->set_halign(Gtk::ALIGN_END);
            grid->attach(*l, 0, row,   1, 1);
            grid->attach(w,  1, row++, 1, 1);
            w.set_hexpand(true);
        };

        entName.set_text(obj.name);
        addRow("Label:", entName);

        for (int i = 1; i <= numSystems; i++) {
            if (i != -1)  // all valid systems
                cmbTarget.append(std::to_string(i), "System " + std::to_string(i));
        }
        cmbTarget.set_active_id(std::to_string(obj.navTargetSystem));
        addRow("Target System:", cmbTarget);

        chkVisible.set_label("Visible at mission start");
        chkVisible.set_active(obj.navVisible);
        grid->attach(chkVisible, 0, row++, 2, 1);

        entVarName.set_text(obj.navVarName);
        entVarName.set_placeholder_text("optional: flag variable name");
        addRow("Variable:", entVarName);

        add_button("Cancel", Gtk::RESPONSE_CANCEL);
        add_button("OK",     Gtk::RESPONSE_OK);
        set_default_response(Gtk::RESPONSE_OK);
        show_all_children();
    }

    void apply() {
        o.name            = entName.get_text();
        o.navTargetSystem = std::stoi(cmbTarget.get_active_id().empty() ? "1" :
                                      cmbTarget.get_active_id());
        o.navVisible      = chkVisible.get_active();
        o.navVarName      = entVarName.get_text();
    }

private:
    MissionObject& o;
    Gtk::Entry       entName, entVarName;
    Gtk::ComboBoxText cmbTarget;
    Gtk::CheckButton  chkVisible;
};

// ── System Properties Dialog ──────────────────────────────────────────────────

class SystemDialog : public Gtk::Dialog {
public:
    SystemDialog(Gtk::Window& parent, MissionSystem& sys)
        : Gtk::Dialog("System Properties", parent, true)
        , s(sys)
    {
        set_default_size(360, 200);
        auto* area = get_content_area();
        auto* grid = Gtk::manage(new Gtk::Grid());
        grid->set_column_spacing(8); grid->set_row_spacing(6);
        grid->set_margin_start(12); grid->set_margin_end(12);
        grid->set_margin_top(8);   grid->set_margin_bottom(8);
        area->pack_start(*grid, true, true, 0);

        int row = 0;
        auto addRow = [&](const char* lbl, Gtk::Widget& w) {
            auto* l = Gtk::manage(new Gtk::Label(lbl));
            l->set_halign(Gtk::ALIGN_END);
            grid->attach(*l, 0, row,   1, 1);
            grid->attach(w,  1, row++, 1, 1);
            w.set_hexpand(true);
        };

        for (const auto& bg : BACKGROUNDS)
            cmbBg.append(bg);
        cmbBg.set_active_text(sys.background);
        addRow("Background:", cmbBg);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", sys.spawnX); entSX.set_text(buf);
        snprintf(buf, sizeof(buf), "%.0f", sys.spawnY); entSY.set_text(buf);
        snprintf(buf, sizeof(buf), "%.0f", sys.spawnZ); entSZ.set_text(buf);
        addRow("Spawn X:", entSX);
        addRow("Spawn Y:", entSY);
        addRow("Spawn Z:", entSZ);

        add_button("Cancel", Gtk::RESPONSE_CANCEL);
        add_button("OK",     Gtk::RESPONSE_OK);
        set_default_response(Gtk::RESPONSE_OK);
        show_all_children();
    }

    void apply() {
        s.background = cmbBg.get_active_text();
        try {
            s.spawnX = std::stod(entSX.get_text());
            s.spawnY = std::stod(entSY.get_text());
            s.spawnZ = std::stod(entSZ.get_text());
        } catch (...) {}
    }

private:
    MissionSystem& s;
    Gtk::ComboBoxText cmbBg;
    Gtk::Entry entSX, entSY, entSZ;
};
