
#include <gtkmm.h>

#include <glib.h>
#include <glibmm/main.h>

#include <algorithm>
#include <cstring>

#include "stapik/app/StapikApplication.hpp"
#include "stapik/app/version.hpp"
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
            .internalName = std::string{ stapik::app::build::NAME },
            .displayName = std::string{ stapik::app::build::DISPLAY_NAME },
            .version = std::string{ stapik::app::build::VERSION },
            .author = AUTHOR,
            .repositoryUrl = REPOSITORY_URL
        };
    }

    void showExistingWindow(Gtk::Window& window)
    {
        window.unminimize();
        window.present();
    }
}

int main(int argumentCount, char *argv[])
{
    const bool selfTest = takeSelfTestOption(argumentCount, argv);

    if (selfTest)
        g_setenv("G_MESSAGES_DEBUG", "all", FALSE);

    // Creating the application initializes the application context. Settings, AppPaths and LocaleManager must not
    // be touched before run(): the command line flags (--data-dir, --portable, --reset-settings) are applied there.
    stapik::app::StapikApplicationOptions options;
    options.checkForUpdates = !selfTest;

    const auto app = stapik::app::StapikApplication::create(createAppInfo(), options);

    if (selfTest)
        stapik::log::setLevel(stapik::log::Level::Info);

    int selfTestFailures = 0;

    app->signal_activate().connect([&]
    {
        // Starting the program again activates the running one, which must not open a second window.
        if (const auto windows = app->get_windows(); !windows.empty())
        {
            showExistingWindow(*windows.front());
            return;
        }

        // StapikApplication binds F1 to the tutorial, which the calendar does not have yet: F1 opens the guide.
        app->set_accels_for_action("win.tutorial", {});
        app->set_accels_for_action("win.guide", { "F1" });

        auto* window = new MainWindow(app->themes());
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
