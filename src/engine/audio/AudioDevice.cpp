#include "audio/AudioDevice.h"

#include "core/Log.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace audio {

    AudioDevice::AudioDevice() {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            core::logWarning("no audio: ", SDL_GetError());
            return;
        }
        if (!MIX_Init()) {
            core::logWarning("cannot start the sound mixer: ", SDL_GetError());
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return;
        }
        const SDL_AudioSpec spec{SDL_AUDIO_S16, 2, 44100}; // 16-bit samples, 2 channels (stereo), 44100 Hz
        m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
        if (m_mixer == nullptr) {
            core::logWarning("cannot open the sound output: ", SDL_GetError());
            MIX_Quit();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return;
        }
        m_music = MIX_CreateTrack(m_mixer);
        for (MIX_Track*& track : m_effects) {
            track = MIX_CreateTrack(m_mixer);
        }
    }

    AudioDevice::~AudioDevice() {
        if (m_mixer != nullptr) {
            MIX_DestroyMixer(m_mixer);
            MIX_Quit();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
    }

}
