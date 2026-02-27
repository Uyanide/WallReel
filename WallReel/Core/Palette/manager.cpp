#include "manager.hpp"

#include "Palette/matchcolor.hpp"
#include "Utils/misc.hpp"
#include "logger.hpp"
#include "predefined.hpp"

WallReel::Core::Palette::Manager::Manager(
    const Config::ThemeConfigItems& config,
    Image::Model& imageModel,
    QObject* parent) : QObject(parent), m_imageModel(imageModel) {
    connect(&m_imageModel, &Image::Model::focusedImageChanged, this, &Manager::updateColor);

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

    // Set default palette if specified
    if (!config.defaultPalette.isEmpty()) {
        for (const auto& p : m_palettes) {
            if (p.name == config.defaultPalette) {
                m_selectedColor   = std::nullopt;
                m_selectedPalette = p;
                emit selectedColorChanged();
                emit selectedPaletteChanged();
                break;
            }
        }
    }
}

void WallReel::Core::Palette::Manager::updateColor() {
    bool hasResult = false;
    Utils::Defer defer([&]() {
        if (!hasResult) {
            m_displayColor     = QColor();
            m_displayColorName = "";
        }
        emit colorChanged();
        emit colorNameChanged();
    });
    auto imageData = m_imageModel.focusedImage();
    if (!imageData || !imageData->isValid()) {
        return;
    }
    // No palette selected, use dominant color
    if (!m_selectedPalette.has_value()) {
        m_displayColor     = imageData->getDominantColor();
        m_displayColorName = "";
        hasResult          = true;
        return;
    }
    // Only palette selected, use the colosest color in the palette
    if (!m_selectedColor.has_value()) {
        auto cached = imageData->getCachedColor(m_selectedPalette->name);
        if (cached.has_value()) {
            auto found = m_selectedPalette.value().getColorItem(cached.value());
            if (found.isValid()) {
                Logger::debug("Using cached color match for image " + imageData->getFileName() +
                              ": " + found.name);
                m_displayColor     = found.color;
                m_displayColorName = found.name;
                hasResult          = true;
                return;
            }
        }
        auto matched = bestMatch(
            imageData->getDominantColor(),
            m_selectedPalette.value().colors);
        // Use dominant color if no valid match found (possibly empty palette)
        if (!matched.isValid()) {
            Logger::debug("No valid color match found for image " + imageData->getFileName() +
                          ", using dominant color: " + imageData->getDominantColor().name());
            m_displayColor     = imageData->getDominantColor();
            m_displayColorName = "";
            hasResult          = true;
            return;
        }
        Logger::debug("Computed color match for image " + imageData->getFileName() + ": " +
                      matched.name);
        imageData->cacheColor(m_selectedPalette->name, matched.name);
        m_displayColor     = matched.color;
        m_displayColorName = matched.name;
        hasResult          = true;
        return;
    }
    // Both are set, use them
    m_displayColor     = m_selectedColor.value().color;
    m_displayColorName = m_selectedColor.value().name;
    hasResult          = true;
}
