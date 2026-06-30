#include "MenuActionHandler.hpp"
#include <gtkmm/application.h>
#include <gtkmm/messagedialog.h>

#include "../cloud/CloudStorageConfigStorage.hpp"
#include "../cloud/ConnectDialog.hpp"
#include "../dialog/AboutDialog.hpp"

MenuActionHandler::MenuActionHandler(Gtk::ApplicationWindow& window, CalendarGrid& calendarGrid): m_window(window), m_calendarGrid(calendarGrid) {}

void MenuActionHandler::registerActions()
{
    m_window.add_action("connect", sigc::mem_fun(*this, &MenuActionHandler::onActionConnect));
    m_window.add_action("quit", sigc::mem_fun(*this, &MenuActionHandler::onActionQuit));
    m_window.add_action("undo", sigc::mem_fun(*this, &MenuActionHandler::onActionUndo));
    m_window.add_action("redo", sigc::mem_fun(*this, &MenuActionHandler::onActionRedo));
    m_window.add_action("about", sigc::mem_fun(*this, &MenuActionHandler::onActionAbout));
}

void MenuActionHandler::onActionConnect() const
{
    auto* dialog = new ConnectDialog(m_window); // NOLINT

    if (const auto config = CloudStorageConfigStorage::load(); config.has_value())
    {
        dialog->prefillConfig(config.value());
    }

    dialog->signal_response().connect([this, dialog](int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (auto result = dialog->getResult(); result.has_value())
            {
                CloudStorageConfigStorage::save(result.value());
                try
                {
                    auto client = std::make_unique<CloudStorageClient>(result.value());
                    m_calendarGrid.setCloudClient(std::move(client));

                    g_print("[Cloud] Połączono z: %s\n", result.value().apiUrl.c_str());

                    auto* infoDialog = new Gtk::MessageDialog( // NOLINT
                        m_window, "Połączono z chmurą", false,
                        Gtk::MessageType::INFO, Gtk::ButtonsType::OK, true);
                    infoDialog->set_secondary_text("Synchronizacja działa poprawnie.");
                    infoDialog->signal_response().connect([infoDialog](int) { infoDialog->hide(); });
                    infoDialog->signal_hide().connect([infoDialog]() { delete infoDialog; }); // NOLINT
                    infoDialog->show();
                }
                catch (const std::exception& e)
                {
                    g_print("[Cloud] Błąd połączenia: %s\n", e.what());

                    auto* errDialog = new Gtk::MessageDialog( // NOLINT
                        m_window, "Błąd połączenia", false,
                        Gtk::MessageType::ERROR, Gtk::ButtonsType::OK, true);
                    errDialog->set_secondary_text(e.what());
                    errDialog->signal_response().connect([errDialog](int) { errDialog->hide(); });
                    errDialog->signal_hide().connect([errDialog]() { delete errDialog; }); // NOLINT
                    errDialog->show();
                }
            }
        }
        dialog->hide();
    });

    dialog->signal_hide().connect([dialog]() { delete dialog; }); // NOLINT
    dialog->show();
}

void MenuActionHandler::onActionQuit()
{
    m_window.get_application()->quit();
}

void MenuActionHandler::onActionUndo()
{
    m_calendarGrid.undo();
}

void MenuActionHandler::onActionRedo()
{
    m_calendarGrid.redo();
}

void MenuActionHandler::onActionAbout() const
{
    auto* dialog = new AboutDialog(m_window);
    dialog->signal_response().connect([dialog](int) { dialog->hide(); });
    dialog->signal_hide().connect([dialog]() { delete dialog; });
    dialog->show();
}
