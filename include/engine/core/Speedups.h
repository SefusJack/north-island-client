#ifndef NORTH_ISLAND_CORE_SPEEDUPS_H
#define NORTH_ISLAND_CORE_SPEEDUPS_H

namespace core {

    /// @brief Optional ways of doing the same work with less time, each off as the client was before (user\extra-settings.json, "speedups").
    /// @details Each one trades something (memory, a later moment for the work, a path Windows is trusted on) for speed; the
    /// picture on the screen is the same with any of them on. They are read when the things they change are made, so they are
    /// set once at the start, before the window.
    struct Speedups {
        /// @brief Software renderer: pictures are kept as static textures, which SDL run-length encodes (opaque runs copied, clear runs skipped).
        /// @details A picture that is also drawn tinted, faded, scaled or additive gets a second, plain copy: more memory.
        bool softwareEncodedTextures = false;

        /// @brief Software renderer: only the parts of a frame that differ from the shown one are copied to the screen.
        /// @details Keeps a copy of the shown frame and compares each new one with it.
        bool softwareChangedAreas = false;

        /// @brief Software renderer: when the map scrolls, the shown picture is moved on the screen and only the rest is copied.
        /// @details Works on top of softwareChangedAreas (nothing without it), on Windows, with the window shown 1 : 1.
        bool softwareScroll = false;

        /// @brief A string drawn twice is kept as one picture instead of up to nine copies a glyph.
        /// @details With the software renderer, and with a GPU renderer at pixel scale 1. Pictures not drawn for 5 s are
        /// dropped; more memory while a lot of text is shown.
        bool textPictures = false;

        /// @brief SDL only does its quick checks of what it is handed (SDL_HINT_INVALID_PARAM_CHECKS "1").
        /// @details The full checks look every texture up in a locked table on each draw; before Windows 7 that lock goes
        /// through the system each time. Without them a draw with a texture that was already freed is not caught by SDL.
        bool quickSdlChecks = false;

        /// @brief A window's pictures are read when they are first drawn (a few ms of each beat loads the rest ahead), not when the window is built.
        /// @details Entering the world is shorter; a window opened right after may load a picture in the frame it first shows.
        bool picturesOnFirstUse = false;

        /// @brief A map's background is decoded some rows at a time, the rows on the screen first.
        /// @details Entering a map is shorter; the rest is decoded in a few ms of each beat while walking.
        bool mapInParts = false;

        /// @brief Worker threads (picture and music decoding) run below the main thread's priority, which gives them a few ms a frame.
        /// @details For a single processor core, where a normal-priority worker takes whole turns from the frame being drawn.
        /// With more cores it only adds those few ms to a frame while something is being decoded.
        bool quietWorkers = false;

        /// @brief Which renderers start on this machine is remembered (user\renderer-check.txt) for a week per graphics card.
        /// @details Saves trying each one at every launch; a renderer that stops or starts working is noticed up to a week late.
        bool rememberRendererChecks = false;

        /// @brief How long before a capped frame is due the sleep ends and a busy wait takes over, ms.
        /// @details 1 as before. Less uses less processor time and relies on the system waking the program on time.
        double limiterSpinMs = 1.0;

        /// @brief Compares all settings.
        /// @param other The settings to compare with.
        /// @return True when every setting matches.
        bool operator==(const Speedups& other) const {
            return softwareEncodedTextures == other.softwareEncodedTextures && softwareChangedAreas == other.softwareChangedAreas
                && softwareScroll == other.softwareScroll && textPictures == other.textPictures && quickSdlChecks == other.quickSdlChecks
                && picturesOnFirstUse == other.picturesOnFirstUse && mapInParts == other.mapInParts && quietWorkers == other.quietWorkers
                && rememberRendererChecks == other.rememberRendererChecks && limiterSpinMs == other.limiterSpinMs;
        }
    };

    /// @brief The speedups this run uses (all off until the settings are read).
    /// @return The one set of them, changed only at the start.
    inline Speedups& speedups() {
        static Speedups chosen;
        return chosen;
    }

}

#endif
