#include "client/viewmodels/hud/MiniMap.h"

#include "client/rules/FormDrag.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game window width, px (the mini map hugs the right edge)
        constexpr int kMarkerSlot = 4; // face slot 4: the tiny player marker face

        /// @brief Names the mini map picture of a map; the maps of the newbie islands share one picture.
        /// @param mapId Map id.
        /// @return Picture name "<map>_Mini_Form".
        std::string pictureName(int mapId) {
            // map ids 10031..10040 (newbie islands) use the picture of map 10003 (Newbie Island)
            const int pictureMap = mapId >= 10031 && mapId <= 10040 ? 10003 : mapId;
            return std::to_string(pictureMap) + "_Mini_Form";
        }

        /// @brief Pixels the player marker is lifted: the marker sits a little higher the further down the map.
        /// @details Original: SetupWorldMiniMap.
        /// @param mapY Y position on the map.
        /// @param mapHeight Map height.
        /// @return Lift in pixels.
        int markerLift(int mapY, int mapHeight) {
            // lift steps by map Y bands (1000/2000/3000/4000/5000): 6, 8, 10, 16, 32, then 36
            const int k = mapY <= 1000 ? 6 : mapY <= 2000 ? 8 : mapY <= 3000 ? 10 : mapY <= 4000 ? 16 : mapY <= 5000 ? 32 : 36;
            return k - static_cast<int>(std::lround(static_cast<double>(mapY) / mapHeight * k));
        }

    }

    MiniMap::MiniMap(
        GameServices& services,
        ui::Screen& screen,
        const MapCharacter& player,
        const std::vector<MapNpc>& npcs
    )
        : m_services(services),
          m_screen(screen),
          m_tile(services, screen, player, npcs),
          m_player(player),
          m_npcs(npcs),
          m_panel(screen.load(services.game.layout("MiniMap.xml"), services.loadContext())),
          m_picture(screen.get<ui::ImageView>("MiniMapPicture")),
          m_minimize(screen.get<ui::ImageButton>("MiniMapMinimize")),
          m_marks(screen.get<ui::DrawingArea>("MiniMapMarks")),
          m_topSpot(screen.get<ui::Spot>("MiniMapTop")),
          m_foldedSpot(screen.get<ui::Spot>("MiniMapFolded")) {
        m_marks.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            const core::Point origin = canvas.origin();
            canvas.setOrigin(origin + at - m_marks.bounds().position());
            paint(canvas);
            canvas.setOrigin(origin);
        });
        screen.on("onMiniMapMinimize", [this](const ui::EventArgs&) { toggleSize(); });
        screen.on("onMiniMapPress", [this](const ui::EventArgs& args) {
            m_grab = args.position;
            m_dragging = true;
        });
        screen.on("onMiniMapDrag", [this](const ui::EventArgs& args) {
            if (!m_dragging) {
                return;
            }
            const core::Rect area = m_marks.bounds();
            const core::Point mouse{area.x + args.position.x, area.y + args.position.y};
            const core::Rect moved = FormDrag::dragForm({mouse.x - m_grab.x, mouse.y - m_grab.y, area.width, area.height}, windowArea());
            moveTo(moved.position(), true);
        });
        screen.on("onMiniMapRelease", [this](const ui::EventArgs&) { m_dragging = false; });
    }

    void MiniMap::moveTo(core::Point topLeft, bool dragging) {
        const core::Rect area = m_picture.bounds();
        const core::Rect fitted = FormDrag::fitForm({topLeft.x, topLeft.y, area.width, area.height}, dragging, windowArea());
        const core::Point button = m_minimize.bounds().position();
        m_minimize.setPosition({button.x + fitted.x - area.x, button.y + fitted.y - area.y});
        m_picture.setBounds(fitted);
        m_marks.setBounds(fitted);
    }

    void MiniMap::showMap(const WorldMap& map, int sceneType) {
        m_tile.showMap(map, sceneType);
        const int mapWidth = map.width();
        m_mapId = map.id();
        m_mapHeight = map.height();
        constexpr int kWorldMapScene = 1; // scene type 1: a world map (outdoor map with a mini map picture)
        const auto picture = sceneType == kWorldMapScene ? m_services.pictures.find(pictureName(map.id())) : std::nullopt;
        m_hasPicture = picture.has_value();
        m_panel.setVisible(m_hasPicture && !m_hidden);
        if (!picture) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*picture);
        m_minimized = false;
        m_dragging = false;
        m_minimize.setPicture(m_services.pictures.find("Btn_Minimize_1").value_or(gfx::PictureId{}));
        m_minimize.setHint("Minimize map");
        m_picture.setPicture(*picture);
        m_picture.setBounds({kScreenWidth - data.width(), m_topSpot.at().y, data.width(), data.height()});
        m_marks.setBounds(m_picture.bounds());
        m_minimize.setPosition({kScreenWidth - m_minimize.bounds().width, m_topSpot.at().y});
        m_scale = std::max(1, mapWidth / data.width());
    }

    void MiniMap::toggleSize() {
        const core::Rect button = m_minimize.bounds();
        m_dragging = false;
        if (!m_minimized) {
            const int boxX = std::min(button.x - m_foldedSpot.at().x, kScreenWidth - m_foldedSpot.size().width);
            const int boxY = button.y - (m_foldedSpot.size().height - button.height) / 2;
            m_picture.setPicture(m_services.pictures.find("Form_HotKey_Mini").value_or(gfx::PictureId{}));
            m_picture.setBounds({boxX, boxY, m_foldedSpot.size().width, m_foldedSpot.size().height});
            m_minimize.setPosition({boxX + m_foldedSpot.at().x, boxY + m_foldedSpot.at().y});
            m_minimize.setPicture(m_services.pictures.find("Btn_Window_1").value_or(gfx::PictureId{}));
            m_minimize.setHint("Open the map");
            m_minimized = true;
        } else {
            const auto picture = m_services.pictures.find(pictureName(m_mapId));
            if (!picture) {
                return;
            }
            const gfx::Picture& data = m_services.pictures.picture(*picture);
            m_picture.setPicture(*picture);
            m_picture.setBounds({button.x + button.width - data.width(), button.y, data.width(), data.height()});
            m_minimize.setPosition(button.position());
            m_minimize.setPicture(m_services.pictures.find("Btn_Minimize_1").value_or(gfx::PictureId{}));
            m_minimize.setHint("Minimize map");
            m_minimized = false;
        }
        m_marks.setBounds(m_picture.bounds());
        moveTo(m_picture.bounds().position(), false);
    }

    void MiniMap::setHidden(bool hidden) {
        m_hidden = hidden;
        m_tile.setHidden(hidden);
        m_panel.setVisible(m_hasPicture && !m_hidden);
    }

    void MiniMap::setDoors(const std::vector<formats::SceneDoor>& doors) {
        m_tile.setDoors(doors);
        m_doorPoints.clear();
        m_doorSigns.clear();
        for (const formats::SceneDoor& door : doors) {
            m_doorPoints[door.index] = door.lightPosition;
            m_doorSigns[door.index] = door.mapSign;
        }
    }

    core::Point MiniMap::toMiniMap(core::Point mapPoint) const {
        const core::Rect area = m_picture.bounds();
        // the marker is 3 px left of the mapped x
        return {mapPoint.x / m_scale + area.x - 3, mapPoint.y / m_scale + area.y - markerLift(mapPoint.y, m_mapHeight)};
    }

    void MiniMap::drawSign(gfx::Canvas& canvas, core::Point mapPoint, int sign) {
        if (const auto picture = m_services.pictures.find("Icon_Sign_" + std::to_string(sign))) {
            const gfx::Picture& data = m_services.pictures.picture(*picture);
            const core::Point at = toMiniMap(mapPoint);
            canvas.drawPicture(*picture, {at.x - data.width() / 2, at.y - data.height() / 2});
        }
    }

    void MiniMap::paint(gfx::Canvas& canvas) {
        if (m_mapHeight <= 0 || m_minimized) {
            return;
        }
        for (const MapNpc& npc : m_npcs) {
            if (npc.visible && npc.mapSign != 0) {
                drawSign(canvas, npc.body.position, npc.mapSign);
            }
        }
        for (const auto& [door, sign] : m_doorSigns) {
            const auto point = m_doorPoints.find(door);
            if (sign != 0 && point != m_doorPoints.end()) {
                drawSign(canvas, point->second, sign);
            }
        }
        m_services.characters.drawPortrait(canvas, m_player.look, toMiniMap(m_player.position), 0, kMarkerSlot);
    }

}
