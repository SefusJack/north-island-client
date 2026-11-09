#include "client/subsystems/battle/BattleSounds.h"

namespace client {

    void BattleSounds::clear() {
        m_sounds.clear();
        m_weapon = {};
        m_body = {};
    }

    void BattleSounds::restart(std::uint64_t nowMs) {
        for (Sound& sound : m_sounds) {
            sound.timeMs = nowMs;
            sound.state = 0;
        }
        for (std::size_t i = 1; i < m_weapon.size(); ++i) {
            m_weapon[i].timeMs = nowMs;
            m_weapon[i].state = 0;
            m_body[i].timeMs = nowMs;
            m_body[i].state = 0;
        }
    }

    void BattleSounds::addSound(const std::string& file, int delay) { m_sounds.push_back(Sound{file, delay, 0, 0}); }

    void BattleSounds::setWeaponSound(int index, const std::string& file, int delay) {
        m_weapon.at(static_cast<std::size_t>(index)) = Sound{file, delay, 0, 0};
    }

    void BattleSounds::setBodySound(int index, const std::string& file, int delay) { m_body.at(static_cast<std::size_t>(index)) = Sound{file, delay, 0, 0}; }

    void BattleSounds::scheduleOne(Sound& sound, int motionPoint, std::uint64_t nowMs) {
        if (sound.state != 0 || sound.file.empty()) {
            return;
        }
        if (sound.delay > 0 && sound.delay < 101) { // delays 1..100 are steps of the motion
            if (sound.delay <= motionPoint) {
                sound.state = 1;
            }
        } else if (static_cast<double>(nowMs - sound.timeMs) >= sound.delay) {
            sound.timeMs = nowMs;
            sound.state = 1;
        }
    }

    void BattleSounds::schedule(int motionPoint, std::uint64_t nowMs) {
        for (Sound& sound : m_sounds) {
            scheduleOne(sound, motionPoint, nowMs);
        }
        for (std::size_t i = 1; i < m_weapon.size(); ++i) {
            scheduleOne(m_weapon[i], motionPoint, nowMs);
        }
        for (std::size_t i = 1; i < m_body.size(); ++i) {
            scheduleOne(m_body[i], motionPoint, nowMs);
        }
    }

    void BattleSounds::play(const std::function<void(const std::string&)>& playSound) {
        const auto due = [&](Sound& sound) {
            if (sound.state == 1) {
                playSound(sound.file);
                sound.state = 2;
            }
        };
        for (Sound& sound : m_sounds) {
            due(sound);
        }
        for (std::size_t i = 1; i < m_weapon.size(); ++i) {
            due(m_weapon[i]);
        }
        for (std::size_t i = 1; i < m_body.size(); ++i) {
            due(m_body[i]);
        }
    }

}
