#include "protocol/FriendLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        /// @brief Reads one friend record (i32 id, pstr8 name, u8 level, job, reborn, element, body type, head, i32 colour code 1, i32 colour code 5, pstr8 nickname, optional pstr8 guild).
        /// @param reader Reader positioned at the record.
        /// @param withGuild Whether a guild name follows the nickname.
        /// @return The friend.
        FriendRecord readRecord(core::ByteReader& reader, bool withGuild) {
            FriendRecord record;
            record.id = reader.i32();
            record.name = reader.shortText();
            record.level = reader.u8();
            record.job = reader.u8();
            record.reborn = reader.u8();
            record.element = reader.u8();
            record.bodyType = reader.u8();
            record.head = reader.u8();
            record.colorCode1 = reader.i32();
            record.colorCode5 = reader.i32();
            record.nickname = reader.shortText();
            if (withGuild) {
                record.guild = reader.shortText();
            }
            return record;
        }

        /// @brief Decodes a payload holding a single friend record.
        /// @param payload Packet payload.
        /// @param withGuild Whether the record carries a guild name.
        /// @return The friend, or nothing when malformed.
        std::optional<FriendRecord> decodeOne(const core::Bytes& payload, bool withGuild) {
            core::ByteReader reader(payload, 2); // skip the action and sub bytes
            FriendRecord record = readRecord(reader, withGuild);
            if (reader.failed()) {
                return std::nullopt;
            }
            return record;
        }

    }

    std::vector<FriendRecord> decodeFriendList(const core::Bytes& payload) {
        std::vector<FriendRecord> friends;
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        while (!reader.atEnd()) {
            FriendRecord record = readRecord(reader, true);
            if (reader.failed()) {
                break;
            }
            friends.push_back(std::move(record));
        }
        return friends;
    }

    std::optional<FriendRecord> decodeFriendInfo(const core::Bytes& payload) { return decodeOne(payload, true); }

    std::optional<FriendRecord> decodeFriendEntry(const core::Bytes& payload) { return decodeOne(payload, false); }

    std::vector<FriendState> decodeFriendStates(const core::Bytes& payload) {
        std::vector<FriendState> states;
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        while (reader.remaining() >= 5) { // a record is i32 friend + u8 state = 5 bytes
            FriendState state;
            state.id = reader.i32();
            state.state = reader.u8();
            states.push_back(state);
        }
        return states;
    }

    std::vector<BlacklistEntry> decodeBlacklist(const core::Bytes& payload) {
        core::ByteReader reader(payload, 3); // skip the action, sub and result bytes
        std::vector<BlacklistEntry> entries;
        while (reader.remaining() > 0) {
            BlacklistEntry entry;
            entry.playerId = reader.i32();
            entry.name = reader.shortText();
            if (reader.failed()) {
                break;
            }
            entries.push_back(entry);
        }
        return entries;
    }

}
