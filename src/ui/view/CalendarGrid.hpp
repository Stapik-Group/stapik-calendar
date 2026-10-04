#pragma once

#include "CalendarCell.hpp"
#include "../../core/model/CalendarDocument.hpp"
#include "../../infrastructure/storage/CalendarDocumentStore.hpp"

#include "stapik/cloud/CloudStorageClient.hpp"
#include "stapik/command/UndoStack.hpp"

#include <gtkmm/grid.h>
#include <sigc++/connection.h>
#include <array>
#include <chrono>

class CalendarGrid : public Gtk::Grid
{
public:
    explicit CalendarGrid();
    void displayMonth(std::chrono::year_month yearMonth);
    [[nodiscard]] stapik::command::UndoStack& undoStack();
    void setCloudClient(std::unique_ptr<CloudStorageClient> client);
    void retrySync();
private:
    static constexpr int ROWS = 6;
    static constexpr int COLUMNS = 7;
    static constexpr int TOTAL_CELLS = ROWS * COLUMNS;

    std::array<CalendarCell, TOTAL_CELLS> m_cells;
    std::chrono::year_month m_currentYearMonth {};
    CalendarDocumentStore m_store;
    CalendarDocument m_document;

    stapik::command::UndoStack m_history;
    sigc::connection m_historyConnection;
    std::unique_ptr<CloudStorageClient> m_cloudClient;

    void initLayout();
    void populateCells();
    void connectCellSignals();
    void saveEntries();
    void syncFromCloud();
    void touchLastUpdate();
    void onHistoryChanged();
    void clearHistorySilently();

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