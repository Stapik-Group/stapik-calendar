#pragma once

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <optional>
#include "../../infrastructure/network/CloudStorageConfig.hpp"

class ConnectDialog : public Gtk::Dialog
{
public:
    explicit ConnectDialog(Gtk::Window& parent);
    std::optional<CloudStorageConfig> getResult() const;
    void prefillConfig(const CloudStorageConfig& config);
private:
    Gtk::Box m_contentBox;
    Gtk::Label m_apiUrlLabel;
    Gtk::Entry m_apiUrlEntry;
    Gtk::Label m_apiKeyLabel;
    Gtk::Entry m_apiKeyEntry;

    void initLayout();
};