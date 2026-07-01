#pragma once

#include "Locale.hpp"
#include <string>
#include <map>

class LocalizationEngine
{
public:
    explicit LocalizationEngine();
    void setLocale(Locale locale);
    Locale getLocale() const;
    std::string translate(const std::string& key) const;
private:
    Locale m_currentLocale;
    std::map<Locale, std::map<std::string, std::string>> m_translations;
    void loadTranslations();
    void loadLocaleFile(Locale locale, const std::string& filePath);
};
