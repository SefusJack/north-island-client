#include "client/data/StarterRoles.h"

#include <cstddef>

namespace client {

    namespace {

        // table: body type, head, description picture, its (x, y), role picture bottom-centre (x, y), in Btn_RolePic order
        const std::array<StarterRole, kStarterRoleCount> kRoles = {{
            {2, 0, "RoleInfo_2_1", {135, 256}, {105, 435}},
            {2, 1, "RoleInfo_2_2", {49, 246}, {225, 469}},
            {1, 0, "RoleInfo_1_1", {168, 293}, {129, 428}},
            {4, 2, "RoleInfo_4_3", {48, 245}, {240, 474}},
            {4, 1, "RoleInfo_4_2", {139, 199}, {105, 469}},
            {3, 0, "RoleInfo_3_1", {46, 207}, {218, 485}},
            {3, 1, "RoleInfo_3_2", {54, 219}, {230, 474}},
            {4, 4, "RoleInfo_4_5", {138, 195}, {109, 475}},
            {4, 5, "RoleInfo_4_6", {160, 210}, {120, 469}},
            {4, 0, "RoleInfo_4_1", {50, 235}, {245, 475}},
            {4, 6, "RoleInfo_4_7", {135, 221}, {104, 469}},
            {3, 2, "RoleInfo_3_3", {57, 214}, {218, 485}},
            {3, 3, "RoleInfo_3_4", {172, 190}, {115, 485}},
            {4, 7, "RoleInfo_4_8", {175, 200}, {114, 469}},
        }};

        // 16 role indexes; table: six item ids per role (RoleStarterEquips)
        const std::array<std::array<std::uint16_t, 6>, 16> kStarterEquipment = {{
            // columns by item type: headgear (13), clothes (12), hand item (weapon type 1), gloves (14), shoes (15), unused
            {0, 0, 0, 0, 0, 0}, // role index 0: none
            {22003, 21002, 0, 0, 24002, 0}, // Star Headdress, Small Dress, -, -, Moppet's Shoes
            {22001, 21003, 0, 0, 24003, 0}, // Wing Headdress, Harness pants, -, -, Pink Shoes
            {22004, 21001, 0, 0, 24001, 0}, // Lovely Headdress, Boy's Suit, -, -, Children's Shoes
            {0, 21008, 0, 0, 24008, 0}, // -, Uniform, -, -, Student's Shoes
            {0, 21007, 0, 23002, 24007, 0}, // -, Racing Suit, -, Competition Gloves, Racing Shoes
            {0, 21004, 0, 0, 24004, 0}, // -, Dust Coat, -, -, Gentleman's Shoes
            {0, 21005, 0, 0, 24005, 0}, // -, Jeans, -, -, Leisure Shoes
            {22002, 21010, 10003, 0, 24010, 0}, // Flower Headdress, Violinists Suit, Violin, -, High-heeled Shoes
            {0, 21013, 0, 0, 24013, 0}, // -, Kimono, -, -, Wooden Shoes
            {22005, 21006, 0, 23001, 24006, 0}, // Sailor's Hat, Sailor's Suit, -, Gloves, White Boots
            {22006, 21011, 10004, 0, 24011, 0}, // Nurse's Hat, Nurse's Clothes, Syringe, -, White Leather Shoes
            {0, 21012, 0, 0, 24012, 0}, // -, Bartender's Uniform, -, -, Great Pumps
            {22007, 21009, 10002, 0, 24009, 0}, // Round Headdress, Restaurant Uniform, plate, -, Canvas
            {22009, 21014, 18002, 0, 24014, 0}, // Headscarf, Craftsman's Clothes, Craftsman's Pestle, -, Craftsman Shoes
            {22008, 21015, 0, 0, 24015, 0}, // Band, Uniform, -, -, Leather Shoes
        }};

        // 16 role indexes; table: STR, CON, INT, WIS, AGI bonus points per role (RoleStatBonuses)
        const std::array<std::array<int, 5>, 16> kStatBonuses = {{
            // one row per role index; the points are the bonus on top of the player's five free points
            {0, 0, 0, 0, 0}, // role index 0: none
            {0, 0, 1, 1, 1},
            {0, 0, 1, 0, 2},
            {0, 0, 2, 0, 1},
            {0, 0, 3, 0, 0},
            {2, 0, 0, 0, 1},
            {1, 2, 0, 0, 0},
            {2, 1, 0, 0, 0},
            {0, 0, 1, 2, 0},
            {0, 0, 2, 1, 0},
            {1, 2, 0, 0, 0},
            {0, 0, 2, 0, 1},
            {0, 0, 0, 2, 1},
            {0, 0, 0, 0, 0},
            {1, 1, 1, 0, 0},
            {0, 0, 1, 1, 1},
        }};

    }

    const StarterRole& starterRole(int number) {
        const int index = number >= 1 && number <= kStarterRoleCount ? number - 1 : 0;
        return kRoles[static_cast<std::size_t>(index)];
    }

    int roleIndex(int bodyType, int head) {
        switch (bodyType) {
            case 1: // body type 1: only head 0, role index 3
                return head == 0 ? 3 : 0; // role index 3
            case 2: { // body type 2: two heads
                constexpr int kRoles2[] = {1, 2};
                return head >= 0 && head < 2 ? kRoles2[head] : 0; // 2 heads
            }
            case 3: { // body type 3: four heads
                constexpr int kRoles3[] = {6, 7, 12, 14};
                return head >= 0 && head < 4 ? kRoles3[head] : 0; // 4 heads
            }
            case 4: { // body type 4: eight heads
                constexpr int kRoles4[] = {10, 5, 4, 13, 8, 9, 11, 15};
                return head >= 0 && head < 8 ? kRoles4[head] : 0; // 8 heads
            }
            default: // other body types have no role
                return 0;
        }
    }

    std::array<std::uint16_t, 6> starterEquipment(int roleIndex) {
        // 16 role indexes (0..15), 6 equipment slots
        return roleIndex >= 0 && roleIndex < 16 ? kStarterEquipment[static_cast<std::size_t>(roleIndex)] : std::array<std::uint16_t, 6>{};
    }

    bool wearsStarterClothes(int bodyType, int head, int clothesItemId) {
        constexpr int kClothes = 1; // column 1 of the starter equipment row = clothes (item type 12)
        return starterEquipment(roleIndex(bodyType, head))[kClothes] == clothesItemId;
    }

    std::array<int, 5> roleStatBonuses(int roleIndex) {
        return roleIndex >= 0 && roleIndex < 16 ? kStatBonuses[static_cast<std::size_t>(roleIndex)] : std::array<int, 5>{}; // 16 role indexes (0..15), 5 stats
    }

}
