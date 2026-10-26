#include "CalendarEntryDialog.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include "../../core/util/EntryTime.hpp"

#include <string>

namespace
{
    bool isBlank(const Glib::ustring& text)
    {
        return text.raw().find_first_not_of(" \t") == std::string::npos;
    }

    void markInvalid(Gtk::Entry& entry, const bool invalid)
    {
        if (invalid)
            entry.add_css_class("error");
        else
            entry.remove_css_class("error");
    }
}

CalendarEntryDialog::CalendarEntryDialog(Window& parent) :
    Dialog(LocaleManager::instance().translate("dialog.entry.new.title"), parent, true),
    m_contentBox(Gtk::Orientation::VERTICAL, CONTENT_SPACING)
{
    initLayout();
}

CalendarEntryDialog::CalendarEntryDialog(Window& parent, const CalendarEntry& existing) :
    Dialog(LocaleManager::instance().translate("dialog.entry.edit.title"), parent, true),
    m_contentBox(Gtk::Orientation::VERTICAL, CONTENT_SPACING)
{
    initLayout();
    m_nameEntry.set_text(existing.name);
    m_linkEntry.set_text(existing.link);
    m_timeEntry.set_text(existing.time);
    m_locationEntry.set_text(existing.location);
    m_colorPicker.setSelectedColor(existing.color);
}

void CalendarEntryDialog::initLayout()
{
    const auto& loc = LocaleManager::instance();

    m_nameLabel.set_text(loc.translate("dialog.entry.name.label"));
    m_nameLabel.set_halign(Gtk::Align::START);
    m_nameEntry.set_placeholder_text(loc.translate("dialog.entry.name.placeholder"));
    m_nameEntry.signal_changed().connect(sigc::mem_fun(*this, &CalendarEntryDialog::onNameChanged));

    m_linkLabel.set_text(loc.translate("dialog.entry.link.label"));
    m_linkLabel.set_halign(Gtk::Align::START);
    m_linkEntry.set_placeholder_text(loc.translate("dialog.entry.link.placeholder"));

    m_timeLabel.set_text(loc.translate("dialog.entry.time.label"));
    m_timeLabel.set_halign(Gtk::Align::START);
    m_timeEntry.set_placeholder_text(loc.translate("dialog.entry.time.placeholder"));
    m_timeEntry.set_max_length(TIME_MAX_LENGTH);
    m_timeEntry.signal_changed().connect(sigc::mem_fun(*this, &CalendarEntryDialog::refreshValidation));

    m_locationLabel.set_text(loc.translate("dialog.entry.location.label"));
    m_locationLabel.set_halign(Gtk::Align::START);
    m_locationEntry.set_placeholder_text(loc.translate("dialog.entry.location.placeholder"));

    m_colorLabel.set_text(loc.translate("dialog.entry.color.label"));
    m_colorLabel.set_halign(Gtk::Align::START);

    m_contentBox.set_margin(CONTENT_MARGIN);
    m_contentBox.append(m_nameLabel);
    m_contentBox.append(m_nameEntry);
    m_contentBox.append(m_linkLabel);
    m_contentBox.append(m_linkEntry);
    m_contentBox.append(m_timeLabel);
    m_contentBox.append(m_timeEntry);
    m_contentBox.append(m_locationLabel);
    m_contentBox.append(m_locationEntry);
    m_contentBox.append(m_colorLabel);
    m_contentBox.append(m_colorPicker);

    get_content_area()->append(m_contentBox);

    auto* cancelButton = add_button(loc.translate("dialog.button.cancel"), Gtk::ResponseType::CANCEL);
    auto* okButton = add_button(loc.translate("dialog.button.ok"), Gtk::ResponseType::OK);

    m_buttonSizeGroup = Gtk::SizeGroup::create(Gtk::SizeGroup::Mode::HORIZONTAL);
    m_buttonSizeGroup->add_widget(*cancelButton);
    m_buttonSizeGroup->add_widget(*okButton);

    set_default_response(Gtk::ResponseType::OK);
    m_nameEntry.set_activates_default(true);
    m_linkEntry.set_activates_default(true);
    m_timeEntry.set_activates_default(true);
    m_locationEntry.set_activates_default(true);
    set_default_size(DEFAULT_WIDTH, -1);

    refreshValidation();
}

void CalendarEntryDialog::onNameChanged()
{
    m_nameEdited = true;
    refreshValidation();
}

void CalendarEntryDialog::refreshValidation()
{
    const bool hasName = !isBlank(m_nameEntry.get_text());
    const bool hasValidTime = entry_time::normalize(m_timeEntry.get_text().raw()).has_value();
    const bool nameRejected = !hasName && m_nameEdited;

    markInvalid(m_nameEntry, nameRejected);
    m_nameEntry.set_tooltip_text(nameRejected ? LocaleManager::instance().translate("dialog.entry.name.required") : std::string{});
    markInvalid(m_timeEntry, !hasValidTime);

    set_response_sensitive(Gtk::ResponseType::OK, hasName && hasValidTime);
}

void CalendarEntryDialog::setLink(const std::string& link)
{
    m_linkEntry.set_text(link);
}

std::optional<CalendarEntry> CalendarEntryDialog::getResult() const
{
    const auto name = m_nameEntry.get_text();

    if (isBlank(name))
        return std::nullopt;

    const auto time = entry_time::normalize(m_timeEntry.get_text().raw());
    if (!time.has_value())
        return std::nullopt;

    return CalendarEntry{ name, m_linkEntry.get_text(), m_colorPicker.getSelectedColor(), m_locationEntry.get_text(), *time, EntryId{} };
}