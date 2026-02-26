#include "matchcolor.hpp"

#include <QDebug>
#include <cmath>
#include <limits>

namespace WallReel::Core::Palette {

const ColorItem& bestMatch(const QColor& target, const QList<ColorItem>& candidates) {
    if (candidates.isEmpty() || !target.isValid()) {
        static ColorItem emptyItem;
        return emptyItem;
    }

    int target_r = target.red();
    int target_g = target.green();
    int target_b = target.blue();

    int target_h    = target.hsvHue();
    double target_s = target.hsvSaturationF();

    const ColorItem* closest_flavor = nullptr;
    double min_distance             = std::numeric_limits<double>::max();

    for (const auto& candidate : candidates) {
        QColor p_color = candidate.color;
        int p_r        = p_color.red();
        int p_g        = p_color.green();
        int p_b        = p_color.blue();

        int p_h = p_color.hsvHue();

        // RGB distance with weighting
        double rmean = (target_r + p_r) / 2.0;
        double dr    = target_r - p_r;
        double dg    = target_g - p_g;
        double db    = target_b - p_b;

        double rgb_distance = std::sqrt((2.0 + rmean / 256.0) * dr * dr + 4.0 * dg * dg +
                                        (2.0 + (255.0 - rmean) / 256.0) * db * db);

        // Hue difference (with wrapping)
        double hue_diff = 0.0;
        if (target_h != -1 && p_h != -1) {
            hue_diff = std::abs(target_h - p_h);
            if (hue_diff > 180.0) {
                hue_diff = 360.0 - hue_diff;
            }
        }

        // Increase hue weight when saturation is high
        double hue_weight = (target_s > 0.20) ? 2.0 : 0.5;

        double total_distance = rgb_distance + (hue_diff * hue_weight * 3.0);

        if (total_distance < min_distance) {
            min_distance   = total_distance;
            closest_flavor = &candidate;
        }
    }

    if (closest_flavor) {
        return *closest_flavor;
    }

    static ColorItem emptyItem;
    return emptyItem;
}

}  // namespace WallReel::Core::Palette
