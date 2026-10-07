
#include <gtkmm.h>

#include <glib.h>
#include <glibmm/main.h>

#include <algorithm>
#include <cstring>

#include "stapik/app/AppContext.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"
#include "stapik/log/Log.hpp"
#include "ui/SelfTest.hpp"
#include "ui/window/MainWindow.hpp"

namespace
{
    constexpr auto APPLICATION_ID = "pl.stapik.calendar";
    constexpr auto AUTHOR = "Sebastian Smoliński";
    constexpr auto REPOSITORY_URL = "https://github.com/Stapik-Group/stapik-calendar";
    constexpr auto SELF_TEST_OPTION = "--self-test";
    constexpr unsigned SELF_TEST_DELAY_MILLISECONDS = 2000;

    bool takeSelfTestOption(int& argumentCount, char* arguments[])
    {
        const auto end = arguments + argumentCount;
        const auto option = std::find_if(arguments + 1, end, [](const char* argument) { return std::strcmp(argument, SELF_TEST_OPTION) == 0; });
        if (option == end)
            return false;

        std::copy(option + 1, end, option);
        --argumentCount;
        return true;
    }

    stapik::app::AppInfo createAppInfo()
    {
        return {
            .applicationId = APPLICATION_ID,
            .internalName = STAPIK_APP_NAME,
            .displayName = STAPIK_APP_DISPLAY_NAME,
            .version = STAPIK_APP_VERSION,
            .author = AUTHOR,
            .repositoryUrl = REPOSITORY_URL
        };
    }
}

int main(int argumentCount, char *argv[])
{
    const bool selfTest = takeSelfTestOption(argumentCount, argv);

    if (selfTest)
        g_setenv("G_MESSAGES_DEBUG", "all", FALSE);

    stapik::app::AppContext::initialize(createAppInfo());

    if (selfTest)
        stapik::log::setLevel(stapik::log::Level::Info);

    const auto app = Gtk::Application::create(APPLICATION_ID);

    auto styleProvider = AppStyleProvider::withCommonThemes(AppPaths::resourcesDir());
    int selfTestFailures = 0;

    app->signal_activate().connect([&]
    {
        styleProvider.apply(ThemeManager::instance().themeId());
        ThemeManager::instance().signalThemeChanged().connect(
            [&styleProvider] { styleProvider.apply(ThemeManager::instance().themeId()); });

        StandardMenu::installShortcuts(*app);
        app->set_accels_for_action("win.guide", { "F1" });

        auto* window = new MainWindow(styleProvider.themes());
        app->add_window(*window);
        window->show();

        if (selfTest)
        {
            Glib::signal_timeout().connect_once([&app, &selfTestFailures, window]
            {
                selfTestFailures = runSelfTest(*window);
                app->quit();
            }, SELF_TEST_DELAY_MILLISECONDS);
        }
    });

    const int status = app->run(argumentCount, argv);
    return selfTest && selfTestFailures > 0 ? 1 : status;
}
