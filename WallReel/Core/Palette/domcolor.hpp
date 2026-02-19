#ifndef WALLREEL_CORE_PALETTE_DOMCOLOR_HPP
#define WALLREEL_CORE_PALETTE_DOMCOLOR_HPP

#include <QImage>

namespace WallReel::Core::Palette {

QColor getDominantColor(const QImage& image);

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_CORE_PALETTE_DOMCOLOR_HPP
