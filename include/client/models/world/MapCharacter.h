#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_MAP_CHARACTER_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_MAP_CHARACTER_H

#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/world/FinePosition.h"
#include "client/data/VehicleKinds.h"
#include "client/rendering/world/ChatBubble.h"
#include "client/rendering/world/EmotionBubble.h"
#include "client/models/world/FloatText.h"
#include "client/models/world/HitReaction.h"
#include "client/models/world/RiddenPet.h"
#include "client/models/world/RiddenVehicle.h"
#include "client/models/world/StallSign.h"
#include "client/models/world/ThrowPose.h"
#include "core/Color.h"
#include "formats/NpcInfo.h"
#include "gfx/PictureId.h"
#include "core/Point.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief A player on the map: who, how they look, where their feet are (map pixels), which way they face and whether they walk.
    /// @details Direction: 0 north, 1 north-west, ... 4 south, ... 7 north-east. Fields carry the original's state names
    /// (FFightState, FRidePetState, FTrojanWarFlag, FHouseRoom, ...) and the packets that set them, noted beside each field.
    struct MapCharacter {
        std::int32_t id = 0;
        std::string name;
        std::string nickname;
        std::string title;
        std::uint32_t guildId = 0;
        // name colour: alpha 0 then 255,255,255 (white)
        core::Color nameColor{0, 255, 255, 255};
        CharacterLook look;
        core::Point position;
        // direction 4 is south
        int direction = 4;
        bool walking = false;
        // default walking speed, pixels per millisecond
        double pixelsPerMs = 0.16;
        FigureAnimation animation;
        std::vector<core::Point> path;
        std::size_t pathIndex = 0;
        std::uint64_t stepMs = 0;
        std::optional<int> arrivalDirection;
        std::optional<FinePosition> fine;
        EmotionBubble emotion;
        FloatText floatText;
        ChatBubble chat;
        int action = 0;
        int actionOffset = 0;
        int fightState = 0;
        std::optional<StallSign> stall;
        std::optional<RiddenPet> riding;
        bool ridingFlag = false;
        std::optional<RiddenVehicle> vehicle;
        std::optional<formats::NpcInfo> horse;
        int cellAttr = 0;
        bool fishing = false;
        bool trojanWar = false;
        bool trojanTarget = false;
        bool autoCollecting = false;
        int rodKind = 0;
        bool fanIcon = false;
        std::optional<std::uint64_t> waitEventSinceMs;
        bool teamLeader = false;
        int houseRoom = 0;
        int usedUtensil = 0;
        int usedSeat = 0;
        bool utensilHidden = false;
        bool holdFirstFrame = false;
        core::Point seatShift;
        gfx::PictureId seatOverlay;
        bool bathing = false;
        int bathFurn = 0;
        std::optional<ThrowPose> throwPose;
        std::optional<HitReaction> hit;
        std::optional<formats::NpcInfo> shape;
        int pvpWins = 0;
        int pvpRank = 0;
        int arenaVictory = 0;
        int survivalVictory = 0;

        /// @brief Tells whether the last cell it stood on was shallow water.
        /// @return True when the cell attribute is 8.
        bool inShallowWater() const { return cellAttr == 8; } // cell attribute 8: shallow water

        /// @brief Gives the walk-block class: 0 on foot, else the class of the vehicle ridden.
        /// @details Original: the WalkBlockMasks index.
        /// @return The class index.
        int walkClass() const { return vehicle ? VehicleKinds::classOf(vehicle->kind) : 0; }

        /// @brief Gives the sprite action slot: walking 0..7, standing 8..15, or the action's.
        /// @return The slot.
        // standing slots start at 8
        int actionSlot() const { return action != 0 ? action + actionOffset : (walking ? 0 : 8) + direction; }
    };

}

#endif
