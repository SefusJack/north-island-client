#include "ui/UiContext.h"

namespace ui {

    void UiContext::raise(const std::string& handlerName, const EventArgs& args) const {
        if (handlerName.empty()) {
            return;
        }
        const auto found = m_handlers.find(handlerName);
        if (found != m_handlers.end() && found->second) {
            found->second(args);
        }
    }

}
