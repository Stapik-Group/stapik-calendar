#pragma once

#include "CalendarCell.hpp"
#include "../../core/command/CalendarCommandHistory.hpp"
#include "../../core/model/CalendarEntries.hpp"

#include "stapik/cloud/CloudStorageClient.hpp"

#include <gtkmm/grid.h>
#include <array>
#include <chrono>

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
    CalendarEntries m_entries;
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
    [[nodiscard]] Gtk::Window* validatedWindowForCell(int cellIndex, int& outDay);
    [[nodiscard]] bool isValidEntryIndex(std::chrono::year_month_day date, int entryIndex) const;

    int firstWeekdayOffset() const;
    int daysInMonth() const;
    bool isToday(int day) const;

    std::chrono::year_month_day cellDate(int day) const;
    int cellDay(int cellIndex) const;
};