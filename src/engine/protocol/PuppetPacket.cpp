#include "protocol/PuppetPacket.h"

#include "core/ByteReader.h"

namespace protocol {

    std::optional<PuppetPacket> PuppetPacket::decode(const core::Bytes& payload) {
        core::ByteReader reader(payload);
        if (reader.u8() != 23 || reader.u8() != 104) { // server action 23, sub 104: Memory Card and puppet training
            return std::nullopt;
        }
        PuppetPacket packet;
        packet.option = reader.u8();
        switch (packet.option) {
            case 1: // option 1: invitation (id the invited player, other who asks)
            case 5: // option 5: the requester wants to stop (id requester, other helper)
            case 10: // option 10: the helper wants to stop (id, other)
            case 15: // option 15: the partner is off line (id, other)
            case 16: // option 16: the partner is on line (id, other; the level follows)
                packet.id = reader.i32();
                packet.other = reader.i32();
                break;
            case 2: // option 2: the helper is set (id, card bag place, helper, colour codes)
                packet.id = reader.i32();
                packet.slot = reader.u8();
                packet.other = reader.i32();
                packet.person.colorCode1 = reader.i32();
                packet.person.colorCode5 = reader.i32();
                break;
            case 3: // option 3: result text (code)
                packet.code = reader.u8();
                break;
            case 4: // option 4: state (code 1 requester, 2 helper, 3 the card's exp is exp)
                packet.code = reader.u8();
                if (packet.code == 3) { // state 3: the card's exp follows
                    packet.exp = reader.i32();
                } else {
                    packet.slot = reader.u8();
                }
                break;
            case 6: // option 6: the training is over (id, slot)
            case 9: // option 9: a card removed (id, slot)
            case 19: // option 19: the card is free (id, slot)
                packet.id = reader.i32();
                packet.slot = reader.u8();
                break;
            case 7: // option 7: reset the requester's form (id)
                packet.id = reader.i32();
                break;
            case 8: // option 8: a card's exp (id, requester, exp, card place; extended: step, colours, guild)
                packet.id = reader.i32();
                packet.other = reader.i32();
                packet.exp = reader.i32();
                packet.slot = reader.u8();
                if (reader.remaining() > 0) {
                    packet.extended = true;
                    packet.step = reader.u8();
                    packet.person.colorCode1 = reader.i32();
                    packet.person.colorCode5 = reader.i32();
                    packet.person.guild = reader.shortText();
                }
                break;
            case 12: // option 12: the requester's card exp (id, exp)
                packet.id = reader.i32();
                packet.exp = reader.i32();
                break;
            case 13: // option 13: a card of the helper (id, slot, exp, online, person, step)
                packet.id = reader.i32();
                packet.slot = reader.u8();
                packet.exp = reader.i32();
                packet.online = reader.u8() != 0;
                packet.person.id = reader.i32();
                packet.person.level = reader.u8();
                packet.person.element = reader.u8();
                packet.person.job = reader.u8();
                packet.person.head = reader.u8();
                packet.person.bodyType = reader.u8();
                packet.person.colorCode1 = reader.i32();
                packet.person.colorCode5 = reader.i32();
                packet.person.name = reader.shortText();
                packet.person.title = reader.shortText();
                packet.person.guild = reader.shortText();
                packet.step = reader.u8();
                break;
            case 14: // option 14: the requester's form (id, person, online, exp)
                packet.id = reader.i32();
                packet.person.name = reader.shortText();
                packet.person.colorCode1 = reader.i32();
                packet.person.colorCode5 = reader.i32();
                packet.person.id = reader.i32();
                packet.online = reader.u8() != 0;
                packet.person.head = reader.u8();
                packet.person.bodyType = reader.u8();
                packet.exp = reader.i32();
                break;
            case 17: // option 17: a card by member id (id, member, mail place, exp, online, step)
                packet.id = reader.i32();
                packet.other = reader.i32();
                packet.slot = reader.u8();
                packet.exp = reader.i32();
                packet.online = reader.u8() != 0;
                packet.step = reader.u8();
                break;
            case 18: // option 18: nothing to read
                break;
            case 20: // option 20: the card is in use (slot)
                packet.slot = reader.u8();
                break;
            case 21: // option 21: time left (slot, code minutes)
                packet.slot = reader.u8();
                packet.code = reader.u16();
                break;
            case 22: // option 22: the card's size (id, step, code)
                packet.id = reader.i32();
                packet.step = reader.u8();
                packet.code = reader.u8();
                break;
            default: // unknown option: rejected
                return std::nullopt;
        }
        if (packet.option == 16) { // option 16: partner on line
            packet.code = reader.u8();
        }
        if (reader.failed()) {
            return std::nullopt;
        }
        return packet;
    }

}
