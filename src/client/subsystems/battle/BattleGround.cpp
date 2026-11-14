#include "client/subsystems/battle/BattleGround.h"

#include "core/WorkerPriority.h"

#include <algorithm>
#include <chrono>
#include <string>

namespace client {

    namespace {

        constexpr int kFirstGround = 59000; // scene id of battle ground 0; ground n is scene 59000 + n
        constexpr int kDefaultGround = 59081; // scene 59081: Desert Grassland, used when the ground scene is missing

        /// @brief The index of the first tile that covers the range starting at from.
        /// @param from Start of the range.
        /// @param at Position of tile 0.
        /// @param size Tile size.
        /// @return Tile index (0 is the picture itself).
        int firstTile(int from, int at, int size) { return from >= at ? (from - at) / size : -((at - from + size - 1) / size); }

        /// @brief The index of the last tile that covers the range ending at to.
        /// @param to End of the range.
        /// @param at Position of tile 0.
        /// @param size Tile size.
        /// @return Tile index (0 is the picture itself).
        int lastTile(int to, int at, int size) { return firstTile(to - 1, at, size); }

        /// @brief Draws a layer, and repeats it mirrored past its edges where a window bigger than the game screen shows beyond it.
        /// @param canvas Target canvas.
        /// @param id Picture of the layer.
        /// @param at Top left of the picture.
        /// @param up Whether the half next to the top edge repeats upward.
        /// @param down Whether the half next to the bottom edge repeats downward.
        void drawMirrorTiled(
            gfx::Canvas& canvas,
            gfx::PictureId id,
            core::Point at,
            bool up,
            bool down
        ) {
            canvas.drawPicture(id, at);
            const gfx::Picture& picture = canvas.pictures().picture(id);
            const int width = picture.width();
            const int height = picture.height();
            const int half = height / 2;
            const core::Rect window = canvas.window();
            // 800 x 600: the game's screen size
            if (width <= 0 || half <= 0 || (window.x >= 0 && window.y >= 0 && window.x + window.width <= 800 && window.y + window.height <= 600)) {
                return;
            }
            const int left = (std::min)(firstTile(window.x, at.x, width), 0);
            const int right = (std::max)(lastTile(window.x + window.width, at.x, width), 0);
            const auto drawRow = [&](int y, bool topBottom, const std::optional<core::Rect>& source, bool skipOwn) {
                for (int column = left; column <= right; ++column) {
                    if (!skipOwn || column != 0) {
                        canvas.drawPictureFlipped(id, {at.x + column * width, y}, column % 2 != 0, topBottom, source);
                    }
                }
            };
            drawRow(at.y, false, std::nullopt, true);
            if (down) {
                const core::Rect bottomHalf{0, height - half, width, half};
                for (int row = 1, y = at.y + height; y < window.y + window.height; ++row, y += half) {
                    drawRow(y, row % 2 != 0, bottomHalf, false);
                }
            }
            if (up) {
                const core::Rect topHalf{0, 0, width, half};
                for (int row = 1, y = at.y - half; y + half > window.y; ++row, y -= half) {
                    drawRow(y, row % 2 != 0, topHalf, false);
                }
            }
        }
    }

    void BattleGround::load(const formats::SceneTable& scenes, int ground, core::Color tint) {
        auto scene = scenes.find(kFirstGround + ground);
        if (!scene) {
            scene = scenes.find(kDefaultGround);
        }
        m_pending.clear(); // waits for an earlier battle's pictures still decoding
        m_backdrop.reset();
        m_far.reset();
        m_near.reset();
        m_tint = tint;
        if (!scene) {
            return;
        }
        const auto picture = [&](int number, gfx::PictureLibrary::Source source, std::optional<gfx::PictureId> BattleGround::*layer) {
            if (number <= 0) {
                return;
            }
            const std::string name = std::to_string(number);
            if (const auto loaded = m_pictures.findLoaded(name, tint, source)) {
                this->*layer = loaded;
                return;
            }
            auto raw = m_pictures.readRaw(name, tint, source);
            if (!raw) {
                this->*layer = m_pictures.find(name, tint, source); // missing, or a pack picture: loaded here as before
                return;
            }
            Pending pending;
            pending.raw = std::make_shared<const gfx::PictureLibrary::Raw>(std::move(*raw));
            pending.source = source;
            pending.layer = layer;
            pending.image = std::async(std::launch::async, [bytes = pending.raw, tint] {
                const core::LowPriorityScope low;
                return gfx::PictureLibrary::decodeRaw(*bytes, tint);
            });
            m_pending.push_back(std::move(pending));
        };
        picture(scene->battlePictures[0], gfx::PictureLibrary::Source::Bitmap, &BattleGround::m_near);
        picture(scene->battlePictures[1], gfx::PictureLibrary::Source::Bitmap, &BattleGround::m_far);
        picture(scene->battlePictures[2], gfx::PictureLibrary::Source::Jpeg, &BattleGround::m_backdrop);
        m_nearY = scene->battleLayerY[0];
        m_farY = scene->battleLayerY[1];
    }

    void BattleGround::upload(Pending& pending) {
        this->*pending.layer = m_pictures.storeDecoded(*pending.raw, m_tint, pending.source, pending.image.get());
    }

    bool BattleGround::poll() {
        // with quiet workers the first picture still decoding gets a few ms of this frame
        int slice = core::workerSliceMs();
        for (auto it = m_pending.begin(); it != m_pending.end(); ++it) {
            const int waitMs = slice;
            slice = 0;
            if (it->image.wait_for(std::chrono::milliseconds(waitMs)) == std::future_status::ready) {
                upload(*it);
                m_pending.erase(it);
                break; // one upload a frame
            }
        }
        return m_pending.empty();
    }

    void BattleGround::finish() {
        for (Pending& pending : m_pending) {
            upload(pending);
        }
        m_pending.clear();
    }

    void BattleGround::draw(gfx::Canvas& canvas, const BattleCamera& camera, core::Point shake) const {
        const core::Point at = camera.position();
        if (m_backdrop) {
            drawMirrorTiled(canvas, *m_backdrop, {-BattleCamera::kRest.x, -BattleCamera::kRest.y}, true, true);
        }
        if (m_far) {
            drawMirrorTiled(canvas, *m_far, {-camera.farX() + shake.x / 2, m_farY - at.y + shake.y / 2}, false, false);
        }
        if (m_near) {
            const bool shaking = shake.x != 0 || shake.y != 0;
            drawMirrorTiled(canvas, *m_near, {-at.x + shake.x, shaking ? m_nearY - at.y + shake.y : nearTop(m_nearY, at.y)}, false, true);
        }
    }

    int BattleGround::nearTop(int layerY, int cameraY) { return std::max(layerY - cameraY, layerY - BattleCamera::kRest.y); }

    core::Point BattleGround::size() const {
        if (!m_near) {
            return {};
        }
        const gfx::Picture& picture = m_pictures.picture(*m_near);
        return {picture.width(), picture.height()};
    }

}
