#include "client/viewmodels/hud/TileMiniMap.h"

#include "client/rules/FormDrag.h"

#include "ui/DrawingArea.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kTileWidth = 362; // ground tile picture width, px
        constexpr int kTileHeight = 181; // ground tile picture height, px
        constexpr int kHalfTileHeight = 91; // half a tile height (181 / 2 rounded up), px
        constexpr int kMostStanding = 511; // most objects ranked in the standing (depth sorted) list
        constexpr int kArrowWidth = 9; // door arrow frame width, px
        constexpr int kArrowHeight = 11; // door arrow frame height, px
        constexpr int kArrowFrames = 3; // frames in the door arrow strip
        constexpr std::uint64_t kArrowFrameMs = 500; // time per arrow frame, ms
        constexpr int kFirstGroundVariantOfTall = 140; // first ground picture number drawn with the tall source (inclusive)
        constexpr int kLastGroundVariantOfTall = 145; // end of that range (exclusive)
        constexpr int kTallGroundExtra = 26; // px the source rectangle of a tall ground picture is shifted down
        constexpr int kObjectSourceWidth = 360; // tile object picture source width, px
        constexpr int kObjectSourceHeight = 329; // tile object picture source height, px
        constexpr int kObjectHeight = 330; // tile object picture height used for the scaled size, px
        constexpr int kBrickBase = 10100; // tile object base 10100: bricks, drawn 1 px higher
        constexpr int kMarkerSlot = 4; // face slot 4: the tiny player marker face
        constexpr int kNoMiniMap = 12533; // map 12533: Lost Desert, no mini map shown

        /// @brief Rounds like Delphi's Round: halves go to the even number.
        /// @param value Value to round.
        /// @return Rounded integer.
        int roundTo(double value) { return static_cast<int>(std::nearbyint(value)); }

        /// @brief Rounds up.
        /// @param value Value to round.
        /// @return Smallest integer not below value.
        int ceilTo(double value) { return static_cast<int>(std::ceil(value)); }

        /// @brief Sets the left edge of a rectangle, keeping the other edges.
        /// @param rect Rectangle changed in place.
        /// @param left New left edge.
        void setLeft(core::Rect& rect, int left) { rect = core::Rect::fromEdges(left, rect.top(), rect.right(), rect.bottom()); }

        /// @brief Sets the top edge of a rectangle, keeping the other edges.
        /// @param rect Rectangle changed in place.
        /// @param top New top edge.
        void setTop(core::Rect& rect, int top) { rect = core::Rect::fromEdges(rect.left(), top, rect.right(), rect.bottom()); }

        /// @brief Sets the right edge of a rectangle, keeping the other edges.
        /// @param rect Rectangle changed in place.
        /// @param right New right edge.
        void setRight(core::Rect& rect, int right) { rect = core::Rect::fromEdges(rect.left(), rect.top(), right, rect.bottom()); }

        /// @brief Sets the bottom edge of a rectangle, keeping the other edges.
        /// @param rect Rectangle changed in place.
        /// @param bottom New bottom edge.
        void setBottom(core::Rect& rect, int bottom) { rect = core::Rect::fromEdges(rect.left(), rect.top(), rect.right(), bottom); }

        /// @brief Names the backdrop picture of some scene types.
        /// @details Original: BuildMiniMap.
        /// @param sceneType Scene type.
        /// @return Picture name, or nullptr when the scene has no backdrop.
        const char* backdropOf(int sceneType) {
            switch (sceneType) {
                case 6: return "MiniMap_1"; // scene type 6: backdrop MiniMap_1
                case 9: return "MiniMap_3"; // scene type 9: backdrop MiniMap_3
                case 10: return "MiniMap_4"; // scene type 10: backdrop MiniMap_4
                case 11: return "MiniMap_2"; // scene type 11: backdrop MiniMap_2
                case 12: return "MiniMap_5"; // scene type 12: backdrop MiniMap_5
                default: return nullptr; // other scene types: no backdrop
            }
        }

    }

    TileMiniMap::TileMiniMap(
        GameServices& services,
        ui::Screen& screen,
        const MapCharacter& player,
        const std::vector<MapNpc>& npcs
    )
        : m_services(services),
          m_player(player),
          m_npcs(npcs),
          m_panel(screen.load(services.game.layout("TileMiniMap.xml"), services.loadContext())),
          m_picture(screen.get<ui::ImageView>("TileMiniMapPicture")),
          m_marks(screen.get<ui::DrawingArea>("TileMiniMapMarks")),
          m_minimize(screen.get<ui::ImageButton>("TileMiniMapMinimize")),
          m_formSpot(screen.get<ui::Spot>("TileMiniMapForm")),
          m_windowSpot(screen.get<ui::Spot>("TileMiniMapWindow")),
          m_buttonSpot(screen.get<ui::Spot>("TileMiniMapButton")),
          m_foldedSpot(screen.get<ui::Spot>("TileMiniMapFolded")),
          m_arrow(services.pictures.find("Btn_ArrowDn_2").value_or(gfx::PictureId{})),
          m_at(m_formSpot.at()) {
        m_marks.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            const core::Point origin = canvas.origin();
            canvas.setOrigin(origin + at - m_marks.bounds().position());
            paint(canvas, nowMs);
            canvas.setOrigin(origin);
        });
        screen.on("onTileMiniMapMinimize", [this](const ui::EventArgs&) { toggleSize(); });
        screen.on("onTileMiniMapPress", [this](const ui::EventArgs& args) {
            m_grab = args.position;
            m_dragging = true;
        });
        screen.on("onTileMiniMapDrag", [this](const ui::EventArgs& args) {
            if (!m_dragging) {
                return;
            }
            const core::Rect area = m_marks.bounds();
            const core::Point mouse{area.x + args.position.x, area.y + args.position.y};
            m_at = FormDrag::dragForm({mouse.x - m_grab.x, mouse.y - m_grab.y, area.width, area.height}, windowArea()).position();
            applyBounds();
        });
        screen.on("onTileMiniMapRelease", [this](const ui::EventArgs&) { m_dragging = false; });
    }

    void TileMiniMap::showMap(const WorldMap& map, int sceneType) {
        m_built = false;
        m_panel.setVisible(false);
        // only maps of tile mode 1 (built from tiles) get this mini map; scene types 6, 9-12 below pick a backdrop
        if (map.file().tileMode() != 1 || m_hidden) {
            return;
        }
        build(map, sceneType);
    }

    void TileMiniMap::setHidden(bool hidden) {
        m_hidden = hidden;
        m_panel.setVisible(m_built && !m_hidden);
    }

    void TileMiniMap::build(const WorldMap& map, int sceneType) {
        m_map = map;
        m_dragging = false;
        const int columns = map.file().tileColumns();
        const int rows = map.file().tileRows();
        const int fitScale = ceilTo(static_cast<double>(columns * kTileHeight + rows * kTileHeight) / m_windowSpot.size().width);
        const int heightScale = ceilTo(static_cast<double>(columns * kHalfTileHeight + rows * kHalfTileHeight) / m_windowSpot.size().height);
        m_scale = std::max(1, std::max(fitScale, heightScale));
        m_cellWidth = kTileHeight / m_scale;
        m_cellHeight = kHalfTileHeight / m_scale;
        m_center = {
            (m_windowSpot.size().width - (columns * m_cellWidth + rows * m_cellWidth)) / 2,
            (m_windowSpot.size().height - (columns * m_cellHeight + rows * m_cellHeight)) / 2
        };
        if (m_minimized) {
            toggleSize();
        }
        m_standing.clear();
        int ranked = 0;
        for (const GroundObject& object : map.objects()) {
            // object kind 3 stands among the people
            if (object.info.kind == 3 && static_cast<int>(m_standing.size()) < kMostStanding) {
                ++ranked;
                Standing standing;
                standing.isGroundObject = true;
                standing.index = ranked;
                if (ranked <= static_cast<int>(map.objects().size())) {
                    standing.sortKey = map.objects()[static_cast<std::size_t>(ranked - 1)].footY();
                }
                m_standing.push_back(standing);
            }
        }
        for (std::size_t at = 0; at < map.tiles().size(); ++at) {
            if (map.tiles()[at].object.isValid() && static_cast<int>(m_standing.size()) < kMostStanding) {
                Standing standing;
                standing.index = static_cast<int>(at);
                standing.sortKey = map.tiles()[at].sortY;
                m_standing.push_back(standing);
            }
        }
        std::stable_sort(m_standing.begin(), m_standing.end(), [](const Standing& a, const Standing& b) { return a.sortKey < b.sortKey; });
        const char* backdrop = backdropOf(sceneType);
        m_backdrop = backdrop ? m_services.pictures.find(backdrop).value_or(gfx::PictureId{}) : gfx::PictureId{};
        m_arrowMs = 0;
        m_built = true;
        applyBounds();
        m_panel.setVisible(map.id() != kNoMiniMap && !m_hidden);
    }

    void TileMiniMap::toggleSize() {
        m_dragging = false;
        if (!m_minimized) {
            m_at = {
                m_at.x + m_buttonSpot.at().x - (m_foldedSpot.size().width - m_buttonSpot.size().width) / 2,
                m_at.y + m_buttonSpot.at().y - (m_foldedSpot.size().height - m_buttonSpot.size().height) / 2
            };
            m_minimize.setPicture(m_services.pictures.find("Btn_Window_1").value_or(gfx::PictureId{}));
            m_minimize.setHint("Open the map");
            m_picture.setPicture(m_services.pictures.find("Form_HotKey_Mini").value_or(gfx::PictureId{}));
            m_minimized = true;
        } else {
            // 117 px left and 2 px up of the button is the form's corner; 800 is the screen width, px
            m_at = {std::min(m_at.x + m_foldedSpot.at().x - 117, 800 - m_formSpot.size().width), m_at.y + m_foldedSpot.at().y - 2};
            m_minimize.setPicture(m_services.pictures.find("Btn_Minimize_1").value_or(gfx::PictureId{}));
            m_minimize.setHint("Minimize map");
            m_picture.setPicture(m_services.pictures.find("MiniMapForm").value_or(gfx::PictureId{}));
            m_minimized = false;
        }
        applyBounds();
    }

    void TileMiniMap::applyBounds() {
        if (m_minimized) {
            m_picture.setBounds({m_at.x, m_at.y, m_foldedSpot.size().width, m_foldedSpot.size().height});
            m_minimize.setBounds({m_at.x + m_foldedSpot.at().x, m_at.y + m_foldedSpot.at().y, m_buttonSpot.size().width, m_buttonSpot.size().height});
            m_marks.setBounds({m_at.x, m_at.y, m_foldedSpot.size().width, m_foldedSpot.size().height});
        } else {
            m_picture.setBounds({m_at.x, m_at.y, m_formSpot.size().width, m_formSpot.size().height});
            m_minimize.setBounds({m_at.x + m_buttonSpot.at().x, m_at.y + m_buttonSpot.at().y, m_buttonSpot.size().width, m_buttonSpot.size().height});
            m_marks.setBounds({m_at.x, m_at.y, m_formSpot.size().width, m_formSpot.size().height});
        }
    }

    TileMiniMap::TileSpot TileMiniMap::tileAt(core::Point mapPoint) const {
        constexpr int kMostSteps = 255; // search limit for the tile row / column
        const int columns = m_map->get().file().tileColumns();
        TileSpot spot;
        while (spot.row <= kMostSteps &&
               mapPoint.y - static_cast<double>((mapPoint.x - (columns * kTileHeight - spot.row * kTileWidth)) * kTileHeight) / kTileWidth > 0) {
            ++spot.row;
        }
        --spot.row;
        while (spot.column <= kMostSteps &&
               mapPoint.y + static_cast<double>((mapPoint.x - (columns * kTileHeight + spot.column * kTileWidth)) * kTileHeight) / kTileWidth > 0) {
            ++spot.column;
        }
        --spot.column;
        spot.offset = {
            mapPoint.x - (columns * kTileHeight + (spot.column - 1) * kTileHeight - (spot.row - 1) * kTileHeight - kTileHeight),
            mapPoint.y - ((spot.column - 1) * kHalfTileHeight + (spot.row - 1) * kHalfTileHeight)
        };
        return spot;
    }

    void TileMiniMap::clipToWindow(core::Rect& destination, core::Rect& source) const {
        const auto share = [](int part, int whole) { return whole != 0 ? static_cast<double>(part) / whole : 0.0; };
        if (destination.top() < 0) {
            core::Rect shown = destination;
            setTop(shown, 0);
            setTop(source, source.bottom() - roundTo(share(shown.height, destination.height) * source.bottom()));
            destination = shown;
        }
        if (destination.left() < 0) {
            core::Rect shown = destination;
            setLeft(shown, 0);
            setLeft(source, roundTo(share(shown.width, destination.width) * source.width));
            destination = shown;
        }
        if (destination.right() > m_windowSpot.size().width) {
            core::Rect shown = destination;
            setRight(shown, m_windowSpot.size().width);
            setRight(source, roundTo(share(shown.width, destination.width) * source.right()));
            destination = shown;
        }
        if (destination.bottom() > m_windowSpot.size().height) {
            core::Rect shown = destination;
            setBottom(shown, m_windowSpot.size().height);
            setBottom(source, source.bottom() - roundTo(share(shown.height, destination.height) * source.bottom()));
            destination = shown;
        }
    }

    void TileMiniMap::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_built || m_minimized || m_hidden || !m_map) {
            return;
        }
        const WorldMap& map = m_map->get();
        const core::Point origin{m_at.x + m_windowSpot.at().x, m_at.y + m_windowSpot.at().y};
        const core::Rect window{origin.x, origin.y, m_formSpot.size().width - 2 * m_windowSpot.at().x, m_formSpot.size().height - 2 * m_windowSpot.at().y};
        canvas.setClip(window);
        canvas.fillRect(window, {0, 0, 0, 255}); // opaque black RGBA
        if (m_backdrop.isValid()) {
            canvas.drawPictureScaled(m_backdrop, window);
        }
        drawTiles(canvas, map);
        drawGroundObjects(canvas, map, 1, 2); // flat objects (kinds 1 and 2)
        drawStanding(canvas, map);
        drawGroundObjects(canvas, map, 4, 5); // objects over everything (kinds 4 and 5)
        canvas.setClip(std::nullopt);
        drawSigns(canvas);
        drawArrows(canvas);
        drawPlayer(canvas, nowMs);
    }

    void TileMiniMap::drawTiles(gfx::Canvas& canvas, const WorldMap& map) const {
        const int columns = map.file().tileColumns();
        const core::Point origin{m_at.x + m_windowSpot.at().x, m_at.y + m_windowSpot.at().y};
        for (const GroundTile& tile : map.tiles()) {
            if (!tile.ground.isValid()) {
                continue;
            }
            const int tileX = columns * m_cellWidth + (tile.row - 1) * m_cellWidth - (tile.column - 1) * m_cellWidth - m_cellWidth;
            const int tileY = (tile.row - 1) * m_cellHeight + (tile.column - 1) * m_cellHeight + 1;
            core::Rect destination = core::Rect::fromEdges(
                tileX + m_center.x,
                tileY + m_center.y,
                m_cellWidth * 2 + tileX + m_center.x + 1,
                m_cellHeight * 2 + tileY + m_center.y + 1
            );
            // ground pictures 140..144 are tall: their source starts 26 px lower
            const bool tall = tile.groundNumber >= kFirstGroundVariantOfTall && tile.groundNumber < kLastGroundVariantOfTall;
            core::Rect source = core::Rect::fromEdges(0, tall ? kTallGroundExtra : 0, kTileWidth, tall ? kTileHeight + kTallGroundExtra : kTileHeight);
            if (destination.right() > m_windowSpot.size().width) {
                core::Rect clipped = destination;
                setRight(clipped, m_windowSpot.size().width);
                setRight(source, roundTo(static_cast<double>(clipped.width) / destination.width * source.right()));
                destination = clipped;
            }
            if (destination.bottom() > m_windowSpot.size().height) {
                core::Rect clipped = destination;
                setBottom(clipped, m_windowSpot.size().height);
                setBottom(source, roundTo(static_cast<double>(clipped.height) / destination.height * source.bottom()));
                destination = clipped;
            }
            if (!destination.isEmpty() && !source.isEmpty()) {
                canvas.drawPictureScaled(tile.ground, destination.translated(origin), source);
            }
        }
    }

    void TileMiniMap::drawGroundObjects(gfx::Canvas& canvas, const WorldMap& map, int firstKind, int lastKind) const {
        for (const GroundObject& object : map.objects()) {
            if (object.info.kind >= firstKind && object.info.kind <= lastKind) {
                drawGroundObject(canvas, object, true);
            }
        }
    }

    void TileMiniMap::drawGroundObject(gfx::Canvas& canvas, const GroundObject& object, bool roundWidthUp) const {
        if (!object.picture.isValid()) {
            return;
        }
        const int columns = m_map->get().file().tileColumns();
        const TileSpot spot = tileAt({object.position.x, object.position.y + 100}); // objects are placed 100 map units below their position
        const int x = roundTo((kTileWidth / m_scale) * (spot.offset.x / static_cast<double>(kTileWidth))) +
                      (columns * m_cellWidth + (spot.column - 1) * m_cellWidth - (spot.row - 1) * m_cellWidth - m_cellWidth);
        const int y = roundTo((kTileHeight / m_scale) * (spot.offset.y / static_cast<double>(kTileHeight))) +
                      ((spot.column - 1) * m_cellHeight + (spot.row - 1) * m_cellHeight - m_cellHeight);
        const int frameHeight = object.info.height / object.info.frames;
        const int left = x + m_center.x;
        const int top = y + m_center.y;
        core::Rect destination = core::Rect::fromEdges(
            left,
            top,
            (roundWidthUp ? ceilTo(static_cast<double>(object.info.width) / m_scale) : object.info.width / m_scale) + left,
            frameHeight / m_scale + top
        );
        core::Rect source{0, 0, object.info.width, frameHeight};
        clipToWindow(destination, source);
        destination.width += 1;
        destination.height += 1;
        if (!destination.isEmpty() && !source.isEmpty()) {
            canvas.drawPictureScaled(object.picture, destination.translated({m_at.x + m_windowSpot.at().x, m_at.y + m_windowSpot.at().y}), source);
        }
    }

    void TileMiniMap::drawStanding(gfx::Canvas& canvas, const WorldMap& map) const {
        for (const Standing& standing : m_standing) {
            if (!standing.isGroundObject) {
                drawTileObject(canvas, map.tiles()[static_cast<std::size_t>(standing.index)]);
            } else if (standing.index <= static_cast<int>(map.objects().size())) {
                drawGroundObject(canvas, map.objects()[static_cast<std::size_t>(standing.index - 1)], false);
            }
        }
    }

    void TileMiniMap::drawTileObject(gfx::Canvas& canvas, const GroundTile& tile) const {
        const int columns = m_map->get().file().tileColumns();
        const int lift = tile.objectBase == kBrickBase ? 1 : 0;
        const int x = columns * m_cellWidth + (tile.row - 1) * m_cellWidth - (tile.column - 1) * m_cellWidth - m_cellWidth;
        const int y = (tile.row - 1) * m_cellHeight + (tile.column - 1) * m_cellHeight - m_cellHeight - lift;
        core::Rect destination =
            core::Rect::fromEdges(x + m_center.x, y + m_center.y, m_cellWidth * 2 + x + m_center.x, kObjectHeight / m_scale + y + m_center.y - 1);
        core::Rect source = core::Rect::fromEdges(0, 0, kObjectSourceWidth, kObjectSourceHeight);
        clipToWindow(destination, source);
        destination.width += 1;
        destination.height += 1;
        if (!destination.isEmpty() && !source.isEmpty()) {
            canvas.drawPictureScaled(tile.object, destination.translated({m_at.x + m_windowSpot.at().x, m_at.y + m_windowSpot.at().y}), source);
        }
    }

    core::Point TileMiniMap::screenOfTile(core::Point mapPoint) const {
        const int columns = m_map->get().file().tileColumns();
        const TileSpot spot = tileAt(mapPoint);
        return {
            roundTo((kTileWidth / m_scale) * (spot.offset.x / static_cast<double>(kTileWidth))) +
                (columns * m_cellWidth + (spot.column - 1) * m_cellWidth - (spot.row - 1) * m_cellWidth - m_cellWidth + m_at.x - 2) + m_center.x,
            roundTo((kTileHeight / m_scale) * (spot.offset.y / static_cast<double>(kTileHeight))) +
                ((spot.column - 1) * m_cellHeight + (spot.row - 1) * m_cellHeight + m_at.y) + m_center.y
        };
    }

    void TileMiniMap::drawSign(gfx::Canvas& canvas, core::Point mapPoint, int sign) const {
        const auto picture = m_services.pictures.find("Icon_Sign_" + std::to_string(sign));
        if (!picture) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*picture);
        const core::Point at = screenOfTile(mapPoint);
        canvas.drawPicture(*picture, {at.x - data.width() / 2 + 5, at.y - data.height() / 2}); // sign is shifted 5 px right
    }

    void TileMiniMap::drawSigns(gfx::Canvas& canvas) const {
        for (const MapNpc& npc : m_npcs) {
            if (npc.visible && npc.mapSign != 0) {
                drawSign(canvas, npc.body.position, npc.mapSign);
            }
        }
        for (const formats::SceneDoor& door : m_doors) {
            const auto sign = m_doorSigns.find(door.index);
            if (sign != m_doorSigns.end() && sign->second != 0) {
                drawSign(canvas, door.lightPosition, sign->second);
            }
        }
    }

    void TileMiniMap::drawArrows(gfx::Canvas& canvas) const {
        if (!m_arrow.isValid()) {
            return;
        }
        for (const formats::SceneDoor& door : m_doors) {
            if (door.cursorKind() != 0) {
                const core::Point at = screenOfTile(door.lightPosition);
                canvas.drawPictureScaled(
                    m_arrow,
                    {at.x, at.y, kArrowWidth, kArrowHeight},
                    core::Rect{0, m_arrowFrame * kArrowHeight - kArrowHeight, kArrowWidth, kArrowHeight}
                );
            }
        }
    }

    void TileMiniMap::drawPlayer(gfx::Canvas& canvas, std::uint64_t nowMs) {
        const int columns = m_map->get().file().tileColumns();
        const TileSpot spot = tileAt(m_player.position);
        const int left = ceilTo(static_cast<double>(kTileWidth) / m_scale * (spot.offset.x / static_cast<double>(kTileWidth))) +
                         (columns * m_cellWidth + (spot.column - 1) * m_cellWidth - (spot.row - 1) * m_cellWidth - m_cellWidth + m_at.x - 2);
        const int top = ceilTo(static_cast<double>(kTileHeight) / m_scale * (spot.offset.y / static_cast<double>(kTileHeight))) +
                        ((spot.column - 1) * m_cellHeight + (spot.row - 1) * m_cellHeight + m_at.y) + 2; // face is 2 px lower
        m_services.characters.drawPortrait(canvas, m_player.look, {ceilTo(kArrowWidth / 2.0) + left + m_center.x, top + m_center.y}, 0, kMarkerSlot);
        if (nowMs - m_arrowMs >= kArrowFrameMs) {
            m_arrowFrame = m_arrowFrame % kArrowFrames + 1;
            m_arrowMs = nowMs;
        }
    }

}
