#ifndef WALLREEL_PALETTE_MANAGER_HPP
#define WALLREEL_PALETTE_MANAGER_HPP

#include "../configmgr.hpp"
#include "data.hpp"

class PaletteManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<PaletteItem> availablePalettes READ availablePalettes CONSTANT)

  public:
    PaletteManager(const Config::PaletteConfigItems& config,
                   QObject* parent = nullptr);

    const QList<PaletteItem>& availablePalettes() const {
        return m_palettes;
    }

    Q_INVOKABLE PaletteItem getPalette(const QString& name) const {
        for (const auto& p : m_palettes) {
            if (p.name == name) return p;
        }
        return PaletteItem();
    }

  private:
    QList<PaletteItem> m_palettes;
};

#endif  // WALLREEL_PALETTE_MANAGER_HPP
