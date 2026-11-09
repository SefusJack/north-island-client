#include "client/models/travel/VehicleWrecks.h"

#include "client/data/VehicleKinds.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr std::uint64_t kFrameMs = 70; // time per fall animation frame, ms
        constexpr int kFallFrames = 3; // frames of the fall animation
        constexpr std::uint64_t kLyingMs = 60000; // time a wreck lies on the ground, ms (one minute)
        constexpr int kRobinsonsRaft = 48016; // item 48016: Robinson's Raft
        constexpr int kWoodenRaft = 48010; // item 48010: Raft
        constexpr int kFallbackWreck = 48005; // item 48005: Jalor (its picture is used when a vehicle has no wreck picture)

    }

    std::optional<std::size_t> VehicleWrecks::freePlace() const {
        for (std::size_t i = 0; i < m_wrecks.size(); ++i) {
            if (!m_wrecks[i]) {
                return i;
            }
        }
        for (std::size_t i = 0; i < m_wrecks.size(); ++i) {
            if (!m_wrecks[i]->fall.isActive()) {
                return i;
            }
        }
        return std::nullopt;
    }

    void VehicleWrecks::add(
        int itemId,
        std::int32_t ownerId,
        core::Point position,
        core::Point screen,
        std::uint64_t nowMs
    ) {
        if (VehicleKinds::isCapsule(itemId)) {
            if (const auto record = m_items.find(itemId)) {
                itemId = record->aliasItemId;
            }
        }
        const auto place = freePlace();
        if (!place) {
            return;
        }
        VehicleWreck wreck;
        wreck.itemId = itemId == kRobinsonsRaft ? kWoodenRaft : itemId;
        wreck.picture = m_pictures.find(std::to_string(wreck.itemId) + "_B").value_or(gfx::PictureId{});
        if (!wreck.picture.isValid()) {
            wreck.itemId = kFallbackWreck;
            wreck.picture = m_pictures.find(std::to_string(wreck.itemId) + "_B").value_or(gfx::PictureId{});
        }
        wreck.position = position;
        wreck.ownerId = ownerId;
        wreck.startMs = nowMs;
        LightEffect::Setup setup;
        setup.picture = wreck.picture;
        setup.frameMs = kFrameMs;
        setup.firstFrame = 1;
        setup.lastFrame = kFallFrames;
        setup.frameCount = kFallFrames;
        setup.motion = LightEffect::Motion::Once;
        setup.loop = false;
        wreck.fall.start(setup, screen, nowMs);
        m_wrecks[*place] = std::move(wreck);
    }

    std::vector<std::int32_t> VehicleWrecks::draw(
        gfx::Canvas& canvas,
        core::Point origin,
        std::uint64_t nowMs,
        const std::function<bool(std::int32_t)>& onMap
    ) {
        std::vector<std::int32_t> seen;
        for (std::optional<VehicleWreck>& slot : m_wrecks) {
            if (!slot) {
                continue;
            }
            VehicleWreck& wreck = *slot;
            if (wreck.fall.isActive()) {
                wreck.fall.update(nowMs);
                wreck.fall.draw(canvas, nowMs);
                continue;
            }
            if (nowMs - wreck.startMs > kLyingMs) {
                slot.reset();
                continue;
            }
            if (wreck.picture.isValid()) {
                const gfx::Picture& picture = m_pictures.picture(wreck.picture);
                const int height = picture.height() / kFallFrames;
                canvas.drawPicture(
                    wreck.picture,
                    {wreck.position.x - origin.x - picture.width() / 2, wreck.position.y - origin.y - height},
                    core::Rect{0, height * 2, picture.width(), height}
                ); // the third (last) frame of three
            }
            if (wreck.ownerId != 0) {
                if (onMap(wreck.ownerId)) {
                    seen.push_back(wreck.ownerId);
                }
                wreck.ownerId = 0;
            }
        }
        return seen;
    }

    void VehicleWrecks::clear() {
        for (std::optional<VehicleWreck>& wreck : m_wrecks) {
            wreck.reset();
        }
    }

}
