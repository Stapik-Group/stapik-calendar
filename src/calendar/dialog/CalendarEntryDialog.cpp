#include "CalendarEntryDialog.hpp"

CalendarEntryDialog::CalendarEntryDialog(Window& parent)
    : Dialog(LocaleManager::instance().translate("dialog.entry.new.title"), parent, true)
    , m_contentBox(Gtk::Orientation::VERTICAL, 8)
{
    initLayout();
}

CalendarEntryDialog::CalendarEntryDialog(Gtk::Window& parent, const CalendarEntry& existing)
    : Dialog(LocaleManager::instance().translate("dialog.entry.edit.title"), parent, true)
    , m_contentBox(Gtk::Orientation::VERTICAL, 8)
{
    initLayout();
    m_nameEntry.set_text(existing.name);
    m_linkEntry.set_text(existing.link);
}

void CalendarEntryDialog::initLayout()
{
    auto& loc = LocaleManager::instance();

    m_nameLabel.set_text(loc.translate("dialog.entry.name.label"));
    m_nameLabel.set_halign(Gtk::Align::START);
    m_nameEntry.set_placeholder_text(loc.translate("dialog.entry.name.placeholder"));

    m_linkLabel.set_text(loc.translate("dialog.entry.link.label"));
    m_linkLabel.set_halign(Gtk::Align::START);
    m_linkEntry.set_placeholder_text(loc.translate("dialog.entry.link.placeholder"));

    m_contentBox.set_margin(16);
    m_contentBox.append(m_nameLabel);
    m_contentBox.append(m_nameEntry);
    m_contentBox.append(m_linkLabel);
    m_contentBox.append(m_linkEntry);

    get_content_area()->append(m_contentBox);

    add_button(loc.translate("dialog.button.cancel"), Gtk::ResponseType::CANCEL);
    add_button(loc.translate("dialog.button.ok"),      Gtk::ResponseType::OK);

    set_default_response(Gtk::ResponseType::OK);
    m_nameEntry.set_activates_default(true);
    m_linkEntry.set_activates_default(true);
    set_default_size(400, -1);
}

std::optional<CalendarEntry> CalendarEntryDialog::getResult() const
{
    const auto name = m_nameEntry.get_text();

    if (name.empty())
        return std::nullopt;

    return CalendarEntry{ name, m_linkEntry.get_text() };
}