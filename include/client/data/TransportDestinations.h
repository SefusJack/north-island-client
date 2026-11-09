#ifndef NORTH_ISLAND_CLIENT_DATA_TRANSPORT_DESTINATIONS_H
#define NORTH_ISLAND_CLIENT_DATA_TRANSPORT_DESTINATIONS_H

#include <string>
#include <vector>

namespace client {

    /// @brief Where the vehicles of the trips go, by VehicleKinds::teleportKind.
    /// @details Original: TransmitVillageNames, TransmitSeaNames, TransmitPlanetNames of DZ_String.pas. The server is told the place's number from 1.
    struct TransportDestinations {
        /// @brief The destination names of a trip kind: the villages, the sea (surface and seabed), the planets.
        /// @param teleportKind Kind from VehicleKinds::teleportKind.
        /// @return Names of the places.
        static const std::vector<std::string>& namesOf(int teleportKind);
    };

}

#endif
