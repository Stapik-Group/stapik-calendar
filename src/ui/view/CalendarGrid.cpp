#include "CalendarGrid.hpp"

#include "../../core/util/UrlTitleFetcher.hpp"
#include "../../core/util/ClipboardUrlDetector.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "../dialog/CalendarEntryDialog.hpp"

#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <nlohmann/json.hpp>

#include "../../core/util/DateUtils.hpp"

CalendarGrid::CalendarGrid(CalendarController& controller) :
    m_controller(controller)
{
    m_currentYearMonth = DateUtils::todayYearMonth();

    m_documentConnection = m_controller.signalDocumentChanged().connect(sigc::mem_fun(*this, &CalendarGrid::populateCells));

    initLayout();
    connectCellSignals();
    populateCells();
}

CalendarGrid::~CalendarGrid()
{
    m_documentConnection.disconnect();
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
            attach(m_cells[index], col, row);
        }
}

void CalendarGrid::connectCellSignals()
{
    for (int i = 0; i < TOTAL_CELLS; ++i)
    {
        m_cells[i].signalDoubleClicked().connect([this, i] { onCellDoubleClicked(i); });
        m_cells[i].signalEditRequested().connect([this, i](const int entryIndex) { onEntryEditRequested(i, entryIndex); });
        m_cells[i].signalDeleteRequested().connect([this, i](const int entryIndex) { onEntryDeleteRequested(i, entryIndex); });
        m_cells[i].signalRightClicked().connect([this, i] { onCellRightClicked(i); });
        m_cells[i].signalColorChangeRequested().connect([this, i](const int entryIndex, const EntryColor color) { onEntryColorChangeRequested(i, entryIndex, color); });
        m_cells[i].signalEntryMoveRequested().connect([this, i](const int sourceCellIndex, const int sourceEntryIndex, const bool isCopy) { onEntryMoveRequested(sourceCellIndex, sourceEntryIndex, i, isCopy); });
    }
}

void CalendarGrid::displayMonth(const std::chrono::year_month yearMonth)
{
    m_currentYearMonth = yearMonth;
    populateCells();
}

void CalendarGrid::populateCells()
{
    const int offset  = firstWeekdayOffset();
    const int numDays = daysInMonth();

    for (int i = 0; i < TOTAL_CELLS; ++i)
    {
        if (const int day = i - offset + 1; day < 1 || day > numDays)
        {
            m_cells[i].clearDay();
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

bool CalendarGrid::isValidEntryIndex(const std::chrono::year_month_day date, const int entryIndex) const
{
    return entryIndex >= 0 && m_controller.findEntry(date, static_cast<std::size_t>(entryIndex)) != nullptr;
}

void CalendarGrid::onCellDoubleClicked(const int cellIndex)
{
    int day = 0;
    auto* window = validatedWindowForCell(cellIndex, day);
    if (window == nullptr)
        return;

    showEntryDialog(*window, cellDate(day), std::nullopt);
}

void CalendarGrid::onEntryEditRequested(const int cellIndex, const int entryIndex)
{
    int day = 0;
    auto* window = validatedWindowForCell(cellIndex, day);
    if (window == nullptr)
        return;

    const auto date = cellDate(day);
    if (!isValidEntryIndex(date, entryIndex))
        return;

    showEntryDialog(*window, date, entryIndex);
}

void CalendarGrid::onEntryDeleteRequested(const int cellIndex, const int entryIndex)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    if (entryIndex < 0)
        return;

    m_controller.deleteEntry(cellDate(day), static_cast<std::size_t>(entryIndex));
}

void CalendarGrid::onEntryMoveRequested(const int sourceCellIndex, const int sourceEntryIndex, const int destCellIndex, const bool isCopy)
{
    const int sourceDay = cellDay(sourceCellIndex);
    const int destDay = cellDay(destCellIndex);
    if (sourceDay < 1 || sourceDay > daysInMonth() || destDay < 1 || destDay > daysInMonth())
        return;

    if (sourceEntryIndex < 0)
        return;

    const auto index = static_cast<std::size_t>(sourceEntryIndex);
    if (isCopy)
        m_controller.copyEntry(cellDate(sourceDay), index, cellDate(destDay));
    else
        m_controller.moveEntry(cellDate(sourceDay), index, cellDate(destDay));
}

void CalendarGrid::showEntryDialog(Gtk::Window& window,
                                   std::chrono::year_month_day date,
                                   std::optional<int> editIndex)
{
    CalendarEntryDialog* dialog = nullptr;

    const auto* existing = editIndex.has_value()
        ? m_controller.findEntry(date, static_cast<std::size_t>(editIndex.value()))
        : nullptr;

    if (existing != nullptr)
    {
        dialog = new CalendarEntryDialog(window, *existing);
    }
    else
    {
        dialog = new CalendarEntryDialog(window);
    }

    dialog->signal_response().connect([this, dialog, date, editIndex](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (auto result = dialog->getResult(); result.has_value())
            {
                if (editIndex.has_value())
                    m_controller.editEntry(date, static_cast<std::size_t>(editIndex.value()), std::move(result.value()));
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

bool CalendarGrid::isToday(const int day) const
{
    using namespace std::chrono;
    return DateUtils::today() == year_month_day{m_currentYearMonth / day};
}

void CalendarGrid::onCellRightClicked(const int cellIndex)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    const auto date = cellDate(day);
    const auto clipboard = get_clipboard();

    ClipboardUrlDetector::detectAsync(
        clipboard,
        [this, date](const std::string& url)
        {
            UrlTitleFetcher::fetchAsync(url, [this, date, url](const std::string& title)
            {
                const CalendarEntry entry{ title.empty() ? LocaleManager::instance().translate("entry.default_name") : title, url };

                Glib::signal_idle().connect_once([this, date, entry]
                {
                    m_controller.addEntry(date, entry);
                });
            });
        },
        [](const std::string&) { /* no URL in clipboard — do nothing */ }
    );
}

void CalendarGrid::onEntryColorChangeRequested(const int cellIndex, const int entryIndex, const EntryColor color)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    if (entryIndex < 0)
        return;

    m_controller.changeEntryColor(cellDate(day), static_cast<std::size_t>(entryIndex), color);
}
