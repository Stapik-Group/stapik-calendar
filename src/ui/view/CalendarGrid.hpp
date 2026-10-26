#pragma once

#include "CalendarCell.hpp"
#include "../../application/CalendarController.hpp"

#include <gtkmm/grid.h>
#include <sigc++/connection.h>
#include <array>
#include <chrono>
#include <optional>
#include <string>

class CalendarEntryDialog;

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
    static constexpr int FIRST_WEEKEND_COLUMN = 5; // weeks start on Monday: Saturday = 5, Sunday = 6
    static constexpr unsigned DAY_CHANGE_CHECK_SECONDS = 30;

    std::array<CalendarCell, TOTAL_CELLS> m_cells;
    std::chrono::year_month m_currentYearMonth {};
    std::chrono::year_month_day m_today {};
    CalendarController& m_controller;
    sigc::connection m_documentConnection;
    sigc::connection m_dayChangeConnection;

    void initLayout();
    void populateCells();
    void refreshIfDayChanged();
    void connectCellSignals();

    void onCellDoubleClicked(int cellIndex);
    void onCellRightClicked(int cellIndex);
    void onEntryLinkClicked(int cellIndex, int entryIndex);
    void onEntryEditRequested(int cellIndex, int entryIndex);
    void onEntryDeleteRequested(int cellIndex, int entryIndex);
    void onEntryColorChangeRequested(int cellIndex, int entryIndex, EntryColor color);
    void onEntryMoveRequested(int sourceCellIndex, int sourceEntryIndex, int destCellIndex, bool isCopy);

    void showEntryDialog(Gtk::Window& window, std::chrono::year_month_day date, std::optional<int> editIndex);
    void showEntryDialogForLink(std::chrono::year_month_day date, const std::string& url);
    void presentEntryDialog(CalendarEntryDialog& entryDialog, std::chrono::year_month_day date, std::optional<int> editIndex);
    [[nodiscard]] Gtk::Window* validatedWindowForCell(int cellIndex, int& outDay);
    [[nodiscard]] bool isValidEntryIndex(std::chrono::year_month_day date, int entryIndex) const;

    int firstWeekdayOffset() const;
    int daysInMonth() const;
    int daysInPreviousMonth() const;
    bool isToday(int day) const;

    std::chrono::year_month_day cellDate(int day) const;
    int cellDay(int cellIndex) const;
};