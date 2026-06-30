#include "LocalizationEngine.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

#include "../AppPaths.hpp"

LocalizationEngine::LocalizationEngine()
    : m_currentLocale(Locale::PL)
{
    loadTranslations();
}

void LocalizationEngine::loadTranslations()
{
    using enum Locale;
    const auto localesDir = AppPaths::resourcesDir() / "locales";

    loadLocaleFile(PL, (localesDir / "pl.json").string());
    loadLocaleFile(EN, (localesDir / "en.json").string());
    loadLocaleFile(DE, (localesDir / "de.json").string());
}

void LocalizationEngine::loadLocaleFile(Locale locale, const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        return;

    try
    {
        const auto json = nlohmann::json::parse(file);
        for (const auto &[key, value]: json.items())
            m_translations[locale][key] = value.get<std::string>();
    } catch (const nlohmann::json::exception &)
    {
        // brak pliku tłumaczeń — pozostaje pusta mapa dla tego locale
    }
}

void LocalizationEngine::setLocale(Locale locale)
{
    m_currentLocale = locale;
}

Locale LocalizationEngine::getLocale() const
{
    return m_currentLocale;
}

std::string LocalizationEngine::translate(const std::string &key) const
{
    const auto localeIt = m_translations.find(m_currentLocale);
    if (localeIt == m_translations.end())
        return key;

    const auto keyIt = localeIt->second.find(key);
    if (keyIt == localeIt->second.end())
        return key;

    return keyIt->second;
}
