#include "EntryCommandBase.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <glibmm/ustring.h>

namespace
{
    constexpr std::size_t MAX_NAME_LENGTH = 30;

    std::string shorten(const std::string& text)
    {
        const Glib::ustring name(text);
        if (!name.validate() || name.size() <= MAX_NAME_LENGTH)
            return text;

        return name.substr(0, MAX_NAME_LENGTH).raw() + "\u2026";
    }
}

EntryCommandBase::EntryCommandBase(CalendarEntries& entries, const std::chrono::year_month_day date) :
    m_entries(entries),
    m_date(date) {}

std::string EntryCommandBase::describe(const std::string& key, const std::string& entryName)
{
    return LocaleManager::instance().translate(key, { { "name", shorten(entryName) } });
}
