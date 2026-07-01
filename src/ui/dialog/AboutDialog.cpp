#include "AboutDialog.hpp"

#include "../../core/locale/LocaleManager.hpp"

namespace
{
    constexpr auto APP_NAME    = "Stapik Calendar";
    constexpr auto APP_VERSION = "1.0.1";
    constexpr auto APP_AUTHOR  = "Sebastian Smoliński";
    constexpr auto APP_GITHUB  = "https://github.com/Stapik-Group/stapik-calendar";
}

AboutDialog::AboutDialog(Window& parent) :
    Dialog(LocaleManager::instance().translate("dialog.about.title"), parent, true),
    m_contentBox(Gtk::Orientation::VERTICAL, 8)
{
    initLayout();
}

void AboutDialog::initLayout()
{
    const auto& loc = LocaleManager::instance();

    m_titleLabel.set_markup(std::string("<b>") + APP_NAME + "</b>");
    m_titleLabel.set_halign(Gtk::Align::CENTER);

    m_versionLabel.set_text(loc.translate("dialog.about.version") + " " + APP_VERSION);
    m_versionLabel.set_halign(Gtk::Align::CENTER);

    m_authorLabel.set_text(loc.translate("dialog.about.author") + " " + APP_AUTHOR);
    m_authorLabel.set_halign(Gtk::Align::CENTER);

    m_githubLabel.set_markup(std::string("<a href=\"") + APP_GITHUB + "\">" + APP_GITHUB + "</a>");
    m_githubLabel.set_halign(Gtk::Align::CENTER);

    m_contentBox.set_margin(24);
    m_contentBox.append(m_titleLabel);
    m_contentBox.append(m_versionLabel);
    m_contentBox.append(m_separator);
    m_contentBox.append(m_authorLabel);
    m_contentBox.append(m_githubLabel);

    get_content_area()->append(m_contentBox);

    add_button(loc.translate("dialog.button.ok"), Gtk::ResponseType::OK);
    set_default_response(Gtk::ResponseType::OK);
    set_default_size(320, -1);
}