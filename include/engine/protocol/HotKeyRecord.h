#ifndef NORTH_ISLAND_PROTOCOL_HOT_KEY_RECORD_H
#define NORTH_ISLAND_PROTOCOL_HOT_KEY_RECORD_H

namespace protocol {

    /// @brief One place of the F1-F8 bar as the server keeps it (40/1): u8 kind (1 item, 2 skill, 0 empty), u16 id, u8 page 1..3, u8 place 1..8.
    struct HotKeyRecord {
        int kind = 0;
        int id = 0;
        int page = 0;
        int slot = 0;
    };

}

#endif
