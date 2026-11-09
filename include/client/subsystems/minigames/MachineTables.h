#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_MACHINE_TABLES_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_MACHINE_TABLES_H

#include "core/Point.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief The prize and position tables of the game hall machines.
    /// @details Original: Re_PubConst.pas, CY_LittleSport.pas; index 0 of a prize table is what a play without a prize gives.
    struct MachineTables {
        static constexpr int kPrizes = 10; // prizes per machine table (index 0 = no prize)
        using Prizes = std::array<int, kPrizes + 1>;

        /// @brief The item ids of the ten prizes of the Fortune Egg.
        /// @return The prize table.
        static const Prizes& eggPrizes();

        /// @brief The item ids of the ten prizes of a slot machine.
        /// @param kind Machine kind (10 or 19; any other gives the kind 8 table).
        /// @return The prize table.
        static const Prizes& slotPrizes(int kind);

        /// @brief The box of a prize of the egg.
        /// @param prize Prize number 1 to 10.
        /// @return Position on the screen.
        /// @details Original: TrunEggItemPos.
        static core::Point eggPrizeAt(int prize);

        /// @brief The prize line of a slot machine.
        /// @param prize Prize number 1 to 10.
        /// @return Position on the screen.
        /// @details Original: Slotmach3PrizeBtnPos.
        static core::Point slotPrizeAt(int prize);

        /// @brief The place of a reel of a slot machine.
        /// @param reel Reel number 1 to 3.
        /// @return Position on the screen.
        /// @details Original: Slotmach3ReelPos.
        static core::Point reelAt(int reel);

        /// @brief When a reel stops after the spin starts.
        /// @param reel Reel number 1 to 3.
        /// @return Time in ms.
        /// @details Original: Slotmach3ReelStopTicks.
        static std::uint32_t reelStopMs(int reel);
    };

}

#endif
