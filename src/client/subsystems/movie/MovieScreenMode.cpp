#include "client/subsystems/movie/MovieScreenMode.h"

namespace client {

    void MovieScreenMode::eventOption(int option) {
        if (option == 1 || option == 3) { // event options 1 and 3: mode 1
            m_mode = kEvent;
        } else if (option == 2 || option == 4) { // event options 2 and 4: mode 4, last picture stays
            m_mode = kHeld;
        }
    }

    bool MovieScreenMode::movieEnded() {
        const bool held = m_mode == kHeld;
        m_mode = held ? 2 : 0; // mode 2: frozen on the last picture
        m_frozen = held;
        return held;
    }

}
