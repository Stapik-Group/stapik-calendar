#pragma once

#include "stapik/ui/widget/CalendarButton.hpp"

#include <sigc++/signal.h>

#include <chrono>

class CalendarDatePicker : public CalendarButton
{
public:
    explicit CalendarDatePicker(std::chrono::year_month_day date);

    void setDate(std::chrono::year_month_day date);

    sigc::signal<void(std::chrono::year_month)>& signalMonthChanged();
    sigc::signal<void(std::chrono::year_month_day)>& signalDayPicked();

protected:
    void onDaySelected() override;
    void refreshLabel() override;

private:
    std::chrono::year_month_day m_date;
    sigc::signal<void(std::chrono::year_month)> m_signalMonthChanged;
    sigc::signal<void(std::chrono::year_month_day)> m_signalDayPicked;
};
