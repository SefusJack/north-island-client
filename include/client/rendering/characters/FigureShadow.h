#ifndef NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_FIGURE_SHADOW_H
#define NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_FIGURE_SHADOW_H

#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"
#include "gfx/RenderDevice.h"

#include <cstddef>
#include <string>
#include <vector>

namespace client {

    /// @brief Draws the shadow under a figure's feet.
    /// @details Kind 0 is the small dithered one of people (part of "Shadow"), 1 the large monster shadow, anything else none
    /// (Npc.dat's shadow column).
    /// @param canvas Target canvas.
    /// @param feet Screen position of the feet.
    /// @param kind Shadow kind from Npc.dat.
    void drawFigureShadow(gfx::Canvas& canvas, core::Point feet, int kind);

    /// @brief Leaves the figures' own shadows out or draws them again (the experimental lighting casts its own on the map).
    /// @param hidden True to leave them out.
    void hideFigureShadows(bool hidden);

    /// @brief Whether the figures' own shadows are left out now (and with them those painted on the map's pictures).
    /// @return True when they are.
    bool figureShadowsHidden();

    /// @brief The figures drawn on the map in a frame, as the sprites they were drawn with (the experimental lighting draws them
    /// again as their shadows).
    struct FigureEcho {
        /// @brief A figure: where it stands and which sprites are its.
        struct Figure {
            core::Point feet; // where the sprites were drawn, as they are
            std::size_t first = 0;
            std::size_t count = 0;
            std::size_t coversFrom = 0; // the first of the covers drawn after it
            int lift = 0; // how high over the ground it was drawn (the feet are on the ground under it)
        };

        /// @brief A thing whose painted shadow was left out: the lighting casts one for it.
        struct Caster {
            core::Point feet; // where it stands on the ground
            std::size_t first = 0; // its covers: the first, and one past the last
            std::size_t end = 0;
            gfx::PictureId picture; // what it was drawn from
            core::Point at; // where the picture's frame was drawn, on the screen
            int frameTop = 0; // the row of the picture the frame starts at
        };

        bool active = false;
        std::vector<gfx::SpriteEcho> sprites;
        std::vector<gfx::SpriteEcho> covers; // what else was drawn since the first figure: it may stand before a figure
        std::vector<Figure> figures;
        std::vector<Caster> casters;
        std::vector<std::pair<std::size_t, std::size_t>> things; // the covers that are a thing standing on the ground by itself: first, and one past the last
    };

    /// @brief The figures noted.
    /// @return Them.
    FigureEcho& figureEcho();

    /// @brief The light a battle effect gives off in a frame (fire, lightning, a flash), for the experimental lighting.
    struct EffectGlow {
        core::Point at; // its middle on the screen
        int radius = 0;
        float r = 0; // its colour, times how strong it is (0 to 1)
        float g = 0;
        float b = 0;
    };

    /// @brief The effect lights noted since the figures were last begun.
    /// @return Them.
    std::vector<EffectGlow>& effectGlows();

    /// @brief Notes the light of an effect's frame as it is drawn, while figures are noted: as bright, as wide and of the
    /// colour its bright pixels are. A dull effect gives none.
    /// @param canvas The canvas it is drawn on.
    /// @param picture Its picture.
    /// @param name The picture's name.
    /// @param source The frame drawn, in the picture.
    /// @param at Where it is drawn on the canvas.
    /// @param translucent True when it is drawn as light added to the screen.
    void noteEffectGlow(gfx::Canvas& canvas, gfx::PictureId picture, const std::string& name, core::Rect source, core::Point at, bool translucent);

    /// @brief Forgets the figures noted and notes those drawn from now on.
    void beginFigureEcho();

    /// @brief Notes what is drawn from now on besides the figures (the things standing on the map), while figures are noted.
    /// @param device The device they are drawn with.
    void noteFigureCovers(const gfx::RenderDevice& device);

    /// @brief Marks the covers noted from now on as a thing that stands on the ground by itself (no piece of a larger picture).
    /// @return Where they start, for endFigureThing.
    std::size_t beginFigureThing();

    /// @brief Ends the thing begun.
    /// @param first What beginFigureThing returned.
    void endFigureThing(std::size_t first);

    /// @brief Marks the covers noted since beginFigureThing as a thing the lighting casts a shadow for.
    /// @param first What beginFigureThing returned.
    /// @param feet Where it stands on the ground, on the screen.
    /// @param picture The picture it was drawn from.
    /// @param at Where the picture's frame was drawn, on the screen.
    /// @param frameTop The row of the picture the frame starts at.
    void noteFigureCaster(std::size_t first, core::Point feet, gfx::PictureId picture, core::Point at, int frameTop);

    /// @brief Tells how high over the ground the figures drawn next are (a jump): their shadows stay on the ground.
    /// @param lift The height in pixels, 0 for on the ground.
    void liftFigures(int lift);

    /// @brief Stops noting figures.
    /// @param device The device they were drawn with.
    void endFigureEcho(const gfx::RenderDevice& device);

    /// @brief Notes what is drawn while it lives as a figure, while figures are noted.
    class FigureEchoScope {
        public:
            /// @brief Starts a figure.
            /// @param canvas The canvas it is drawn on.
            /// @param feet Where it stands on the canvas.
            FigureEchoScope(const gfx::Canvas& canvas, core::Point feet);

            ~FigureEchoScope();
            FigureEchoScope(const FigureEchoScope&) = delete;
            FigureEchoScope& operator=(const FigureEchoScope&) = delete;

        private:
            const gfx::RenderDevice* m_device = nullptr; // nullptr: nothing is noted
            core::Point m_feet;
            int m_lift = 0;
            std::size_t m_first = 0;
    };

}

#endif
