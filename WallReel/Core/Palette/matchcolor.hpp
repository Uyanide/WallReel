#ifndef WALLREEL_CORE_PALETTE_MATCHCOLOR_HPP
#define WALLREEL_CORE_PALETTE_MATCHCOLOR_HPP

#include "data.hpp"

namespace WallReel::Core::Palette {

const ColorItem& bestMatch(const QColor& target, const QList<ColorItem>& candidates);

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_CORE_PALETTE_MATCHCOLOR_HPP
