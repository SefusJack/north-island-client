#include "protocol/PlayerAppearance.h"

namespace protocol {

    void PlayerAppearance::readEquipment(core::ByteReader& reader) {
        const int count = reader.u8();
        equipment.clear();
        for (int i = 0; i < count; ++i) {
            equipment.push_back(reader.u16());
        }
    }

}
