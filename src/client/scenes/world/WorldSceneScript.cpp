#include "client/services/input/ScreenPointFinder.h"
#include "client/services/input/ScriptCommand.h"
#include "client/scenes/world/WorldScene.h"

#include <string>

namespace client {

    namespace {

        constexpr core::Rect kScreen{0, 0, 800, 600}; // the game screen: x, y, width, height in px

    }

    std::optional<core::Point> WorldScene::scriptPointIn(const core::Rect& area, const std::function<bool(core::Point)>& hits) {
        return ScreenPointFinder::find(area, kScreen, [&](core::Point point) { return hits(point) && m_screen.isTopmostAt("WorldView", point); });
    }

    std::optional<ScriptSighting> WorldScene::sightNpc(const MapNpc& npc) {
        ScriptSighting sighting;
        sighting.label = "npc " + std::to_string(npc.info.npcId) + " (place " + std::to_string(npc.index) + ")";
        sighting.map = npc.body.position;
        if (!npc.visible) {
            return sighting;
        }
        const core::Point feet = m_camera.toScreen(npc.body.position);
        const int index = npc.index;
        // search area around the feet: 110 left, 220 up, 220 wide, 240 high (pixels)
        sighting.screen = scriptPointIn({feet.x - 110, feet.y - 220, 220, 240}, [&](core::Point point) {
            return npcAt(point) == index && m_ground.indexAt(point, m_camera) == 0;
        });
        return sighting;
    }

    std::optional<ScriptSighting> WorldScene::sightFurniture(int furnIndex) {
        if (!m_house.inFloorRoom()) {
            return std::nullopt;
        }
        const int slot = m_house.utensils().slotOfFurniture(furnIndex);
        const FurnitureRecord& piece = m_house.furniture().piece(furnIndex);
        if (slot == 0 || piece.itemId == 0 || piece.room != m_house.room()) {
            return std::nullopt;
        }
        ScriptSighting sighting;
        sighting.label = "furniture piece " + std::to_string(furnIndex) + " item " + std::to_string(piece.itemId) + " cell (" + std::to_string(piece.cellX) +
                         "," + std::to_string(piece.cellY) + "," + std::to_string(piece.cellZ) + ")";
        sighting.map = m_house.utensils().utensil(slot).world;
        sighting.screen = scriptPointIn(m_house.utensils().pictureArea(slot, m_camera.origin()), [&](core::Point point) {
            return m_housePainter.shelfItemAt(m_services.pictures, m_camera, point) == 0 &&
                   m_housePainter.caseItemAt(m_services.pictures, m_camera, point) == 0 &&
                   m_housePainter.pieceSlotAt(m_services.pictures, m_camera, point) == slot;
        });
        return sighting;
    }

    std::optional<ScriptSighting> WorldScene::sight(const ScriptObjectRef& object) {
        switch (object.kind) {
            case ScriptObjectKind::Self: { // the own player
                ScriptSighting sighting;
                sighting.label = "own player";
                sighting.map = m_player.position;
                sighting.screen = scriptPointIn(ownFigureArea(), [&](core::Point point) { return ownPlayerAt(point); });
                return sighting;
            }
            case ScriptObjectKind::Npc: // an NPC by its NPC id
            case ScriptObjectKind::NpcIndex: // an NPC by its place index
                for (const MapNpc& npc : m_npcs) {
                    if ((object.kind == ScriptObjectKind::Npc ? npc.info.npcId : npc.index) == object.id) {
                        return sightNpc(npc);
                    }
                }
                return std::nullopt;
            case ScriptObjectKind::Player: { // another player on the map
                const auto found = m_others.find(object.id);
                if (found == m_others.end()) {
                    return std::nullopt;
                }
                const MapCharacter& other = found->second;
                ScriptSighting sighting;
                sighting.label = "player " + std::to_string(object.id) + " " + other.name;
                sighting.map = other.position;
                const core::Rect area = m_services.characters.figureFrameArea(other.look, m_camera.toScreen(other.position), other.actionSlot());
                const std::int32_t id = object.id;
                sighting.screen = scriptPointIn(area, [&](core::Point point) { return playerAt(point) == id; });
                return sighting;
            }
            case ScriptObjectKind::Ground: { // an item lying on the ground
                const auto found = m_ground.things().find(object.id);
                if (found == m_ground.things().end()) {
                    return std::nullopt;
                }
                ScriptSighting sighting;
                sighting.label = "ground index " + std::to_string(object.id) + " item " + std::to_string(found->second.itemId);
                sighting.map = found->second.position;
                const int index = object.id;
                sighting.screen =
                    scriptPointIn(m_ground.areaOf(found->second, m_camera), [&](core::Point point) { return m_ground.indexAt(point, m_camera) == index; });
                return sighting;
            }
            case ScriptObjectKind::Furniture: // a furniture piece by its index
                return sightFurniture(object.id);
            case ScriptObjectKind::FurnitureItem: // the first furniture piece of an item id
                for (int index = 1; index <= HomeFurniture::kMost; ++index) {
                    const FurnitureRecord& piece = m_house.furniture().piece(index);
                    if (piece.itemId == object.id && piece.room == m_house.room() && m_house.utensils().slotOfFurniture(index) != 0) {
                        return sightFurniture(index);
                    }
                }
                return std::nullopt;
        }
        return std::nullopt;
    }

    bool WorldScene::startWalk(core::Point mapPoint, std::string& reason) {
        if (!canWalk()) {
            reason = "the player cannot walk now (a battle, an event, a window that holds the world, ...)";
            return false;
        }
        if (m_player.stall) {
            reason = "the player keeps a stall";
            return false;
        }
        m_walkMark.start(walkTo(mapPoint));
        return true;
    }

}
