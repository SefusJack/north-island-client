#include "client/rendering/effects/TreasureLights.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::uint64_t kFrameMs = 70; // light frame time, ms
        constexpr int kFrames = 11; // frames in picture S10061
        constexpr int kAlpha = 30; // light blend strength (0..255)
        constexpr int kBelowPlace = 40; // light is drawn 40 px below the treasure's place

    }

    void TreasureLights::start(core::Point place, std::uint64_t nowMs) {
        const auto picture = m_pictures.find("S10061");
        if (!picture) {
            return;
        }
        auto free = std::find_if(m_slots.begin(), m_slots.end(), [](const Slot& slot) { return !slot.light.isActive(); });
        if (free == m_slots.end()) {
            free = m_slots.begin();
        }
        LightEffect::Setup setup;
        setup.picture = *picture;
        setup.frameMs = kFrameMs;
        setup.firstFrame = 1;
        setup.lastFrame = kFrames;
        setup.frameCount = kFrames;
        setup.motion = LightEffect::Motion::Once;
        setup.alpha = kAlpha;
        free->place = place;
        free->light.start(setup, {place.x, place.y + kBelowPlace}, nowMs);
    }

    void TreasureLights::draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) {
        for (Slot& slot : m_slots) {
            if (slot.light.isActive()) {
                slot.light.setPosition({slot.place.x, slot.place.y + kBelowPlace});
                slot.light.draw(canvas, nowMs, origin);
            }
        }
    }

}
