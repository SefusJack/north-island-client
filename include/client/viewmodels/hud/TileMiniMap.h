#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_TILE_MINI_MAP_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_TILE_MINI_MAP_H

#include "client/services/GameServices.h"
#include "client/models/world/MapCharacter.h"
#include "client/models/world/MapNpc.h"
#include "client/models/world/WorldMap.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "formats/SceneDoor.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace client {

    /// @brief The small map of maps built from tiles: tiles and objects shrunk into the 214 x 132 window of the MiniMapForm picture, then NPC and door signs, door arrows and the own player's face.
    /// @details Original: TileMiniMap.xml, CH_TMiniMapForm. The Minimize button folds it into a small box at its place.
    class TileMiniMap {
        public:
            /// @brief Loads the tile mini map layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param player The own player on the map.
            /// @param npcs NPCs of the current map.
            TileMiniMap(
                GameServices& services,
                ui::Screen& screen,
                const MapCharacter& player,
                const std::vector<MapNpc>& npcs
            );

            /// @brief Shows the mini map of a new map, or hides it for maps not built from tiles.
            /// @details Original: BuildMiniMap for maps of tile mode 1, HideCtrl for the others.
            /// @param map The new map.
            /// @param sceneType Scene type; picks the backdrop of some scenes.
            void showMap(const WorldMap& map, int sceneType);

            /// @brief Sets the doors of the map; every map load puts each door's sign back to the eve's.
            /// @param doors Doors of the new map.
            void setDoors(const std::vector<formats::SceneDoor>& doors) {
                m_doors = doors;
                m_doorSigns.clear();
                for (const formats::SceneDoor& door : doors) {
                    m_doorSigns[door.index] = door.mapSign;
                }
            }

            /// @brief Sets the sign shown for a door.
            /// @param doorIndex Door index.
            /// @param sign Sign number; 0 shows none.
            void setDoorSign(int doorIndex, int sign) { m_doorSigns[doorIndex] = sign; }

            /// @brief Hides or shows the map while the battle screen is up.
            /// @param hidden Whether the map is hidden.
            void setHidden(bool hidden);

        private:
            /// @brief The tile a map point lies in and where in it.
            /// @details Original: MapPosToTile result.
            struct TileSpot {
                int column = 1;
                int row = 1;
                core::Point offset;
            };

            /// @brief One thing standing on the map, sorted by its foot.
            /// @details Original: TMiniItem.
            struct Standing {
                bool isGroundObject = false;
                int index = 0;
                int sortKey = 0;
            };

            /// @brief The window in the panel's coordinates (a bigger window lets the form go to its edges).
            /// @return Window rectangle relative to the panel.
            core::Rect windowArea() const {
                const core::Rect& view = m_services.view;
                const core::Point panel = m_panel.bounds().position();
                return {view.x - panel.x, view.y - panel.y, view.width, view.height};
            }

            /// @brief Works out the scale, the standing things and the look for a new map.
            /// @details Original: BuildMiniMap.
            /// @param map The new map.
            /// @param sceneType Scene type; picks the backdrop of some scenes.
            void build(const WorldMap& map, int sceneType);

            /// @brief Minimizes or opens the map: the form folds into a small box at its button and back.
            void toggleSize();

            /// @brief Puts the picture, button and marks at the form's position for its open or folded state.
            void applyBounds();

            /// @brief Paints tiles, objects, signs, arrows and the player's face.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Finds the tile a map point lies in and the offset inside it.
            /// @details Original: MapPosToTile.
            /// @param mapPoint Point in map coordinates.
            /// @return Tile column, row and offset.
            TileSpot tileAt(core::Point mapPoint) const;

            /// @brief Draws the ground tiles scaled into the window.
            /// @details Original: DrawMiniMapTiles.
            /// @param canvas Target canvas.
            /// @param map Map drawn.
            void drawTiles(gfx::Canvas& canvas, const WorldMap& map) const;

            /// @brief Draws the ground objects whose kind lies in a range.
            /// @param canvas Target canvas.
            /// @param map Map drawn.
            /// @param firstKind First object kind drawn.
            /// @param lastKind Last object kind drawn.
            void drawGroundObjects(gfx::Canvas& canvas, const WorldMap& map, int firstKind, int lastKind) const;

            /// @brief Draws one ground object scaled into the window.
            /// @param canvas Target canvas.
            /// @param object Object drawn.
            /// @param roundWidthUp Whether the scaled width rounds up (flat and high objects) or down.
            void drawGroundObject(gfx::Canvas& canvas, const GroundObject& object, bool roundWidthUp) const;

            /// @brief Draws what stands among the people, in the order of its foot.
            /// @param canvas Target canvas.
            /// @param map Map drawn.
            void drawStanding(gfx::Canvas& canvas, const WorldMap& map) const;

            /// @brief Draws the object of one tile.
            /// @param canvas Target canvas.
            /// @param tile Tile whose object is drawn.
            void drawTileObject(gfx::Canvas& canvas, const GroundTile& tile) const;

            /// @brief Draws the signs of NPCs and doors.
            /// @details Original: DrawMiniMapSigns.
            /// @param canvas Target canvas.
            void drawSigns(gfx::Canvas& canvas) const;

            /// @brief Draws one sign centred on a map point.
            /// @param canvas Target canvas.
            /// @param mapPoint Point in map coordinates.
            /// @param sign Sign number (picture Icon_Sign_N).
            void drawSign(gfx::Canvas& canvas, core::Point mapPoint, int sign) const;

            /// @brief Draws a bobbing arrow over each door with a light.
            /// @details Original: DrawEventArrows.
            /// @param canvas Target canvas.
            void drawArrows(gfx::Canvas& canvas) const;

            /// @brief Draws the own player's face and advances the arrows' frame every half second.
            /// @details Original: DrawMiniMapPlayer.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in milliseconds.
            void drawPlayer(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Shrinks a destination into the window and the source with it.
            /// @details Original: ClipToMiniMap.
            /// @param destination Destination rectangle, clipped in place.
            /// @param source Source rectangle, adjusted in place.
            void clipToWindow(core::Rect& destination, core::Rect& source) const;

            /// @brief Finds where a map point lies on screen, to the pixel the signs and arrows are placed by.
            /// @param mapPoint Point in map coordinates.
            /// @return Screen point.
            core::Point screenOfTile(core::Point mapPoint) const;

            GameServices& m_services;
            const MapCharacter& m_player;
            const std::vector<MapNpc>& m_npcs;
            std::vector<formats::SceneDoor> m_doors;
            std::map<int, int> m_doorSigns;
            ui::Element& m_panel;
            ui::ImageView& m_picture;
            ui::DrawingArea& m_marks;
            ui::ImageButton& m_minimize;
            const ui::Spot& m_formSpot;
            const ui::Spot& m_windowSpot;
            const ui::Spot& m_buttonSpot;
            const ui::Spot& m_foldedSpot;
            std::optional<std::reference_wrapper<const WorldMap>> m_map;
            std::vector<Standing> m_standing;
            gfx::PictureId m_backdrop;
            gfx::PictureId m_arrow;
            core::Point m_at;
            core::Point m_grab;
            bool m_dragging = false;
            bool m_minimized = false;
            bool m_hidden = false;
            bool m_built = false;
            int m_scale = 1;
            int m_cellWidth = 0;
            int m_cellHeight = 0;
            core::Point m_center;
            int m_arrowFrame = 1;
            std::uint64_t m_arrowMs = 0;
    };

}

#endif
