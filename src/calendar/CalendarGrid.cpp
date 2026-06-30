#include "CalendarGrid.hpp"

#include <glibmm/main.h>
#include <gtkmm/window.h>

#include "../util/UrlTitleFetcher.hpp"
#include "command/AddEntryCommand.hpp"
#include "command/DeleteEntryCommand.hpp"
#include "command/EditEntryCommand.hpp"
#include "detector/ClipboardUrlDetector.hpp"
#include "dialog/CalendarEntryDialog.hpp"
#include "storage/CalendarStorage.hpp"
#include <nlohmann/json.hpp>

CalendarGrid::CalendarGrid()
{
    const auto today = std::chrono::year_month_day{
        std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())
    };

    m_currentYearMonth = std::chrono::year_month{today.year(), today.month()};
    m_entries = CalendarStorage::load();

    initLayout();
    connectCellSignals();
    populateCells();
}

void CalendarGrid::initLayout()
{
    set_row_homogeneous(true);
    set_column_homogeneous(true);
    set_hexpand(true);
    set_vexpand(true);

    for (int row = 0; row < ROWS; ++row)
        for (int col = 0; col < COLUMNS; ++col)
            attach(m_cells[row * COLUMNS + col], col, row);
}

void CalendarGrid::connectCellSignals()
{
    for (int i = 0; i < TOTAL_CELLS; ++i)
    {
        m_cells[i].signalDoubleClicked().connect([this, i] { onCellDoubleClicked(i); });
        m_cells[i].signalEditRequested().connect([this, i](const int entryIndex) { onEntryEditRequested(i, entryIndex); });
        m_cells[i].signalDeleteRequested().connect([this, i](const int entryIndex) { onEntryDeleteRequested(i, entryIndex); });
        m_cells[i].signalRightClicked().connect([this, i] { onCellRightClicked(i); });
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

            if (const auto date = cellDate(day); m_entries.contains(date))
                m_cells[i].setEntries(m_entries.at(date));
            else
                m_cells[i].setEntries({});
        }
    }
}

void CalendarGrid::onCellDoubleClicked(const int cellIndex)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    auto* window = dynamic_cast<Gtk::Window*>(get_root());
    if (window == nullptr)
        return;

    showEntryDialog(*window, cellDate(day), std::nullopt);
}

void CalendarGrid::onEntryEditRequested(const int cellIndex, int entryIndex)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    auto* window = dynamic_cast<Gtk::Window*>(get_root());
    if (window == nullptr)
        return;

    showEntryDialog(*window, cellDate(day), entryIndex);
}

void CalendarGrid::onEntryDeleteRequested(const int cellIndex, const int entryIndex)
{
    const int day = cellDay(cellIndex);
    if (day < 1 || day > daysInMonth())
        return;

    const auto date = cellDate(day);
    if (!m_entries.contains(date))
        return;

    if (auto& entries = m_entries[date]; entryIndex < 0 || entryIndex >= static_cast<int>(entries.size()))
        return;

    m_history.execute(std::make_unique<DeleteEntryCommand>(m_entries, date, entryIndex));
    saveEntries();
    populateCells();
}

void CalendarGrid::showEntryDialog(Gtk::Window& window,
                                   std::chrono::year_month_day date,
                                   std::optional<int> editIndex)
{
    CalendarEntryDialog* dialog = nullptr;

    if (editIndex.has_value() && m_entries.contains(date))
    {
        const auto& existing = m_entries.at(date)[editIndex.value()];
        dialog = new CalendarEntryDialog(window, existing);
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
                    m_history.execute(std::make_unique<EditEntryCommand>(m_entries, date, editIndex.value(), std::move(result.value())));
                else
                    m_history.execute(std::make_unique<AddEntryCommand>(m_entries, date, std::move(result.value())));

                saveEntries();
                populateCells();
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

    const auto today = year_month_day{
        floor<days>(system_clock::now())
    };

    return today == year_month_day{m_currentYearMonth / day};
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
                    m_history.execute(std::make_unique<AddEntryCommand>(m_entries, date, entry));
                    saveEntries();
                    populateCells();
                });
            });
        },
        [](const std::string&) { /* brak URL w schowku — nic nie robimy */ }
    );
}

void CalendarGrid::saveEntries() const
{
    CalendarStorage::save(m_entries);
    if (m_cloudClient != nullptr)
    {
        g_print("[Cloud] Wysyłanie danych do chmury...\n");
        try
        {
            m_cloudClient->saveJson(CalendarStorage::toJson(m_entries));
            g_print("[Cloud] Zapis do chmury zakończony pomyślnie.\n");
        }
        catch (const std::exception& e)
        {
            g_print("[Cloud] Błąd zapisu do chmury: %s\n", e.what());
        }
    }
}

void CalendarGrid::undo()
{
    m_history.undo();
    saveEntries();
    populateCells();
}

void CalendarGrid::redo()
{
    m_history.redo();
    saveEntries();
    populateCells();
}

void CalendarGrid::setCloudClient(std::unique_ptr<CloudStorageClient> client)
{
    m_cloudClient = std::move(client);
    syncFromCloud();
}

void CalendarGrid::syncFromCloud()
{
    if (m_cloudClient == nullptr)
        return;

    g_print("[Cloud] Pobieranie danych z chmury...\n");

    try
    {
        const auto json = m_cloudClient->loadJson();
        if (json.empty())
        {
            g_print("[Cloud] Brak danych w chmurze.\n");
            return;
        }

        m_entries = CalendarStorage::fromJson(json);
        g_print("[Cloud] Synchronizacja zakończona pomyślnie.\n");
        populateCells();
    }
    catch (const std::exception& e)
    {
        g_print("[Cloud] Błąd synchronizacji: %s\n", e.what());
    }
}

