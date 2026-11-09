#include "client/data/RideOffsets.h"

#include "client/data/RideOffsetData.h"

namespace client {

    namespace {

        /// @brief Index into the offset tables (1..8) for a direction slot of the riding pose.
        /// @details Original: the riding pose starts at slot 18, or 46 for the skills that sit.
        /// @param slot Direction slot of the riding pose.
        /// @param skillId Ride skill of the pet.
        /// @return The level (direction index); negative when the slot is before the pose.
        int levelOf(int slot, int skillId) {
            return RideOffsets::sits(skillId) ? slot - 45 : slot - 17;
        } // 46 / 18 = first riding pose slot, minus 1 for the 1-based direction

        /// @brief Looks an offset up in a table by skill, body type and direction level.
        /// @param data Offset table by ride skill.
        /// @param bodyType Body type of the rider (0..4).
        /// @param level Direction index (0..8).
        /// @param skillId Ride skill of the pet.
        /// @return The offset, or (0, 0) when out of range or unlisted.
        core::Point lookUp(const std::map<int, RideOffsetSet>& data, int bodyType, int level, int skillId) {
            const auto found = data.find(skillId);
            // body types 0..4, directions 0..8
            if (found == data.end() || bodyType < 0 || bodyType > 4 || level < 0 || level > 8) {
                return {};
            }
            return {found->second.x[bodyType][level], found->second.y[bodyType][level]};
        }

    }

    core::Point RideOffsets::rider(int bodyType, int slot, int skillId) {
        const int level = levelOf(slot, skillId);
        if (level < 0) {
            return {};
        }
        const auto found = riderOffsetData().find(skillId);
        if (found == riderOffsetData().end()) {
            // slots 20..21 and 23..24 are the two sides of the riding pose; shift 5 px
            if (slot >= 20 && slot <= 21) {
                return {5, 0};
            }
            if (slot >= 23 && slot <= 24) {
                return {-5, 0};
            }
            return {};
        }
        return lookUp(riderOffsetData(), bodyType, level, skillId);
    }

    core::Point RideOffsets::pet(int bodyType, int slot, int skillId) {
        // skill ids that GetRideOffset leaves out of its sitting list; 46 / 18 = first riding pose slot, minus 1
        // 1192, 1562, 1573, 1657 = ride skill ids (no names in the .dat tables)
        const bool sitting = sits(skillId) && skillId != 1192 && skillId != 1562 && skillId != 1573 && skillId != 1657;
        const int level = sitting ? slot - 45 : slot - 17;
        return level < 0 ? core::Point{} : lookUp(rideOffsetData(), bodyType, level, skillId);
    }

    bool RideOffsets::isListed(int skillId) {
        // ride skill ids (original IsListedRideSkill: 1317..1318, 1456..1457, 1562..1563, 1568..1569, 1657..1658 are ranges); no names in the .dat tables
        switch (skillId) {
            case 1146:
            case 1185:
            case 1192:
            case 1317:
            case 1318:
            case 1447:
            case 1456:
            case 1457:
            case 1562:
            case 1563: // listed ride skills (pet sprite ids)
            case 1568:
            case 1569:
            case 1573:
            case 1575:
            case 1577:
            case 1657:
            case 1658:
            case 1668:
            case 1670:
            case 1761: // listed ride skills (pet sprite ids)
            case 1764: // listed ride skill (pet sprite id)
                return true;
            default: // any other skill is not listed
                return false;
        }
    }

    bool RideOffsets::sits(int skillId) {
        // ride skill ids (original GetRideOffset: 1310..1312 is a range); no names in the .dat tables
        switch (skillId) {
            case 1043:
            case 1046:
            case 1122:
            case 1128:
            case 1192:
            case 1210:
            case 1307:
            case 1310:
            case 1311:
            case 1312: // skills that seat the rider in the sitting pose
            case 1316:
            case 1324:
            case 1342:
            case 1562:
            case 1573:
            case 1657: // skills that seat the rider in the sitting pose
                return true;
            default: // any other skill: standing pose
                return false;
        }
    }

}
