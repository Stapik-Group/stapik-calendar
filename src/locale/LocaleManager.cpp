#include "LocaleManager.hpp"
#include <fstream>

LocaleManager & LocaleManager::instance()
{
    static LocaleManager manager;
    return manager;
}

LocaleManager::LocaleManager()
{
    m_engine.setLocale(loadSavedLocale());
}

void LocaleManager::setLocale(Locale locale)
{
    m_engine.setLocale(locale);
    saveLocale(locale);
    m_signalLocaleChanged.emit();
}

Locale LocaleManager::getLocale() const
{
    return m_engine.getLocale();
}

std::string LocaleManager::translate(const std::string &key) const
{
    return m_engine.translate(key);
}

sigc::signal<void()> & LocaleManager::signalLocaleChanged()
{
    return m_signalLocaleChanged;
}

void LocaleManager::saveLocale(Locale locale) const
{
    const auto path = localeConfigPath();
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path);
    file << localeToString(locale);
}

Locale LocaleManager::loadSavedLocale()
{
    const auto path = localeConfigPath();
    if (!std::filesystem::exists(path))
        return Locale::EN;

    std::ifstream file(path);
    if (!file.is_open())
        return Locale::EN;

    std::string content;
    file >> content;

    return localeFromString(content);
}

std::filesystem::path LocaleManager::localeConfigPath()
{
    const auto* home = std::getenv("HOME");
    if (home == nullptr)
        return std::filesystem::temp_directory_path() / "stapikcalendar_locale.txt";

    return std::filesystem::path(home) / ".local" / "share" / "stapikcalendar" / "locale.txt";
}

std::string LocaleManager::localeToString(Locale locale)
{
    switch (locale)
    {
        case Locale::PL: return "pl";
        case Locale::EN: return "en";
        case Locale::DE: return "de";
    }
    return "en";
}

Locale LocaleManager::localeFromString(const std::string& str)
{
    if (str == "en") return Locale::EN;
    if (str == "de") return Locale::DE;
    return Locale::EN;
}
