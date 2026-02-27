#ifndef WALLREEL_PALETTE_DATA_HPP
#define WALLREEL_PALETTE_DATA_HPP

#include <qcolor.h>

#include <QColor>
#include <QList>
#include <QObject>
#include <QString>

namespace WallReel::Core::Palette {

struct ColorItem {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)

  public:
    QString name;
    QColor color;

    bool operator==(const ColorItem& other) const {
        return name == other.name;
    }

    bool isValid() const {
        return !name.isEmpty() && color.isValid();
    }
};

struct PaletteItem {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QList<ColorItem> colors MEMBER colors CONSTANT)

  public:
    QString name;
    // We need to keep the order of colors, and the number of colors is usually limited,
    // so a flat list and O(n) lookup should be fine here.
    QList<ColorItem> colors;

    Q_INVOKABLE QColor getColor(const QString& colorName) const {
        for (const auto& entry : colors) {
            if (entry.name == colorName) return entry.color;
        }
        return QColor();
    }

    ColorItem getColorItem(const QString& colorName) const {
        for (const auto& entry : colors) {
            if (entry.name == colorName) return entry;
        }
        return ColorItem();
    }

    bool operator==(const PaletteItem& other) const {
        return name == other.name;
    }

    bool isValid() const {
        return !name.isEmpty() && !colors.isEmpty();
    }
};

}  // namespace WallReel::Core::Palette

Q_DECLARE_METATYPE(WallReel::Core::Palette::ColorItem)
Q_DECLARE_METATYPE(WallReel::Core::Palette::PaletteItem)

#endif  // WALLREEL_PALETTE_DATA_HPP
