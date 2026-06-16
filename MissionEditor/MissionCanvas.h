#pragma once
#include <gtkmm.h>
#include <functional>
#include "MissionData.h"

class MissionCanvas : public Gtk::DrawingArea {
public:
    MissionCanvas();
    ~MissionCanvas() override = default;

    void setMission(MissionData* m)  { mission = m; }
    void setActiveSystem(int idx)    { activeSystemIdx = idx; queue_draw(); }

    // Called by MainWindow when a property changes externally
    void refreshObject(int idx)      { queue_draw(); }
    void refresh()                   { queue_draw(); }

    // Signals
    sigc::signal<void, int> signalObjectSelected;   // param: object index (-1 = none)
    sigc::signal<void, int> signalObjectMoved;      // param: object index
    sigc::signal<void, double, double> signalCanvasClick;  // world x,y of empty click

    int  selectedIndex() const { return selIdx; }

private:
    MissionData* mission       = nullptr;
    int          activeSystemIdx = 0;
    int          selIdx          = -1;

    // View state
    double viewX   = 0;   // world coords at canvas centre
    double viewY   = 0;
    double scale   = 0.05; // pixels per world unit

    // Drag state
    bool   draggingObj   = false;
    bool   panningCanvas = false;
    double dragStartMouseX = 0, dragStartMouseY = 0;
    double dragStartObjX   = 0, dragStartObjY   = 0;
    double panStartViewX   = 0, panStartViewY   = 0;

    // Coordinate conversion
    double worldToScreenX(double wx) const;
    double worldToScreenY(double wy) const;
    double screenToWorldX(double sx) const;
    double screenToWorldY(double sy) const;

    // Hit-testing: returns index into system objects, or -1
    int  hitTest(double sx, double sy) const;
    double iconRadius(ShipSize sz) const;

    // Drawing helpers
    void drawGrid(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h) const;
    void drawObject(const Cairo::RefPtr<Cairo::Context>& cr,
                    const MissionObject& obj, double sx, double sy, bool sel) const;
    void drawShipIcon(const Cairo::RefPtr<Cairo::Context>& cr,
                      double cx, double cy, ShipSize sz,
                      double r, double g, double b, bool sel) const;
    void drawNavIcon(const Cairo::RefPtr<Cairo::Context>& cr,
                     double cx, double cy, bool sel) const;
    void drawAsteroidIcon(const Cairo::RefPtr<Cairo::Context>& cr,
                          double cx, double cy, bool sel) const;
    void drawSpawnMarker(const Cairo::RefPtr<Cairo::Context>& cr,
                         double sx, double sy) const;

    // GTK signal handlers
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* e) override;
    bool on_button_release_event(GdkEventButton* e) override;
    bool on_motion_notify_event(GdkEventMotion* e) override;
    bool on_scroll_event(GdkEventScroll* e) override;
};
