#include "EntryColorPickerWidget.hpp"
#include "../../core/util/EntryColorUtils.hpp"

EntryColorPickerWidget::EntryColorPickerWidget() : Box(Gtk::Orientation::HORIZONTAL, SWATCH_SPACING)
{
    initLayout();
}

void EntryColorPickerWidget::initLayout()
{
    const auto& colors = EntryColorUtils::allColors();
    for (std::size_t i = 0; i < colors.size(); ++i)
    {
        const auto color = colors[i];
        auto& swatch = m_swatches[i];
        swatch.set_size_request(SWATCH_SIZE, SWATCH_SIZE);
        swatch.add_css_class("color-swatch");
        if (const auto cssClass = EntryColorUtils::cssClass(color); !cssClass.empty())
            swatch.add_css_class(cssClass);
        swatch.signal_toggled().connect([this, color, &swatch]
        {
            if (swatch.get_active())
                selectColor(color);
        });
        append(swatch);
    }
    m_swatches.front().set_active(true);
}

void EntryColorPickerWidget::selectColor(const EntryColor color)
{
    m_selected = color;
    const auto& colors = EntryColorUtils::allColors();
    for (std::size_t i = 0; i < colors.size(); ++i)
        m_swatches[i].set_active(colors[i] == color);
    m_signalColorSelected.emit(color);
}

void EntryColorPickerWidget::setSelectedColor(const EntryColor color)
{
    selectColor(color);
}

EntryColor EntryColorPickerWidget::getSelectedColor() const
{
    return m_selected;
}

sigc::signal<void(EntryColor)>& EntryColorPickerWidget::signalColorSelected()
{
    return m_signalColorSelected;
}