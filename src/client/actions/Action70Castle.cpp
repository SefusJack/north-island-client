#include "client/actions/Action70Castle.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <optional>
#include <string>

namespace client {

    namespace {

        /// @brief Gives the text of a Trojan war message that is one line of the message box.
        /// @details Original: RecvTrojanWarMsg.
        /// @param message The u8 message number of 70/1.
        /// @return The line, or nullopt when the message is not a box line.
        std::optional<std::string> boxLine(int message) {
            switch (message) {
                case 1: return "The same guild can come to the field"; // 70/1 message 1
                case 2: return "Please go to branch server 1 to join in the Trojan War event"; // 70/1 message 2
                case 3: return "Preparing for the Trojan War"; // 70/1 message 3
                case 9: return "The time of Trojan War is:\rDuring 7:00 PM-9:00 PM on every Sunday"; // 70/1 message 9
                case 12: return "Some members can't meet the battle second limit requirement"; // 70/1 message 12
                case 13: return "In Trojan War event"; // 70/1 message 13
                case 14: return "Some memebers are not qualified"; // 70/1 message 14
                case 16: return "Attacker and defender can't team up with each other."; // 70/1 message 16
                case 17: return "Different guilds cannot team up"; // 70/1 message 17
                case 18: return "If players level haven't reached 10, they can't participate in"; // 70/1 message 18
                case 20: return "Pet's amity is less than 40, cannot join in the fighting"; // 70/1 message 20
                case 25: return "Haven't owned the castle for 2 weeks"; // 70/1 message 25
                case 26: return "Attacker cannot join in the defensive party"; // 70/1 message 26
                case 27: return "One of the defensive people is in our party"; // 70/1 message 27
                case 28: return "Viewing battle, cannot team up."; // 70/1 message 28
                case 29: return "Viewing battle, cannot join in the battle."; // 70/1 message 29
                case 30: return "The item can't be enhanced anymore"; // 70/1 message 30
                case 31: return "The item is enhanced successfully"; // 70/1 message 31
                case 32: return "Item enhancing failed"; // 70/1 message 32
                default: return std::nullopt; // not a box line
            }
        }

        /// @brief Gives the text of a Trojan war message that is a chat line.
        /// @param message The u8 message number of 70/1.
        /// @return The line, or nullopt when the message is not a chat line.
        std::optional<std::string> chatLine(int message) {
            switch (message) {
                case 4: return "5 minutes later, the weekly Trojan War event will begin"; // 70/1 message 4
                case 5: return "Trojan War begins now\xA1I"; // 70/1 message 5
                case 6: return "5 minutes later, the weekly Trojan War event will be end. Try your best to get a good mark"; // 70/1 message 6
                case 7: return "Welcome to come to the next week Trojan War event"; // 70/1 message 7
                case 19: return "Trojan War event begins"; // 70/1 message 19
                case 24: return "Please leave the scene, because the scene will be cleared 5 minutes before the activity starts"; // 70/1 message 24
                default: return std::nullopt; // not a chat line
            }
        }

    }

    void handleAction70Castle(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1 && payload.size() > 2) { // 70/1 with at least one byte after action and sub id
            core::ByteReader reader(payload, 2); // skip the action and sub id bytes
            const int message = reader.u8();
            if (const auto line = boxLine(message)) {
                context.events.onCastleNotice(*line, false);
                return;
            }
            if (const auto line = chatLine(message)) {
                context.events.onCastleNotice(*line, true);
                return;
            }
            if (message == 10) { // 70/1 message 10: new leader of the city
                const int result = reader.u8();
                const std::string guild = reader.shortText();
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onCastleOwner(result == 1 ? guild : std::string());
                if (result == 1) {
                    context.events.onCastleNotice("Congratulations [ " + guild + " ] Become the new leader of the city!", false);
                } else if (result == 2) { // result 2: the castle has no guild
                    context.events.onCastleNotice("No guild here", false);
                } else if (result == 3) { // result 3: the war is suspended
                    context.events.onCastleNotice("This week's Trojan War event is suspended", false);
                }
                return;
            }
            if (message == 23) { // 70/1 message 23: who owns the castle
                const std::string guild = reader.shortText();
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onCastleOwner(guild);
                return;
            }
        }
        switch (id.sub) {
            case 10: // 70/10: Gold Eagle prefix list (u8 castle, u8 count)
            case 11: { // 70/11: Gold Eagle prefix update (u8 count, u8 castle)
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int first = reader.u8();
                const int second = reader.u8();
                const int castle = id.sub == 10 ? first : second;
                const int count = id.sub == 10 ? second : first;
                std::vector<std::pair<int, std::string>> entries;
                for (int i = 0; i < count && !reader.failed(); ++i) {
                    const int slot = reader.u8();
                    entries.emplace_back(slot, reader.shortText());
                }
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onGoldEaglePrefix(castle, id.sub == 10, entries);
                return;
            }
            case 3: // 70/3: Defense NPC prefix list (u8 castle, u8 count)
            case 4: { // 70/4: Defense NPC prefix update (u8 count, u8 castle)
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int first = reader.u8();
                const int second = reader.u8();
                const int castle = id.sub == 3 ? first : second;
                const int count = id.sub == 3 ? second : first;
                std::vector<std::pair<int, std::string>> entries;
                for (int i = 0; i < count && !reader.failed(); ++i) {
                    const int slot = reader.u8();
                    entries.emplace_back(slot, reader.shortText());
                }
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onDefenseNpcPrefix(castle, id.sub == 3, entries);
                return;
            }
            case 5: { // 70/5: Defense NPC prefix removed (u8 castle, u8 slot)
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int castle = reader.u8();
                const int slot = reader.u8();
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onDefenseNpcPrefixRemoved(castle, slot);
                return;
            }
            case 7: // 70/7: Stone Eagle prefix list (u8 castle, u8 count)
            case 8: { // 70/8: Stone Eagle prefix update (u8 count, u8 castle)
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int first = reader.u8();
                const int second = reader.u8();
                const int castle = id.sub == 7 ? first : second;
                const int count = id.sub == 7 ? second : first;
                std::vector<std::pair<int, std::string>> entries;
                for (int i = 0; i < count && !reader.failed(); ++i) {
                    const int slot = reader.u8();
                    entries.emplace_back(slot, reader.shortText());
                }
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onStoneEaglePrefix(castle, id.sub == 7, entries);
                return;
            }
            case 9: { // 70/9: Stone Eagle prefix removed (u8 castle, u8 slot)
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int castle = reader.u8();
                const int slot = reader.u8();
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onStoneEaglePrefixRemoved(castle, slot);
                return;
            }
            case 12: { // 70/12: Gold Eagle prefix removed (u8 castle, u8 slot)
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int castle = reader.u8();
                const int slot = reader.u8();
                if (reader.failed()) {
                    return context.events.onUnhandled(id);
                }
                context.events.onGoldEaglePrefixRemoved(castle, slot);
                return;
            }
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
