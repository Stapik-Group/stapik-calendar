#include "AppStyleProvider.hpp"
#include <gtkmm/stylecontext.h>

void AppStyleProvider::load(const std::string& cssFilePath)
{
    const auto cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_path(cssFilePath);

    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        cssProvider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}