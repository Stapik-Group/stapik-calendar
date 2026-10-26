#pragma once

#include "CalendarCell.hpp"
#include "../../application/CalendarController.hpp"
#include "../../core/util/WeekRules.hpp"

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
    CalendarGrid(CalendarController& controller, const WeekRules& weekRules);
    ~CalendarGrid() override;

    void displayMonth(std::chrono::year_month yearMonth);
private:
    static constexpr int ROWS = 6;
    static constexpr int COLUMNS = 7;
    static constexpr int TOTAL_CELLS = ROWS * COLUMNS;
    static constexpr unsigned DAY_CHANGE_CHECK_SECONDS = 30;

    std::array<CalendarCell, TOTAL_CELLS> m_cells;
    std::chrono::year_month m_currentYearMonth {};
    std::chrono::year_month_day m_today {};
    CalendarController& m_controller;
    const WeekRules& m_weekRules;
    sigc::connection m_documentConnection;
    sigc::connection m_dayChangeConnection;

    void initLayout();
    void populateCells();
    void refreshIfDayChanged();
    void connectCellSignals();

    void onCellDoubleClicked(int cellIndex);
    void onCellRightClicked(int cellIndex);
    void onEntryLinkClicked(int cellIndex, const EntryId& entryId);
    void onEntryEditRequested(int cellIndex, const EntryId& entryId);
    void onEntryDeleteRequested(int cellIndex, const EntryId& entryId);
    void onEntryColorChangeRequested(int cellIndex, const EntryId& entryId, EntryColor color);
    void onEntryMoveRequested(int sourceCellIndex, const EntryId& entryId, int destCellIndex, bool isCopy);

    void showEntryDialog(Gtk::Window& window, std::chrono::year_month_day date, const std::optional<EntryId>& editId);
    void showEntryDialogForLink(std::chrono::year_month_day date, const std::string& url);
    void presentEntryDialog(CalendarEntryDialog& entryDialog, std::chrono::year_month_day date, const std::optional<EntryId>& editId);
    [[nodiscard]] Gtk::Window* validatedWindowForCell(int cellIndex, int& outDay);

    int firstWeekdayOffset() const;
    int daysInMonth() const;
    int daysInPreviousMonth() const;
    bool isToday(int day) const;

    std::chrono::year_month_day cellDate(int day) const;
    int cellDay(int cellIndex) const;
};