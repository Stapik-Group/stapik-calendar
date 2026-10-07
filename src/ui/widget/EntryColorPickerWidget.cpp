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
        if (m_swatches.size() > 1)
            swatch.set_group(m_swatches.front());
        swatch.set_size_request(SWATCH_SIZE, SWATCH_SIZE);
        swatch.add_css_class("stapik-color-swatch");
        swatch.add_css_class(entryColorCssClass(color));
        if (color.has_value())
            swatch.set_tooltip_text(loc.translate(stapik::domain::categoryColorNameKey(*color)));
        swatch.signal_toggled().connect([this, color, &swatch]
        {
            if (swatch.get_active() && color != m_selected)
                selectColor(color);
        });
        append(swatch);
    }
    m_swatches.front().set_active(true);
}

void EntryColorPickerWidget::selectColor(const EntryColor color)
{
    // Set first: the toggled handler of the activated swatch must see no change and stay silent.
    m_selected = color;
    for (std::size_t i = 0; i < m_colors.size(); ++i)
    {
        if (m_colors[i] == color)
            m_swatches[i].set_active(true);
    }
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
