#include "client/rendering/hud/GaugeWipe.h"

#include <algorithm>
#include <cmath>

namespace client {

    int gaugePercent(long long value, long long maximum) {
        if (maximum <= 0) {
            return 100; // no maximum shows a full gauge
        }
        int percent = static_cast<int>(std::ceil(static_cast<double>(value) / static_cast<double>(maximum) * 100)); // fraction to percent
        if (percent <= 0 && value > 0) {
            percent = 1;
        }
        return std::clamp(percent, 0, 100); // percent range
    }

    formats::Image wipeGauge(const formats::Image& gauge, int percent, int detail) {
        const bool narrow = gauge.width() / detail <= 100; // HP gauges are up to 100 game pixels wide, SP gauges wider
        const int pivotX = narrow ? 46 : 58; // x of the wipe line's pivot for HP / SP
        const int offsetX = narrow ? 9 : 21; // x where the wipe line starts for HP / SP
        // the line runs 82 pixels across and 24 pixels up over the full percent range
        const int lineX = 82 * percent / 100 + offsetX;
        const int lineY = 24 - 24 * percent / 100;
        const double slope = lineX == pivotX ? 999.0 : static_cast<double>(lineY) / (lineX - pivotX); // 999: stand-in for a vertical line
        formats::Image result = gauge;
        for (int y = 0; y < result.height(); ++y) {
            for (int x = 0; x < result.width(); ++x) {
                core::Color& pixel = result.at(x, y);
                if (pixel.a == 0) {
                    continue;
                }
                const double gameX = (x + 0.5) / detail - 0.5;
                const double gameY = (y + 0.5) / detail - 0.5;
                const double side = (gameX - pivotX) * slope - gameY;
                const bool keep = slope >= 0 ? side < 0 : side > 0;
                if (!keep) {
                    pixel = {0, 0, 0, 0};
                }
            }
        }
        return result;
    }

}
