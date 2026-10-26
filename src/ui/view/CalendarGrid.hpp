#pragma once

#include "CalendarCell.hpp"
#include "ViewMode.hpp"
#include "../../application/CalendarController.hpp"
#include "../../core/util/WeekRules.hpp"

#include <gtkmm/grid.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
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

    void display(ViewMode viewMode, std::chrono::year_month_day anchorDate, std::optional<std::chrono::year_month_day> markedDate);
    sigc::signal<void(std::chrono::year_month_day)>& signalDaySelected();
    void addEntryOn(std::chrono::year_month_day date);
private:
    static constexpr int ROWS = 6;
    static constexpr int COLUMNS = 7;
    static constexpr int TOTAL_CELLS = ROWS * COLUMNS;
    static constexpr unsigned DAY_CHANGE_CHECK_SECONDS = 30;

    std::array<CalendarCell, TOTAL_CELLS> m_cells;
    ViewMode m_viewMode = ViewMode::Month;
    std::chrono::year_month_day m_anchorDate {};
    std::chrono::year_month m_currentYearMonth {};
    std::array<std::optional<std::chrono::year_month_day>, TOTAL_CELLS> m_cellDates {};
    std::chrono::year_month_day m_today {};
    std::optional<std::chrono::year_month_day> m_markedDate;
    CalendarController& m_controller;
    const WeekRules& m_weekRules;
    sigc::connection m_documentConnection;
    sigc::connection m_dayChangeConnection;
    sigc::signal<void(std::chrono::year_month_day)> m_signalDaySelected;

    void initLayout();
    void layoutCells();
    void updateCellDates();
    void populateCells();
    void populateCell(int cellIndex);
    void refreshIfDayChanged();
    void connectCellSignals();

    void onCellClicked(int cellIndex);
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
    [[nodiscard]] Gtk::Window* windowForCell(int cellIndex);

    int firstWeekdayOffset() const;
    int daysInMonth() const;
    int daysInPreviousMonth() const;
    [[nodiscard]] std::chrono::weekday weekdayOfCell(int cellIndex) const;
};
