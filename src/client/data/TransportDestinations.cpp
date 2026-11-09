#include "client/data/TransportDestinations.h"

namespace client {

    const std::vector<std::string>& TransportDestinations::namesOf(int teleportKind) {
        static const std::vector<std::string> none;
        static const std::vector<std::string> villages = {
            "Kelan Village",
            "Welling Village",
            "Holy Village ",
            "Fishing Village",
            "Mayan Tribe",
            "Long-eared Tribe",
            "Kyoto",
            "Calcutta",
            "Penguin Village",
            "Inchon",
            "Cairo",
            "Bangkok",
            "Inca",
            "Yamataikoku",
            "Athens",
            "Cornwall",
            "Domremy",
            "Persia",
            "Rome"
        };
        static const std::vector<std::string> sea = {"Emerge to the surface", "Seabed"};
        static const std::vector<std::string> planets = {"Earth", "Moon", "Mars"};
        switch (teleportKind) {
            case 1: return villages; // village robot trips
            case 2: return sea; // submarine trips
            case 3: return planets; // landing craft trips
            default: return none; // no trips
        }
    }

}
