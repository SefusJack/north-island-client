#include "client/rendering/characters/FigureShadow.h"

#include "core/Rect.h"
#include "gfx/PictureLibrary.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace client {

    namespace {

        bool g_hidden = false; // the map's lighting casts the shadows itself
        int g_lift = 0; // how high over the ground the figures drawn now are

    }

    void hideFigureShadows(bool hidden) { g_hidden = hidden; }

    bool figureShadowsHidden() { return g_hidden; }

    void liftFigures(int lift) { g_lift = lift; }

    FigureEcho& figureEcho() {
        static FigureEcho echo;
        return echo;
    }

    namespace {

        // what a frame of an effect's picture shines like, worked out once from its pixels
        struct Shine {
            float r = 0;
            float g = 0;
            float b = 0;
            float strength = 0;
            float x = 0; // the middle of its light, in the frame
            float y = 0;
            float spread = 0;
        };
        struct Sheet {
            int frameHeight = 0;
            std::vector<Shine> frames;
        };

        /// @brief Works out what each frame of an effect's picture shines like.
        /// @param image The picture's pixels.
        /// @param frameHeight The height of a frame.
        /// @param translucent True when it is drawn as light added to the screen.
        /// @return A shine per frame.
        std::vector<Shine> shinesOf(const formats::Image& image, int frameHeight, bool translucent) {
            std::vector<Shine> frames;
            constexpr int kStep = 2; // every second pixel is looked at
            constexpr float kDim = 24; // a pixel of an added light under this is nothing
            constexpr float kBright = 150; // a pixel of a solid picture shines from this on
            constexpr float kFull = 170; // the square root of the bright pixels a light of full strength has
            constexpr float kSpark = 0.2f; // up to this much is a spark of a hit, which lights nothing
            const int count = std::max(1, image.height() / frameHeight);
            for (int frame = 0; frame < count; ++frame) {
                double all = 0;
                double sums[3] = {0, 0, 0};
                double sumX = 0;
                double sumY = 0;
                double sumXX = 0;
                double sumYY = 0;
                for (int y = 0; y < frameHeight && frame * frameHeight + y < image.height(); y += kStep) {
                    for (int x = 0; x < image.width(); x += kStep) {
                        const core::Color pixel = image.at(x, frame * frameHeight + y);
                        if (pixel.a == 0) {
                            continue;
                        }
                        const float most = static_cast<float>(std::max({pixel.r, pixel.g, pixel.b}));
                        const float weight = translucent ? (most < kDim ? 0.0f : most / 255.0f) : std::max(0.0f, most - kBright) / (255.0f - kBright);
                        if (weight <= 0) {
                            continue;
                        }
                        all += weight;
                        sums[0] += weight * pixel.r;
                        sums[1] += weight * pixel.g;
                        sums[2] += weight * pixel.b;
                        sumX += weight * x;
                        sumY += weight * y;
                        sumXX += weight * x * x;
                        sumYY += weight * y * y;
                    }
                }
                Shine shine;
                const double bright = all * kStep * kStep;
                if (bright >= 40) { // less is a speck
                    const double top = std::max({sums[0], sums[1], sums[2], 1.0});
                    shine.r = static_cast<float>(sums[0] / top);
                    shine.g = static_cast<float>(sums[1] / top);
                    shine.b = static_cast<float>(sums[2] / top);
                    shine.strength = std::max(0.0f, std::min(1.0f, static_cast<float>(std::sqrt(bright)) / kFull) - kSpark) / (1 - kSpark);
                    shine.x = static_cast<float>(sumX / all);
                    shine.y = static_cast<float>(sumY / all);
                    shine.spread = static_cast<float>(std::sqrt(std::max(0.0, sumXX / all - (sumX / all) * (sumX / all)) + std::max(0.0, sumYY / all - (sumY / all) * (sumY / all))));
                }
                frames.push_back(shine);
            }
            return frames;
        }

    }

    std::vector<EffectGlow>& effectGlows() {
        static std::vector<EffectGlow> glows;
        return glows;
    }

    void noteEffectGlow(gfx::Canvas& canvas, gfx::PictureId picture, const std::string& name, core::Rect source, core::Point at, bool translucent) {
        if (!figureEcho().active || !picture.isValid() || source.width <= 0 || source.height <= 0) {
            return;
        }
        static std::unordered_map<int, Sheet> known;
        Sheet& sheet = known[picture.value];
        if (sheet.frameHeight != source.height) {
            sheet.frameHeight = source.height;
            sheet.frames.clear();
            if (const auto image = canvas.pictures().decode(name)) {
                sheet.frames = shinesOf(*image, source.height, translucent);
            }
        }
        const std::size_t frame = static_cast<std::size_t>(std::max(0, source.y) / source.height);
        if (frame >= sheet.frames.size() || sheet.frames[frame].strength <= 0) {
            return;
        }
        const Shine& shine = sheet.frames[frame];
        const core::Point origin = canvas.origin();
        EffectGlow glow;
        glow.at = {at.x + origin.x + static_cast<int>(shine.x) - source.x, at.y + origin.y + static_cast<int>(shine.y)};
        glow.radius = static_cast<int>(std::clamp(shine.spread * 2.2f + 50, 70.0f, 300.0f));
        glow.r = shine.r * shine.strength;
        glow.g = shine.g * shine.strength;
        glow.b = shine.b * shine.strength;
        effectGlows().push_back(glow);
    }

    void beginFigureEcho() {
        FigureEcho& echo = figureEcho();
        echo.sprites.clear();
        echo.covers.clear();
        echo.things.clear();
        echo.casters.clear();
        effectGlows().clear();
        echo.figures.clear();
        echo.active = true;
    }

    void noteFigureCovers(const gfx::RenderDevice& device) {
        FigureEcho& echo = figureEcho();
        if (echo.active && device.spriteEcho() == nullptr) {
            device.setSpriteEcho(&echo.covers);
        }
    }

    std::size_t beginFigureThing() { return figureEcho().covers.size(); }

    void endFigureThing(std::size_t first) {
        FigureEcho& echo = figureEcho();
        if (echo.active && echo.covers.size() > first) {
            echo.things.emplace_back(first, echo.covers.size());
        }
    }

    void noteFigureCaster(std::size_t first, core::Point feet, gfx::PictureId picture, core::Point at, int frameTop) {
        FigureEcho& echo = figureEcho();
        if (echo.active && echo.covers.size() > first) {
            echo.casters.push_back({feet, first, echo.covers.size(), picture, at, frameTop});
        }
    }

    void endFigureEcho(const gfx::RenderDevice& device) {
        figureEcho().active = false;
        device.setSpriteEcho(nullptr);
    }

    FigureEchoScope::FigureEchoScope(const gfx::Canvas& canvas, core::Point feet) {
        FigureEcho& echo = figureEcho();
        if (!echo.active || (canvas.device().spriteEcho() != nullptr && canvas.device().spriteEcho() != &echo.covers)) { // not noted, or a part of a figure that is noted already
            return;
        }
        m_device = &canvas.device();
        m_lift = g_lift;
        m_feet = feet + canvas.origin() + core::Point{0, m_lift};
        m_first = echo.sprites.size();
        m_device->setSpriteEcho(&echo.sprites);
    }

    FigureEchoScope::~FigureEchoScope() {
        if (m_device == nullptr) {
            return;
        }
        FigureEcho& echo = figureEcho();
        m_device->setSpriteEcho(echo.active ? &echo.covers : nullptr); // what is drawn from here on may cover it
        if (echo.sprites.size() > m_first) {
            echo.figures.push_back({m_feet, m_first, echo.sprites.size() - m_first, echo.covers.size(), m_lift});
        }
    }

    void drawFigureShadow(gfx::Canvas& canvas, core::Point feet, int kind) {
        if (g_hidden) {
            return;
        }
        if (kind == 1) { // shadow kind 1: the large monster shadow
            if (const auto shadow = canvas.pictures().find("Monster_Shadow")) {
                canvas.drawPicture(*shadow, {feet.x - 47, feet.y - 18}); // monster shadow offset from the feet: 47 left, 18 up
            }
        } else if (kind == 0) { // shadow kind 0: the small dithered shadow of people
            if (const auto shadow = canvas.pictures().find("Shadow")) {
                // people shadow offset from the feet: 15 left, 4 up; source rect (49,88)-(80,105) of the "Shadow" picture
                canvas.drawPicture(*shadow, {feet.x - 15, feet.y - 4}, core::Rect::fromEdges(49, 88, 80, 105));
            }
        }
    }

}
