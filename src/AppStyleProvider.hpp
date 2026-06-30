#pragma once

#include <gtkmm/cssprovider.h>
#include <string>

class AppStyleProvider
{
public:
    static void load(const std::string& cssFilePath);
};