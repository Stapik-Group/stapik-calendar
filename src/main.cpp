#include <gtkmm.h>

#include "AppPaths.hpp"
#include "AppStyleProvider.hpp"
#include "MainWindow.hpp"

int main(const int argc, char *argv[])
{
    const auto app = Gtk::Application::create("pl.stapik.calendar");

    app->signal_activate().connect([&]
    {
        AppStyleProvider::load((AppPaths::resourcesDir() / "style.css").string());

        auto* window = new MainWindow();
        app->add_window(*window);
        window->show();
    });

    return app->run(argc, argv);
}
