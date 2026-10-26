#include "CalendarHeader.hpp"
#include "stapik/locale/LocaleManager.hpp"

CalendarHeader::CalendarHeader(const WeekRules& weekRules) :
    m_weekRules(weekRules)
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

    for (std::size_t i = 0; i < COLUMNS; i++)
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
    }

    layoutCells();
}

void CalendarHeader::layoutCells()
{
    for (auto& cell : m_cells)
        if (cell.get_parent() == this)
            remove(cell);

    if (m_viewMode == ViewMode::Day)
    {
        attach(m_cells[0], 0, 0, COLUMNS, 1);
        return;
    }

    for (std::size_t i = 0; i < COLUMNS; i++)
        attach(m_cells[i], static_cast<int>(i), 0);
}

void CalendarHeader::display(const ViewMode viewMode, const std::chrono::year_month_day selectedDate)
{
    const bool modeChanged = viewMode != m_viewMode;

    m_viewMode = viewMode;
    m_selectedDate = selectedDate;

    if (modeChanged)
        layoutCells();

    refreshLabels();
}

void CalendarHeader::refreshLabels()
{
    const auto& loc = LocaleManager::instance();

    if (m_viewMode == ViewMode::Day)
    {
        const std::chrono::weekday weekday{std::chrono::sys_days{m_selectedDate}};
        m_labels[0].set_text(loc.translate(DAY_KEYS[weekday.c_encoding()]));
        return;
    }

    for (std::size_t i = 0; i < COLUMNS; i++)
        m_labels[i].set_text(loc.translate(DAY_KEYS[m_weekRules.weekdayInColumn(static_cast<int>(i)).c_encoding()]));
}