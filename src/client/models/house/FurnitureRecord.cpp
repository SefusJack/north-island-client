#include "client/models/house/FurnitureRecord.h"

namespace client {

    namespace {

        // byte offsets inside the 57-byte record (item u16 @0, cell x/y/z i32 @2/@6/@10, rotation u8 @14, parent u16 @15)
        constexpr std::size_t kRoomOffset = 47; // room number, u8
        constexpr std::size_t kRetainedOffset = 48; // retained flag, u8
        constexpr std::size_t kUsedCountOffset = 49; // times used, u8
        constexpr std::size_t kLinksOffset = 17; // 11 linked-piece numbers, u16 each

    }

    FurnitureRecord FurnitureRecord::read(core::ByteReader& reader) {
        FurnitureRecord record;
        const core::Bytes bytes = reader.bytes(kSize);
        if (reader.failed()) {
            return record;
        }
        core::ByteReader fields(bytes);
        record.itemId = fields.u16();
        record.cellX = fields.i32();
        record.cellY = fields.i32();
        record.cellZ = fields.i32();
        record.rotation = fields.u8();
        record.parent = fields.u16();
        fields.seek(kLinksOffset);
        for (int& link : record.links) {
            link = fields.u16();
        }
        fields.seek(kRoomOffset);
        record.room = fields.u8();
        fields.seek(kRetainedOffset);
        record.retained = fields.u8();
        fields.seek(kUsedCountOffset);
        record.usedCount = fields.u8();
        return record;
    }

}
