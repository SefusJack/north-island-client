#ifndef NORTH_ISLAND_CLIENT_MODELS_CHAT_SYSTEM_ANNOUNCE_H
#define NORTH_ISLAND_CLIENT_MODELS_CHAT_SYSTEM_ANNOUNCE_H

#include <string>

namespace client {

    /// @brief A prize announcement of the server: what the game builds the line from.
    /// @details Original: server 2/15, RecvSystemAnnounce.
    struct SystemAnnounce {
        int kind = 0;
        int sourceId = 0;
        int prizeId = 0;
        int count = 1;
        int machine = 0;
        std::string player;
        std::string text;

        /// @brief Builds the announcement line.
        /// @param sourceName What the id of the source is called.
        /// @param prizeName What the id of the prize is called.
        /// @return The line, "" for a kind or machine the game does not announce.
        std::string line(const std::string& sourceName, const std::string& prizeName) const;
    };

}

#endif
