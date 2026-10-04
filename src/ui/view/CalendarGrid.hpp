#pragma once

#include "CalendarCell.hpp"
#include "../../application/CalendarController.hpp"

#include <gtkmm/grid.h>
#include <sigc++/connection.h>
#include <array>
#include <chrono>

class CalendarGrid : public Gtk::Grid
{
public:
    explicit CalendarGrid(CalendarController& controller);
    ~CalendarGrid() override;

    void displayMonth(std::chrono::year_month yearMonth);
private:
    static constexpr int ROWS = 6;
    static constexpr int COLUMNS = 7;
    static constexpr int TOTAL_CELLS = ROWS * COLUMNS;

    std::array<CalendarCell, TOTAL_CELLS> m_cells;
    std::chrono::year_month m_currentYearMonth {};
    CalendarController& m_controller;
    sigc::connection m_documentConnection;

    void initLayout();
    void populateCells();
    void connectCellSignals();

    void onCellDoubleClicked(int cellIndex);
    void onCellRightClicked(int cellIndex);
    void onEntryEditRequested(int cellIndex, int entryIndex);
    void onEntryDeleteRequested(int cellIndex, int entryIndex);
    void onEntryColorChangeRequested(int cellIndex, int entryIndex, EntryColor color);
    void onEntryMoveRequested(int sourceCellIndex, int sourceEntryIndex, int destCellIndex, bool isCopy);

    void showEntryDialog(Gtk::Window& window, std::chrono::year_month_day date, std::optional<int> editIndex);
    [[nodiscard]] Gtk::Window* validatedWindowForCell(int cellIndex, int& outDay);
    [[nodiscard]] bool isValidEntryIndex(std::chrono::year_month_day date, int entryIndex) const;

    int firstWeekdayOffset() const;
    int daysInMonth() const;
    bool isToday(int day) const;

    std::chrono::year_month_day cellDate(int day) const;
    int cellDay(int cellIndex) const;
};