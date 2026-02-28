#ifndef WALLREEL_PALETTE_MANAGER_HPP
#define WALLREEL_PALETTE_MANAGER_HPP

#include <qcolor.h>

#include "Config/data.hpp"
#include "Image/manager.hpp"
#include "data.hpp"

namespace WallReel::Core::Palette {

class Manager : public QObject {
    Q_OBJECT

  public:
    Manager(const Config::ThemeConfigItems& config,
            Image::Manager& imageManager,
            QObject* parent = nullptr);

    // Properties

    const QList<PaletteItem>& availablePalettes() const { return m_palettes; }

    const QColor& color() const { return m_displayColor; }

    const QString& colorName() const { return m_displayColorName; }

    QVariant selectedPalette() const {
        if (m_selectedPalette) {
            return QVariant::fromValue(*m_selectedPalette);
        }
        return QVariant();
    }

    QVariant selectedColor() const {
        if (m_selectedColor) {
            return QVariant::fromValue(*m_selectedColor);
        }
        return QVariant();
    }

    void setSelectedPalette(const QVariant& paletteVar) {
        if (paletteVar.isNull() || !paletteVar.isValid()) {
            m_selectedPalette = std::nullopt;
        } else {
            m_selectedPalette = paletteVar.value<PaletteItem>();
        }
        m_selectedColor = std::nullopt;
        emit selectedPaletteChanged();
        emit selectedColorChanged();
    }

    void setSelectedColor(const QVariant& colorVar) {
        if (colorVar.isNull() || !colorVar.isValid()) {
            m_selectedColor = std::nullopt;
        } else {
            m_selectedColor = colorVar.value<ColorItem>();
        }
        emit selectedColorChanged();
    }

    // Getters

    /**
     * @brief Get the name of the currently selected palette
     *
     * @return QString The name of the currently selected palette, or an empty string if no palette is selected
     */
    QString getSelectedPaletteName() const {
        return m_selectedPalette ? m_selectedPalette->name : QString();
    }

    /**
     * @brief Get the name of the currently selected color
     *
     * @return QString The name of the currently selected color, or an empty string if the color does not have
     *         a pretty name
     */
    QString getCurrentColorName() const {
        return m_displayColorName;
    }

    /**
     * @brief Get the hex string of the currently selected color
     *
     * @return QString The hex string of the currently selected color, or an empty string if the color is invalid
     */
    QString getCurrentColorHex() const {
        return m_displayColor.isValid()
                   ? m_displayColor.name()
                   : QString();
    }

  public slots:
    void updateColor(const QString& imageId);

  signals:
    void selectedPaletteChanged();
    void selectedColorChanged();
    void colorChanged();
    void colorNameChanged();

  private:
    Image::Manager& m_imageManager;

    QList<PaletteItem> m_palettes;
    // Null means auto
    std::optional<PaletteItem> m_selectedPalette = std::nullopt;
    std::optional<ColorItem> m_selectedColor     = std::nullopt;

    QColor m_displayColor;
    QString m_displayColorName;
};

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_PALETTE_MANAGER_HPP
