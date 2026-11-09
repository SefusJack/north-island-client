#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_PROBE_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_PROBE_H

#include "client/services/input/ScriptObjectRef.h"
#include "client/services/input/ScriptSighting.h"
#include "core/Point.h"

#include <cstdint>
#include <optional>
#include <string>

namespace client {

    /// @brief What a test script (InputScript) may ask of the scene it plays into. Test-only: a scene with no use for scripts does not offer it.
    class ScriptProbe {
        public:
            virtual ~ScriptProbe() = default;

            /// @brief Finds the object in the scene now.
            /// @param object The object a script named.
            /// @return Its sighting, empty when it is not there.
            virtual std::optional<ScriptSighting> sight(const ScriptObjectRef& object) = 0;

            /// @brief Does what a click on the map does: the player sets off for a map point.
            /// @param mapPoint Target map point.
            /// @param reason Receives why the player cannot walk now.
            /// @return False, with the reason, when he cannot walk now.
            virtual bool startWalk(core::Point mapPoint, std::string& reason) = 0;

            /// @brief Tells whether the player is walking.
            /// @return True while walking.
            virtual bool isWalking() const = 0;

            /// @brief The player's map point.
            /// @return The point.
            virtual core::Point ownPosition() const = 0;

            /// @brief The map the player is on.
            /// @return The map id.
            virtual int mapId() const = 0;

            /// @brief Counts the chat and message lines shown so far.
            /// @return The count, to pass to textSince later.
            virtual std::uint64_t textMark() const = 0;

            /// @brief Tells whether a line added after the count contains the text.
            /// @param mark Count from textMark().
            /// @param text Text to search for.
            /// @return True when found.
            virtual bool textSince(std::uint64_t mark, const std::string& text) const = 0;
    };

}

#endif
