#include "protocol/ActivityLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 2;
        constexpr std::size_t kMostActivities = 30; // at most 30 events in the schedule list

    }

    std::optional<ActivityList> decodeActivityList(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        ActivityList list;
        list.weekDay = reader.u8();
        list.version = reader.i32();
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() > 1 && list.activities.size() < kMostActivities) {
            ActivityRecord record;
            reader.u8();
            record.weekDay = reader.u8();
            record.name = reader.shortText();
            record.time = reader.f64();
            record.duration = reader.u16();
            record.mapId = reader.u16();
            record.mapX = reader.u16();
            record.mapY = reader.u16();
            record.minLevel = reader.u8();
            record.state = reader.u8();
            record.description = reader.shortText();
            if (reader.failed()) {
                break;
            }
            list.activities.push_back(std::move(record));
        }
        return list;
    }

    std::optional<ActivityStates> decodeActivityStates(const core::Bytes& payload) {
        if (payload.size() <= kHeader) {
            return std::nullopt;
        }
        core::ByteReader reader(payload, kHeader);
        ActivityStates states;
        if (payload[kHeader] == 0) {
            reader.u8();
            states.newDay = true;
            states.weekDay = reader.u8();
        }
        while (reader.remaining() >= 2) { // a pair is u8 number + u8 state
            const int number = reader.u8();
            const int state = reader.u8();
            states.states.emplace_back(number, state);
        }
        return states;
    }

}
