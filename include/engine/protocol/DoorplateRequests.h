#ifndef NORTH_ISLAND_PROTOCOL_DOORPLATE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_DOORPLATE_REQUESTS_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Builds client 62/46 with the doorplate text; an empty text is 62/45 with no data: the doorplate goes.
    /// @param text Text shown (at most 14 bytes).
    /// @return The encoded request.
    core::Bytes doorplateRequest(const std::string& text);

}

#endif
