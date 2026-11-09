#include "formats/MotionTable.h"

#include "core/ByteReader.h"

#include <cstring>

namespace formats {

    namespace {

        constexpr std::size_t kIndexEntry = 10; // bytes per index entry: u16 motion id + u32 offset + u32 size

        /// @brief Reads a point stored as two i32 (x, y).
        /// @param reader Source positioned at the point.
        /// @return The point.
        core::Point readPoint(core::ByteReader& reader) {
            const std::int32_t x = reader.i32();
            const std::int32_t y = reader.i32();
            return {x, y};
        }

        /// @brief Reads a little-endian f64.
        /// @param reader Source positioned at the value.
        /// @return The decoded double.
        double readDouble(core::ByteReader& reader) {
            const std::uint32_t low = reader.u32();
            const std::uint32_t high = reader.u32();
            const std::uint64_t bits = (static_cast<std::uint64_t>(high) << 32) | low; // high word shift (bits)
            double value = 0;
            std::memcpy(&value, &bits, sizeof value);
            return value;
        }

        /// @brief Reads a little-endian f32 (a Delphi Single).
        /// @param reader Source positioned at the value.
        /// @return The decoded float.
        float readSingle(core::ByteReader& reader) {
            const std::uint32_t bits = reader.u32();
            float value = 0;
            std::memcpy(&value, &bits, sizeof value);
            return value;
        }

        /// @brief Reads one motion record.
        /// @details Original: LoadMotionSkillRecords.
        /// @param data Whole file bytes.
        /// @param at Offset of the record.
        /// @param size Size of the record in bytes.
        /// @return The record, or nothing when it is out of range or truncated.
        std::optional<MotionRecord> readRecord(const core::Bytes& data, std::size_t at, std::size_t size) {
            if (at + size > data.size()) {
                return std::nullopt;
            }
            const core::Bytes bytes(data.begin() + static_cast<std::ptrdiff_t>(at), data.begin() + static_cast<std::ptrdiff_t>(at + size));
            core::ByteReader reader(bytes);
            MotionRecord record;
            record.fileWord = reader.u16();
            record.fileByte = reader.u8();
            record.moveKind1 = reader.u8();
            record.moveKind2 = reader.u8();
            record.fileInt = reader.i32();
            record.jumpScale1 = reader.i32();
            record.jumpScale2 = reader.i32();
            record.fileByte2 = reader.u8();
            record.hitMotionId = reader.u16();
            record.hitTime = reader.i32();
            record.hitMode = reader.u8();
            const int hitCount = reader.u8();
            for (int i = 0; i < hitCount; ++i) {
                const int time = reader.i32();
                const int percent = reader.u8();
                record.hits.emplace_back(time, percent);
            }
            const int frameCount = reader.u8();
            for (int i = 0; i < frameCount; ++i) {
                MotionFrame frame;
                frame.slot = reader.u8();
                frame.hold = reader.u8();
                frame.slope = readDouble(reader);
                frame.durationMs = reader.i32();
                record.frames.push_back(frame);
            }
            record.pathPointCount = reader.u8();
            for (int i = 0; i < frameCount; ++i) {
                record.pathPoints.push_back(readPoint(reader));
            }
            const int layerCount = reader.u8();
            for (int i = 0; i < layerCount; ++i) {
                MotionLayer layer;
                layer.effectId = reader.i32();
                const int pointCount = reader.u8();
                for (int j = 0; j < pointCount; ++j) {
                    layer.points.push_back(readPoint(reader));
                }
                layer.slope = readDouble(reader);
                layer.frameDelayMs = reader.i32();
                layer.frameCount = reader.u8();
                layer.startFrame = reader.u8();
                layer.endFrame = reader.u8();
                layer.value = reader.u8();
                layer.startDelay = reader.i32();
                layer.playMode = reader.u8();
                layer.moveMode = reader.u8();
                layer.repeatCount = reader.u8();
                layer.arcDirection = reader.u8();
                layer.direction = reader.u8();
                layer.arcHeight = readSingle(reader);
                record.layers.push_back(layer);
            }
            for (int i = 0; i <= layerCount; ++i) {
                record.drawOrder.push_back(reader.u8());
            }
            const int soundCount = reader.u8();
            for (int i = 0; i < soundCount; ++i) {
                std::array<std::uint8_t, 7> sound{}; // a sound entry is 7 bytes
                for (std::uint8_t& byte : sound) {
                    byte = reader.u8();
                }
                record.sounds.push_back(sound);
            }
            record.cameraMode = reader.u8();
            const int cameraCount = reader.u8();
            for (int i = 0; i < cameraCount; ++i) {
                MotionCameraStep step;
                step.point = readPoint(reader);
                step.speed = readDouble(reader);
                step.until = reader.i32();
                step.focus = reader.u8();
                record.camera.push_back(step);
            }
            record.groundMode = reader.u8();
            const int groundCount = reader.u8();
            for (int i = 0; i < groundCount; ++i) {
                record.groundPoints.push_back(readPoint(reader));
            }
            const int segmentCount = reader.u8();
            for (int i = 0; i < segmentCount; ++i) {
                const int from = reader.u8();
                const int to = reader.u8();
                record.groundSegments.emplace_back(from, to);
            }
            const int effectCount = reader.u8();
            for (int i = 0; i < effectCount; ++i) {
                MotionScreenEffect effect;
                effect.kind = reader.u8();
                effect.start = reader.i32();
                effect.end = reader.i32();
                record.screenEffects.push_back(effect);
            }
            for (auto& row : record.weaponSounds) {
                for (int& sound : row) {
                    sound = reader.u16();
                }
            }
            for (int& delay : record.weaponSoundDelays) {
                delay = reader.i32();
            }
            for (int& byte : record.fileBytes) {
                byte = reader.u8();
            }
            for (int& weapon : record.usableWeapons) {
                weapon = reader.u8();
            }
            for (auto& row : record.bodySounds) {
                for (int& sound : row) {
                    sound = reader.u16();
                }
            }
            for (int& delay : record.bodySoundDelays) {
                delay = reader.i32();
            }
            for (int& byte : record.fileBytes2) {
                byte = reader.u8();
            }
            if (reader.failed()) {
                return std::nullopt;
            }
            return record;
        }

    }

    std::optional<MotionTable> MotionTable::parse(const core::Bytes& data) {
        // 2: size of the trailing u16 entry count
        if (data.size() < 2) {
            return std::nullopt;
        }
        core::ByteReader countReader(data, data.size() - 2);
        const std::size_t count = countReader.u16();
        if (data.size() < 2 + count * kIndexEntry) {
            return std::nullopt;
        }
        MotionTable table;
        core::ByteReader index(data, data.size() - 2 - count * kIndexEntry);
        for (std::size_t i = 0; i < count; ++i) {
            const int id = index.u16();
            const std::size_t at = index.u32();
            const std::size_t size = index.u32();
            if (auto record = readRecord(data, at, size)) {
                record->id = id;
                table.m_records.emplace(id, std::move(*record));
            }
        }
        return table;
    }

    std::optional<std::reference_wrapper<const MotionRecord>> MotionTable::find(int id) const {
        const auto found = m_records.find(id);
        if (found == m_records.end()) {
            return std::nullopt;
        }
        return std::cref(found->second);
    }

}
