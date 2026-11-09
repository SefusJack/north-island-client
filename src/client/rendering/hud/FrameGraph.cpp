#include "client/rendering/hud/FrameGraph.h"

#include "gfx/TextRenderer.h"

#include <algorithm>
#include <cstring>
#include <cstdio>

namespace client {

    namespace {

        constexpr int kMargin = 4; // px: gap to the screen edge
        constexpr int kPadding = 6; // px: inner padding of the panel
        constexpr int kLineHeight = 15; // px per text line
        constexpr int kGraphHeight = 80; // px: height of the bar graph

        const core::Color kBackground{0, 0, 0, 170}; // translucent black panel, RGBA
        const core::Color kEvents{230, 230, 230, 255}; // light grey: event handling time
        const core::Color kUpdate{80, 160, 255, 255}; // blue: update time
        const core::Color kDraw{80, 220, 120, 255}; // green: draw time
        const core::Color kPresent{255, 170, 60, 255}; // orange: present time
        const core::Color kWait{140, 140, 140, 255}; // grey: frame-limiter wait
        const core::Color kGpu{255, 80, 255, 255}; // magenta: GPU time dot
        const core::Color kGuide{255, 255, 255, 60}; // faint white guide lines
        const gfx::TextStyle kText{{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt}; // white text with shadow

        /// @brief Formats up to four numbers with a printf pattern.
        /// @param pattern printf pattern taking doubles.
        /// @param a First value.
        /// @param b Second value.
        /// @param c Third value.
        /// @param d Fourth value.
        /// @return The formatted text.
        std::string format(
            const char* pattern,
            double a,
            double b = 0,
            double c = 0,
            double d = 0
        ) {
            char line[128]; // line buffer size in characters
            std::snprintf(line, sizeof line, pattern, a, b, c, d);
            return line;
        }

        /// @brief Draws one bar segment of the graph column.
        /// @param canvas Target canvas.
        /// @param x Column x position.
        /// @param bottom Y position of the graph bottom.
        /// @param base Milliseconds already stacked below the segment.
        /// @param ms Milliseconds of the segment.
        /// @param scaleMs Milliseconds at the top of the graph.
        /// @param color Segment colour.
        void segment(
            gfx::Canvas& canvas,
            int x,
            int bottom,
            double base,
            double ms,
            double scaleMs,
            core::Color color
        ) {
            const int from = static_cast<int>(std::min(base, scaleMs) * kGraphHeight / scaleMs);
            const int to = static_cast<int>(std::min(base + ms, scaleMs) * kGraphHeight / scaleMs);
            if (to > from) {
                canvas.fillRect({x, bottom - to, 1, to - from}, color);
            }
        }

    }

    void FrameGraph::draw(
        gfx::Canvas& canvas,
        const FrameStats& stats,
        const std::string& backend,
        bool gpuMeasured,
        int screenWidth
    ) {
        constexpr int kTextLines = 3; // lines of numbers above the graph
        constexpr int kLegendLines = 1; // line of colour legend below the graph
        // 3 paddings: above the text, above the graph, below the graph
        const int height = kPadding * 3 + kTextLines * kLineHeight + kGraphHeight + kLegendLines * kLineHeight;
        const core::Rect box{screenWidth - kWidth - kMargin, kMargin, kWidth, height};
        canvas.fillRect(box, kBackground);

        const FrameSample mean = stats.average();
        const int textX = box.x + kPadding;
        int y = box.y + kPadding;
        canvas.drawText({textX, y}, format("frame %.1f ms  %.0f fps  1%% low %.0f", mean.totalMs, stats.fps(), stats.lowFps()), kText);
        y += kLineHeight;
        canvas.drawText({textX, y}, format("cpu %.1f: upd %.1f draw %.1f pres %.1f", mean.busyMs(), mean.updateMs, mean.drawMs, mean.presentMs), kText);
        y += kLineHeight;
        const std::string gpu = !gpuMeasured ? "gpu n/a" : mean.gpuMs ? format("gpu %.2f ms", *mean.gpuMs) : std::string("gpu ...");
        canvas.drawText({textX, y}, gpu + "  wait " + format("%.1f", mean.waitMs) + "  " + backend, kText);
        y += kLineHeight + kPadding;

        double longest = 0;
        for (std::size_t i = 0; i < stats.size(); ++i) {
            longest = std::max(longest, stats.at(i).totalMs);
        }
        const double scaleMs = longest > 50 ? 100 : longest > 10 ? 50 : 10; // graph scale in ms: 100, 50 or 10 depending on the longest frame
        const int graphLeft = box.x + (kWidth - static_cast<int>(FrameStats::kCapacity)) / 2;
        const int bottom = y + kGraphHeight;
        for (const double guideMs : {1000.0 / 240, 1000.0 / 144, 1000.0 / 60, 1000.0 / 30}) { // guide lines at 240, 144, 60 and 30 fps (ms per frame)
            if (guideMs > scaleMs || (scaleMs > 10 && guideMs < 10)) { // guides under 10 ms are hidden on larger scales
                continue;
            }
            canvas.fillRect({graphLeft, bottom - static_cast<int>(guideMs * kGraphHeight / scaleMs), static_cast<int>(FrameStats::kCapacity), 1}, kGuide);
        }
        const int first = graphLeft + static_cast<int>(FrameStats::kCapacity - stats.size());
        for (std::size_t i = 0; i < stats.size(); ++i) {
            const FrameSample& s = stats.at(i);
            const int x = first + static_cast<int>(i);
            double base = 0;
            for (const auto& [ms, color] :
                 {std::pair{s.eventsMs, kEvents},
                  std::pair{s.updateMs, kUpdate},
                  std::pair{s.drawMs, kDraw},
                  std::pair{s.presentMs, kPresent},
                  std::pair{s.waitMs, kWait}}) {
                segment(canvas, x, bottom, base, ms, scaleMs, color);
                base += ms;
            }
            if (s.gpuMs) {
                const int at = static_cast<int>(std::min(*s.gpuMs, scaleMs) * kGraphHeight / scaleMs);
                canvas.fillRect({x, bottom - at - 1, 1, 2}, kGpu); // GPU dot is 1 pixel wide and 2 high
            }
        }
        y = bottom + kPadding;

        int x = textX;
        for (const auto& [word, color] :
             {std::pair{"upd", kUpdate}, std::pair{"draw", kDraw}, std::pair{"pres", kPresent}, std::pair{"wait", kWait}, std::pair{"gpu", kGpu}}) {
            canvas.fillRect({x, y + 4, 8, 8}, color); // legend swatch: 4 pixels down, 8 x 8 pixels
            x += 11; // swatch width plus gap before the word
            canvas.drawText({x, y}, word, kText);
            x += static_cast<int>(std::strlen(word)) * gfx::TextRenderer::charWidth(kText.effect) + 8; // gap before the next legend entry
        }
        canvas.drawText({x, y}, format("%.0f ms", scaleMs), kText);
    }

}
