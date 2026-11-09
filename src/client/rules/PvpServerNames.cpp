#include "client/rules/PvpServerNames.h"

namespace client {

    std::string pvpServerPlayerName(const std::string& name, int serverCode, bool onPvpServer) {
        return onPvpServer ? std::to_string(serverCode) + "-" + name : name;
    }

}
