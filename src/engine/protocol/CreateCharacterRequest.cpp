#include "protocol/CreateCharacterRequest.h"

#include "core/ByteWriter.h"
#include "protocol/MessageId.h"

namespace protocol {

    namespace {

        /// @brief Narrows a value to a byte.
        /// @param value The value.
        /// @return Its low byte.
        std::uint8_t byte(int value) { return static_cast<std::uint8_t>(value); }

    }

    core::Bytes CreateCharacterRequest::encode() const {
        core::ByteWriter writer;
        writer.u8(kActionCreateCharacter).u8(1).u8(byte(bodyType)).u8(0).u8(byte(head)).u8(byte(bodySprite));
        writer.i32(colorCode1).i32(colorCode2).u8(byte(element));
        // stats indexes 0,4,3,2,1 pick STR, AGI, WIS, INT, CON out of the wizard's order STR, CON, INT, WIS, AGI
        writer.u8(byte(stats[0])).u8(byte(stats[4])).u8(byte(stats[3])).u8(byte(stats[2])).u8(byte(stats[1]));
        if (cipher) {
            writer.shortText(*cipher);
        }
        return writer.take();
    }

}
