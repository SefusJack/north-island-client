#include "audio/MusicPlayer.h"

#include "audio/AudioDevice.h"
#include "core/Log.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <algorithm>
#include <string>

namespace audio {

    void MusicPlayer::AudioDeleter::operator()(MIX_Audio* audio) const { MIX_DestroyAudio(audio); }

    MusicPlayer::~MusicPlayer() { stop(); }

    MusicPlayer::Loaded MusicPlayer::decode(MIX_Mixer* mixer, const std::string& name) {
        Loaded loaded;
        loaded.track = Track(MIX_LoadAudio(mixer, name.c_str(), true));
        if (loaded.track) {
            const Sint64 frames = MIX_GetAudioDuration(loaded.track.get());
            loaded.durationMs = frames > 0 ? static_cast<std::uint64_t>(MIX_AudioFramesToMS(loaded.track.get(), frames)) : 0;
        }
        return loaded;
    }

    void MusicPlayer::playLooped(const std::filesystem::path& file, std::uint64_t startMs) {
        if (!m_device.get().isOpen() || (startMs == 0 && isPlaying(file))) {
            return;
        }
        stop();
        m_loadingFile = file;
        m_loadingStartMs = startMs;
        const std::string name = file.string();
        MIX_Mixer* mixer = m_device.get().mixer();
        m_loading.start([mixer, name] { return decode(mixer, name); });
    }

    std::uint64_t MusicPlayer::positionMs() const { return m_track ? m_clock.positionMs(SDL_GetTicks()) : 0; }

    void MusicPlayer::update() {
        m_dropped.erase(
            std::remove_if(m_dropped.begin(), m_dropped.end(), [](const core::BackgroundLoad<Loaded>& load) { return load.ready(); }),
            m_dropped.end()
        );
        if (!m_loading.pending() || !m_loading.ready()) {
            return;
        }
        Loaded loaded = m_loading.take();
        Track track = std::move(loaded.track);
        const std::filesystem::path file = std::move(m_loadingFile);
        m_loadingFile.clear();
        if (!track) {
            core::logWarning("cannot load the music ", file.string(), ": ", SDL_GetError());
            return;
        }
        m_track = std::move(track);
        m_file = file;
        m_clock.start(SDL_GetTicks(), m_loadingStartMs, loaded.durationMs);
        MIX_Track* music = m_device.get().musicTrack();
        MIX_SetTrackAudio(music, m_track.get());
        const SDL_PropertiesID options = SDL_CreateProperties();
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
        if (loaded.durationMs != 0) {
            SDL_SetNumberProperty(options, MIX_PROP_PLAY_START_MILLISECOND_NUMBER, static_cast<Sint64>(m_loadingStartMs % loaded.durationMs));
        }
        MIX_PlayTrack(music, options);
        SDL_DestroyProperties(options);
    }

    void MusicPlayer::stop() {
        if (m_loading.pending()) {
            m_dropped.push_back(std::move(m_loading));
            m_loading = core::BackgroundLoad<Loaded>();
            m_loadingFile.clear();
        }
        if (m_track) {
            MIX_StopTrack(m_device.get().musicTrack(), 0);
            MIX_SetTrackAudio(m_device.get().musicTrack(), nullptr);
            m_track.reset();
        }
        m_file.clear();
    }

    bool MusicPlayer::isPlaying(const std::filesystem::path& file) const { return (m_track && m_file == file) || m_loadingFile == file; }

    void MusicPlayer::setVolume(int percent) {
        if (m_device.get().isOpen()) {
            MIX_SetTrackGain(m_device.get().musicTrack(), static_cast<float>(std::clamp(percent, 0, 100)) / 100.0f); // percent range 0..100 mapped to gain 0..1
        }
    }

    void MusicPlayer::pause() {
        if (m_device.get().isOpen()) {
            MIX_PauseTrack(m_device.get().musicTrack());
            m_clock.pause(SDL_GetTicks());
        }
    }

    void MusicPlayer::resume() {
        if (m_device.get().isOpen()) {
            MIX_ResumeTrack(m_device.get().musicTrack());
            m_clock.resume(SDL_GetTicks());
        }
    }

}
