#include <gtkmm.h>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    // APPLICATION_NON_UNIQUE: allow multiple instances and avoid the
    // single-instance redirect that silently swallows subsequent launches.
    auto app = Gtk::Application::create(argc, argv,
                                        "org.xwingrevival.missioneditor",
                                        Gio::APPLICATION_NON_UNIQUE);
    MainWindow win;
    return app->run(win);
}
