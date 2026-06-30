#pragma once

class CalendarCommand
{
public:
    virtual ~CalendarCommand();
    virtual void execute() = 0;
    virtual void undo() = 0;
};