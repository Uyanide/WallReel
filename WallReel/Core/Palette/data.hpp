#ifndef WALLREEL_PALETTE_DATA_HPP
#define WALLREEL_PALETTE_DATA_HPP

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
};

struct PaletteItem {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QList<ColorItem> colors MEMBER colors CONSTANT)

  public:
    QString name;
    QList<ColorItem> colors;

    Q_INVOKABLE QColor getColor(const QString& colorName) const {
        for (const auto& entry : colors) {
            if (entry.name == colorName) return entry.color;
        }
        return QColor();
    }
};

}  // namespace WallReel::Core::Palette

Q_DECLARE_METATYPE(WallReel::Core::Palette::ColorItem)
Q_DECLARE_METATYPE(WallReel::Core::Palette::PaletteItem)

#endif  // WALLREEL_PALETTE_DATA_HPP
