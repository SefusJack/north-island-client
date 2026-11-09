#include "client/data/GiftBoxContents.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        /// @brief Gives the table of the random packs (Original: DZ_String.pas GiftBox<id>Items and GiftBox34172Pets; a pet box lists the items' pets only).
        /// @return The table of boxes.
        const std::vector<GiftBoxContents::Box>& boxes() {
            // rows: {pack item id, lists pets, {{item id, count}, ...}}; "xN" in a comment = count N (count 0 = pet box, pets only)
            static const std::vector<GiftBoxContents::Box> table = {
                {34171, // Gift Pack
                 false,
                 {{34105, 1}, // Repairing Potion
                  {34137, 1}, // Pet Lethe Scroll
                  {34022, 1}, // Tent Palette
                  {34120, 1}, // Silver Amity Jelly
                  {34167, 1}, // Sewing Tools
                  {30101, 1}, // Strong Scroll
                  {34133, 1}, // Space Contract
                  {34121, 1}, // Gold Amity Jelly
                  {34160, 1}, // Shape Clay
                  {34122, 1}, // Lethe Scroll
                  {34141, 1}, // Big Magical Egg
                  {25144, 1}, // Solid Waistband
                  {25033, 1}, // Spirit Medal
                  {25032, 1}, // Honor Medal
                  {25031, 1}, // Attack Medal
                  {25030, 1}, // Defence Medal
                  {25029, 1}, // Resistent Necklace
                  {25027, 1}, // Evil Necklace
                  {25006, 1}, // Spirit Ring
                  {25034, 1}, // Holy Medal
                  {25026, 1}, // Demon Necklace
                  {25019, 1}, // Snake Earrings
                  {25010, 1}, // Qing Ring
                  {25041, 1}, // Black Ring
                  {21093, 1}, // White Armor
                  {24119, 1}, // Hero Shoes
                  {21163, 1}, // Sky Clothes
                  {24142, 1}, // Silver Loops
                  {24168, 1}, // Leisure Shoes
                  {21198, 1}, // Sun Robe
                  {24167, 1}, // Long White Boots
                  {21222, 1}, // Soldier's Clothes
                  {34104, 1}, // Repairing Spanner
                  {34155, 1}, // Small Tear Plier
                  {21238, 1}}}, // Verdant Wing Suit
                {34174, // EXP Pack
                 false,
                 {{34108, 1}, // 2X EXP Potion
                  {34109, 1}, // 2.5X EXP Potion
                  {34110, 1}, // 3X EXP Potion
                  {34111, 1}, // 3.5X EXP Potion
                  {34112, 1}, // 4X EXP Potion
                  {34113, 1}, // 4.5X EXP Potion
                  {34114, 1}, // 5X EXP Potion
                  {34115, 1}, // 5.5X EXP Potion
                  {34116, 1}, // 6X EXP Potion
                  {34123, 1}, // 8X EXP Potion
                  {34173, 1}, // 10X EXP Potion
                  {34101, 1}, // 2X EXP Gold Card
                  {34101, 2}, // 2X EXP Gold Card x2
                  {34101, 3}, // 2X EXP Gold Card x3
                  {34101, 4}, // 2X EXP Gold Card x4
                  {34101, 5}, // 2X EXP Gold Card x5
                  {34136, 1}, // 1% EXP Capsule
                  {34145, 1}, // 2% EXP Capsule
                  {34086, 1}}}, // 3% EXP Capsule
                {34192, // War Pack
                 false,
                 {{30101, 1}, // Strong Scroll
                  {33054, 1}, // Lollipop
                  {33052, 1}, // System Grenade
                  {33055, 1}, // Big Bowling
                  {33053, 1}, // Korean Cake
                  {34121, 1}, // Gold Amity Jelly
                  {33030, 1}, // Hot Potato
                  {33029, 1}, // Hot Rice Cake
                  {34163, 1}, // Teleport Scroll
                  {25228, 1}, // Steel Hammer 200T
                  {34104, 1}, // Repairing Spanner
                  {34155, 1}, // Small Tear Plier
                  {24175, 1}, // Wild Boots
                  {25194, 1}, // Fixed Stick
                  {34152, 1}, // Tear Plier
                  {34165, 1}}}, // Win Plate
                {34193, // Water Pack
                 false,
                 {{34146, 1}, // 2 Holy EXP Potion
                  {34147, 1}, // 2.5 Holy EXP Potion
                  {34131, 1}, // 3 Holy EXP Potion
                  {34185, 1}, // 3.5X Holy EXP Potion
                  {34143, 1}, // 4 Holy EXP Potion
                  {34186, 1}, // 4.5X Holy EXP Potion
                  {34144, 1}, // 5 Holy EXP Potion
                  {34187, 1}, // 5.5X Holy EXP Potion
                  {34188, 1}, // 6X Holy EXP Potion
                  {34189, 1}, // 8X Holy EXP Potion
                  {34190, 1}, // 10X Holy EXP Potion
                  {34191, 1}, // 2X EXP Gold Statue
                  {34191, 2}, // 2X EXP Gold Statue x2
                  {34191, 3}, // 2X EXP Gold Statue x3
                  {34191, 4}, // 2X EXP Gold Statue x4
                  {34191, 5}, // 2X EXP Gold Statue x5
                  {34136, 1}, // 1% EXP Capsule
                  {34145, 1}, // 2% EXP Capsule
                  {34086, 1}}}, // 3% EXP Capsule
                {34199, // Strengthen Pack
                 false,
                 {{27000, 1}, // Banana Boat
                  {27007, 1}, // Beast Pill
                  {27005, 1}, // Energy Pill
                  {27008, 1}, // Fairy Honey
                  {27001, 1}, // Orchid Candy
                  {27002, 1}, // Great Syrup
                  {27004, 1}, // Pirate's Wine
                  {27017, 1}, // Mellow Wine
                  {27006, 1}, // Elin's Oil
                  {27003, 1}, // Roca Perfume
                  {27009, 1}, // Fairy Bean
                  {27019, 1}, // Blessing Strawberry
                  {27010, 1}, // Demon Fruit
                  {27011, 1}, // Cotton Candy
                  {27018, 1}, // Braised Tilapia
                  {27012, 1}, // Wolf Deity Bean
                  {27013, 1}, // Delicate Riceball
                  {27014, 1}, // Great Soup
                  {27015, 1}, // Angel's Feather
                  {27016, 1}, // Magic Bean
                  {27020, 1}, // Passion Water
                  {27021, 1}, // Wisdom Water
                  {27022, 1}, // Black Tea
                  {34139, 3}}}, // Pet Return Scroll x3
                {34172, // 7 Colors Egg (pet box)
                 true,
                 {{30240, 0}, // Sano Voucher
                  {30241, 0}, // Wuwan Voucher
                  {30242, 0}, // Hou Yi Voucher
                  {30243, 0}, // Yangs' Saga Voucher
                  {30193, 0}, // Tortoise Voucher
                  {30194, 0}, // Crane Voucher
                  {30195, 0}, // Ox Voucher
                  {30196, 0}, // Zebra Voucher
                  {30197, 0}, // Genie Voucher
                  {30198, 0}, // Lion Voucher
                  {30199, 0}, // Snake Girl Voucher
                  {30200, 0}, // Lord Flower Voucher
                  {30201, 0}, // Panax Elf Voucher
                  {30202, 0}, // Butterfly Voucher
                  {30203, 0}, // Buffalo Voucher
                  {30204, 0}, // Harley Voucher
                  {30232, 0}, // Globefish Voucher
                  {30233, 0}, // Camel Voucher
                  {30234, 0}, // Pterosaur Voucher
                  {30235, 0}, // Octopus Card
                  {30236, 0}, // Sea Lion Voucher
                  {30237, 0}, // Skeleton Card
                  {30244, 0}, // Water Monster Card
                  {30238, 0}, // Gold Griffin Voucher
                  {30239, 0}, // Unicorn Card
                  {30296, 0}}}, // Holy Pegasus Voucher
                {34229, // Gear Pack
                 false,
                 {{22232, 1}, // Arez's Armor
                  {22231, 1}, // Traveller's Hat
                  {22234, 1}, // Pirate's Eye Patch
                  {22230, 1}, // Banana Peel Helmet
                  {23164, 1}, // Emerald Armlet
                  {22233, 1}, // Thunder Hairpin
                  {22235, 1}, // Deity Headscarf
                  {24179, 1}, // Cowboy Boots
                  {23165, 1}, // Plasma Machine
                  {23166, 1}, // Demon Bell
                  {22236, 1}, // Fairy Crest
                  {21233, 1}, // Qlaya's Shawl
                  {21234, 1}, // Kanako's Jaket
                  {23167, 1}, // Mayan Gloves
                  {21235, 1}, // Red Bellyband
                  {24180, 1}, // Strong Beast Boots
                  {24181, 1}, // Angel Rope Boots
                  {24182, 1}, // Thunder Boots
                  {25637, 1}, // Aurora Necklace
                  {22512, 1}, // Magic War Helmet
                  {22513, 1}}}, // Magic Love Helmet
                {34201, // Transferred Pack
                 false,
                 {{34219, 1}, // Jelly Riceball
                  {34204, 1}, // Chicken Riceball
                  {34211, 1}, // Shibainu Riceball
                  {34215, 1}, // Monkey Riceball
                  {34205, 1}, // Troll Riceball
                  {34210, 1}, // Moai Riceball
                  {34203, 1}, // Spirit Riceball
                  {34202, 1}, // Holyspirit Riceball
                  {34206, 1}, // Mummy Riceball
                  {34216, 1}, // River Boy Riceball
                  {34213, 1}, // Mars Riceball
                  {34218, 1}, // Butterfly Riceball
                  {34224, 1}, // Sea Lion Riceball
                  {34217, 1}, // Pumpkin Riceball
                  {34226, 1}, // Ninja Riceball
                  {34212, 1}, // Greendragon Riceball
                  {34220, 1}, // Ginseng Riceball
                  {34209, 1}, // Mahabrah Riceball
                  {34227, 1}, // Robinson Riceball
                  {34225, 1}, // Dracula Riceball
                  {34214, 1}, // Niss Riceball
                  {34208, 1}, // Roca Riceball
                  {34223, 1}, // Charlotte Riceball
                  {34221, 1}, // Eva Riceball
                  {34207, 1}, // Fred Riceball
                  {34222, 1}}}, // Angela Riceball
                {34248, // Scroll Pack
                 false,
                 {{34137, 1}, // Pet Lethe Scroll
                  {34181, 1}, // Training Voucher
                  {30101, 1}, // Strong Scroll
                  {34122, 1}, // Lethe Scroll
                  {34133, 1}, // Space Contract
                  {34163, 1}, // Teleport Scroll
                  {34247, 1}, // Small Demon Scroll
                  {34246, 1}, // Principality Scroll
                  {34245, 1}, // Virtues Scroll
                  {34244, 1}, // Ofanim Scroll
                  {34243, 1}, // Cherubim Scroll
                  {34238, 1}}}, // Seraph Scroll
                {34296, // Bless Pack
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {30371, 1}, // Purple Chest Key
                  {32163, 50}, // Fruit Cake x50
                  {32164, 50}, // Neapolitan Sundaes x50
                  {32173, 10}, // Pork Bone Noodle x10
                  {32174, 5}, // Pork Miso Noodle x5
                  {34108, 1}, // 2X EXP Potion
                  {33031, 2}, // Phoenix Blood x2
                  {33019, 3}, // Recovery Potion x3
                  {34335, 1}, // HP Bottle
                  {34336, 1}, // SP Bottle
                  {34133, 1}, // Space Contract
                  {34167, 1}, // Sewing Tools
                  {34170, 1}, // Weapon Model
                  {34122, 1}, // Lethe Scroll
                  {34137, 1}, // Pet Lethe Scroll
                  {34269, 1}, // Potential Pill
                  {30101, 1}}}, // Strong Scroll
                {34297, // Ghost War Horse Pack
                 false,
                 {{34337, 1}, // Good HP Bottle
                  {34340, 1}, // Super SP Bottle
                  {34105, 1}, // Repairing Potion
                  {34270, 1}, // Space Capsule
                  {34181, 1}, // Training Voucher
                  {34143, 1}, // 4 Holy EXP Potion
                  {34133, 1}, // Space Contract
                  {34163, 1}, // Teleport Scroll
                  {34269, 1}, // Potential Pill
                  {30101, 1}, // Strong Scroll
                  {34238, 1}, // Seraph Scroll
                  {30350, 1}}}, // Ghost Horse Ticket
                {34346, // Naughty Dolphin Pack
                 false,
                 {{34337, 1}, // Good HP Bottle
                  {34338, 1}, // Good SP Bottle
                  {34022, 1}, // Tent Palette
                  {34270, 1}, // Space Capsule
                  {34181, 1}, // Training Voucher
                  {34143, 1}, // 4 Holy EXP Potion
                  {34133, 1}, // Space Contract
                  {34163, 1}, // Teleport Scroll
                  {30101, 1}, // Strong Scroll
                  {34238, 1}, // Seraph Scroll
                  {61062, 1}, // Chaos Crystal
                  {30370, 1}}}, // Naughty Dolphin Card
                {34349, // Fortune Pack
                 false,
                 {{35114, 1}, // HP Restore Set II
                  {35115, 1}, // SP Restore Set II
                  {34022, 1}, // Tent Palette
                  {34133, 1}, // Space Contract
                  {34101, 1}, // 2X EXP Gold Card
                  {34105, 1}, // Repairing Potion
                  {34137, 1}, // Pet Lethe Scroll
                  {34269, 5}, // Potential Pill x5
                  {61062, 5}, // Chaos Crystal x5
                  {30101, 3}, // Strong Scroll x3
                  {34238, 3}, // Seraph Scroll x3
                  {34086, 1}, // 3% EXP Capsule
                  {34165, 1}}}, // Win Plate
                {34365, // Potential Pack
                 false,
                 {{35114, 1}, // HP Restore Set II
                  {35115, 1}, // SP Restore Set II
                  {34133, 1}, // Space Contract
                  {34167, 1}, // Sewing Tools
                  {34181, 1}, // Training Voucher
                  {34269, 1}, // Potential Pill
                  {34269, 2}, // Potential Pill x2
                  {34269, 4}, // Potential Pill x4
                  {34269, 6}, // Potential Pill x6
                  {34269, 8}, // Potential Pill x8
                  {34269, 10}, // Potential Pill x10
                  {34350, 2}}}, // Super Potential Pill x2
                {34366, // Chaos Pack
                 false,
                 {{35114, 1}, // HP Restore Set II
                  {35115, 1}, // SP Restore Set II
                  {34133, 1}, // Space Contract
                  {34170, 1}, // Weapon Model
                  {34181, 1}, // Training Voucher
                  {61062, 1}, // Chaos Crystal
                  {61062, 2}, // Chaos Crystal x2
                  {61062, 4}, // Chaos Crystal x4
                  {61062, 6}, // Chaos Crystal x6
                  {61062, 8}, // Chaos Crystal x8
                  {61062, 10}, // Chaos Crystal x10
                  {61080, 2}}}, // Pure Chaos Crystal x2
                {34367, // Naughty Monkey Pack
                 false,
                 {{34337, 1}, // Good HP Bottle
                  {34338, 1}, // Good SP Bottle
                  {34022, 1}, // Tent Palette
                  {33054, 1}, // Lollipop
                  {34181, 1}, // Training Voucher
                  {34143, 1}, // 4 Holy EXP Potion
                  {34133, 1}, // Space Contract
                  {34163, 1}, // Teleport Scroll
                  {30101, 1}, // Strong Scroll
                  {34351, 1}, // Anti-Potential Pill
                  {34238, 1}, // Seraph Scroll
                  {30382, 1}}}, // Naughty Monkey Card
                {34381, // Test (placeholder item in the data)
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {25218, 1}, // Monster Net
                  {34105, 1}, // Repairing Potion
                  {34102, 1}, // 4 Leaves Grass
                  {34185, 1}, // 3.5X Holy EXP Potion
                  {34122, 1}, // Lethe Scroll
                  {24210, 1}, // Test (placeholder item)
                  {24211, 1}, // Test (placeholder item)
                  {22517, 1}, // Test (placeholder item)
                  {22518, 1}, // Test (placeholder item)
                  {21303, 1}, // Test (placeholder item)
                  {21304, 1}, // Test (placeholder item)
                  {50003, 1}}}, // Test (placeholder item)
                {34382, // Test (placeholder item in the data)
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {30371, 1}, // Purple Chest Key
                  {34105, 1}, // Repairing Potion
                  {34102, 1}, // 4 Leaves Grass
                  {34185, 1}, // 3.5X Holy EXP Potion
                  {30101, 1}, // Strong Scroll
                  {24212, 1}, // Test (placeholder item)
                  {24213, 1}, // Test (placeholder item)
                  {22519, 1}, // Test (placeholder item)
                  {22520, 1}, // Test (placeholder item)
                  {21305, 1}, // Test (placeholder item)
                  {21306, 1}, // Test (placeholder item)
                  {50004, 1}}}, // Test (placeholder item)
                {34383, // Test (placeholder item in the data)
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {34022, 1}, // Tent Palette
                  {34166, 1}, // Delay Clock
                  {34102, 1}, // 4 Leaves Grass
                  {33054, 1}, // Lollipop
                  {34143, 1}, // 4 Holy EXP Potion
                  {24214, 1}, // Test (placeholder item)
                  {24215, 1}, // Test (placeholder item)
                  {22521, 1}, // Test (placeholder item)
                  {22522, 1}, // Test (placeholder item)
                  {21307, 1}, // Test (placeholder item)
                  {21308, 1}, // Test (placeholder item)
                  {50005, 1}}}, // Test (placeholder item)
                {34384, // Test (placeholder item in the data)
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {30371, 1}, // Purple Chest Key
                  {34166, 1}, // Delay Clock
                  {34167, 1}, // Sewing Tools
                  {33053, 1}, // Korean Cake
                  {34143, 1}, // 4 Holy EXP Potion
                  {24216, 1}, // Test (placeholder item)
                  {24217, 1}, // Test (placeholder item)
                  {22523, 1}, // Test (placeholder item)
                  {22524, 1}, // General East
                  {21309, 1}, // Test (placeholder item)
                  {21310, 1}, // Test (placeholder item)
                  {50006, 1}}}, // Test (placeholder item)
                {34385, // Nostalgic Love Pack
                 false,
                 {{35114, 1}, // HP Restore Set II
                  {35115, 1}, // SP Restore Set II
                  {33053, 1}, // Korean Cake
                  {34170, 1}, // Weapon Model
                  {33031, 3}, // Phoenix Blood x3
                  {34181, 1}, // Training Voucher
                  {24202, 1}, // Valentine Shoes
                  {24203, 1}, // Valentine Shoes
                  {22259, 1}, // White Day Veil
                  {22260, 1}, // White Day Hat
                  {21262, 1}, // M-White Day Suit
                  {21263, 1}, // F-White Day Suit
                  {50007, 1}}}, // Nostalgic Love Card
                {34386, // Nostalgic Wing Pack
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {30371, 1}, // Purple Chest Key
                  {34164, 1}, // Escape Button
                  {34166, 1}, // Delay Clock
                  {33054, 1}, // Lollipop
                  {34143, 1}, // 4 Holy EXP Potion
                  {34181, 1}, // Training Voucher
                  {34105, 1}, // Repairing Potion
                  {34122, 1}, // Lethe Scroll
                  {30101, 1}, // Strong Scroll
                  {34351, 1}, // Anti-Potential Pill
                  {61079, 1}, // Shadow Chaos Crystal
                  {50008, 1}}}, // Nostalgic Wing Card
                {34387, // Nostalgic Cat Pack
                 false,
                 {{34169, 1}, // Bamboo Dragonfly
                  {30371, 1}, // Purple Chest Key
                  {33053, 1}, // Korean Cake
                  {25196, 1}, // VIP Card
                  {34167, 1}, // Sewing Tools
                  {34143, 1}, // 4 Holy EXP Potion
                  {34377, 1}, // EXP Bag
                  {34105, 1}, // Repairing Potion
                  {34137, 1}, // Pet Lethe Scroll
                  {33052, 1}, // System Grenade
                  {34269, 1}, // Potential Pill
                  {34238, 1}, // Seraph Scroll
                  {50009, 1}}}, // Nostalgic Cat Card
                {34388, // Nostalgic Bunny Pack
                 false,
                 {{35114, 1}, // HP Restore Set II
                  {35115, 1}, // SP Restore Set II
                  {25604, 1}, // Fast Jet
                  {33054, 1}, // Lollipop
                  {34170, 1}, // Weapon Model
                  {34143, 1}, // 4 Holy EXP Potion
                  {34377, 1}, // EXP Bag
                  {34168, 1}, // Time Mantle
                  {30101, 1}, // Strong Scroll
                  {33055, 1}, // Big Bowling
                  {34269, 1}, // Potential Pill
                  {61079, 1}, // Shadow Chaos Crystal
                  {50010, 1}}}, // Nostalgic Bunny Card
                {34302, // Anniversary Big Bag
                 false,
                 {{32218, 50}, // Garlic Noodles x50
                  {30137, 3}, // Ship Voucher x3
                  {33033, 1}, // Secular Bird's Blood
                  {34030, 1}, // Glaze Bag
                  {34091, 1}, // 5% EXP Bolus
                  {34253, 1}, // Training Ticket
                  {24142, 1}, // Silver Loops
                  {22218, 1}, // Sunny Doll
                  {34061, 1}, // 5% EXP Capsule
                  {34139, 1}, // Pet Return Scroll
                  {61007, 1}, // Blue Devil's Spar
                  {30051, 1}}}, // Turtle Car Voucher
                {34306, // Crystal Ice Cube
                 false,
                 {{32233, 50}, // Milk Tea x50
                  {33035, 5}, // Black Cffee x5 (sic, as in the data)
                  {30137, 5}, // Ship Voucher x5
                  {34030, 3}, // Glaze Bag x3
                  {34253, 1}, // Training Ticket
                  {34154, 8}, // Dragon Banger x8
                  {34139, 1}, // Pet Return Scroll
                  {30204, 1}, // Harley Voucher
                  {34086, 1}, // 3% EXP Capsule
                  {34063, 1}}}, // 15% EXP Capsule
                {34309, // Super Gift Box
                 false,
                 {{32177, 10}, // Soy Porkbone Noodle x10
                  {32308, 10}, // Curry Rice x10
                  {34030, 3}, // Glaze Bag x3
                  {33020, 3}, // Undead Bird Feather x3
                  {34125, 1}, // Jade Grass
                  {34085, 1}, // Dark Chocolate
                  {32311, 1}, // Hawaii Pizza
                  {34147, 1}, // 2.5 Holy EXP Potion
                  {25200, 1}, // Baked Potato
                  {34012, 1}, // Alchemy Book 4
                  {34139, 1}, // Pet Return Scroll
                  {61009, 1}, // Black Devil's Spar
                  {30071, 1}}}, // Goddess Voucher
                {34311, // Friendship Pumpkin
                 false,
                 {{32177, 10}, // Soy Porkbone Noodle x10
                  {32308, 10}, // Curry Rice x10
                  {34030, 3}, // Glaze Bag x3
                  {33020, 3}, // Undead Bird Feather x3
                  {34125, 1}, // Jade Grass
                  {32311, 1}, // Hawaii Pizza
                  {30255, 1}, // Holy Pumpkin Voucher
                  {33054, 1}, // Lollipop
                  {33053, 1}, // Korean Cake
                  {34131, 1}, // 3 Holy EXP Potion
                  {33052, 1}, // System Grenade
                  {61010, 1}, // Shining Spar
                  {30191, 1}}}, // Hook Elfs Voucher
                {34316, // Thanksgiving Egg
                 false,
                 {{32099, 10}, // Pearl Milk Tea x10
                  {34026, 2}, // Protective EXP Pill x2
                  {30049, 1}, // Steam Ship Voucher
                  {34089, 1}, // Forgotten Scroll
                  {34088, 2}, // Small Love Bouquet x2
                  {25194, 1}, // Fixed Stick
                  {33054, 1}, // Lollipop
                  {34132, 1}, // Loudspeaker
                  {30239, 1}, // Unicorn Card
                  {61011, 1}, // Nimbus Spar
                  {34169, 1}, // Bamboo Dragonfly
                  {34164, 1}}}, // Escape Button
                {34348, // Super Easter Box
                 false,
                 {{34125, 1}, // Jade Grass
                  {34092, 1}, // 10% XP Bolus
                  {34139, 1}, // Pet Return Scroll
                  {34008, 1}, // Fun Token
                  {34061, 1}, // 5% EXP Capsule
                  {32310, 1}, // Holy Water
                  {34146, 1}, // 2 Holy EXP Potion
                  {30303, 1}, // Cerberus Ticket
                  {34253, 1}, // Training Ticket
                  {34239, 1}}}, // Demon Scroll
                {34352, // IGG Happy Pack
                 false,
                 {{34251, 10}, // Water Ball x10
                  {61061, 1}, // Yellow Diamond
                  {32042, 3}, // Minced Pork Congee x3
                  {32176, 4}, // Fugu Hot Pot x4
                  {32310, 1}, // Holy Water
                  {34146, 1}, // 2 Holy EXP Potion
                  {34088, 1}, // Small Love Bouquet
                  {34024, 3}, // Olla Seafood Noodles x3
                  {34239, 1}, // Demon Scroll
                  {34227, 1}}}, // Robinson Riceball
                {34353, // IGG Super Pack
                 false,
                 {{34125, 1}, // Jade Grass
                  {34008, 2}, // Fun Token x2
                  {34146, 1}, // 2 Holy EXP Potion
                  {32310, 3}, // Holy Water x3
                  {34253, 1}, // Training Ticket
                  {34092, 1}, // 10% XP Bolus
                  {34139, 1}, // Pet Return Scroll
                  {34239, 1}, // Demon Scroll
                  {34061, 1}, // 5% EXP Capsule
                  {30302, 1}}}, // Irondino Ticket
                {34354, // IGG Bliss Pack
                 false,
                 {{34148, 1}, // Space Agreement
                  {34030, 1}, // Glaze Bag
                  {61005, 1}, // Green Devil's Spar
                  {34131, 1}, // 3 Holy EXP Potion
                  {34207, 1}, // Fred Riceball
                  {34070, 1}, // Pet Rice Roll
                  {18071, 1}, // Lover's Bouquet
                  {34089, 1}, // Forgotten Scroll
                  {34255, 1}, // Archangel Scroll
                  {30301, 1}}}, // Kirin Ticket
                {34368, // Halloween Bliss Pack
                 false,
                 {{30303, 1}, // Cerberus Ticket
                  {34202, 1}, // Holyspirit Riceball
                  {34203, 1}, // Spirit Riceball
                  {34225, 1}}}, // Dracula Riceball
                {34372, // Colorful Egg
                 false,
                 {{32320, 5}, // Nian Congee x5
                  {34139, 1}, // Pet Return Scroll
                  {34016, 1}, // Encyclopedia
                  {34089, 1}, // Forgotten Scroll
                  {34008, 1}, // Fun Token
                  {25194, 1}, // Fixed Stick
                  {33054, 1}, // Lollipop
                  {34242, 1}, // Spring Red Pack
                  {30239, 1}, // Unicorn Card
                  {61014, 1}, // Orange Spar
                  {34169, 1}, // Bamboo Dragonfly
                  {33044, 1}, // Hot Sweet Potato
                  {34136, 1}, // 1% EXP Capsule
                  {34138, 1}, // 1% EXP Bolus
                  {62001, 1}, // 10% Bomb Diamond
                  {34245, 1}}}, // Virtues Scroll
                {34394, // 2011 New Year Pack
                 false,
                 {{30239, 1}, // Unicorn Card
                  {21159, 1}, // Defense Armor
                  {34102, 1}, // 4 Leaves Grass
                  {61008, 1}, // Purple Devil's Spar
                  {33030, 1}, // Hot Potato
                  {34007, 3}, // Carnie Token x3
                  {34026, 1}, // Protective EXP Pill
                  {34169, 2}, // Bamboo Dragonfly x2
                  {34283, 1}, // Super HP Potion
                  {34284, 1}, // Super SP Potion
                  {34030, 1}, // Glaze Bag
                  {34185, 1}, // 3.5X Holy EXP Potion
                  {34253, 1}, // Training Ticket
                  {25196, 1}, // VIP Card
                  {34239, 1}, // Demon Scroll
                  {34089, 1}, // Forgotten Scroll
                  {34139, 1}, // Pet Return Scroll
                  {33029, 1}}}, // Hot Rice Cake
                {34396, // 2010 Jolly Pack
                 false,
                 {{34283, 1}, // Super HP Potion
                  {34284, 1}, // Super SP Potion
                  {34132, 1}, // Loudspeaker
                  {34242, 1}, // Spring Red Pack
                  {32162, 1}, // Pizza
                  {34137, 1}, // Pet Lethe Scroll
                  {34089, 1}, // Forgotten Scroll
                  {61016, 1}, // Sanguine Spar
                  {34245, 1}, // Virtues Scroll
                  {62002, 1}, // 12% Bomb Diamond
                  {30301, 1}, // Kirin Ticket
                  {34289, 1}}}, // Potential Pill
                {34405, // 2011 Valentine's Pack
                 false,
                 {{34242, 1}, // Spring Red Pack
                  {30334, 1}, // Purple Chest Key
                  {30367, 5}, // Cacao Bean x5
                  {30368, 1}, // Love Candy
                  {24203, 1}, // Valentine Shoes
                  {24202, 1}, // Valentine Shoes
                  {23181, 1}, // White Day Gloves
                  {22260, 1}, // White Day Hat
                  {22259, 1}, // White Day Veil
                  {21263, 1}, // F-White Day Suit
                  {21262, 1}, // M-White Day Suit
                  {18075, 1}}}, // Bliss Bouquet
                {34409, // 2011 Anniversary Box
                 false,
                 {{32062, 5}, // Meat Noodles x5
                  {32273, 5}, // Gyrophora Chicken x5
                  {34242, 1}, // Spring Red Pack
                  {30334, 1}, // Purple Chest Key
                  {32311, 1}, // Hawaii Pizza
                  {34007, 3}, // Carnie Token x3
                  {34181, 1}, // Training Voucher
                  {34115, 1}, // 5.5X EXP Potion
                  {48029, 1}, // Cabriolet Capsule
                  {34137, 1}, // Pet Lethe Scroll
                  {34246, 1}, // Principality Scroll
                  {34122, 1}, // Lethe Scroll
                  {34269, 1}, // Potential Pill
                  {30288, 1}}}, // Pegasus Voucher
            };
            return table;
        }

        // item ids of the 38 random packs
        constexpr std::array<int, 38> kRandomPacks{
            34171, // Gift Pack
            34174, // EXP Pack
            34192, // War Pack
            34193, // Water Pack
            34199, // Strengthen Pack
            34172, // 7 Colors Egg
            34229, // Gear Pack
            34201, // Transferred Pack
            34248, // Scroll Pack
            34296, // Bless Pack
            34297, // Ghost War Horse Pack
            34346, // Naughty Dolphin Pack
            34349, // Fortune Pack
            34365, // Potential Pack
            34366, // Chaos Pack
            34367, // Naughty Monkey Pack
            34381, // Test (placeholder item in the data)
            34382, // Test (placeholder item in the data)
            34383, // Test (placeholder item in the data)
            34384, // Test (placeholder item in the data)
            34385, // Nostalgic Love Pack
            34386, // Nostalgic Wing Pack
            34387, // Nostalgic Cat Pack
            34388, // Nostalgic Bunny Pack
            34302, // Anniversary Big Bag
            34306, // Crystal Ice Cube
            34309, // Super Gift Box
            34311, // Friendship Pumpkin
            34316, // Thanksgiving Egg
            34348, // Super Easter Box
            34353, // IGG Super Pack
            34354, // IGG Bliss Pack
            34368, // Halloween Bliss Pack
            34372, // Colorful Egg
            34394, // 2011 New Year Pack
            34396, // 2010 Jolly Pack
            34405, // 2011 Valentine's Pack
            34409 // 2011 Anniversary Box
        };

    }

    std::optional<std::reference_wrapper<const GiftBoxContents::Box>> GiftBoxContents::find(int boxId) {
        const auto& table = boxes();
        const auto found = std::find_if(table.begin(), table.end(), [boxId](const Box& box) { return box.boxId == boxId; });
        if (found == table.end()) {
            return std::nullopt;
        }
        return std::cref(*found);
    }

    bool GiftBoxContents::isRandomPack(const formats::ItemTable& items, int itemId) {
        return items.find(itemId).has_value() && std::find(kRandomPacks.begin(), kRandomPacks.end(), itemId) != kRandomPacks.end();
    }

}