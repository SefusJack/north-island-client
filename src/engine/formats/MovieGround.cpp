#include "formats/MovieGround.h"

namespace formats {

    namespace {
        constexpr std::size_t kPrefixSize = 3;
    }

    MovieGround::Kind MovieGround::kind() const {
        const std::string prefix = name.substr(0, kPrefixSize);
        if (prefix == "JRP") { // prefix marking a movie picture of the jrp set
            return Kind::Jrp;
        }
        return prefix == "Map" ? Kind::Map : Kind::Picture;
    }

    std::string MovieGround::pictureName() const { return kind() == Kind::Picture ? name : name.substr(kPrefixSize); }

}
