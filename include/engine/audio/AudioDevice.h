#ifndef NORTH_ISLAND_AUDIO_AUDIO_DEVICE_H
#define NORTH_ISLAND_AUDIO_AUDIO_DEVICE_H

#include <array>

struct MIX_Mixer;
struct MIX_Track;

namespace audio {

    /// @brief Owns the sound output (SDL3_mixer mixer plus its music and effect tracks); without a device the game runs silent.
    class AudioDevice {
        public:
            static constexpr int kEffectChannels = 8;

            /// @brief Opens the default playback device and creates the music and effect tracks; stays closed on failure.
            AudioDevice();

            /// @brief Tag selecting the never-opened device.
            struct Silent {};

            /// @brief Makes a device that is never opened (the qol client's party members, and --background test runs).
            explicit AudioDevice(
                Silent
            ) {}

            /// @brief Closes the mixer (and its tracks) when the device was opened.
            ~AudioDevice();
            AudioDevice(const AudioDevice&) = delete;
            AudioDevice& operator=(const AudioDevice&) = delete;

            /// @brief Tells whether a sound output was opened.
            /// @return True when the mixer exists.
            bool isOpen() const { return m_mixer != nullptr; }

            /// @brief Gives the mixer.
            /// @return The mixer, or null when closed.
            MIX_Mixer* mixer() const { return m_mixer; }

            /// @brief Gives the track that plays music.
            /// @return The music track, or null when closed.
            MIX_Track* musicTrack() const { return m_music; }

            /// @brief Gives one of the shared effect tracks.
            /// @param channel Effect channel, 0 .. kEffectChannels-1.
            /// @return The effect track, or null when closed.
            MIX_Track* effectTrack(int channel) const { return m_effects[static_cast<std::size_t>(channel)]; }

        private:
            MIX_Mixer* m_mixer = nullptr;
            MIX_Track* m_music = nullptr;
            std::array<MIX_Track*, kEffectChannels> m_effects{};
    };

}

#endif
