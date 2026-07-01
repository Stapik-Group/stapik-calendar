#pragma once

#include <gtkmm/grid.h>
#include <array>
#include <chrono>
#include "CalendarCell.hpp"
#include "../../infrastructure/network/CloudStorageClient.hpp"
#include "../../core/command/CalendarCommandHistory.hpp"
#include "../../infrastructure/storage/CalendarStorage.hpp"

class CalendarGrid : public Gtk::Grid
{
public:
    explicit CalendarGrid();
    void displayMonth(std::chrono::year_month yearMonth);
    void undo();
    void redo();
    void setCloudClient(std::unique_ptr<CloudStorageClient> client);
private:
    static constexpr int ROWS = 6;
    static constexpr int COLUMNS = 7;
    static constexpr int TOTAL_CELLS = ROWS * COLUMNS;

    std::array<CalendarCell, TOTAL_CELLS> m_cells;
    std::chrono::year_month m_currentYearMonth {};
    CalendarStorage::Entries m_entries;
    CalendarCommandHistory m_history;
    std::unique_ptr<CloudStorageClient> m_cloudClient;

    void initLayout();
    void populateCells();
    void connectCellSignals();
    void saveEntries() const;
    void syncFromCloud();

    void onCellDoubleClicked(int cellIndex);
    void onCellRightClicked(int cellIndex);
    void onEntryEditRequested(int cellIndex, int entryIndex);
    void onEntryDeleteRequested(int cellIndex, int entryIndex);

    void showEntryDialog(Gtk::Window& window, std::chrono::year_month_day date, std::optional<int> editIndex);

    int firstWeekdayOffset() const;
    int daysInMonth() const;
    bool isToday(int day) const;

    std::chrono::year_month_day cellDate(int day) const;
    int cellDay(int cellIndex) const;
};