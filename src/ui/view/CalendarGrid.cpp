#include "CalendarGrid.hpp"

#include "../../core/util/ClipboardUrlDetector.hpp"
#include "../../core/util/UrlOpener.hpp"
#include "../dialog/CalendarEntryDialog.hpp"

#include <glibmm/main.h>
#include <gtkmm/window.h>

#include "../../core/util/DateUtils.hpp"

CalendarGrid::CalendarGrid(CalendarController& controller, const WeekRules& weekRules) :
    m_controller(controller),
    m_weekRules(weekRules)
{
    m_anchorDate = DateUtils::today();
    m_currentYearMonth = DateUtils::todayYearMonth();

    m_documentConnection = m_controller.signalDocumentChanged().connect(sigc::mem_fun(*this, &CalendarGrid::populateCells));
    m_dayChangeConnection = Glib::signal_timeout().connect_seconds(
        [this]
        {
            refreshIfDayChanged();
            return true;
        },
        DAY_CHANGE_CHECK_SECONDS);

    initLayout();
    connectCellSignals();
    updateCellDates();
    populateCells();
}

CalendarGrid::~CalendarGrid()
{
    m_documentConnection.disconnect();
    m_dayChangeConnection.disconnect();
}

void CalendarGrid::initLayout()
{
    set_row_homogeneous(true);
    set_column_homogeneous(true);
    set_hexpand(true);
    set_vexpand(true);

    for (int index = 0; index < TOTAL_CELLS; ++index)
        cellAt(index).setCellIndex(index);

    layoutCells();
}

void CalendarGrid::layoutCells()
{
    for (auto& cell : m_cells)
        if (cell.get_parent() == this)
            remove(cell);

    switch (m_viewMode)
    {
        using enum ViewMode;
        case Month:
            for (int index = 0; index < TOTAL_CELLS; ++index)
                attach(cellAt(index), index % COLUMNS, index / COLUMNS);
            break;
        case Week:
            for (int column = 0; column < COLUMNS; ++column)
                attach(cellAt(column), column, 0);
            break;
        case Day:
            attach(m_cells[0], 0, 0, COLUMNS, 1);
        break;
    }
}

void CalendarGrid::connectCellSignals()
{
    for (int i = 0; i < TOTAL_CELLS; ++i)
    {
        cellAt(i).signalClicked().connect([this, i] { onCellClicked(i); });
        cellAt(i).signalDoubleClicked().connect([this, i] { onCellDoubleClicked(i); });
        cellAt(i).signalEntryLinkClicked().connect([this, i](const EntryId& entryId) { onEntryLinkClicked(i, entryId); });
        cellAt(i).signalEditRequested().connect([this, i](const EntryId& entryId) { onEntryEditRequested(i, entryId); });
        cellAt(i).signalDeleteRequested().connect([this, i](const EntryId& entryId) { onEntryDeleteRequested(i, entryId); });
        cellAt(i).signalRightClicked().connect([this, i] { onCellRightClicked(i); });
        cellAt(i).signalColorChangeRequested().connect([this, i](const EntryId& entryId, const EntryColor color) { onEntryColorChangeRequested(i, entryId, color); });
        cellAt(i).signalEntryMoveRequested().connect([this, i](const int sourceCellIndex, const EntryId& entryId, const bool isCopy) { onEntryMoveRequested(sourceCellIndex, entryId, i, isCopy); });
    }
}

void CalendarGrid::display(const ViewMode viewMode, const std::chrono::year_month_day anchorDate, const std::optional<std::chrono::year_month_day> markedDate)
{
    const bool modeChanged = viewMode != m_viewMode;

    m_viewMode = viewMode;
    m_anchorDate = anchorDate;
    m_currentYearMonth = std::chrono::year_month{anchorDate.year(), anchorDate.month()};
    m_markedDate = markedDate;

    if (modeChanged)
        layoutCells();

    updateCellDates();
    populateCells();
}

sigc::signal<void(std::chrono::year_month_day)>& CalendarGrid::signalDaySelected()
{
    return m_signalDaySelected;
}

void CalendarGrid::refreshIfDayChanged()
{
    if (DateUtils::today() != m_today)
        populateCells();
}

void CalendarGrid::updateCellDates()
{
    using namespace std::chrono;

    m_cellDates.fill(std::nullopt);

    switch (m_viewMode)
    {
        using enum ViewMode;
        case Month:
            for (int index = 0; index < TOTAL_CELLS; ++index)
            {
                if (const int day = index - firstWeekdayOffset() + 1; day >= 1 && day <= daysInMonth())
                    dateAt(index) = m_currentYearMonth / day;
            }
            break;
        case Week:
            for (int column = 0; column < COLUMNS; ++column)
                dateAt(column) = DateUtils::addDays(m_weekRules.startOfWeek(m_anchorDate), column);
            break;
        case Day:
            m_cellDates[0] = m_anchorDate;
            break;
    }
}

void CalendarGrid::populateCells()
{
    m_today = DateUtils::today();

    for (int index = 0; index < TOTAL_CELLS; ++index)
        populateCell(index);
}

void CalendarGrid::populateCell(const int cellIndex)
{
    auto& cell = cellAt(cellIndex);
    const auto& date = dateAt(cellIndex);

    cell.setWeekend(WeekRules::isWeekend(weekdayOfCell(cellIndex)));

    if (!date.has_value())
    {
        const int day = cellIndex - firstWeekdayOffset() + 1;
        cell.setAdjacentMonthDay(day < 1 ? daysInPreviousMonth() + day : day - daysInMonth());
        return;
    }

    cell.setDay(static_cast<int>(static_cast<unsigned>(date->day())));
    cell.markAsToday(*date == m_today);
    cell.markAsSelected(m_markedDate == date);

    if (const auto* entries = m_controller.entriesOn(*date); entries != nullptr)
        cell.setEntries(*entries);
    else
        cell.setEntries({});
}

Gtk::Window* CalendarGrid::windowForCell(const int cellIndex)
{
    if (!dateAt(cellIndex).has_value())
        return nullptr;

    return dynamic_cast<Gtk::Window*>(get_root());
}

void CalendarGrid::addEntryOn(const std::chrono::year_month_day date)
{
    if (auto* window = dynamic_cast<Gtk::Window*>(get_root()); window != nullptr)
        showEntryDialog(*window, date, std::nullopt);
}

void CalendarGrid::onCellClicked(const int cellIndex)
{
    if (const auto& date = dateAt(cellIndex); date.has_value())
        m_signalDaySelected.emit(*date);
}

void CalendarGrid::onCellDoubleClicked(const int cellIndex)
{
    if (auto* window = windowForCell(cellIndex); window != nullptr)
        showEntryDialog(*window, *dateAt(cellIndex), std::nullopt);
}

void CalendarGrid::onEntryLinkClicked(const int cellIndex, const EntryId& entryId)
{
    if (windowForCell(cellIndex) == nullptr)
        return;

    const auto* entry = m_controller.findEntry(*dateAt(cellIndex), entryId);
    if (entry == nullptr || entry->link.empty())
        return;

    UrlOpener::open(entry->link);
}

void CalendarGrid::onEntryEditRequested(const int cellIndex, const EntryId& entryId)
{
    auto* window = windowForCell(cellIndex);
    if (window == nullptr)
        return;

    const auto date = *dateAt(cellIndex);
    if (m_controller.findEntry(date, entryId) == nullptr)
        return;

    showEntryDialog(*window, date, entryId);
}

void CalendarGrid::onEntryDeleteRequested(const int cellIndex, const EntryId& entryId)
{
    const auto& date = dateAt(cellIndex);
    if (!date.has_value())
        return;

    // Changing the document rebuilds the cells and destroys the entry widget that raised this request.
    // Letting its signal handler return first avoids destroying a widget from inside its own signal.
    Glib::signal_idle().connect_once([this, date = *date, entryId]
    {
        m_controller.deleteEntry(date, entryId);
    });
}

void CalendarGrid::onEntryMoveRequested(const int sourceCellIndex, const EntryId& entryId, const int destCellIndex, const bool isCopy)
{
    const auto& sourceDate = dateAt(sourceCellIndex);
    const auto& destDate = dateAt(destCellIndex);
    if (!sourceDate.has_value() || !destDate.has_value())
        return;

    // The drag source widget is destroyed by the rebuild that follows, so the change waits until the drop is handled.
    Glib::signal_idle().connect_once(
        [this, sourceDate = *sourceDate, entryId, destDate = *destDate, isCopy]
        {
            if (isCopy)
                m_controller.copyEntry(sourceDate, entryId, destDate);
            else
                m_controller.moveEntry(sourceDate, entryId, destDate);
        });
}

void CalendarGrid::showEntryDialog(Gtk::Window& window,
                                   const std::chrono::year_month_day date,
                                   const std::optional<EntryId>& editId) const
{
    const auto* existing = editId.has_value() ? m_controller.findEntry(date, *editId) : nullptr;

    auto* dialog = existing != nullptr
        ? new CalendarEntryDialog(window, *existing)
        : new CalendarEntryDialog(window);

    presentEntryDialog(*dialog, date, editId);
}

void CalendarGrid::showEntryDialogForLink(const std::chrono::year_month_day date, const std::string& url)
{
    auto* window = dynamic_cast<Gtk::Window*>(get_root());
    if (window == nullptr)
        return;

    auto* dialog = new CalendarEntryDialog(*window);
    dialog->setLink(url);
    presentEntryDialog(*dialog, date, std::nullopt);
}

void CalendarGrid::presentEntryDialog(CalendarEntryDialog& entryDialog,
                                      const std::chrono::year_month_day date,
                                      const std::optional<EntryId>& editId) const
{
    auto* dialog = &entryDialog;

    dialog->signal_response().connect([this, dialog, date, editId](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            auto result = dialog->getResult();
            if (!result.has_value())
                return;

            if (editId.has_value())
                m_controller.saveEditedEntry(date, *editId, std::move(result.value()));
            else
                m_controller.addEntry(date, std::move(result.value()));
        }
        dialog->hide();
    });

    dialog->signal_hide().connect([dialog] { delete dialog; });
    dialog->show();
}

int CalendarGrid::firstWeekdayOffset() const
{
    using namespace std::chrono;

    const year_month_day firstDay{m_currentYearMonth / 1};

    return m_weekRules.columnOf(weekday{sys_days{firstDay}});
}

int CalendarGrid::daysInMonth() const
{
    using namespace std::chrono;

    const year_month_day lastDay{m_currentYearMonth / last};
    return static_cast<int>(static_cast<unsigned>(lastDay.day()));
}

int CalendarGrid::daysInPreviousMonth() const
{
    using namespace std::chrono;

    const year_month previousMonth = m_currentYearMonth - months{1};
    const year_month_day lastDay{previousMonth / last};
    return static_cast<int>(static_cast<unsigned>(lastDay.day()));
}

std::chrono::weekday CalendarGrid::weekdayOfCell(const int cellIndex) const
{
    using namespace std::chrono;

    if (m_viewMode == ViewMode::Day)
        return weekday{sys_days{m_anchorDate}};

    return m_weekRules.weekdayInColumn(cellIndex % COLUMNS);
}

void CalendarGrid::onCellRightClicked(const int cellIndex)
{
    const auto& date = dateAt(cellIndex);
    if (!date.has_value())
        return;

    // A copied address opens the new entry window at once, with the link filled in.
    ClipboardUrlDetector::detectAsync(
        get_clipboard(),
        [this, date = *date](const std::string& url) { showEntryDialogForLink(date, url); },
        [](const std::string&) { /* no URL in clipboard — do nothing */ }
    );
}

void CalendarGrid::onEntryColorChangeRequested(const int cellIndex, const EntryId& entryId, const EntryColor color)
{
    const auto& date = dateAt(cellIndex);
    if (!date.has_value())
        return;

    // Raised from inside the color popover of the entry widget that the rebuild destroys, so it must not run in place.
    Glib::signal_idle().connect_once([this, date = *date, entryId, color]
    {
        m_controller.changeEntryColor(date, entryId, color);
    });
}

CalendarCell& CalendarGrid::cellAt(const int index)
{
    return m_cells[static_cast<std::size_t>(index)];
}

CalendarGrid::CellDate& CalendarGrid::dateAt(const int index)
{
    return m_cellDates[static_cast<std::size_t>(index)];
}

const CalendarGrid::CellDate& CalendarGrid::dateAt(const int index) const
{
    return m_cellDates[static_cast<std::size_t>(index)];
}