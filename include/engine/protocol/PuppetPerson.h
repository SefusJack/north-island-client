#ifndef NORTH_ISLAND_PROTOCOL_PUPPET_PERSON_H
#define NORTH_ISLAND_PROTOCOL_PUPPET_PERSON_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Who plays for whom in puppet training, as the Memory Card's forms hold a person.
    /// @details Original: the fields of a THuman they use: who, level, job, element, face (body type, head, two colour codes), the title
    /// (FExtraName28) and the guild name (FExtraName37).
    struct PuppetPerson {
        std::int32_t id = 0;
        std::string name;
        std::string title;
        std::string guild;
        int level = 0;
        int job = 0;
        int element = 0;
        int bodyType = 0;
        int head = 0;
        std::int32_t colorCode1 = 0;
        std::int32_t colorCode5 = 0;
    };

}

#endif
