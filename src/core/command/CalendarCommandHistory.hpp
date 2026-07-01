#pragma once

#include "CalendarCommand.hpp"

#include <memory>
#include <vector>

class CalendarCommandHistory
{
public:
    void execute(std::unique_ptr<CalendarCommand> command);
    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;
private:
    std::vector<std::unique_ptr<CalendarCommand>> m_undoStack;
    std::vector<std::unique_ptr<CalendarCommand>> m_redoStack;
};