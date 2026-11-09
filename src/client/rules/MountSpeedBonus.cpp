#include "client/rules/MountSpeedBonus.h"

namespace client {

    double MountSpeedBonus::of(int spriteId, int bodyType, int level) {
        constexpr double kSlower = -0.02; // speed change in px/ms
        constexpr double kMuchSlower = -0.08; // speed change in px/ms
        constexpr double kFaster = 0.02; // speed change in px/ms
        if (bodyType != 0) {
            switch (bodyType) {
                case 1:
                case 2:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 10:
                case 15:
                case 16:
                case 17:
                case 20: // body types 1, 2, 4-8, 10, 15-17, 20 slow the rider a little
                    return kSlower;
                case 3:
                case 9:
                case 11:
                case 12:
                case 13:
                case 14:
                case 18:
                case 19: // body types 3, 9, 11-14, 18, 19 slow the rider a lot
                    return kMuchSlower;
                default: // any other body type: no change
                    return 0;
            }
        }
        switch (spriteId) {
            case 1010: { // sprite 1010: speed grows with the pet's level
                constexpr int kLevelMask = 255; // keeps the low byte of the level (the pet's level, 0..255)
                constexpr int kBaseLevel = 5; // level at which the bonus is 0.04 px/ms
                return ((level & kLevelMask) - kBaseLevel) * 0.0007 + 0.04; // 0.0007 px/ms per level above the base, plus 0.04 px/ms
            }
            case 1091:
            case 1092:
            case 1093:
            case 1094:
            case 1102:
            case 1128:
            case 1129:
            case 1131:
            case 1132:
            case 1133:
            case 1123:
            case 1122:
            case 1124:
            case 1118: // sprite ids 1091-1094, 1102, 1118, 1122-1124, 1128-1133 (not in the name tables): the faster mounts
                return kFaster;
            default: // any other sprite: no change
                return 0;
        }
    }

}
