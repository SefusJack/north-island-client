#include "client/rendering/hud/FrameGraph.h"

#include "gfx/TextRenderer.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>

namespace client {

    namespace {

        constexpr int kMargin = 4; // px: gap to the screen edge and to the function buttons
        constexpr int kFunctionButtonsHeight = 36; // px: the function buttons' bar at the top right, at HUD size 100
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
        const gfx::TextStyle kText{{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt, true}; // white text with shadow, changing numbers
        constexpr std::uint64_t kWordEveryMs = 250; // how often the numbers are worded anew

        /// @brief Words a share of 100 ("14%"), or "n/a" when it is not known.
        /// @param percent The share.
        /// @return The text.
        std::string percentText(const std::optional<double>& percent);

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

    namespace {

        std::string percentText(const std::optional<double>& percent) { return percent ? format("%.0f%%", *percent) : std::string("n/a"); }

    }

    void FrameGraph::draw(
        gfx::Canvas& canvas,
        const FrameStats& stats,
        const std::string& backend,
        bool gpuMeasured,
        int screenWidth,
        int hudPercent,
        int level,
        const LoadReading& load,
        std::uint64_t nowMs
    ) {
        if (level < 1) {
            return;
        }
        if (level != m_linesLevel || m_lines.empty() || nowMs - m_wordedMs >= kWordEveryMs) {
            m_linesLevel = level;
            m_wordedMs = nowMs;
            m_lines.clear();
            const FrameSample mean = stats.average();
            const std::string gpuTime = !gpuMeasured ? std::string("n/a") : mean.gpuMs ? format("%.2f ms", *mean.gpuMs) : std::string("...");
            // with one core "of all cores" and "of one core" are the same number
            const std::string cpuUse = percentText(load.cpuPercent) + (load.cores > 1 && load.corePercent ? " (core " + percentText(load.corePercent) + ")" : std::string());
            const std::string ramText = load.ramMb ? format("%.0f MB", *load.ramMb) : std::string("n/a");
            if (level == 1) {
                m_lines.push_back(format("%.0f fps", stats.fps()));
            } else if (level == 2) {
                m_lines.push_back(format("fps      %.0f", stats.fps()));
                m_lines.push_back("cpu use  " + cpuUse);
                m_lines.push_back("gpu use  " + percentText(load.gpuPercent));
                m_lines.push_back("ram use  " + ramText);
                m_lines.push_back(format("cpu time %.2f ms", mean.busyMs()));
                m_lines.push_back("gpu time " + gpuTime);
            } else {
                m_lines.push_back(format("frame %.1f ms  %.0f fps  1%% low %.0f", mean.totalMs, stats.fps(), stats.lowFps()));
                m_lines.push_back("cpu " + cpuUse + " gpu " + percentText(load.gpuPercent) + " ram " + ramText);
                m_lines.push_back(format("cpu %.1f: upd %.1f draw %.1f pres %.1f", mean.busyMs(), mean.updateMs, mean.drawMs, mean.presentMs));
                m_lines.push_back("gpu " + gpuTime + "  wait " + format("%.1f", mean.waitMs) + "  " + backend);
            }
        }
        const int textLines = static_cast<int>(m_lines.size());
        constexpr int kLegendLines = 1; // line of colour legend below the graph
        const bool graph = level >= 3;
        int width = kWidth;
        if (!graph) {
            std::size_t longestLine = 0;
            for (const std::string& line : m_lines) {
                longestLine = std::max(longestLine, line.size());
            }
            width = static_cast<int>(longestLine) * gfx::TextRenderer::charWidth(kText.effect) + kPadding * 2;
        }
        // paddings: above the text and below it; with the graph one more below the graph
        const int height = graph ? kPadding * 3 + textLines * kLineHeight + kGraphHeight + kLegendLines * kLineHeight : kPadding * 2 + textLines * kLineHeight;
        // at the right edge, below the function buttons (FunctionButtons.xml, the bar across the top right corner)
        const int buttonsBottom = kFunctionButtonsHeight * hudPercent / 100;
        const core::Rect box{screenWidth - width - kMargin, buttonsBottom + kMargin, width, height};
        canvas.fillRect(box, kBackground);

        const int textX = box.x + kPadding;
        int y = box.y + kPadding;
        for (const std::string& line : m_lines) {
            canvas.drawText({textX, y}, line, kText);
            y += kLineHeight;
        }
        if (!graph) {
            return;
        }
        y += kPadding;

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
