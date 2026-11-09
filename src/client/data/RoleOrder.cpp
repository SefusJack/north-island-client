#include "client/data/RoleOrder.h"

namespace client {

    int roleOrder(int bodyType, int head) {
        // body types above 10 are outside the figures; heads above 10 count as head 0
        if (bodyType > 10) {
            return 0;
        }
        if (head > 10) {
            head = 0;
        }
        // key is bodyType * 10 + head (10 = heads per body type); the result is the role's order number 1..15
        switch (bodyType * 10 + head) {
            case 10: return 1; // body 1 head 0 -> order 1
            case 20: return 2; // body 2 head 0 -> order 2
            case 21: return 3; // body 2 head 1 -> order 3
            case 30: return 4; // body 3 head 0 -> order 4
            case 31: return 5; // body 3 head 1 -> order 5
            case 32: return 6; // body 3 head 2 -> order 6
            case 33: return 14; // body 3 head 3 -> order 14
            case 40: return 7; // body 4 head 0 -> order 7
            case 41: return 8; // body 4 head 1 -> order 8
            case 42: return 9; // body 4 head 2 -> order 9
            case 43: return 10; // body 4 head 3 -> order 10
            case 44: return 11; // body 4 head 4 -> order 11
            case 45: return 12; // body 4 head 5 -> order 12
            case 46: return 13; // body 4 head 6 -> order 13
            case 47: return 15; // body 4 head 7 -> order 15
            default: return 0; // not a figure
        }
    }

    std::optional<std::pair<int, int>> splitRoleIndex(int role) {
        // (bodyType, head) of roles 1..13
        // each pair is {body type (1..4), head index within that body type}
        static const std::pair<int, int> kRoles[] = {{2, 0}, {2, 1}, {1, 0}, {4, 2}, {4, 1}, {3, 0}, {3, 1}, {4, 4}, {4, 5}, {4, 0}, {4, 6}, {3, 2}, {4, 3}};
        // valid role numbers are 1..13
        if (role < 1 || role > 13) {
            return std::nullopt;
        }
        return kRoles[role - 1];
    }

}
