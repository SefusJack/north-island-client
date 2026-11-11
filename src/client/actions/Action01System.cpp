#include "client/actions/Action01System.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/ServerSelected.h"

namespace client {

    void handleAction01System(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: { // 1/1: i32 id, a player left the map
                core::ByteReader reader(payload, 2);
                context.events.onPlayerLeft(reader.i32());
                break;
            }
            case 3: { // 1/3: open character creation (u8 hasCipher)
                core::ByteReader reader(payload, 2);
                context.events.onCreateCharacter(reader.u8() == 0);
                break;
            }
            case 11: // 1/11: the map is ready, show the world
                context.events.onEnterWorldFinished();
                break;
            case protocol::kSystemTimeout: // 1/5: time out
                context.events.onLoginRejected(LoginRejection::Timeout);
                break;
            case protocol::kSystemWrongPassword: // 1/6: wrong account or password
                context.events.onLoginRejected(LoginRejection::WrongPassword);
                break;
            case protocol::kSystemConnectionFailed: // 1/7: connection failed
                context.events.onLoginRejected(LoginRejection::ConnectionFailed);
                break;
            case protocol::kSystemCharacterLocked: // 1/8: u32 seconds, a character is locked
            case protocol::kSystemAccountLocked: { // 1/10: u32 seconds, the account is locked
                core::ByteReader reader(payload, 2);
                context.events.onLocked(id.sub == protocol::kSystemAccountLocked, reader.u32());
                break;
            }
            case protocol::kSystemServerSelected: // 1/9: the server accepted the connection
                if (const auto message = protocol::ServerSelected::decode(payload)) {
                    context.events.onServerSelected(*message);
                }
                break;
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
