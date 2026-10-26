#include "CalendarGrid.hpp"

#include "../../core/util/ClipboardUrlDetector.hpp"
#include "../../core/util/UrlOpener.hpp"
#include "../dialog/CalendarEntryDialog.hpp"

#include <glibmm/main.h>
#include <gtkmm/window.h>


#include "../../core/util/DateUtils.hpp"

CalendarGrid::CalendarGrid(CalendarController& controller) :
    m_controller(controller)
{
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

    for (int row = 0; row < ROWS; ++row)
        for (int col = 0; col < COLUMNS; ++col)
        {
            const int index = row * COLUMNS + col;
            m_cells[index].setCellIndex(index);
            m_cells[index].setWeekend(col >= FIRST_WEEKEND_COLUMN);
            attach(m_cells[index], col, row);
        }
}

void CalendarGrid::connectCellSignals()
{
    for (int i = 0; i < TOTAL_CELLS; ++i)
    {
        m_cells[i].signalDoubleClicked().connect([this, i] { onCellDoubleClicked(i); });
        m_cells[i].signalEntryLinkClicked().connect([this, i](const EntryId& entryId) { onEntryLinkClicked(i, entryId); });
        m_cells[i].signalEditRequested().connect([this, i](const EntryId& entryId) { onEntryEditRequested(i, entryId); });
        m_cells[i].signalDeleteRequested().connect([this, i](const EntryId& entryId) { onEntryDeleteRequested(i, entryId); });
        m_cells[i].signalRightClicked().connect([this, i] { onCellRightClicked(i); });
        m_cells[i].signalColorChangeRequested().connect([this, i](const EntryId& entryId, const EntryColor color) { onEntryColorChangeRequested(i, entryId, color); });
        m_cells[i].signalEntryMoveRequested().connect([this, i](const int sourceCellIndex, const EntryId& entryId, const bool isCopy) { onEntryMoveRequested(sourceCellIndex, entryId, i, isCopy); });
    }
}

void CalendarGrid::displayMonth(const std::chrono::year_month yearMonth)
{
    m_currentYearMonth = yearMonth;
    populateCells();
}

void CalendarGrid::refreshIfDayChanged()
{
    if (DateUtils::today() != m_today)
        populateCells();
}

void CalendarGrid::populateCells()
{
    m_today = DateUtils::today();

    const int offset  = firstWeekdayOffset();
    const int numDays = daysInMonth();
    const int numDaysPreviousMonth = daysInPreviousMonth();

    for (int i = 0; i < TOTAL_CELLS; ++i)
    {
        const int day = i - offset + 1;

        if (day < 1)
        {
            m_cells[i].setAdjacentMonthDay(numDaysPreviousMonth + day);
        }
        else if (day > numDays)
        {
            m_cells[i].setAdjacentMonthDay(day - numDays);
        }
        else
        {
            m_cells[i].setDay(day);
            m_cells[i].markAsToday(isToday(day));

            if (const auto* entries = m_controller.entriesOn(cellDate(day)); entries != nullptr)
                m_cells[i].setEntries(*entries);
            else
                m_cells[i].setEntries({});
        }
    }
}

Gtk::Window* CalendarGrid::validatedWindowForCell(const int cellIndex, int& outDay)
{
    outDay = cellDay(cellIndex);
    if (outDay < 1 || outDay > daysInMonth())
        return nullptr;

    return dynamic_cast<Gtk::Window*>(get_root());
}

void CalendarGrid::onCellDoubleClicked(const int cellIndex)
{
    int day = 0;
    auto* window = validatedWindowForCell(cellIndex, day);
    if (window == nullptr)
        return;

    showEntryDialog(*window, cellDate(day), std::nullopt);
}

void CalendarGrid::onEntryLinkClicked(const int cellIndex, const EntryId& entryId)
{
    int day = 0;
    if (validatedWindowForCell(cellIndex, day) == nullptr)
        return;

    const auto* entry = m_controller.findEntry(cellDate(day), entryId);
    if (entry == nullptr || entry->link.empty())
        return;

    UrlOpener::open(entry->link);
}

void CalendarGrid::onEntryEditRequested(const int cellIndex, const EntryId& entryId)
{
    int day = 0;
    auto* window = validatedWindowForCell(cellIndex, day);
    if (window == nullptr)
        return;

    const auto date = cellDate(day);
    if (m_controller.findEntry(date, entryId) == nullptr)
        return;

    showEntryDialog(*window, date, entryId);
}

void CalendarGrid::onEntryDeleteRequested(const int cellIndex, const EntryId& entryId)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    // Changing the document rebuilds the cells and destroys the entry widget that raised this request.
    // Letting its signal handler return first avoids destroying a widget from inside its own signal.
    Glib::signal_idle().connect_once([this, date = cellDate(day), entryId]
    {
        m_controller.deleteEntry(date, entryId);
    });
}

void CalendarGrid::onEntryMoveRequested(const int sourceCellIndex, const EntryId& entryId, const int destCellIndex, const bool isCopy)
{
    const int sourceDay = cellDay(sourceCellIndex);
    const int destDay = cellDay(destCellIndex);
    if (sourceDay < 1 || sourceDay > daysInMonth() || destDay < 1 || destDay > daysInMonth())
        return;

    // The drag source widget is destroyed by the rebuild that follows, so the change waits until the drop is handled.
    Glib::signal_idle().connect_once(
        [this, sourceDate = cellDate(sourceDay), entryId, destDate = cellDate(destDay), isCopy]
        {
            if (isCopy)
                m_controller.copyEntry(sourceDate, entryId, destDate);
            else
                m_controller.moveEntry(sourceDate, entryId, destDate);
        });
}

void CalendarGrid::showEntryDialog(Gtk::Window& window,
                                   std::chrono::year_month_day date,
                                   const std::optional<EntryId>& editId)
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
                                      const std::optional<EntryId>& editId)
{
    auto* dialog = &entryDialog;

    dialog->signal_response().connect([this, dialog, date, editId](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (auto result = dialog->getResult(); result.has_value())
            {
                if (editId.has_value())
                    m_controller.saveEditedEntry(date, *editId, std::move(result.value()));
                else
                    m_controller.addEntry(date, std::move(result.value()));
            }
        }
        dialog->hide();
    });

    dialog->signal_hide().connect([dialog] { delete dialog; });
    dialog->show();
}

int CalendarGrid::cellDay(const int cellIndex) const
{
    return cellIndex - firstWeekdayOffset() + 1;
}

std::chrono::year_month_day CalendarGrid::cellDate(const int day) const
{
    return std::chrono::year_month_day{m_currentYearMonth / day};
}

int CalendarGrid::firstWeekdayOffset() const
{
    using namespace std::chrono;

    const year_month_day firstDay{m_currentYearMonth / 1};
    const weekday wd{sys_days{firstDay}};

    return static_cast<int>(wd.iso_encoding()) - 1;
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

bool CalendarGrid::isToday(const int day) const
{
    using namespace std::chrono;
    return m_today == year_month_day{m_currentYearMonth / day};
}

void CalendarGrid::onCellRightClicked(const int cellIndex)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    // A copied address opens the new entry window at once, with the link filled in.
    ClipboardUrlDetector::detectAsync(
        get_clipboard(),
        [this, date = cellDate(day)](const std::string& url) { showEntryDialogForLink(date, url); },
        [](const std::string&) { /* no URL in clipboard — do nothing */ }
    );
}

void CalendarGrid::onEntryColorChangeRequested(const int cellIndex, const EntryId& entryId, const EntryColor color)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    // Raised from inside the color popover of the entry widget that the rebuild destroys, so it must not run in place.
    Glib::signal_idle().connect_once([this, date = cellDate(day), entryId, color]
    {
        m_controller.changeEntryColor(date, entryId, color);
    });
}
