#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_WORLD_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_WORLD_PAINTER_H

#include "client/models/world/Camp.h"
#include "client/models/house/HouseEntry.h"
#include "client/models/travel/Vehicle.h"
#include "client/rendering/characters/CharacterRenderer.h"
#include "client/models/world/Camera.h"
#include "client/models/world/GroundItems.h"
#include "client/models/world/MapCharacter.h"
#include "client/models/world/MapNpc.h"
#include "client/models/world/MapPet.h"
#include "client/models/world/WorldMap.h"
#include "formats/EmblemBlock.h"
#include "formats/ItemTable.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <vector>

namespace client {

    /// @brief Draws the world map each frame: ground, standing things sorted by y, door lights and names.
    /// @details Original: DrawMapGround, DrawSceneObjects, DrawAllHumInfo.
    class WorldPainter {
        public:
            /// @brief What the human info form lets show, plus the per-frame hooks and state the painter needs.
            /// @details Original: TH_HuminfoForm's HumInfo1..9, DrawAllHumInfo, Se_SystemForm.FOnPvpServer, DrawCamp.
            struct Display {
                std::int32_t ownId = 0;
                bool ownPetNames = true;
                bool otherPetNames = true;
                bool otherPets = true;
                bool ownNickname = true;
                bool ownTitle = true;
                bool otherNames = true;
                bool otherNicknames = true;
                bool otherTitles = true;
                bool humInfo = true;
                std::optional<core::Point> mouse;
                bool onPvpServer = false;
                std::set<std::int32_t> teamMembers;
                bool showTents = false;
                bool trojanWarMap = false;
                std::function<std::string(int)> goldEaglePrefix;
                std::function<std::string(int)> defenseNpcPrefix;
                std::function<std::string(int)> stoneEaglePrefix;
                int mapId = 0;
                std::function<void(gfx::Canvas&)> underStanding;
                std::function<void(gfx::Canvas&)> houseFloor;
                std::function<void(gfx::Canvas&, const std::vector<HouseEntry>&)> houseCells;
                int houseRoom = 0;
                std::function<std::optional<core::Point>(const MapCharacter&)> bathCell;
                std::function<bool(const MapCharacter&)> petsHidden;
                std::function<void(gfx::Canvas&, const MapCharacter&, bool afterFigure)> bathExtras;
                bool hideOwnPlayer = false;
                std::function<void(gfx::Canvas&, core::Point)> afterOwnPlayer;
                std::function<void(gfx::Canvas&, core::Point)> afterMakerPet;
                std::function<void(gfx::Canvas&, std::int32_t)> afterPlayer;
                std::function<void(gfx::Canvas&)> overStanding;
                std::function<void(gfx::Canvas&)> beforeLabels;
                std::function<core::Point(core::Point)> onWindow;
                int sceneType = 0;
                std::function<std::optional<std::reference_wrapper<const formats::EmblemBlock>>(std::uint32_t)> badgeOf;
            };

            /// @brief A thing on the ground in the drawing order, by its index.
            struct GroundEntry {
                int index = 0;
                std::reference_wrapper<const GroundItems::Item> item;
            };

            /// @brief Builds the painter over the character renderer, the picture library and the item table.
            /// @param characters Draws figures, NPCs, vehicles and signs.
            /// @param pictures Picture library for ground, plates and icons.
            /// @param items Item table (tent sprites).
            WorldPainter(
                CharacterRenderer& characters,
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items
            )
                : m_characters(characters),
                  m_pictures(pictures),
                  m_items(items) {}

            /// @brief Draws one frame of the map: ground, standing things from the top down, lights, then names.
            /// @param canvas Target canvas.
            /// @param map The map (background, tiles, objects, door lights).
            /// @param camera Camera that converts map points to the screen.
            /// @param players Players on the map.
            /// @param npcs NPCs on the map.
            /// @param pets Pets and partners on the map.
            /// @param camps Tents by slot.
            /// @param vehicles Parked vehicles by slot.
            /// @param ground Things lying on the ground.
            /// @param hoveredGround Index of the ground thing under the mouse (0: none).
            /// @param showPeople False until the server put the own player in the world; hides players and NPCs.
            /// @param display What to show and the drawing hooks.
            /// @param nowMs Current time in milliseconds.
            void paint(
                gfx::Canvas& canvas,
                WorldMap& map,
                const Camera& camera,
                const std::vector<std::reference_wrapper<MapCharacter>>& players,
                std::vector<MapNpc>& npcs,
                std::vector<MapPet>& pets,
                const std::map<int, Camp>& camps,
                std::map<int, Vehicle>& vehicles,
                const GroundItems& ground,
                int hoveredGround,
                bool showPeople,
                const Display& display,
                std::uint64_t nowMs
            );

            /// @brief Draws the ground of a movie map: background, tiles and tile objects.
            /// @details Original: DrawMovieMapGround, DrawMovieMapTiles.
            /// @param canvas Target canvas.
            /// @param map The movie's map.
            /// @param camera Camera of the movie.
            void paintMovieGround(gfx::Canvas& canvas, WorldMap& map, const Camera& camera);

            /// @brief Draws one ground object when the movie's draw list reaches it.
            /// @details Original: DrawMovieGroundObj.
            /// @param canvas Target canvas.
            /// @param object The ground object.
            /// @param camera Camera of the movie.
            /// @param nowMs Current time in milliseconds.
            void paintMovieObject(gfx::Canvas& canvas, GroundObject& object, const Camera& camera, std::uint64_t nowMs);

            /// @brief Screen area of the plate of a stall kept by someone standing at these feet (the TVenderItem that takes the mouse).
            /// @param feet Screen position of the keeper's feet.
            /// @return The plate rectangle, empty when the plate picture is missing.
            core::Rect stallPlateArea(core::Point feet) const;

        private:
            /// @brief Draws a picture at a screen point when any of it shows.
            /// @param canvas Target canvas.
            /// @param picture Picture to draw.
            /// @param at Screen position.
            void drawPicture(gfx::Canvas& canvas, gfx::PictureId picture, core::Point at);

            /// @brief Draws the ground of a map built from tiles over its background.
            /// @details Original: DrawMapTiles, DrawMapLargeTiles.
            /// @param canvas Target canvas.
            /// @param map The map.
            /// @param camera Camera of the view.
            void drawTiles(gfx::Canvas& canvas, const WorldMap& map, const Camera& camera);

            /// @brief Draws a ground object's current frame; translucent and marked objects are drawn as lights.
            /// @param canvas Target canvas.
            /// @param object The ground object (its frame advances).
            /// @param camera Camera of the view.
            /// @param nowMs Current time in milliseconds.
            void drawObject(gfx::Canvas& canvas, GroundObject& object, const Camera& camera, std::uint64_t nowMs);

            /// @brief Draws a door's glow, a quarter of its height sunk into the ground, and advances its frame.
            /// @details Original: DrawDoorLight.
            /// @param canvas Target canvas.
            /// @param light The door light.
            /// @param camera Camera of the view.
            /// @param nowMs Current time in milliseconds.
            void drawDoorLight(gfx::Canvas& canvas, DoorLight& light, const Camera& camera, std::uint64_t nowMs);

            /// @brief Draws a player's figure and the icons over it (counsellor fan, team banner, float text, wait icon).
            /// @details Original: DrawBasePlayer, DrawTeamBanner, DrawWaitEventIcon.
            /// @param canvas Target canvas.
            /// @param player The player.
            /// @param camera Camera of the view.
            /// @param nowMs Current time in milliseconds.
            void drawPlayer(gfx::Canvas& canvas, MapCharacter& player, const Camera& camera, std::uint64_t nowMs);

            /// @brief Draws only a player's body in whichever form it takes (battle sign, vehicle, pet, swimmer, shape, plain figure).
            /// @details Original: DrawHumanSprite, DrawHumanLayers, THuman.Draw.
            /// @param canvas Target canvas.
            /// @param player The player.
            /// @param camera Camera of the view.
            /// @param nowMs Current time in milliseconds.
            void drawPlayerFigure(gfx::Canvas& canvas, MapCharacter& player, const Camera& camera, std::uint64_t nowMs);

            /// @brief Draws a player riding a pet: the pet under him and the rider on it in the riding pose.
            /// @details Original: HandleRidePetMountPacket, DrawHumanSprite, SetRidingAction.
            /// @param canvas Target canvas.
            /// @param player The rider.
            /// @param feet Screen position of his feet.
            /// @param nowMs Current time in milliseconds.
            void drawRider(gfx::Canvas& canvas, MapCharacter& player, core::Point feet, std::uint64_t nowMs);

            /// @brief Draws the fisher's rod behind the figure, or over it when front is set and he faces the way of the slots 49 to 51.
            /// @details Original: DrawHumanLayers; the rod sprites are the 009 bank's 9001 + body type + 4 per two rod kinds.
            /// @param canvas Target canvas.
            /// @param player The fisher.
            /// @param feet Screen position of his feet.
            /// @param slot Action slot of the figure.
            /// @param frame Frame of the figure.
            /// @param front True for the pass over the figure.
            void drawFishingRod(
                gfx::Canvas& canvas,
                const MapCharacter& player,
                core::Point feet,
                int slot,
                int frame,
                bool front
            );

            /// @brief Draws the rod sprite of a fisher at his feet moved by the rod offset for his body type and slot.
            /// @param canvas Target canvas.
            /// @param player The fisher.
            /// @param feet Screen position of the point to shift from.
            /// @param slot Action slot.
            /// @param frame Frame of the sprite.
            void drawRodSprite(
                gfx::Canvas& canvas,
                const MapCharacter& player,
                core::Point feet,
                int slot,
                int frame
            );

            /// @brief Draws the rod of the driver of a vehicle of kind 10 or 12, shifted for the way he faces.
            /// @param canvas Target canvas.
            /// @param driver The driver.
            /// @param feet Screen position of the vehicle's feet.
            /// @param pose Riding pose slot.
            /// @param nowMs Current time in milliseconds.
            void drawVehicleRod(
                gfx::Canvas& canvas,
                MapCharacter& driver,
                core::Point feet,
                int pose,
                std::uint64_t nowMs
            );

            /// @brief Draws a vehicle with its riders: the back, the riders on their seats, the front layer and the emotions.
            /// @details Original: DrawHumanSprite for FTrafficState 2 (DrawVehicleBack, DrawHumanOnVehicle, DrawVehicleFront).
            /// @param canvas Target canvas.
            /// @param driver The driver of the vehicle.
            /// @param feet Screen position of the vehicle's feet.
            /// @param camera Camera of the view.
            /// @param nowMs Current time in milliseconds.
            void drawVehicleRider(
                gfx::Canvas& canvas,
                MapCharacter& driver,
                core::Point feet,
                const Camera& camera,
                std::uint64_t nowMs
            );

            /// @brief Tells whether a player is a passenger drawn with his driver (seat 2..4, driver on the map).
            /// @param player The player to test.
            /// @return True for a passenger.
            bool isPassenger(const MapCharacter& player) const;

            /// @brief Screen point of a rider's seat: his driver's feet (or his own) plus the seat shift.
            /// @details Original: CalcVehicleSeatPos.
            /// @param player The rider.
            /// @param camera Camera of the view.
            /// @param seat Seat number.
            /// @return Screen position of the seat.
            core::Point seatPoint(const MapCharacter& player, const Camera& camera, int seat) const;

            /// @brief Draws an NPC with its shadow, hover glow and emotion.
            /// @param canvas Target canvas.
            /// @param npc The NPC.
            /// @param camera Camera of the view.
            /// @param display Display state (mouse for the hover glow).
            /// @param nowMs Current time in milliseconds.
            void drawNpc(
                gfx::Canvas& canvas,
                MapNpc& npc,
                const Camera& camera,
                const Display& display,
                std::uint64_t nowMs
            );

            /// @brief Draws a pet or partner as an NPC of its kind, walking or standing.
            /// @details Original: DrawFollowNpc.
            /// @param canvas Target canvas.
            /// @param pet The pet.
            /// @param camera Camera of the view.
            /// @param display Display state (make time bar hook).
            /// @param nowMs Current time in milliseconds.
            void drawPet(
                gfx::Canvas& canvas,
                MapPet& pet,
                const Camera& camera,
                const Display& display,
                std::uint64_t nowMs
            );

            /// @brief Draws the word floating over a character, a sprite's height above its feet.
            /// @details Original: DrawFloatText.
            /// @param canvas Target canvas.
            /// @param text The floating text (updated).
            /// @param feet Screen position of the feet.
            /// @param spriteHeight Height of the sprite.
            /// @param pet True for a pet, whose text is smaller.
            /// @param nowMs Current time in milliseconds.
            void drawFloatText(
                gfx::Canvas& canvas,
                FloatText& text,
                core::Point feet,
                int spriteHeight,
                bool pet,
                std::uint64_t nowMs
            );

            /// @brief Draws a tent: its own look for its owner (and for everybody with the ninth tick off a Trojan war map).
            /// @param canvas Target canvas.
            /// @param camp The tent.
            /// @param camera Camera of the view.
            /// @param display Display state.
            void drawCamp(gfx::Canvas& canvas, const Camp& camp, const Camera& camera, const Display& display);

            /// @brief Draws a vehicle standing on the map once the icon that flew there has landed.
            /// @details Original: DrawParkedVehicle.
            /// @param canvas Target canvas.
            /// @param vehicle The vehicle.
            /// @param camera Camera of the view.
            /// @param nowMs Current time in milliseconds.
            void drawParkedVehicle(gfx::Canvas& canvas, Vehicle& vehicle, const Camera& camera, std::uint64_t nowMs);

            /// @brief Draws "<owner>'s home " over a tent with the owner's title above it.
            /// @param canvas Target canvas.
            /// @param camp The tent.
            /// @param camera Camera of the view.
            /// @param display Display state.
            void drawCampOwnerName(gfx::Canvas& canvas, const Camp& camp, const Camera& camera, const Display& display);

            /// @brief Where an NPC kind's name goes over its feet.
            /// @details Original: DrawHumInfo, LoadNpcLook (FSpriteHeight).
            /// @param info The NPC kind.
            /// @param body The character body (position).
            /// @param camera Camera of the view.
            /// @param slot Sprite slot used for the height.
            /// @return Screen point of the name.
            core::Point nameSpot(const formats::NpcInfo& info, const MapCharacter& body, const Camera& camera, int slot = 0);

            /// @brief Maps a label's point to where the window shows it (Display::onWindow), or leaves it.
            /// @param display Display state.
            /// @param point Point on the map view.
            /// @return Point as the window shows it.
            static core::Point shownAt(const Display& display, core::Point point) { return display.onWindow ? display.onWindow(point) : point; }

            /// @brief Slot of the sprite a partner at its work is drawn in.
            /// @details Original: PlayHumanAction 46.
            /// @param pet The partner.
            /// @return Sprite slot.
            int workingSlot(const MapPet& pet) const;

            /// @brief Draws a name centred on a point with a shadow.
            /// @param canvas Target canvas.
            /// @param name Text to draw.
            /// @param color Text colour.
            /// @param at Centre-top point of the text.
            void drawName(gfx::Canvas& canvas, const std::string& name, core::Color color, core::Point at);

            /// @brief Draws the stall's picture centred on the feet.
            /// @details Original: DrawBasePlayer.
            /// @param canvas Target canvas.
            /// @param stall The stall.
            /// @param feet Screen position of the keeper's feet.
            void drawStallPicture(gfx::Canvas& canvas, const StallSign& stall, core::Point feet);

            /// @brief Draws the plate over the head with the stall's name.
            /// @details Original: DrawStallSign.
            /// @param canvas Target canvas.
            /// @param stall The stall.
            /// @param feet Screen position of the keeper's feet.
            void drawStallPlate(gfx::Canvas& canvas, const StallSign& stall, core::Point feet);

            std::map<std::int32_t, std::reference_wrapper<MapCharacter>> m_people;
            int m_sceneType = 0;
            const WorldMap* m_lastMap = nullptr; // the map and where its ground was drawn in the last frame
            core::Point m_lastGroundAt{0, 0};

            CharacterRenderer& m_characters;
            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            // the last frame's y order of the standing objects and tiles, which do not move: checked each frame, sorted again only when it no longer holds
            std::vector<std::size_t> m_fixedOrder;
    };

}

#endif
