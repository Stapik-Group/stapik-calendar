
#include <gtkmm.h>

#include "stapik/app/AppContext.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"
#include "ui/window/MainWindow.hpp"

namespace
{
    constexpr auto APPLICATION_ID = "pl.stapik.calendar";
    constexpr auto AUTHOR = "Sebastian Smoliński";
    constexpr auto REPOSITORY_URL = "https://github.com/Stapik-Group/stapik-calendar";

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

int main(const int argc, char *argv[])
{
    stapik::app::AppContext::initialize(createAppInfo());
    const auto app = Gtk::Application::create(APPLICATION_ID);

    auto styleProvider = AppStyleProvider::withCommonThemes(AppPaths::resourcesDir());

    app->signal_activate().connect([&]
    {
        styleProvider.apply(ThemeManager::instance().getTheme());
        ThemeManager::instance().signalThemeChanged().connect(
            [&styleProvider] { styleProvider.apply(ThemeManager::instance().getTheme()); });

        auto* window = new MainWindow();
        app->add_window(*window);
        window->show();
    });
    return app->run(argc, argv);
}
