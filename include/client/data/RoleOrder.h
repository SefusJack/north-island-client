#ifndef NORTH_ISLAND_CLIENT_DATA_ROLE_ORDER_H
#define NORTH_ISLAND_CLIENT_DATA_ROLE_ORDER_H

#include <optional>
#include <utility>

namespace client {

    /// @brief The place of a body type and head among the figures; the voices and portraits go by it.
    /// @details Original: GetRoleOrder.
    /// @param bodyType Body type.
    /// @param head Head number.
    /// @return The order, 0 for a pair outside the figures.
    int roleOrder(int bodyType, int head);

    /// @brief Splits a role number into its body type and head.
    /// @details Original: SplitRoleIndex.
    /// @param role Role number (1..13).
    /// @return (bodyType, head), or none for another number.
    std::optional<std::pair<int, int>> splitRoleIndex(int role);

}

#endif
