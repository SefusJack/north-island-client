#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SPORT_SCENE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SPORT_SCENE_H

#include "core/Bytes.h"

#include <cstdint>

namespace client {

    /// @brief A sport that takes the whole screen; one scene is what one game kind adds to the shared sport flow.
    /// @details Original: CY_Sport.pas TSportManage (game kind in FActiveSport, dispatched by kind); RecvSportStart creates the game,
    /// the sport loop calls update/draw and HandleSportMouseDown / Move / Up, 35/4 points and 71/kind messages reach the game,
    /// EndSportGame / FreeSportGame take it down.
    class SportScene {
        public:
            virtual ~SportScene() = default;

            /// @brief The game kind of RecvSportStart this scene is.
            /// @return The game kind.
            virtual int kind() const = 0;

            /// @brief Receives what 57/1 carries besides the kind, before the game is created.
            /// @param value The packet's value.
            /// @param level The packet's level.
            virtual void startedWith(int value, int level) {
                (void)value;
                (void)level;
            }

            /// @brief Creates the game; it shows under the instructions.
            /// @param nowMs Current time in ms.
            virtual void open(std::uint64_t nowMs) = 0;

            /// @brief Takes the scene down.
            /// @details Original: FreeSportGame, or the game ending.
            virtual void close() = 0;

            /// @brief The instructions' Leave and close buttons (the game's exit).
            virtual void leave() = 0;

            /// @brief The instructions' Begin button.
            /// @param nowMs Current time in ms.
            virtual void begin(std::uint64_t nowMs) { (void)nowMs; }

            /// @brief Handles a 71/kind message while the sport runs.
            /// @param payload Packet bytes.
            virtual void receive(const core::Bytes& payload) { (void)payload; }

            /// @brief Handles 35/4 while the sport runs: the account's points.
            /// @param points The sum of the two counts.
            virtual void pointsArrived(std::int32_t points) { (void)points; }
    };

}

#endif
