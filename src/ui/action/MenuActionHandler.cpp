#include "MenuActionHandler.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/cloud/CloudStorageException.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/CloudStorageConfigStorage.hpp"
#include "stapik/ui/dialog/AboutDialog.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"

#include <gtkmm/application.h>
#include <gtkmm/messagedialog.h>
#include <tuple>

MenuActionHandler::MenuActionHandler(Gtk::ApplicationWindow& window, CalendarGrid& calendarGrid):
    m_window(window),
    m_calendarGrid(calendarGrid) {}

void MenuActionHandler::registerActions()
{
    m_window.add_action("connect", sigc::mem_fun(*this, &MenuActionHandler::onActionConnect));
    m_window.add_action("quit", sigc::mem_fun(*this, &MenuActionHandler::onActionQuit));
    m_window.add_action("undo", sigc::mem_fun(*this, &MenuActionHandler::onActionUndo));
    m_window.add_action("redo", sigc::mem_fun(*this, &MenuActionHandler::onActionRedo));
    m_window.add_action("about", sigc::mem_fun(*this, &MenuActionHandler::onActionAbout));
    m_window.add_action("sync", sigc::mem_fun(*this, &MenuActionHandler::onActionSync));
}

void MenuActionHandler::onActionConnect() const
{
    showConnectDialog(
        m_window,
        CloudStorageConfigStorage::load(appName()),
        [this](const CloudStorageConfig& config) { handleConnectResult(config); });
}

void MenuActionHandler::handleConnectResult(const CloudStorageConfig& config) const
{
    if (!CloudStorageConfigStorage::save(config, appName()))
        g_warning("[Cloud] Cannot save the cloud configuration; it will be lost after restart.");

    applyCloudConfig(config);
}

std::string MenuActionHandler::appName()
{
    return stapik::app::AppContext::instance().info().internalName;
}

void MenuActionHandler::applyCloudConfig(const CloudStorageConfig& config) const
{
    const auto& loc = LocaleManager::instance();

    try
    {
        auto client = std::make_unique<CloudStorageClient>(config, CALENDAR_FILENAME);
        std::ignore = client->loadDocument();

        m_calendarGrid.setCloudClient(std::move(client));

        g_message("[Cloud] Connected: %s", config.apiUrl.c_str());
        showMessageDialog(m_window, loc.translate("cloud.connected"), loc.translate("cloud.connected.secondary"), Gtk::MessageType::INFO);
    }
    catch (const CloudStorageException& e)
    {
        g_warning("[Cloud] Cloud connection error: %s", e.what());
        showMessageDialog(m_window, loc.translate("cloud.failed.header"), e.what(), Gtk::MessageType::ERROR);
    }
}

void MenuActionHandler::onActionQuit() const
{
    m_window.get_application()->quit();
}

void MenuActionHandler::onActionUndo() const
{
    m_calendarGrid.undo();
}

void MenuActionHandler::onActionRedo() const
{
    m_calendarGrid.redo();
}

void MenuActionHandler::onActionAbout() const
{
    showAboutDialog(m_window);
}

void MenuActionHandler::onActionSync() const
{
    m_calendarGrid.retrySync();
}
