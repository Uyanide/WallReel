#ifndef WALLREEL_PALETTE_MANAGER_HPP
#define WALLREEL_PALETTE_MANAGER_HPP

#include <qcolor.h>

#include "Config/data.hpp"
#include "Image/model.hpp"
#include "data.hpp"

namespace WallReel::Core::Palette {

class Manager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<PaletteItem> availablePalettes READ availablePalettes CONSTANT)
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)
    Q_PROPERTY(QString colorName READ colorName NOTIFY colorNameChanged)

  public:
    Manager(const Config::PaletteConfigItems& config,
            Image::Model& imageModel,
            QObject* parent = nullptr);

    const QList<PaletteItem>& availablePalettes() const {
        return m_palettes;
    }

    const QColor& color() const {
        return m_displayColor;
    }

    const QString& colorName() const {
        return m_displayColorName;
    }

    Q_INVOKABLE void setSelectedPalette(const QVariant& paletteVar) {
        if (paletteVar.isNull() || !paletteVar.isValid()) {
            m_selectedPalette = std::nullopt;
        } else {
            m_selectedPalette = paletteVar.value<PaletteItem>();
        }
        updateColor();
    }

    Q_INVOKABLE void setSelectedColor(const QVariant& colorVar) {
        if (colorVar.isNull() || !colorVar.isValid()) {
            m_selectedColor = std::nullopt;
        } else {
            m_selectedColor = colorVar.value<ColorItem>();
        }
        updateColor();
    }

    QString getSelectedPaletteName() const {
        return m_selectedPalette ? m_selectedPalette->name : QString();
    }

    QString getCurrentColorName() const {
        return m_displayColorName;
    }

    QString getCurrentColorHex() const {
        return m_displayColor.isValid()
                   ? m_displayColor.name()
                   : QString();
    }

  public slots:
    void updateColor();

  signals:
    void colorChanged();
    void colorNameChanged();

  private:
    Image::Model& m_imageModel;

    QList<PaletteItem> m_palettes;
    std::optional<PaletteItem> m_selectedPalette = std::nullopt;
    std::optional<ColorItem> m_selectedColor     = std::nullopt;

    QColor m_displayColor;
    QString m_displayColorName;
};

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_PALETTE_MANAGER_HPP
