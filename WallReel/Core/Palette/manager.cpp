#include "manager.hpp"

#include "predefined.hpp"

WallReel::Core::Palette::Manager::Manager(
    const Config::PaletteConfigItems& config,
    QObject* parent) : QObject(parent) {
    // The new ones overrides the old ones, use a hashtable to track
    // the latest index of each palette name, then only insert the
    // ones whose index matches the latest index in the hashtable
    QHash<QString, int> lastSeen;
    lastSeen.reserve(preDefinedPalettes.size() + config.palettes.size());

    for (int i = 0; i < preDefinedPalettes.size(); i++) {
        lastSeen[preDefinedPalettes[i].name] = i;
    }
    for (int i = 0; i < config.palettes.size(); i++) {
        lastSeen[config.palettes[i].name] = preDefinedPalettes.size() + i;
    }

    m_palettes.reserve(lastSeen.size());
    for (int i = 0; i < preDefinedPalettes.size(); i++) {
        const auto& p = preDefinedPalettes[i];
        if (lastSeen[p.name] == i) {
            m_palettes.append({p.name, p.colors});
        }
    }
    for (int i = 0; i < config.palettes.size(); i++) {
        const auto& p = config.palettes[i];
        if (lastSeen[p.name] == preDefinedPalettes.size() + i) {
            auto newP = PaletteItem{p.name, {}};
            newP.colors.reserve(p.colors.size());
            for (const auto& c : p.colors) {
                if (!c.value.isValid()) {
                    continue;
                }
                newP.colors.append({c.name, c.value});
            }
            m_palettes.append(newP);
        }
    }
}
