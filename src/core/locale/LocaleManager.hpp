#pragma once

#include "Locale.hpp"
#include "LocalizationEngine.hpp"

#include <sigc++/signal.h>
#include <filesystem>

class LocaleManager
{
public:
    static LocaleManager& instance();

    void setLocale(Locale locale);
    Locale getLocale() const;
    std::string translate(const std::string& key) const;

    sigc::signal<void()>& signalLocaleChanged();
private:
    LocaleManager();

    LocalizationEngine m_engine;
    sigc::signal<void()> m_signalLocaleChanged;

    static void saveLocale(Locale locale);
    static Locale loadSavedLocale();

    static std::filesystem::path localeConfigPath();
    static std::string localeToString(Locale locale);
    static Locale localeFromString(const std::string& str);
};