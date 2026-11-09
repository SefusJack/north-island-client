#ifndef NORTH_ISLAND_AUDIO_SOUND_EFFECTS_H
#define NORTH_ISLAND_AUDIO_SOUND_EFFECTS_H

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

struct MIX_Audio;

namespace audio {

    class AudioDevice;

    /// @brief Loads short sound files once and plays them on free effect channels; a sound is dropped when all channels are busy, as in the original.
    class SoundEffects {
        public:
            /// @brief Makes the effects player on a device.
            /// @param device Audio device whose effect tracks are used.
            explicit SoundEffects(
                const AudioDevice& device
            )
                : m_device(device) {}

            SoundEffects(const SoundEffects&) = delete;
            SoundEffects& operator=(const SoundEffects&) = delete;

            /// @brief Plays a sound once on a free channel.
            /// @param file Sound file (sound\wav0151.wav ...).
            void play(const std::filesystem::path& file);

            /// @brief Stops the loop the old handle names, then plays the file over and over on a free channel.
            /// @param file Sound file.
            /// @param oldHandle Handle of the loop to stop first (0 for none).
            /// @return The new handle (channel + 1), 0 when nothing plays.
            /// @details Original: PlayLoopedSoundEffect.
            int playLooped(const std::filesystem::path& file, int oldHandle);

            /// @brief Ends a loop: stops the handle's channel and clears its looped state.
            /// @param handle Loop handle from playLooped.
            /// @details Original: Stop and Looped := False on the handle's channel.
            void stopLooped(int handle);

            /// @brief Silences every effect channel.
            /// @details Original: StopSoundEffects.
            void stopAll();

            /// @brief Sets the effects loudness for sounds started from now on.
            /// @param percent 0..100 percent of full loudness.
            void setVolume(int percent);

            /// @brief Silences all channels, drops the loaded sounds and plays on another device from now on (the qol client's party).
            /// @param device The new audio device.
            void setDevice(const AudioDevice& device);

        private:
            /// @brief Deleter that frees a MIX_Audio.
            struct AudioDeleter {
                /// @brief Destroys the audio.
                /// @param audio Audio to destroy.
                void operator()(MIX_Audio* audio) const;
            };

            /// @brief Gives the loaded sound of a file, loading it on first use.
            /// @param file Sound file.
            /// @return The audio, null when it cannot be loaded.
            MIX_Audio* audioOf(const std::filesystem::path& file);

            /// @brief Finds the first effect channel that is quiet (as SDL_mixer 2's Mix_PlayChannel(-1) took).
            /// @return The channel, -1 when all are busy.
            int freeChannel() const;

            /// @brief Plays a sound on a free channel.
            /// @param audio Sound to play.
            /// @param looped Play over and over when true.
            /// @return The channel used, -1 when none was free.
            int start(MIX_Audio* audio, bool looped);

            std::reference_wrapper<const AudioDevice> m_device;
            std::set<int> m_loops;
            std::map<std::string, std::unique_ptr<MIX_Audio, AudioDeleter>> m_sounds;
            float m_gain = 1.0f;
    };

}

#endif
