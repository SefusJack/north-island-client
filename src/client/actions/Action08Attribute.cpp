#include "client/actions/Action08Attribute.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    namespace {

        constexpr int kPetKind = 4;

        /// @brief Reads an own pet's attribute change (8/2) and reports it.
        /// @param payload Raw message bytes.
        /// @param context Connection and event sinks of the current session.
        void handlePetAttribute(const core::Bytes& payload, MessageContext& context) {
            core::ByteReader reader(payload, 2);
            const int kind = reader.u8();
            const int place = reader.u16();
            const int attribute = reader.u8();
            const int sign = reader.u8();
            std::int32_t value = reader.i32();
            if (sign == 2) { // sign 2 means negative
                value = -value;
            }
            const std::int32_t extra = reader.i32();
            if (reader.failed() || kind != kPetKind) {
                context.events.onUnhandled(protocol::MessageId::of(payload));
                return;
            }
            context.events.onPetAttributeChanged(place, attribute, value, extra);
        }

        /// @brief Reads a teammate's attribute change (8/3) and reports it.
        /// @details Original: HandleWorldPlayerStatPacket.
        /// @param payload Raw message bytes.
        /// @param context Connection and event sinks of the current session.
        void handleWorldPlayerAttribute(const core::Bytes& payload, MessageContext& context) {
            core::ByteReader reader(payload, 2);
            const std::int32_t playerId = reader.i32();
            const int attribute = reader.u8();
            const int sign = reader.u8();
            std::int32_t value = reader.i32();
            if (sign == 2) { // sign 2 means negative
                value = -value;
            }
            const std::int32_t extra = reader.i32();
            if (reader.failed()) {
                context.events.onUnhandled(protocol::MessageId::of(payload));
                return;
            }
            context.events.onWorldPlayerAttribute(playerId, attribute, value, extra);
        }

    }

    void handleAction08Attribute(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 2) { // 8/2: an own pet's attribute
            handlePetAttribute(payload, context);
            return;
        }
        if (id.sub == 3) { // 8/3: a teammate's attribute
            handleWorldPlayerAttribute(payload, context);
            return;
        }
        if (id.sub != 1) {
            context.events.onUnhandled(id);
            return;
        }
        core::ByteReader reader(payload, 2);
        const int attribute = reader.u8();
        const int sign = reader.u8();
        std::int32_t value = reader.i32();
        if (sign == 2) { // sign 2 means negative
            value = -value;
        }
        if (reader.failed()) {
            context.events.onUnhandled(id);
            return;
        }
        const std::int32_t extra = reader.remaining() >= 4 ? reader.i32() : 0; // 4 bytes = one i32 left
        context.events.onAttributeChanged(attribute, value, extra);
    }

}
