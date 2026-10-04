#pragma once
#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>
#include <deque>
#include <vector>
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
    std::vector<EntryColor> m_colors;
    std::deque<Gtk::ToggleButton> m_swatches;
    EntryColor m_selected;
    sigc::signal<void(EntryColor)> m_signalColorSelected;
    void initLayout();
    void selectColor(EntryColor color);
};
