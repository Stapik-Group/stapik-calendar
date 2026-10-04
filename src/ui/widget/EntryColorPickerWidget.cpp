#include "EntryColorPickerWidget.hpp"

#include "EntryColorStyle.hpp"

#include "stapik/locale/LocaleManager.hpp"

EntryColorPickerWidget::EntryColorPickerWidget() : Box(Gtk::Orientation::HORIZONTAL, SWATCH_SPACING)
{
    initLayout();
}

void EntryColorPickerWidget::initLayout()
{
    const auto& loc = LocaleManager::instance();

    m_colors.emplace_back(std::nullopt);
    for (const auto color : stapik::domain::allCategoryColors())
        m_colors.emplace_back(color);

    for (const auto color : m_colors)
    {
        auto& swatch = m_swatches.emplace_back();
        swatch.set_size_request(SWATCH_SIZE, SWATCH_SIZE);
        swatch.add_css_class("stapik-color-swatch");
        swatch.add_css_class(entryColorCssClass(color));
        if (color.has_value())
            swatch.set_tooltip_text(loc.translate(stapik::domain::categoryColorNameKey(*color)));
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
    for (std::size_t i = 0; i < m_colors.size(); ++i)
        m_swatches[i].set_active(m_colors[i] == color);
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
