#ifndef NORTH_ISLAND_CLIENT_RULES_ELEMENT_NAMES_H
#define NORTH_ISLAND_CLIENT_RULES_ELEMENT_NAMES_H

#include <string>

namespace client {

    /// @brief Gives the hint of an element's icon: None, Earth, Water, Fire or Wind.
    /// @details Original: SetupElementIcon.
    /// @param element Element number 0..4.
    /// @return The element's name.
    std::string elementName(int element);

}

#endif
