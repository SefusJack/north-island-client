#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_STAR_SKY_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_STAR_SKY_H

#include "client/services/GameServices.h"
#include "client/rendering/effects/LightEffect.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "gfx/Canvas.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace client {

    /// @brief The star book: the sky of the stars a player has won, and the flight of a new star up into it.
    /// @details Original: CH_StarSystem. The sky of the stars a player has won drifts past; a new star flies up from the map (phase 1),
    /// comes from the bottom of the sky to its middle (2), shines for a moment (3), and the screen fades back to the map; the notebook's
    /// Character Introductions opens the sky for looking (4).
    class StarSky {
        public:
            static constexpr int kStars = 88; // stars 1..88 are the won stars of the sky
            static constexpr int kOwnStar = 89; // star number 89 is the player's own star (kStars + 1)

            /// @brief The phase of the star book (the original's FFlightPhase).
            enum class Phase { None = 0, Rising = 1, Flying = 2, Shining = 3, Sky = 4 }; // FFlightPhase values 0..4

            /// @brief Callbacks the star book uses to reach the rest of the game.
            struct Hooks {
                std::function<void(bool shown)> hud;
                std::function<void(bool shown)> quitButton;
                std::function<void()> startMusic;
                std::function<void()> restoreMusic;
                std::function<std::optional<core::Point>()> ownPoint;
                std::function<std::optional<core::Point>(int npcIndex)> npcPoint;
                std::function<bool()> fighting;
            };

            /// @brief Creates the star book.
            /// @param services Game services (pictures, sound, data).
            /// @param hooks Callbacks into the rest of the game.
            StarSky(
                GameServices& services,
                Hooks hooks
            );

            /// @brief The current phase.
            /// @return The phase.
            Phase phase() const { return m_phase; }

            /// @brief Tells whether the star book is on screen.
            /// @return True unless the phase is None.
            bool active() const { return m_phase != Phase::None; }

            /// @brief Tells whether a star's introduction is up.
            /// @details Original: FRoleInfoShown.
            /// @return True while shown.
            bool roleInfoShown() const { return m_roleInfoShown; }

            /// @brief Sets whether a star's introduction is up.
            /// @param shown New value.
            void setRoleInfoShown(bool shown) { m_roleInfoShown = shown; }

            /// @brief The star under the mouse.
            /// @details Original: FHoverStar.
            /// @return Star number (0 none, 89 the own one).
            int hoverStar() const { return m_hoverStar; }

            /// @brief The state of the screen fade.
            /// @return The fade state.
            int fadeState() const { return m_fadeState; }

            /// @brief Loads the list of the stars the player has won.
            /// @details Original: LoadOwnedStars, server action 15/19; a number above 88 is the own star.
            /// @param payload Packet payload with the star numbers.
            void loadOwned(const core::Bytes& payload);

            /// @brief A star was won; starts its flight.
            /// @details Original: RecvStarAcquired, server action 15/20 (npcIndex 0: the player himself).
            /// @param star Star number.
            /// @param npcIndex Map NPC the star was won from.
            /// @param nowMs Current tick in milliseconds.
            void acquired(int star, int npcIndex, std::uint64_t nowMs);

            /// @brief Opens the sky for looking (phase 4).
            /// @details Original: OpenStarSky and the notebook's FFlightPhase 4.
            /// @param nowMs Current tick in milliseconds.
            void open(std::uint64_t nowMs);

            /// @brief The right button was let go in the sky: the screen fades out unless a star's introduction is up.
            /// @details Original: TForm1.FormMouseUp.
            void rightReleased();

            /// @brief Leaves the star book.
            /// @details Original: CH_StarSysExit.QuitStarSystem.
            void quit();

            /// @brief Draws a frame of the sky or the flight, with the fade over it.
            /// @details Original: DrawStarSky / DrawStarFlightScene / DrawFlyingStar.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current tick in milliseconds.
            /// @param mouse Mouse position on the screen.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs, core::Point mouse);

            /// @brief A battle starts: the star is won at once and the flight ends.
            /// @details Original: RevFightStart.
            void battleStarted();

        private:
            /// @brief The picture of a star (Star_n).
            /// @param star Star number.
            /// @return The picture id, empty when missing.
            std::optional<gfx::PictureId> starPicture(int star);

            /// @brief The layout slot of a star.
            /// @details Original: GetStarLayoutSlot (with the fixed layout).
            /// @param star Star number.
            /// @return The slot number.
            static int layoutSlot(int star);

            /// @brief The place of a star in the sky.
            /// @details Original: GetStarPos.
            /// @param star Star number.
            /// @return Position on the sky layer.
            core::Point position(int star) const;

            /// @brief Starts the flight of a won star.
            /// @details Original: StartStarFlight.
            /// @param star Star number.
            /// @param npcIndex Map NPC the star comes from.
            /// @param nowMs Current tick in milliseconds.
            void startFlight(int star, int npcIndex, std::uint64_t nowMs);

            /// @brief Advances the flight phases.
            /// @details Original: UpdateStarFlight.
            /// @param nowMs Current tick in milliseconds.
            void updateFlight(std::uint64_t nowMs);

            /// @brief Moves the flying star towards its goal.
            /// @details Original: MoveFlyingStar.
            /// @param nowMs Current tick in milliseconds.
            void moveFlyingStar(std::uint64_t nowMs);

            /// @brief Draws the flying star.
            /// @details Original: DrawFlyingStar.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current tick in milliseconds.
            void drawFlyingStar(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the sky's background.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current tick in milliseconds.
            void drawSkyBackground(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Finds the star under the mouse.
            /// @details Original: HitTestStars.
            /// @param mouse Mouse position on the screen.
            void hitTest(core::Point mouse);

            /// @brief Draws the stars of a frame of the sky.
            /// @param canvas Canvas to draw on.
            void drawStars(gfx::Canvas& canvas);

            /// @brief Steps and draws the fade of the screen.
            /// @details Original: UpdateStarFade.
            /// @param canvas Canvas to draw on.
            void updateFade(gfx::Canvas& canvas);

            GameServices& m_services;
            Hooks m_hooks;
            Phase m_phase = Phase::None;
            std::array<bool, kStars + 1> m_owned{};
            std::array<int, kStars + 1> m_offsets{};
            int m_ownStarOffset = 0;
            int m_ownStar = 0;
            int m_flyStar = 0;
            bool m_flyOwn = false;
            int m_hoverStar = 0;
            int m_skyX = 0;
            int m_skyY = 0;
            std::uint64_t m_skyMs = 0;
            int m_fadeState = 0;
            int m_fadeLevel = 0;
            int m_skyXBefore = 0;
            std::uint64_t m_skyStepMs = 0;
            core::Point m_flyBefore;
            std::uint64_t m_flyStepMs = 0;
            int m_fadeFrom = 0;
            std::uint64_t m_fadeStepMs = 0;
            std::uint64_t m_layerStepMs = 0;
            bool m_roleInfoShown = false;
            core::Point m_flyFrom;
            core::Point m_flyTo;
            core::Point m_fly;
            int m_flyDistX = 0;
            int m_flyDistY = 0;
            core::Rect m_flySource;
            int m_flyFrame = 0;
            std::uint64_t m_flightMs = 0;
            std::uint64_t m_moveXMs = 0;
            std::uint64_t m_moveYMs = 0;
            LightEffect m_light;
    };

}

#endif
