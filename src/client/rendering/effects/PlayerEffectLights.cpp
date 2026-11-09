#include "client/rendering/effects/PlayerEffectLights.h"

namespace client {

    namespace {

        constexpr int kSparkleFrames = 20; // frames in the sparkle picture L10251
        constexpr int kKindFrames = 6; // frames in the kind picture L10252 (one per light kind)
        constexpr std::uint64_t kKindMs = 4000; // frame time of the kind picture, ms (the single frame is held 4 s, then replayed)
        constexpr int kFirstEnd = 249; // first of the list-ending marker kinds 249..254 (not real light kinds)

    }

    int PlayerEffectLights::random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

    void PlayerEffectLights::set(std::int32_t playerId, core::Point feet, const std::vector<int>& kinds, std::uint64_t nowMs) {
        constexpr std::size_t kMost = 3; // at most 3 lights per player
        if (kinds.size() > kMost) {
            return;
        }
        std::array<Slot, 3>& slots = m_players[playerId]; // 3: lights per player
        slots = {};
        for (std::size_t i = 0; i < kinds.size(); ++i) {
            if (kinds[i] >= kFirstEnd && kinds[i] <= kFirstEnd + 5) { // kinds 249..254 end the list
                break;
            }
            begin(slots[i], static_cast<int>(i) + 1, feet, kinds[i], nowMs);
        }
    }

    void PlayerEffectLights::play(
        Slot& slot,
        const char* picture,
        std::uint64_t frameMs,
        int first,
        int last,
        int offsetY,
        bool once,
        std::uint64_t nowMs
    ) {
        const auto found = m_pictures.find(picture);
        if (!found) {
            return;
        }
        LightEffect::Setup setup;
        setup.picture = *found;
        setup.frameMs = frameMs;
        setup.firstFrame = first;
        setup.lastFrame = last;
        setup.frameCount = once ? kSparkleFrames : kKindFrames;
        setup.motion = once ? LightEffect::Motion::Once : LightEffect::Motion::Stay;
        setup.loop = true;
        setup.offsetY = offsetY;
        slot.light.start(setup, slot.place, nowMs);
    }

    void PlayerEffectLights::begin(
        Slot& slot,
        int slotNo,
        core::Point feet,
        int kind,
        std::uint64_t nowMs
    ) {
        constexpr std::uint64_t kStartMs = 40; // frame time of the start flash, ms
        constexpr int kStartFrames = 4; // the start flash plays sparkle frames 1..4
        constexpr int kSideSpread = 50; // px: random x range, and spacing between the three light slots
        constexpr int kDepth = 40; // px: random y range
        constexpr int kBelow = 20; // px: minimum distance below the player's feet
        slot.used = true;
        slot.stage = 0;
        slot.kind = kind;
        // slotNo 1..3 shifts the spot by (slotNo - 2) side spreads, so the three lights fan out left, middle, right
        slot.place = {random(kSideSpread) + (slotNo - 2) * kSideSpread + feet.x, random(kDepth) + kBelow + feet.y};
        // draw offset down from the place: random 30..59 pixels
        play(slot, "L10251", kStartMs, 1, kStartFrames, random(30) + 30, true, nowMs);
    }

    void PlayerEffectLights::next(Slot& slot, std::uint64_t nowMs) {
        constexpr int kOffsetY = 50; // picture is drawn 50 px down from the place
        constexpr int kFromFrame = 5; // sparkle repeats start no earlier than frame 5
        switch (slot.stage) {
            case 0:
                play(slot, "L10251", 20, random(7) + kFromFrame, kSparkleFrames, kOffsetY, true, nowMs);
                break; // 1st repeat: 20 ms frames, start frame random 5..11
            case 1:
                play(slot, "L10251", 20, random(8) + kFromFrame, kSparkleFrames, kOffsetY, true, nowMs);
                break; // 2nd repeat: 20 ms frames, start frame random 5..12
            case 2:
                play(slot, "L10251", 40, random(9) + kFromFrame, kSparkleFrames, kOffsetY, true, nowMs);
                break; // 3rd repeat: 40 ms frames, start frame random 5..13
            case 3:
                play(slot, "L10251", 60, random(10) + kFromFrame, kSparkleFrames, kOffsetY, true, nowMs);
                break; // 4th repeat: 60 ms frames, start frame random 5..14
            case 4:
                play(slot, "L10252", kKindMs, slot.kind, slot.kind, kOffsetY, false, nowMs);
                break; // final stage: the picture of the light's kind, kept for good
            default: break; // stage past the last: nothing more to play
        }
        ++slot.stage;
    }

    void PlayerEffectLights::draw(gfx::Canvas& canvas, std::int32_t playerId, core::Point origin, std::uint64_t nowMs) {
        const auto found = m_players.find(playerId);
        if (found == m_players.end()) {
            return;
        }
        for (Slot& slot : found->second) {
            if (!slot.used) {
                continue;
            }
            slot.light.setPosition(slot.place);
            slot.light.draw(canvas, nowMs, origin);
            if (!slot.light.isActive()) {
                next(slot, nowMs);
            }
        }
    }

}
