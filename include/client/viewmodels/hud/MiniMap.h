#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_MINI_MAP_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_MINI_MAP_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/world/MapCharacter.h"
#include "client/viewmodels/hud/TileMiniMap.h"
#include "client/models/world/MapNpc.h"
#include "client/models/world/WorldMap.h"
#include "core/Point.h"
#include "formats/SceneDoor.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <map>
#include <vector>

namespace client {

    /// @brief The small world map at the top right of the world, with the player's face and NPC and door signs.
    /// @details Original: MiniMap.xml, CH_TMiniMapForm_World; the map's picture is "<map>_Mini_Form". Maps without such a picture show none; maps built from tiles have their own small map, TileMiniMap.
    class MiniMap {
        public:
            /// @brief Loads the mini map layout and creates the tile mini map.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param player The own player on the map.
            /// @param npcs NPCs of the current map.
            MiniMap(
                GameServices& services,
                ui::Screen& screen,
                const MapCharacter& player,
                const std::vector<MapNpc>& npcs
            );

            /// @brief Sets the sign shown for a door.
            /// @param doorIndex Door index (its eve.Emg index).
            /// @param sign Sign number; 0 takes it away.
            void setDoorSign(int doorIndex, int sign) {
                m_doorSigns[doorIndex] = sign;
                m_tile.setDoorSign(doorIndex, sign);
            }

            /// @brief Sets the doors of the map and resets their signs to the map's own.
            /// @details Original: every map load puts each door's sign back to the eve's (byte $18, BK_EventManage.pas:2751).
            /// @param doors Doors of the new map.
            void setDoors(const std::vector<formats::SceneDoor>& doors);

            /// @brief Shows the mini map of a new map.
            /// @details Original: SetupWorldMiniMap; the form is set up only for scenes of type 1 (LoadGameMap, DZ_Game).
            /// @param map The new map.
            /// @param sceneType Type of its scene.
            void showMap(const WorldMap& map, int sceneType);

            /// @brief Brings the form over the forms made after it.
            /// @details Original: SetupWorldMiniMap's ShowCtrl on a map change.
            void raise() { m_screen.context().bringToFront(m_panel.id()); }

            /// @brief Hides or shows the map while the battle screen is up.
            /// @details Original: HideCtrl while the fight field is up.
            /// @param hidden Whether the map is hidden.
            void setHidden(bool hidden);

        private:
            /// @brief The window in the panel's coordinates (a bigger window lets the form go to its edges).
            /// @return Window rectangle relative to the panel.
            core::Rect windowArea() const {
                const core::Rect& view = m_services.view;
                const core::Point panel = m_panel.bounds().position();
                return {view.x - panel.x, view.y - panel.y, view.width, view.height};
            }

            /// @brief Paints the NPC and door signs, then the own player's face.
            /// @param canvas Target canvas.
            void paint(gfx::Canvas& canvas);

            /// @brief Minimizes or opens the map.
            /// @details Original: ToggleWorldMapSize.
            void toggleSize();

            /// @brief Converts a map point to a point on the mini map picture.
            /// @details Original: WorldPosToMiniMap.
            /// @param mapPoint Point in map coordinates.
            /// @return Point on screen.
            core::Point toMiniMap(core::Point mapPoint) const;

            /// @brief Moves the form (picture, marks and button) to a top left, pulled onto the screen as TSe_Form.RenderWithChildren does.
            /// @param topLeft New top left.
            /// @param dragging Whether the move comes from a mouse drag.
            void moveTo(core::Point topLeft, bool dragging);

            /// @brief Draws the sign of an NPC or door centred on its map point.
            /// @param canvas Target canvas.
            /// @param mapPoint Point in map coordinates.
            /// @param sign Sign number (picture Icon_Sign_N).
            void drawSign(gfx::Canvas& canvas, core::Point mapPoint, int sign);

            GameServices& m_services;
            ui::Screen& m_screen;
            TileMiniMap m_tile;
            const MapCharacter& m_player;
            const std::vector<MapNpc>& m_npcs;
            std::map<int, core::Point> m_doorPoints;
            std::map<int, int> m_doorSigns;
            ui::Element& m_panel;
            ui::ImageView& m_picture;
            ui::ImageButton& m_minimize;
            ui::DrawingArea& m_marks;
            const ui::Spot& m_topSpot;
            const ui::Spot& m_foldedSpot;
            int m_mapId = 0;
            int m_mapHeight = 0;
            int m_scale = 1;
            bool m_hasPicture = false;
            bool m_hidden = false;
            bool m_minimized = false;
            core::Point m_grab;
            bool m_dragging = false;
    };

}

#endif
