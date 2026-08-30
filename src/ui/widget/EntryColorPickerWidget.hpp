#pragma once
#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>
#include <array>
#include "../../core/model/EntryColor.hpp"

class EntryColorPickerWidget : public Gtk::Box
{
public:
    EntryColorPickerWidget();
    void setSelectedColor(EntryColor color);
    [[nodiscard]] EntryColor getSelectedColor() const;
    sigc::signal<void(EntryColor)>& signalColorSelected();
private:
    static constexpr int SWATCH_SPACING = 4;
    static constexpr int SWATCH_SIZE = 16;
    std::array<Gtk::ToggleButton, 7> m_swatches;
    EntryColor m_selected = EntryColor::Default;
    sigc::signal<void(EntryColor)> m_signalColorSelected;
    void initLayout();
    void selectColor(EntryColor color);
};