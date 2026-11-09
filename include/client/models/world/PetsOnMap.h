#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_PETS_ON_MAP_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_PETS_ON_MAP_H

#include "client/models/world/MapPet.h"
#include "client/models/world/PlayerTrails.h"
#include "client/rules/Walking.h"
#include "formats/NpcInfo.h"

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace client {

    /// @brief The fighting pets of the players on the map (and the partner/walker followers).
    /// @details Original: SetActivePet, TBasePlayer.FollowActivePet, TFollowNpc.UpdateMove. A pet heads in a straight line for its
    /// owner's third trail point; one called back walks to where its owner stood and disappears. The pets of players in a team are
    /// not shown (ShowOnlyActivePet).
    class PetsOnMap {
        public:
            static constexpr std::size_t kMakerPoint = 1;

            /// @brief Creates the pet list.
            /// @param trails Trails of the players the pets follow.
            explicit PetsOnMap(
                const PlayerTrails& trails
            )
                : m_trails(trails) {}

            /// @brief The owner's pet comes out where the owner stands (the one out before is called back).
            /// @details Original: SetActivePet.
            /// @param ownerId Owner player id.
            /// @param info The pet's NPC info.
            /// @param name The pet's name.
            /// @param tradable 1 when the pet may be renamed.
            /// @param nowMs Current time in ms.
            void show(
                std::int32_t ownerId,
                const formats::NpcInfo& info,
                const std::string& name,
                int tradable,
                std::uint64_t nowMs
            );

            /// @brief The pet walks back to its owner.
            /// @details Original: SetActivePet 0 / ClearActivePet.
            /// @param ownerId Owner player id.
            /// @param nowMs Current time in ms.
            void recall(std::int32_t ownerId, std::uint64_t nowMs);

            /// @brief Sets what the owner's pet out holds (lookId 0: nothing).
            /// @details Original: FEquips[3] of the pet.
            /// @param ownerId Owner player id.
            /// @param weapon The weapon.
            void setWeapon(std::int32_t ownerId, const PetWeapon& weapon);

            /// @brief Removes the owner's pets at once (the owner left the map).
            /// @param ownerId Owner player id.
            void remove(std::int32_t ownerId);

            /// @brief Renames the owner's pet of a kind.
            /// @param ownerId Owner player id.
            /// @param npcId The pet's NPC id.
            /// @param name The new name.
            void rename(std::int32_t ownerId, int npcId, const std::string& name);

            /// @brief Turns the pet of a kind into another kind, with the new kind's name.
            /// @details Original: RecvScenePetNpcChange (LoadFromNpcTable gives it the new kind's name too).
            /// @param ownerId Owner player id.
            /// @param npcId The old NPC id.
            /// @param info The new kind's NPC info.
            void changeKind(std::int32_t ownerId, int npcId, const formats::NpcInfo& info);

            /// @brief The owner was put somewhere; its pet stands there too.
            /// @details Original: PlaceFollowNpcAtOwner.
            /// @param ownerId Owner player id.
            void ownerPlaced(std::int32_t ownerId);

            /// @brief Notes that the owner joined or left a team (a team member's pet is not shown).
            /// @param ownerId Owner player id.
            /// @param inTeam True when joined, false when left.
            void setInTeam(std::int32_t ownerId, bool inTeam);

            /// @brief Hides or shows the owner's pet (the wedding's flag).
            /// @details Original: FHidePet, set by RecvPlayerFlagOn / Off.
            /// @param ownerId Owner player id.
            /// @param hidden True to hide.
            void setHidden(std::int32_t ownerId, bool hidden);

            /// @brief Tells whether the owner's pet is hidden.
            /// @param ownerId Owner player id.
            /// @return True when hidden.
            bool isHidden(std::int32_t ownerId) const { return m_hidden.count(ownerId) != 0; }

            /// @brief Sets whether the own player is in the wedding hall (no pet is drawn there).
            /// @details Original: DrawFollowNpc.
            /// @param inHall True in the hall.
            void setHallMap(bool inHall);

            /// @brief Starts the express pet walker: it leaves over the map's width (mode 1, the pet itself hidden) or comes from 400 pixels to the right (3, and 2 / 4 to leave again).
            /// @details Original: HandlePetWalkAnimPacket, server 25/21 (the own player's).
            /// @param ownerId Owner player id.
            /// @param info The pet's NPC info.
            /// @param mode Walk mode 1..4.
            /// @param mapWidth The map's width in pixels.
            /// @param nowMs Current time in ms.
            void petWalk(
                std::int32_t ownerId,
                const formats::NpcInfo& info,
                int mode,
                int mapWidth,
                std::uint64_t nowMs
            );

            /// @brief The house partner comes out where the owner stands and walks to the owner's second trail point.
            /// @details Original: HandleMakePacket 10 (ShowFollowPet). It is drawn only while visible (UpdateMove: the house's room it works
            /// in), facing the piece once there.
            /// @param ownerId Owner player id.
            /// @param info The partner's NPC info.
            /// @param name The partner's name.
            /// @param tradable 1 when it may be renamed.
            /// @param nowMs Current time in ms.
            void showMaker(
                std::int32_t ownerId,
                const formats::NpcInfo& info,
                const std::string& name,
                int tradable,
                std::uint64_t nowMs
            );

            /// @brief Removes the owner's house partner.
            /// @details Original: HandleMakePacket 3 to 10: FPetVisible 0 for the partner.
            /// @param ownerId Owner player id.
            void hideMaker(std::int32_t ownerId);

            /// @brief Sets whether the partner is seen and the piece it faces.
            /// @details Original: UpdateMove of the partner; the piece is Player.FMakeX, FMakeY.
            /// @param ownerId Owner player id.
            /// @param visible Whether it is seen.
            /// @param workAt The piece's position.
            void driveMaker(std::int32_t ownerId, bool visible, core::Point workAt);

            /// @brief Starts an NPC walking across the map along the owner's row at 0.36 pixels a millisecond.
            /// @details Original: HandleNpcWalkByPacket, server 25/33. Mode 1 walks off to the map's edge then back; mode 2 comes from 400 pixels to the right then goes off.
            /// @param ownerId Owner player id.
            /// @param info The NPC's info.
            /// @param mode Walk mode 1 or 2.
            /// @param mapWidth The map's width in pixels.
            /// @param nowMs Current time in ms.
            void npcWalkBy(
                std::int32_t ownerId,
                const formats::NpcInfo& info,
                int mode,
                int mapWidth,
                std::uint64_t nowMs
            );

            /// @brief Moves every pet one frame and drops the ones that arrived after being called back.
            /// @param nowMs Current time in ms.
            /// @param steps Whole-pixel or sub-pixel walking.
            void update(std::uint64_t nowMs, WalkSteps steps = WalkSteps::Whole);

            /// @brief The pets on the map.
            /// @return Mutable list of pets.
            std::vector<MapPet>& pets() { return m_pets; }

        private:
            const PlayerTrails& m_trails;
            std::vector<MapPet> m_pets;

            /// @brief Tells whether the pet of a player may be drawn.
            /// @param ownerId Owner player id.
            /// @return True when it may.
            bool shown(std::int32_t ownerId) const;

            std::set<std::int32_t> m_inTeam;
            std::set<std::int32_t> m_hidden;
            std::set<std::int32_t> m_away;
            int m_mapWidth = 0;
            bool m_inHall = false;
    };

}

#endif
