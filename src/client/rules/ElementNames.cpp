#include "client/rules/ElementNames.h"

namespace client {

    std::string elementName(int element) {
        switch (element) {
            case 1: return "Earth"; // element 1: Earth
            case 2: return "Water"; // element 2: Water
            case 3: return "Fire"; // element 3: Fire
            case 4: return "Wind"; // element 4: Wind
            default: return "None"; // element 0 or unknown: None
        }
    }

}
