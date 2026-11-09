#include "audio/SoundEffects.h"

#include "audio/AudioDevice.h"
#include "core/Log.h"

#include <SDL3_mixer/SDL_mixer.h>

#include <algorithm>

namespace audio {

    void SoundEffects::AudioDeleter::operator()(MIX_Audio* audio) const { MIX_DestroyAudio(audio); }

    MIX_Audio* SoundEffects::audioOf(const std::filesystem::path& file) {
        const std::string key = file.string();
        auto found = m_sounds.find(key);
        if (found == m_sounds.end()) {
            std::unique_ptr<MIX_Audio, AudioDeleter> audio(MIX_LoadAudio(m_device.get().mixer(), key.c_str(), true));
            if (!audio) {
                core::logWarning("cannot load the sound ", key, ": ", SDL_GetError());
            }
            found = m_sounds.emplace(key, std::move(audio)).first;
        }
        return found->second.get();
    }

    int SoundEffects::freeChannel() const {
        for (int channel = 0; channel < AudioDevice::kEffectChannels; ++channel) {
            if (!MIX_TrackPlaying(m_device.get().effectTrack(channel)) && !MIX_TrackPaused(m_device.get().effectTrack(channel))) {
                return channel;
            }
        }
        return -1;
    }

    int SoundEffects::start(MIX_Audio* audio, bool looped) {
        const int channel = freeChannel();
        if (channel < 0) {
            return -1;
        }
        MIX_Track* track = m_device.get().effectTrack(channel);
        MIX_SetTrackAudio(track, audio);
        MIX_SetTrackGain(track, m_gain);
        const SDL_PropertiesID options = SDL_CreateProperties();
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, looped ? -1 : 0);
        const bool started = MIX_PlayTrack(track, options);
        SDL_DestroyProperties(options);
        return started ? channel : -1;
    }

    void SoundEffects::play(const std::filesystem::path& file) {
        if (!m_device.get().isOpen()) {
            return;
        }
        if (MIX_Audio* audio = audioOf(file)) {
            start(audio, false);
        }
    }

    int SoundEffects::playLooped(const std::filesystem::path& file, int oldHandle) {
        stopLooped(oldHandle);
        if (!m_device.get().isOpen()) {
            return 0;
        }
        MIX_Audio* audio = audioOf(file);
        if (!audio) {
            return 0;
        }
        const int channel = start(audio, true);
        if (channel < 0) {
            return 0;
        }
        m_loops.insert(channel + 1);
        return channel + 1;
    }

    void SoundEffects::stopLooped(int handle) {
        if (m_loops.erase(handle) != 0) {
            MIX_StopTrack(m_device.get().effectTrack(handle - 1), 0);
        }
    }

    void SoundEffects::stopAll() {
        m_loops.clear();
        if (!m_device.get().isOpen()) {
            return;
        }
        for (int channel = 0; channel < AudioDevice::kEffectChannels; ++channel) {
            MIX_StopTrack(m_device.get().effectTrack(channel), 0);
        }
    }

    void SoundEffects::setDevice(const AudioDevice& device) {
        stopAll();
        m_sounds.clear();
        m_device = std::cref(device);
    }

    void SoundEffects::setVolume(int percent) { m_gain = static_cast<float>(std::clamp(percent, 0, 100)) / 100.0f; } // percent range 0..100 mapped to gain 0..1

}
