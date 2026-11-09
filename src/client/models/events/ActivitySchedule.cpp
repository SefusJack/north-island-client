#include "client/models/events/ActivitySchedule.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "core/FileSystem.h"

#include <algorithm>

namespace client {

    namespace {

        // record size: weekday 1, name 21, time 8, minutes/map/x/y 2 each, level 1, description 255
        constexpr std::size_t kRecordSize = 1 + 21 + 8 + 2 + 2 + 2 + 2 + 1 + 255;
        constexpr std::size_t kNameSize = 20; // name text capacity, bytes (field is 21 with the length byte)
        constexpr std::size_t kDescriptionSize = 254; // description text capacity, bytes (field is 255 with the length byte)
        constexpr int kOver = 3; // activity state 3: over (1 preparing, 2 blinking)

        /// @brief Writes a Pascal short string padded with zeros to a fixed size.
        /// @param writer The output.
        /// @param text The text, cut to size.
        /// @param size Capacity of the string field.
        void writeShortString(core::ByteWriter& writer, const std::string& text, std::size_t size) {
            const std::string kept = text.substr(0, size);
            writer.u8(static_cast<std::uint8_t>(kept.size())).text(kept).text(std::string(size - kept.size(), '\0'));
        }

        /// @brief Reads a Pascal short string of a fixed field size.
        /// @param reader The input.
        /// @param size Capacity of the string field.
        /// @return The text.
        std::string readShortString(core::ByteReader& reader, std::size_t size) {
            const std::size_t length = std::min<std::size_t>(reader.u8(), size);
            std::string text = reader.text(length);
            reader.skip(size - length);
            return text;
        }

    }

    void ActivitySchedule::replace(const protocol::ActivityList& list) {
        m_weekDay = list.weekDay;
        m_version = list.version;
        m_activities = list.activities;
        if (m_activities.size() > static_cast<std::size_t>(kMost)) {
            m_activities.resize(kMost);
        }
    }

    bool ActivitySchedule::applyStates(const protocol::ActivityStates& states) {
        if (states.newDay) {
            m_weekDay = states.weekDay;
            resetStates();
        }
        int last = 0;
        for (const auto& [number, state] : states.states) {
            if (number >= 1 && number <= count()) {
                m_activities[static_cast<std::size_t>(number - 1)].state = state;
            }
            last = number;
        }
        return last >= 1 && last <= count() && at(last).state == 2; // state 2: the blinking state
    }

    void ActivitySchedule::resetStates() {
        for (protocol::ActivityRecord& activity : m_activities) {
            activity.state = 1; // state 1: preparing
        }
    }

    std::vector<int> ActivitySchedule::listed() const {
        std::vector<int> numbers;
        for (int number = 1; number <= count(); ++number) {
            const protocol::ActivityRecord& activity = at(number);
            if (activity.state != kOver && (activity.weekDay == 0 || activity.weekDay == m_weekDay)) {
                numbers.push_back(number);
            }
        }
        return numbers;
    }

    void ActivitySchedule::load(const std::filesystem::path& file) {
        const auto data = core::readFile(file);
        if (!data || data->size() < kRecordSize) {
            return;
        }
        const std::size_t records = data->size() / kRecordSize;
        core::ByteReader reader(*data);
        // skip the weekday (1) and the name (21); the time field is 8 bytes
        reader.skip(1 + 21);
        m_version = static_cast<std::int32_t>(reader.f64());
        reader.skip(kRecordSize - (1 + 21 + 8));
        m_activities.clear();
        for (std::size_t i = 1; i < records && i <= static_cast<std::size_t>(kMost); ++i) {
            protocol::ActivityRecord record;
            record.weekDay = reader.u8();
            record.name = readShortString(reader, kNameSize);
            record.time = reader.f64();
            record.duration = reader.u16();
            record.mapId = reader.u16();
            record.mapX = reader.u16();
            record.mapY = reader.u16();
            record.minLevel = reader.u8();
            record.description = readShortString(reader, kDescriptionSize);
            m_activities.push_back(std::move(record));
        }
    }

    void ActivitySchedule::save(const std::filesystem::path& file) const {
        core::ByteWriter writer;
        writer.u8(0);
        writeShortString(writer, "", kNameSize);
        writer.f64(static_cast<double>(m_version));
        writer.text(std::string(kRecordSize - (1 + 21 + 8), '\0')); // pad the rest after weekday 1, name 21, time 8
        for (const protocol::ActivityRecord& record : m_activities) {
            writer.u8(static_cast<std::uint8_t>(record.weekDay));
            writeShortString(writer, record.name, kNameSize);
            writer.f64(record.time).u16(static_cast<std::uint16_t>(record.duration)).u16(static_cast<std::uint16_t>(record.mapId));
            writer.u16(static_cast<std::uint16_t>(record.mapX)).u16(static_cast<std::uint16_t>(record.mapY));
            writer.u8(static_cast<std::uint8_t>(record.minLevel));
            writeShortString(writer, record.description, kDescriptionSize);
        }
        core::writeFile(file, writer.take());
    }

}
