#ifndef WALLREEL_CORE_PALETTE_DOMCOLOR_HPP
#define WALLREEL_CORE_PALETTE_DOMCOLOR_HPP

#include <QImage>

namespace WallReel::Core::Palette {

/**
 * @brief Get the dominant color of the given image.
 *
 * @param image The input image
 * @return QColor An empty QColor() case error occurs, otherwise the dominant color of the image
 */
QColor getDominantColor(const QImage& image);

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_CORE_PALETTE_DOMCOLOR_HPP
