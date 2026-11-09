#include "client/data/MagicMapPlaces.h"

#include <vector>

namespace client {

    namespace {

        /// @brief One place of the magic map: name, world map point, hidden flag, hint cell, Mediterranean mark and map id.
        struct Place {
            const char* name;
            int mapX;
            int mapY;
            bool hidden;
            int cellX;
            int cellY;
            bool mediterranean;
            int mapId;
        };

        // rows: {name, mapX, mapY (12000-wide world map units), hidden, cellX, cellY, mediterranean, map id}; row 0 unused
        constexpr Place kPlaces[MagicMapPlaces::kCount + 1] = {
            {"", 0, 0, false, 0, 0, false, 0},
            {"Fishing Village", 6900, 3050, false, 6900, 3050, false, 12100}, // map 12100: Fishing Village
            {"Calcutta", 4842, 4000, false, 4842, 4000, false, 12300}, // map 12300: Calcutta
            {"Oslya", 6782, 8555, false, 6782, 8555, false, 60006}, // map 60006: Oslya
            {"Mayan Forest", 11722, 5955, false, 11722, 5955, false, 60005}, // map 60005: Mayan Forest
            {"Universal", 4950, 6650, false, 4950, 6650, false, 60007}, // map 60007: Universal
            {"Revival Isle ", 10662, 9700, false, 10662, 9700, false, 60004}, // map 60004: Revival Isle
            {"Big Island of Hawaii", 9000, 4000, false, 9000, 4000, false, 60008}, // map 60008: Large Island (Hawaii)
            {"South Pole", 6460, 11195, false, 6460, 11195, false, 12355}, // map 12355: South Pole
            {"Korea", 6960, 2255, false, 6960, 2255, false, 60013}, // map 60013: Korea
            {"Iceberg", 11482, 10675, false, 11482, 10675, false, 12356}, // map 12356: Iceberg
            {"Koyota", 7692, 2235, false, 7692, 2235, false, 60011}, // map 60011: Koyota
            {"Taiwan ", 7200, 3600, false, 7200, 3600, false, 60012}, // map 60012: Taiwan (no name in maps.tsv)
            {"South Island", 7500, 9700, false, 7500, 9700, false, 60002}, // map 60002: South Island
            {"North Island", 7450, 8460, false, 7450, 8460, false, 60000}, // map 60000: North Island
            {"Stoowa Isle", 7100, 10200, false, 7100, 10200, false, 60003}, // map 60003: Stoowa Isle
            {"Egypt", 1300, 3135, false, 1300, 3135, false, 12500}, // map 12500: Cairo (Egypt)
            {"Madagascar", 2002, 6765, false, 2002, 6765, false, 11127}, // map 11127: Madagascar
            {"Polo Island", 6012, 5675, false, 6012, 5675, false, 10069}, // map 10069: Polo Island
            {"Hezrou Island", 6682, 6675, false, 6682, 6675, false, 10070}, // map 10070: Hezrou Island
            {"Snow Island", 2202, 10975, false, 2202, 10975, false, 12367}, // map 12367: 1F, Snow Cave (Snow Island)
            {"Inca", 11462, 7895, false, 11462, 7895, false, 60015}, // map 60015: Inca
            {"Ghost Isle", 9162, 1335, false, 9162, 1335, false, 12286}, // map 12286: Ghost Isle Cave
            {"Shayii Hill", 8422, 1635, false, 8422, 1635, false, 12281}, // map 12281: Shayii Hill
            {"Ancient Yazhou City", 6350, 3900, false, 6350, 3900, false, 12105}, // map 12105: Ancient Yazhou City
            {"Bangkok", 5882, 4435, false, 5882, 4435, false, 12601}, // map 12601: Harbor (Bangkok)
            {"Lion Isle", 4382, 4875, false, 4382, 4875, false, 11145}, // map 11145: Lion Isle
            {"Centaur Island", 3902, 6215, false, 3902, 6215, false, 11135}, // map 11135: Centaur Island
            {"Sky City", 4402, 10275, false, 4402, 10275, false, 11152}, // map 11152: Barnya Sky City
            {"African Villages", 1422, 5415, false, 1422, 5415, false, 12700}, // map 12700: African Villages
            {"Yamataikoku", 7750, 2570, false, 7750, 2570, false, 12850}, // map 12850: Yamataikoku
            {"Athens ", 900, 2235, false, 3057, 2573, true, 12800}, // map 12800: Athens
            {"Small Island of Hawaii", 8800, 3600, false, 8800, 3600, false, 60009}, // map 60009: Small Island (Hawaii)
            {"Training Island", 8530, 7601, false, 8530, 7601, false, 13050}, // map 13050: Training Island
            {"Rome", 350, 2000, false, 2635, 2242, true, 13200}, // map 13200: Rome
            {"Persia", 3000, 3050, false, 3000, 3050, false, 13100}, // map 13100: Persia
            {"Filipine", 7390, 4375, false, 7390, 4375, false, 12952}, // map 12952: Philippine
            {"Lost Jungle", -100, 1700, false, 2207, 1952, true, 13261}, // map 13261: Lost Jungle
            {"Fog Jungle", -200, 900, false, 1987, 1219, true, 13164}, // map 13164: Fog Jungle
            {"Chang'an", 6175, 2900, false, 6175, 2900, false, 12110}, // map 12110: Chang'an
            {"Deep Jungle", 1300, 2485, false, 3427, 2717, true, 12905}, // map 12905: Deep Jungle
        };

    }

    const std::string& MagicMapPlaces::name(int place) {
        static const std::string none;
        static const std::vector<std::string> names = [] {
            std::vector<std::string> list;
            for (const Place& entry : kPlaces) {
                list.emplace_back(entry.name);
            }
            return list;
        }();
        return place >= 1 && place <= kCount ? names[static_cast<std::size_t>(place)] : none;
    }

    core::Point MagicMapPlaces::mapPoint(int place) {
        return place >= 1 && place <= kCount ? core::Point{kPlaces[place].mapX, kPlaces[place].mapY} : core::Point{};
    }

    bool MagicMapPlaces::isHidden(int place) { return place >= 1 && place <= kCount && kPlaces[place].hidden; }

    core::Point MagicMapPlaces::cell(int place) {
        return place >= 1 && place <= kCount ? core::Point{kPlaces[place].cellX, kPlaces[place].cellY} : core::Point{};
    }

    int MagicMapPlaces::mapId(int place) { return place >= 1 && place <= kCount ? kPlaces[place].mapId : 0; }

}
