#ifndef WALLREEL_CORE_PALETTE_MATCHCOLOR_HPP
#define WALLREEL_CORE_PALETTE_MATCHCOLOR_HPP

#include "data.hpp"

namespace WallReel::Core::Palette {

/**
 * @brief Find the best matching color from the candidates for the given target color.
 *
 * @param target
 * @param candidates
 * @return const ColorItem& The best matching color item, or an empty ColorItem if no candidates are provided
 */
const ColorItem& bestMatch(const QColor& target, const QList<ColorItem>& candidates);

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_CORE_PALETTE_MATCHCOLOR_HPP
