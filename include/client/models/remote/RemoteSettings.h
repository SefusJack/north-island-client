#ifndef NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_SETTINGS_H
#define NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_SETTINGS_H

#include <array>

namespace client {

    /// @brief What the Remote Control is set to do.
    /// @details Original: Player.FAfkSettings, FAfkValues, FAfkItemSlots, FAfkKeepItemIds. Ticks are numbered as the game does:
    /// 1 auto fight, 2 auto move, 3 auto supply after a battle, 4 leave when no supply is left, 5 leave when the player died too often,
    /// 6 the same for the pet, 7 (supply of the least value), 8 leave after some minutes, 9 take off damaged gear,
    /// 10 discard the chosen items, 11 show the information over the screen.
    struct RemoteSettings {
        static constexpr int kTicks = 11; // tick boxes 1..11 (listed above)
        static constexpr int kKeepItems = 5; // item ids that can be chosen (FAfkKeepItemIds)

        std::array<bool, kTicks + 1> ticks{};
        // default supply thresholds in percent of the most HP/SP
        int hpPercent = 50;
        int spPercent = 50;
        int petHpPercent = 50;
        int petSpPercent = 50;
        // default number of deaths after which the player leaves
        int deathLimit = 3;
        int petDeathLimit = 3;
        int leaveMinutes = 0;
        int itemFrom = 1;
        int itemTo = 50; // last place of the bag (50 places)
        std::array<int, kKeepItems + 1> keepItems{};

        /// @brief Gives the defaults (original ResetAutoPlayDefaults): only the information tick is on.
        /// @return The default settings.
        static RemoteSettings defaults() {
            RemoteSettings settings;
            settings.ticks[11] = true; // tick 11: show the information over the screen
            return settings;
        }

        /// @brief Gets a value by its number (original FAfkValues.V[1..7]).
        /// @param index Value number 1..7.
        /// @return The value.
        int value(int index) const {
            // index 0 unused, then the seven values V[1..7]
            const std::array<int, 8> values = {0, hpPercent, spPercent, petHpPercent, petSpPercent, deathLimit, petDeathLimit, leaveMinutes};
            return values[static_cast<std::size_t>(index)];
        }
    };

}

#endif
