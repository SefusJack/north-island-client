#include "client/models/chat/SystemAnnounce.h"

#include <optional>
#include <string>

namespace client {

    namespace {

        /// @brief The name of a game machine kind as announced.
        /// @param machine The game machine's kind.
        /// @return The name, none for a machine the game does not announce.
        std::optional<std::string> machineName(int machine) {
            switch (machine) {
                case 6: return "Egg Machine"; // machine kind 6
                case 7: return "Moppet Machine"; // machine kind 7
                case 8: return "Pull Machine"; // machine kind 8
                case 9: return "Boxing Machine"; // machine kind 9
                case 10: // machine kind 10 shares the name of kind 11
                case 11: return "Potion Happy Machine"; // machine kind 11
                case 12: return "Pull Machine"; // machine kind 12
                case 14: return "Challenge Machine"; // machine kind 14
                case 16: return "Double Machine"; // machine kind 16
                case 19: return "Stone Pull Machine"; // machine kind 19
                case 21: return "Lucky Wheel"; // machine kind 21
                default: return std::nullopt; // not announced
            }
        }

    }

    std::string SystemAnnounce::line(const std::string& sourceName, const std::string& prizeName) const {
        const std::string times = count > 1 ? "x" + std::to_string(count) : std::string();
        switch (kind) {
            case 0: // 2/15 kind 0: a plain line
                return text;
            case 1: // 2/15 kind 1: an item used for a prize item
                return "Congratulations [" + player + "] Used " + sourceName + " Obtained " + prizeName + times;
            case 2: // 2/15 kind 2: an item used for a pet
                return "Congratulations [" + player + "] Used " + sourceName + " Obtained " + prizeName;
            case 3: { // 2/15 kind 3: a game machine's prize
                const auto name = machineName(machine);
                if (!name) {
                    return {};
                }
                return "Congratulations [" + player + "] Used Game Machine: " + *name + " Obtained " + prizeName + times;
            }
            case 4: // 2/15 kind 4: an online draw's prize
                return "Congratulations [" + player + "Take part in the online draw Obtained " + prizeName + times;
            default: // unknown kind: not announced
                return {};
        }
    }

}
