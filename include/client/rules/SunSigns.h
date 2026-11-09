#ifndef NORTH_ISLAND_CLIENT_RULES_SUN_SIGNS_H
#define NORTH_ISLAND_CLIENT_RULES_SUN_SIGNS_H

#include <cstddef>
#include <cstdint>

namespace client {

    /// @brief Gives the star sign of a birth date (Original: CalcSunSign).
    /// @param year Birth year.
    /// @param month Birth month.
    /// @param day Birth day.
    /// @return 1 Aries (21 March) .. 12 Pisces; 0 without a whole date (year, month and day all given).
    int sunSign(int year, int month, int day);

    /// @brief Gives the heart number a profile gives on the setup tab's heart radar (Original: UpdateHeartRadar).
    /// @details Two players of different body types whose numbers add up to 9 find each other on the radar.
    /// @param birthYear Birth year, counted from 1900.
    /// @param birthMonth Birth month.
    /// @param birthDay Birth day.
    /// @param sunSign Star sign of the birth date.
    /// @param profileChoice The chosen profile value.
    /// @param playerId Id of the player.
    /// @param nameLength Length of the player's name.
    /// @return The heart number, 0..9.
    int heartRadarNumber(
        int birthYear,
        int birthMonth,
        int birthDay,
        int sunSign,
        int profileChoice,
        std::int32_t playerId,
        std::size_t nameLength
    );

}

#endif
