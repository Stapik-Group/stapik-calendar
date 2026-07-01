#include "CalendarCommandHistory.hpp"

void CalendarCommandHistory::execute(std::unique_ptr<CalendarCommand> command)
{
    command->execute();
    m_undoStack.push_back(std::move(command));
    m_redoStack.clear();
}

void CalendarCommandHistory::undo()
{
    if (canUndo())
    {
        auto command = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        command->undo();
        m_redoStack.push_back(std::move(command));
    }
}

void CalendarCommandHistory::redo()
{
    if (canRedo())
    {
        auto command = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        command->execute();
        m_undoStack.push_back(std::move(command));
    }
}

bool CalendarCommandHistory::canUndo() const
{
    return !m_undoStack.empty();
}

bool CalendarCommandHistory::canRedo() const
{
    return !m_redoStack.empty();
}