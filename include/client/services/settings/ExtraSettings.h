#ifndef NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_QOL_SETTINGS_H
#define NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_QOL_SETTINGS_H

#include <filesystem>
#include <optional>
#include <string>

namespace client {

    struct LaunchOptions;

    /// @brief The qol client's own settings (user\extra-settings.json), the ones the original never had: frame pacing, frame graph, window and renderer.
    /// @details Defaults are the original's (a 30 ms beat, an 800 x 600 window, SDL's renderer). The command line overrides them for one run; the first run writes the file from what it ran with.
    struct ExtraSettings {
        /// @brief How frames are paced.
        enum class Frames { Original, Vsync, Unlimited, Capped };

        /// @brief How the window is shown.
        enum class Display { Windowed, Borderless, Fullscreen };

        Frames frames = Frames::Original;
        int fpsCap = 144; // default frame cap in frames per second
        bool frameGraph = false;
        int windowWidth = 800; // the original's window width in pixels
        int windowHeight = 600; // the original's window height in pixels
        bool maximized = false;
        bool rememberWindow = false;
        Display display = Display::Windowed;
        std::string renderer;
        int pixelScale = 1;
        int hudScale = 100; // HUD size in percent (100 = original)
        std::string pixelArt = "off";
        bool pixelArtPictures = true;
        bool pixelArtSprites = true;
        bool pixelArtText = true;
        int loadingMusicVolume = -1; // -1: full loudness as the original
        bool loadingMusicOn = true;
        int loadingSoundVolume = -1; // -1: full loudness as the original
        bool loadingSoundOn = true;
        bool sortButtons = false;
        bool partyLogin = false;

        /// @brief Tells whether frames are paced by anything but the original's beat.
        /// @return True unless the mode is Frames::Original.
        bool freeFrames() const { return frames != Frames::Original; }

        /// @brief Tells whether frames wait for the display's vertical sync.
        /// @return True in Frames::Vsync.
        bool vsync() const { return frames == Frames::Vsync; }

        /// @brief Gives the frames a second the loop waits for.
        /// @return The cap in Frames::Capped, else 0 (none).
        int frameCap() const { return frames == Frames::Capped ? fpsCap : 0; }

        /// @brief Gives the path of the settings file.
        /// @param userRoot The user folder.
        /// @return The path of extra-settings.json in it.
        static std::filesystem::path fileIn(const std::filesystem::path& userRoot);

        /// @brief Reads the settings file; without one the old qol-settings.json, then the older qol-window.ini (--window last), is taken over.
        /// @param userRoot The user folder.
        /// @return The settings, or nothing when neither file is there.
        static std::optional<ExtraSettings> loadIn(const std::filesystem::path& userRoot);

        /// @brief Parses the settings JSON; keys missing or of the wrong kind keep their defaults.
        /// @param json The text.
        /// @return The settings, or nothing when the text is not a JSON object.
        static std::optional<ExtraSettings> parse(const std::string& json);

        /// @brief Writes the settings as JSON text.
        /// @return The text.
        std::string toJson() const;

        /// @brief Saves the settings to a file, creating its folder.
        /// @param file The file to write.
        /// @return True when the file was written.
        bool save(const std::filesystem::path& file) const;

        /// @brief Replaces the settings with what was given on the command line.
        /// @param options The launch options.
        void override(const LaunchOptions& options);

        /// @brief Compares all settings.
        /// @param other The settings to compare with.
        /// @return True when every setting matches.
        bool operator==(const ExtraSettings& other) const;

        /// @brief Compares all settings for difference.
        /// @param other The settings to compare with.
        /// @return True when any setting differs.
        bool operator!=(const ExtraSettings& other) const { return !(*this == other); }
    };

    /// @brief Gives the name of a frame mode as written in the settings file.
    /// @param frames The mode.
    /// @return "original", "vsync", "unlimited" or "cap".
    const char* frameModeName(ExtraSettings::Frames frames);

    /// @brief Gives the name of a display mode as written in the settings file.
    /// @param display The mode.
    /// @return "windowed", "borderless" or "fullscreen".
    const char* displayModeName(ExtraSettings::Display display);

}

#endif
