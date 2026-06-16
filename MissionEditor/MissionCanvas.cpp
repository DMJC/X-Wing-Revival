#include "MissionCanvas.h"
#include <cmath>
#include <algorithm>

MissionCanvas::MissionCanvas() {
    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
               Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK |
               Gdk::BUTTON_MOTION_MASK);
    set_can_focus(true);
}

// ── coordinate conversion ─────────────────────────────────────────────────────

double MissionCanvas::worldToScreenX(double wx) const {
    return (wx - viewX) * scale + get_width()  * 0.5;
}
double MissionCanvas::worldToScreenY(double wy) const {
    return -(wy - viewY) * scale + get_height() * 0.5;
}
double MissionCanvas::screenToWorldX(double sx) const {
    return (sx - get_width()  * 0.5) / scale + viewX;
}
double MissionCanvas::screenToWorldY(double sy) const {
    return -((sy - get_height() * 0.5) / scale) + viewY;
}

// ── hit testing ──────────────────────────────────────────────────────────────

double MissionCanvas::iconRadius(ShipSize sz) const {
    switch (sz) {
        case ShipSize::Fighter: return 10;
        case ShipSize::Medium:  return 18;
        case ShipSize::Capital: return 34;
    }
    return 10;
}

int MissionCanvas::hitTest(double sx, double sy) const {
    if (!mission || activeSystemIdx < 0 || activeSystemIdx >= (int)mission->systems.size())
        return -1;
    const auto& objs = mission->systems[activeSystemIdx].objects;
    // Iterate in reverse so topmost-drawn (last) wins
    for (int i = (int)objs.size() - 1; i >= 0; i--) {
        const auto& obj = objs[i];
        double ox = worldToScreenX(obj.x);
        double oy = worldToScreenY(obj.y);
        double r  = (obj.objType == ObjectType::NavPoint)    ? 12 :
                    (obj.objType == ObjectType::AsteroidField)? 20 :
                    iconRadius(obj.size);
        double dx = sx - ox, dy = sy - oy;
        if (dx*dx + dy*dy <= r*r) return i;
    }
    return -1;
}

// ── drawing ───────────────────────────────────────────────────────────────────

bool MissionCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    int W = get_width(), H = get_height();

    // Background
    cr->set_source_rgb(0.08, 0.08, 0.12);
    cr->paint();

    drawGrid(cr, W, H);

    if (!mission || activeSystemIdx < 0 || activeSystemIdx >= (int)mission->systems.size())
        return true;

    const auto& sys = mission->systems[activeSystemIdx];

    // Player spawn marker
    drawSpawnMarker(cr, worldToScreenX(sys.spawnX), worldToScreenY(sys.spawnY));

    // Objects
    for (int i = 0; i < (int)sys.objects.size(); i++) {
        const auto& obj = sys.objects[i];
        double sx = worldToScreenX(obj.x);
        double sy = worldToScreenY(obj.y);
        drawObject(cr, obj, sx, sy, obj.selected);
    }

    // Origin cross
    double ox = worldToScreenX(0), oy = worldToScreenY(0);
    cr->set_source_rgba(0.5, 0.5, 0.5, 0.4);
    cr->set_line_width(1);
    cr->move_to(ox - 6, oy); cr->line_to(ox + 6, oy); cr->stroke();
    cr->move_to(ox, oy - 6); cr->line_to(ox, oy + 6); cr->stroke();

    // Coordinates label at bottom-left
    cr->set_source_rgba(0.7, 0.7, 0.7, 0.6);
    cr->set_font_size(11);
    cr->move_to(8, H - 8);
    char buf[64];
    snprintf(buf, sizeof(buf), "Scale: %.3f  View: (%.0f, %.0f)", scale, viewX, viewY);
    cr->show_text(buf);

    return true;
}

void MissionCanvas::drawGrid(const Cairo::RefPtr<Cairo::Context>& cr, int W, int H) const {
    // Choose grid spacing so we get ~8-15 lines on screen
    double worldW = W / scale;
    double rawStep = worldW / 10.0;
    double magnitude = std::pow(10.0, std::floor(std::log10(rawStep)));
    double step = magnitude;
    if (rawStep / magnitude > 5) step = magnitude * 5;
    else if (rawStep / magnitude > 2) step = magnitude * 2;

    double left   = screenToWorldX(0);
    double right  = screenToWorldX(W);
    double bottom = screenToWorldY(H);
    double top    = screenToWorldY(0);

    double startX = std::floor(left  / step) * step;
    double startY = std::floor(bottom/ step) * step;

    cr->set_line_width(0.5);

    for (double wx = startX; wx <= right + step; wx += step) {
        double sx = worldToScreenX(wx);
        bool isOrigin = std::fabs(wx) < step * 0.01;
        cr->set_source_rgba(isOrigin ? 0.5 : 0.25,
                            isOrigin ? 0.5 : 0.25,
                            isOrigin ? 0.7 : 0.35,
                            isOrigin ? 0.8 : 0.4);
        cr->move_to(sx, 0); cr->line_to(sx, H); cr->stroke();
    }
    for (double wy = startY; wy <= top + step; wy += step) {
        double sy = worldToScreenY(wy);
        bool isOrigin = std::fabs(wy) < step * 0.01;
        cr->set_source_rgba(isOrigin ? 0.5 : 0.25,
                            isOrigin ? 0.5 : 0.25,
                            isOrigin ? 0.7 : 0.35,
                            isOrigin ? 0.8 : 0.4);
        cr->move_to(0, sy); cr->line_to(W, sy); cr->stroke();
    }

    // Grid scale label
    cr->set_source_rgba(0.6, 0.6, 0.6, 0.5);
    cr->set_font_size(10);
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f u", step);
    cr->move_to(6, 14);
    cr->show_text(buf);
}

void MissionCanvas::drawSpawnMarker(const Cairo::RefPtr<Cairo::Context>& cr,
                                     double sx, double sy) const {
    cr->set_source_rgba(0.2, 0.9, 0.5, 0.5);
    cr->set_line_width(1.5);
    double r = 14;
    cr->arc(sx, sy, r, 0, 2*M_PI);
    cr->stroke();
    cr->move_to(sx - r, sy); cr->line_to(sx + r, sy); cr->stroke();
    cr->move_to(sx, sy - r); cr->line_to(sx, sy + r); cr->stroke();

    cr->set_font_size(9);
    cr->set_source_rgba(0.2, 0.9, 0.5, 0.6);
    cr->move_to(sx + r + 2, sy + 4);
    cr->show_text("SPAWN");
}

void MissionCanvas::drawObject(const Cairo::RefPtr<Cairo::Context>& cr,
                                const MissionObject& obj,
                                double sx, double sy, bool sel) const {
    std::string pt = mission ? mission->playerTeam : "rebel";
    if (obj.objType == ObjectType::NavPoint) {
        drawNavIcon(cr, sx, sy, sel);
        // Label
        cr->set_source_rgba(0.7, 0.9, 1.0, 0.9);
        cr->set_font_size(10);
        cr->move_to(sx + 14, sy - 4);
        cr->show_text(obj.name.empty() ? "NavPoint" : obj.name.c_str());
        cr->set_source_rgba(0.5, 0.7, 0.9, 0.7);
        cr->set_font_size(9);
        cr->move_to(sx + 14, sy + 8);
        char buf[32];
        snprintf(buf, sizeof(buf), "→ Sys %d", obj.navTargetSystem);
        cr->show_text(buf);
    } else if (obj.objType == ObjectType::AsteroidField) {
        drawAsteroidIcon(cr, sx, sy, sel);
        cr->set_source_rgba(0.7, 0.7, 0.7, 0.8);
        cr->set_font_size(10);
        cr->move_to(sx + 22, sy + 4);
        char buf[32];
        snprintf(buf, sizeof(buf), "Asteroids (%d)", obj.asteroidCount);
        cr->show_text(buf);
    } else {
        int role = colorRole(obj.objType, obj.faction, pt);
        double r = (role == 0) ? 0.2 : (role == 1) ? 0.9 : 0.6;
        double g = (role == 0) ? 0.8 : (role == 1) ? 0.2 : 0.6;
        double b = (role == 0) ? 0.3 : (role == 1) ? 0.2 : 0.6;
        drawShipIcon(cr, sx, sy, obj.size, r, g, b, sel);
        // Name label
        if (!obj.name.empty()) {
            cr->set_source_rgba(r, g, b, 0.9);
            cr->set_font_size(10);
            double labelOffset = iconRadius(obj.size) + 4;
            cr->move_to(sx + labelOffset, sy - 3);
            cr->show_text(obj.name.c_str());
        }
        // Ship class in smaller text
        cr->set_source_rgba(r * 0.8, g * 0.8, b * 0.8, 0.7);
        cr->set_font_size(9);
        double lo = iconRadius(obj.size) + 4;
        cr->move_to(sx + lo, sy + 9);
        cr->show_text(obj.shipClass.c_str());
    }
}

void MissionCanvas::drawShipIcon(const Cairo::RefPtr<Cairo::Context>& cr,
                                  double cx, double cy, ShipSize sz,
                                  double r, double g, double b, bool sel) const {
    double radius = iconRadius(sz);
    int sides = (sz == ShipSize::Fighter) ? 4 : (sz == ShipSize::Medium) ? 6 : 8;

    // Fill
    cr->begin_new_path();
    for (int i = 0; i < sides; i++) {
        double angle = 2.0 * M_PI * i / sides - M_PI / 2.0;
        double px = cx + radius * std::cos(angle);
        double py = cy + radius * std::sin(angle);
        if (i == 0) cr->move_to(px, py);
        else        cr->line_to(px, py);
    }
    cr->close_path();
    cr->set_source_rgba(r * 0.25, g * 0.25, b * 0.25, 0.7);
    cr->fill_preserve();
    cr->set_source_rgba(r, g, b, sel ? 1.0 : 0.85);
    cr->set_line_width(sel ? 2.0 : 1.2);
    cr->stroke();

    // Selection glow
    if (sel) {
        cr->arc(cx, cy, radius + 4, 0, 2*M_PI);
        cr->set_source_rgba(1, 1, 0, 0.4);
        cr->set_line_width(1.5);
        cr->stroke();
    }
}

void MissionCanvas::drawNavIcon(const Cairo::RefPtr<Cairo::Context>& cr,
                                 double cx, double cy, bool sel) const {
    double r = 10;
    cr->arc(cx, cy, r, 0, 2*M_PI);
    cr->set_source_rgba(0.1, 0.1, 0.15, 0.7);
    cr->fill_preserve();
    cr->set_source_rgba(0.4, 0.8, 1.0, sel ? 1.0 : 0.8);
    cr->set_line_width(sel ? 2.0 : 1.2);
    cr->stroke();

    // Crosshair inside
    cr->set_line_width(1.0);
    cr->move_to(cx - r, cy); cr->line_to(cx + r, cy); cr->stroke();
    cr->move_to(cx, cy - r); cr->line_to(cx, cy + r); cr->stroke();

    if (sel) {
        cr->arc(cx, cy, r + 4, 0, 2*M_PI);
        cr->set_source_rgba(1, 1, 0, 0.4);
        cr->set_line_width(1.5);
        cr->stroke();
    }
}

void MissionCanvas::drawAsteroidIcon(const Cairo::RefPtr<Cairo::Context>& cr,
                                      double cx, double cy, bool sel) const {
    // Irregular polygon
    static const double pts[][2] = {
        {0,-18},{10,-12},{18,-5},{14,8},{6,18},{-8,16},{-18,5},{-15,-10},{-6,-18}
    };
    cr->begin_new_path();
    for (int i = 0; i < 9; i++) {
        if (i==0) cr->move_to(cx+pts[i][0], cy+pts[i][1]);
        else      cr->line_to(cx+pts[i][0], cy+pts[i][1]);
    }
    cr->close_path();
    cr->set_source_rgba(0.3, 0.25, 0.2, 0.6);
    cr->fill_preserve();
    cr->set_source_rgba(0.65, 0.60, 0.55, sel ? 1.0 : 0.8);
    cr->set_line_width(sel ? 2.0 : 1.2);
    cr->stroke();

    if (sel) {
        cr->arc(cx, cy, 22, 0, 2*M_PI);
        cr->set_source_rgba(1, 1, 0, 0.4);
        cr->set_line_width(1.5);
        cr->stroke();
    }
}

// ── event handlers ────────────────────────────────────────────────────────────

bool MissionCanvas::on_button_press_event(GdkEventButton* e) {
    grab_focus();
    if (!mission) return true;

    if (e->button == 1) {
        int hit = hitTest(e->x, e->y);
        if (hit >= 0 && activeSystemIdx < (int)mission->systems.size()) {
            // Deselect old
            for (auto& o : mission->systems[activeSystemIdx].objects)
                o.selected = false;
            selIdx = hit;
            mission->systems[activeSystemIdx].objects[hit].selected = true;
            auto& obj = mission->systems[activeSystemIdx].objects[hit];
            draggingObj   = true;
            dragStartMouseX = e->x;
            dragStartMouseY = e->y;
            dragStartObjX   = obj.x;
            dragStartObjY   = obj.y;
            signalObjectSelected.emit(hit);
        } else {
            // Deselect all, start pan
            if (activeSystemIdx < (int)mission->systems.size())
                for (auto& o : mission->systems[activeSystemIdx].objects)
                    o.selected = false;
            selIdx = -1;
            panningCanvas  = true;
            dragStartMouseX= e->x;
            dragStartMouseY= e->y;
            panStartViewX  = viewX;
            panStartViewY  = viewY;
            signalObjectSelected.emit(-1);
        }
        queue_draw();
    }
    return true;
}

bool MissionCanvas::on_button_release_event(GdkEventButton* e) {
    if (e->button == 1) {
        if (draggingObj) signalObjectMoved.emit(selIdx);
        draggingObj   = false;
        panningCanvas = false;
    }
    return true;
}

bool MissionCanvas::on_motion_notify_event(GdkEventMotion* e) {
    if (!mission) return true;

    if (draggingObj && selIdx >= 0 && activeSystemIdx < (int)mission->systems.size()) {
        double dx = (e->x - dragStartMouseX) / scale;
        double dy = -(e->y - dragStartMouseY) / scale;
        auto& obj = mission->systems[activeSystemIdx].objects[selIdx];
        obj.x = dragStartObjX + dx;
        obj.y = dragStartObjY + dy;
        signalObjectMoved.emit(selIdx);
        queue_draw();
    } else if (panningCanvas) {
        double dx = (e->x - dragStartMouseX) / scale;
        double dy = -(e->y - dragStartMouseY) / scale;
        viewX = panStartViewX - dx;
        viewY = panStartViewY - dy;
        queue_draw();
    }
    return true;
}

bool MissionCanvas::on_scroll_event(GdkEventScroll* e) {
    // Zoom toward mouse position
    double preWX = screenToWorldX(e->x);
    double preWY = screenToWorldY(e->y);

    if (e->direction == GDK_SCROLL_UP || e->direction == GDK_SCROLL_SMOOTH) {
        double dy = 0;
        if (e->direction == GDK_SCROLL_SMOOTH)
            dy = -e->delta_y;
        else
            dy = 1.0;
        scale *= std::pow(1.1, dy);
    } else if (e->direction == GDK_SCROLL_DOWN) {
        scale /= 1.1;
    }
    scale = std::max(0.001, std::min(scale, 5.0));

    // Adjust view so the mouse position stays fixed
    double postSX = worldToScreenX(preWX);
    double postSY = worldToScreenY(preWY);
    viewX += (postSX - e->x) / scale;
    viewY -= (postSY - e->y) / scale;

    queue_draw();
    return true;
}
