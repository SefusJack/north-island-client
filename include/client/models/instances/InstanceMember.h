#ifndef NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_MEMBER_H
#define NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_MEMBER_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief A player of an instance in the detail window's list.
    struct InstanceMember {
        std::string name;
        std::int32_t id = 0;
    };

}

#endif
