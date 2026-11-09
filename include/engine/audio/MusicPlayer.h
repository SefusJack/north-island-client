#ifndef NORTH_ISLAND_AUDIO_MUSIC_PLAYER_H
#define NORTH_ISLAND_AUDIO_MUSIC_PLAYER_H

#include "audio/TrackClock.h"
#include "core/BackgroundLoad.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

struct MIX_Audio;
struct MIX_Mixer;

namespace audio {

    class AudioDevice;

    /// @brief Plays one looping background music track at a time, decoded on another thread.
    class MusicPlayer {
        public:
            /// @brief Makes a player on a device.
            /// @param device Audio device whose music track is used.
            explicit MusicPlayer(
                const AudioDevice& device
            )
                : m_device(device) {}

            MusicPlayer(const MusicPlayer&) = delete;
            MusicPlayer& operator=(const MusicPlayer&) = delete;

            /// @brief Stops the music.
            ~MusicPlayer();

            /// @brief Asks for a track; it replaces the playing one once decoded and then loops.
            /// @param file Music file (MS-ADPCM WAV).
            /// @param startMs Offset into the track where it begins (the original saved it as FMusicResumePos around a movie).
            void playLooped(const std::filesystem::path& file, std::uint64_t startMs = 0);

            /// @brief Starts the track whose decoding has ended; call once a frame.
            void update();

            /// @brief Stops the track and drops any decoding in progress.
            void stop();

            /// @brief Tells whether a given track is on or on its way.
            /// @param file Music file to test.
            /// @return True when that file plays or is being decoded.
            bool isPlaying(const std::filesystem::path& file) const;

            /// @brief Tells whether some track is on or on its way.
            /// @return True when a track plays or is being decoded.
            bool playing() const { return m_track != nullptr || !m_loadingFile.empty(); }

            /// @brief Gives the track that is on or on its way (what a movie saves to put back).
            /// @return The file, empty for none.
            std::filesystem::path file() const { return m_track ? m_file : m_loadingFile; }

            /// @brief Gives where in the track the music is.
            /// @return Position in ms, 0 when none is on.
            std::uint64_t positionMs() const;

            /// @brief Sets the music loudness.
            /// @param percent 0..100 percent of full loudness.
            void setVolume(int percent);

            /// @brief Holds the track where it is (music switched off).
            void pause();

            /// @brief Goes on from where the track was held.
            void resume();

            /// @brief Stops the track and plays on another device from now on (the qol client's party).
            /// @param device The new audio device.
            void setDevice(const AudioDevice& device) {
                stop();
                m_device = std::cref(device);
            }

        private:
            /// @brief Deleter that frees a MIX_Audio.
            struct AudioDeleter {
                /// @brief Destroys the audio.
                /// @param audio Audio to destroy.
                void operator()(MIX_Audio* audio) const;
            };

            using Track = std::unique_ptr<MIX_Audio, AudioDeleter>;

            /// @brief A decoded track and its length.
            struct Loaded {
                Track track;
                std::uint64_t durationMs = 0;
            };

            /// @brief Decodes a music file whole (SDL_mixer loads from any thread).
            /// @param mixer Mixer to decode for.
            /// @param name File path.
            /// @return The decoded track and its length; track is null on failure.
            static Loaded decode(MIX_Mixer* mixer, const std::string& name);

            std::reference_wrapper<const AudioDevice> m_device;
            Track m_track;
            std::filesystem::path m_file;
            core::BackgroundLoad<Loaded> m_loading;
            std::filesystem::path m_loadingFile;
            std::uint64_t m_loadingStartMs = 0;
            std::vector<core::BackgroundLoad<Loaded>> m_dropped;
            TrackClock m_clock;
    };

}

#endif
