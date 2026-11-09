#ifndef NORTH_ISLAND_UI_UI_CONTEXT_H
#define NORTH_ISLAND_UI_UI_CONTEXT_H

#include "core/Rect.h"
#include "ui/EventArgs.h"

#include <cstdint>
#include "ui/DragItem.h"

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace ui {

    /// @brief What controls can reach while handling input: event handlers by layout name, and a few program services.
    class UiContext {
        public:
            using Handler = std::function<void(const EventArgs&)>;

            /// @brief Registers the handler for an event name.
            /// @param name Handler name as the layout gives it.
            /// @param handler Function called when the event is raised.
            void setHandler(const std::string& name, Handler handler) { m_handlers[name] = std::move(handler); }

            /// @brief Calls the handler of that name; an event without a handler is ignored.
            /// @param handlerName Handler name.
            /// @param args Event arguments.
            void raise(const std::string& handlerName, const EventArgs& args) const;

            std::function<void(const std::string& soundFile)> playSound;
            std::function<std::string()> clipboardText;

            std::uint64_t nowMs = 0;
            core::Rect view{0, 0, 800, 600}; // the game's 800 x 600 screen
            bool ctrlHeld = false;
            bool releaseOfDoubleClick = false;

            int focusedId = 0;
            std::function<void(const std::string& elementName)> focusByName;
            std::optional<DragItem> drag;

            std::function<void(int elementId)> bringToFront;
            std::function<void(int elementId)> bringAbovePopups;

        private:
            std::map<std::string, Handler> m_handlers;
    };

}

#endif
