#include "domcolor.hpp"

#include "logger.hpp"

WALLREEL_DECLARE_SENDER("DomColor")

static constexpr int scaleMaxWidth  = 128;
static constexpr int scaleMaxHeight = 128;
// See /misc/ColorArgTest/index.html for visualizing the effect of different powers
static constexpr double weightSaturationPower = 2.0;
static constexpr double weightLightnessPower  = 2.0;
static constexpr int numBins                  = 36;  // 360 degrees / 10 degrees per bin

static double getWeight(const QColor& color) {
    int h, s, l;
    color.getHsl(&h, &s, &l);
    if (h < 0) return 0.0;  // Skip undefined hues

    double sNorm = s / 255.0;

    double dist  = std::abs(l - 128.0) / 128.0;
    double lNorm = 1.0 - dist;

    // // satPower=2, lightPower=2
    // return std::pow(sNorm, weightSaturationPower) * std::pow(lNorm, weightLightnessPower);
    // Since these are just simple squares, no need to use std::pow() here
    return sNorm * sNorm * lNorm * lNorm;
}

static QColor getWeightedDominantColor(const QImage& image) {
    if (image.isNull()) {
        WR_WARN("Image is null");
        return QColor();
    }

    // QImage scaledImg = image.scaled(128, 128, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage* scaledImg = nullptr;
    if (image.width() > ::scaleMaxWidth || image.height() > ::scaleMaxHeight) {
        scaledImg = new QImage(image.scaled(::scaleMaxWidth, ::scaleMaxHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        scaledImg = new QImage(image);
    }

    struct HueBin {
        double totalWeight = 0.0;
        double sumR        = 0.0;
        double sumG        = 0.0;
        double sumB        = 0.0;
    };

    QVector<HueBin> bins(::numBins);

    for (int y = 0; y < scaledImg->height(); ++y) {
        for (int x = 0; x < scaledImg->width(); ++x) {
            QColor color = scaledImg->pixelColor(x, y);

            int hue = color.hue();
            if (hue < 0) continue;  // Skip grayscale pixels

            double weight = getWeight(color);
            // Filter out low-weight pixels to reduce noise
            if (weight < 0.05) continue;

            int binIndex = (hue / 10) % ::numBins;

            bins[binIndex].totalWeight += weight;
            bins[binIndex].sumR += color.redF() * weight;
            bins[binIndex].sumG += color.greenF() * weight;
            bins[binIndex].sumB += color.blueF() * weight;
        }
    }
    delete scaledImg;

    // Find the bin with the highest total weight
    int maxBinIndex  = -1;
    double maxWeight = 0.0;
    for (int i = 0; i < ::numBins; ++i) {
        if (bins[i].totalWeight > maxWeight) {
            maxWeight   = bins[i].totalWeight;
            maxBinIndex = i;
        }
    }

    // If all filtered (e.g. grayscale image), return a default color
    if (maxBinIndex == -1 || maxWeight <= 0.0) {
        return QColor(Qt::gray);
    }

    // Calculate the average color for the winning bin
    const HueBin& winningBin = bins[maxBinIndex];

    int finalR = std::round((winningBin.sumR / winningBin.totalWeight) * 255.0);
    int finalG = std::round((winningBin.sumG / winningBin.totalWeight) * 255.0);
    int finalB = std::round((winningBin.sumB / winningBin.totalWeight) * 255.0);

    return QColor(finalR, finalG, finalB);
}

QColor WallReel::Core::Palette::getDominantColor(const QImage& image) {
    return ::getWeightedDominantColor(image);
}
