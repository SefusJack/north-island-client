#ifndef NORTH_ISLAND_CLIENT_SCENES_SCENE_H
#define NORTH_ISLAND_CLIENT_SCENES_SCENE_H

#include "client/scenes/SceneKind.h"
#include "client/services/input/ScriptProbe.h"
#include "gfx/Canvas.h"
#include "ui/Screen.h"

#include <functional>
#include <cstdint>
#include <optional>

namespace client {

    /// @brief Base class of a program stage (loading, login, world): its screen plus the logic behind it.
    class Scene {
        public:
            virtual ~Scene() = default;

            /// @brief Runs one logic beat of the scene (on the 30 ms beat).
            /// @param nowMs Current time in ms.
            virtual void update(std::uint64_t nowMs) = 0;

            /// @brief Draws the scene.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in ms.
            virtual void draw(gfx::Canvas& canvas, std::uint64_t nowMs) = 0;

            /// @brief Moves what moves smoothly (the people on the map and the view) before every drawn frame with free frames (--fps).
            /// @param nowMs Current time in ms.
            virtual void move(std::uint64_t nowMs) { (void)nowMs; }

            /// @brief Tells whether frames may be drawn between the beats now; a scene whose drawing steps animations per drawn frame keeps to the beat.
            /// @return True when free frames are allowed.
            virtual bool freeFrames() const { return false; }

            /// @brief Gives the time the scene's last update held the clock to (the battle's 35 ms sleeps) with free frames.
            /// @return Time in ms; the next beat comes after it and the frames until then are drawn at it (0: not held).
            virtual std::uint64_t heldUntilMs() const { return 0; }

            /// @brief Gives the screen the window's input goes to.
            /// @return The scene's screen.
            virtual ui::Screen& screen() = 0;

            /// @brief Gives the scene to switch to, once this one is done.
            /// @return The next scene, or nothing while this one goes on.
            virtual std::optional<SceneKind> nextScene() const = 0;

            /// @brief Gives what a test script may ask of the scene.
            /// @return The probe, or nothing (the default: it offers nothing).
            virtual std::optional<std::reference_wrapper<ScriptProbe>> scriptProbe() { return std::nullopt; }
    };

}

#endif
