#include "client/rules/BoxItems.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        // item ids of the boxes and packs opened with the 23/75 request (the earlier boxes)
        constexpr std::array<int, 41> kBoxes{{
            34080, // Strange Peach
            34082, // Bless Pack
            34118, // Special Dumpling
            34127, // Festival Dumpling
            34162, // Magical Dumpling
            34141, // Big Magical Egg
            34142, // Magical Egg
            34149, // Date Moon Cake
            34150, // Fruit Moon Cake
            34151, // Ice Cream Moon Cake
            34016, // Encyclopedia
            34020, // Magic Set Box
            34015, // Glazed Stone Bag
            34159, // New Year Box
            34025, // Pet Egg
            34030, // Glaze Bag
            34023, // Box
            34174, // EXP Pack
            34172, // 7 Colors Egg
            34171, // Gift Pack
            34178, // Birthday Gift Box
            34193, // Water Pack
            34192, // War Pack
            34199, // Strengthen Pack
            34197, // Mooncake
            30256, // Halloween Pumpkin
            34229, // Gear Pack
            34201, // Transferred Pack
            30265, // X'mas Gift Stocking
            34234, // New Year Box
            34241, // New Year Pack
            34242, // Spring Red Pack
            34248, // Scroll Pack
            34252, // Water Splashing
            34257, // Perfume Pack
            34260, // Newbie Box
            34262, // Summer Events Box
            34266, // Mid-autumn Box
            34268, // Purple Chest
            34256, // anniversary gift bag
            34263, // Summer Vacation Swee... (name cut off in the data)
        }};

        // item ids of the later boxes and packs opened with the 23/128 request (some ids appear twice)
        constexpr std::array<int, 70> kLaterBoxes{{
            30340, // Mysterious Pumpkin
            34274, // Christmas Box
            34288, // New Year Bless Pack
            34293, // Sweet Gift Box
            34296, // Bless Pack
            34297, // Ghost War Horse Pack
            34308, // Festival Gift Box
            34310, // Fantasy Pumpkin
            34322, // Christmas Gift Pack
            34333, // Lucky Pack
            34341, // Love Pack
            34334, // Lantern Festival Box
            34343, // White Day Pack
            34346, // Naughty Dolphin Pack
            34349, // Fortune Pack
            34355, // Golden Dumpling
            34357, // Summer Gift Pack
            34363, // Reunion Gift Box
            34365, // Potential Pack
            34366, // Chaos Pack
            34367, // Naughty Monkey Pack
            34369, // Naughty Pumpkin
            34379, // Anniversary Gift Box
            34380, // 2011 Xmas Box
            34381, // Test
            34382, // Test
            34383, // Test
            34384, // Test
            34385, // Nostalgic Love Pack
            34386, // Nostalgic Wing Pack
            34387, // Nostalgic Cat Pack
            34388, // Nostalgic Bunny Pack
            34400, // Jade Rabbit Pack
            34401, // 2011 Lantern Festiva... (name cut off in the data)
            34406, // White Valentine's Gi... (name cut off in the data)
            34276, // Christmas Sock
            34279, // Bless Pack
            34280, // Super Bless Pack
            34294, // Easter Basket
            34295, // Big Easter Basket
            34300, // Anniversary Bag
            34301, // Anniversary Big Bag
            34302, // Anniversary Big Bag
            34305, // Crystal Ice Cube
            34304, // Unclear Ice Cube
            34306, // Crystal Ice Cube
            34308, // Festival Gift Box
            34309, // Super Gift Box
            34310, // Fantasy Pumpkin
            34311, // Friendship Pumpkin
            34313, // Turkey
            34315, // Thanksgiving Box
            34316, // Thanksgiving Egg
            34319, // Christmas Stockings
            34347, // Easter Box
            34348, // Super Easter Box
            34352, // IGG Happy Pack
            34353, // IGG Super Pack
            34354, // IGG Bliss Pack
            30383, // 2010 Halloween Pumpk... (name cut off in the data)
            34368, // Halloween Bliss Pack
            34370, // Delicious Turkey
            34371, // Thanksgiving Pack
            34372, // Colorful Egg
            34394, // 2011 New Year Pack
            34395, // Christmas Stocking
            34396, // 2010 Jolly Pack
            34405, // 2011 Valentine's Pac... (name cut off in the data)
            34408, // 2011 Anniversary bag
            34409, // 2011 Anniversary Box
        }};

    }

    bool BoxItems::opensWithBox(int itemId) { return std::find(kBoxes.begin(), kBoxes.end(), itemId) != kBoxes.end(); }

    bool BoxItems::opensWithBox2(int itemId) { return std::find(kLaterBoxes.begin(), kLaterBoxes.end(), itemId) != kLaterBoxes.end(); }

}
