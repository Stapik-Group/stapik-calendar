#pragma once

enum class Locale
{
    PL,
    EN,
    DE
};

inline const char* toString(const Locale locale)
{
    switch (locale)
    {
        case Locale::PL: return "PL";
        case Locale::EN: return "EN";
        case Locale::DE: return "DE";
        default: return "[Unknown Locale]";
    }
}
