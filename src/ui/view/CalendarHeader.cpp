#include "CalendarHeader.hpp"

#include "stapik/locale/LocaleManager.hpp"

CalendarHeader::CalendarHeader()
{
    initLayout();
    refreshLabels();

    LocaleManager::instance().signalLocaleChanged().connect(
        [this] { refreshLabels(); });
}

void CalendarHeader::initLayout()
{
    set_column_homogeneous(true);
    set_hexpand(true);

    for (int i = 0; i < COLUMNS; i++)
    {
        using enum Gtk::Align;

        m_labels[i].set_halign(CENTER);
        m_labels[i].set_valign(CENTER);
        m_labels[i].set_hexpand(true);
        m_labels[i].set_margin(LABEL_MARGIN);

        m_cells[i].set_halign(FILL);
        m_cells[i].set_valign(FILL);
        m_cells[i].set_hexpand(true);
        m_cells[i].add_css_class("calendar-header-cell");
        m_cells[i].append(m_labels[i]);

        attach(m_cells[i], i, 0);
    }
}

void CalendarHeader::refreshLabels()
{
    const auto& loc = LocaleManager::instance();

    for (int i = 0; i < COLUMNS; i++)
        m_labels[i].set_text(loc.translate(DAY_KEYS[i]));
}