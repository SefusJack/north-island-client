#include "client/models/house/HouseBoxes.h"

#include "core/ByteReader.h"

namespace client {

    namespace {

        constexpr int kLockerOption = 7;
        constexpr int kFourthBoxOption = 9;

        /// @brief The box an option of 62/57 is about.
        /// @details Original: server action 62/57 options 1/2 Fridge, 3/4 Material Cabinet, 5/6 Potion Cabinet, 7/8 Locker, 9/10 Medicine Case.
        /// @param option The option byte of 62/57 (1..10).
        /// @return The box number 1..5.
        int boxOfOption(int option) {
            if (option >= kFourthBoxOption) {
                return 4; // box 4: the Medicine Case
            }
            if (option >= kLockerOption) {
                return HouseBoxes::kLocker;
            }
            return (option + 1) / 2;
        }

    }

    bool HouseBoxes::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the first 2 payload bytes (action and sub ids)
        const int option = reader.u8();
        const int place = reader.u8();
        if (reader.failed() || option < 1 || option > 2 * kBoxes) {
            return false;
        }
        const int box = boxOfOption(option);
        if (place < 1 || place > placesOf(box)) {
            return false;
        }
        Place& slot = m_places[static_cast<std::size_t>(box)][static_cast<std::size_t>(place)];
        if (option % 2 == 0) {
            slot = {};
            return true;
        }
        const int itemId = reader.u16();
        const int count = reader.u8();
        if (reader.failed()) {
            return false;
        }
        slot = {itemId, count};
        return true;
    }

    const HouseBoxes::Place& HouseBoxes::place(int box, int place) const {
        static const Place none;
        if (box < 1 || box > kBoxes || place < 1 || place > placesOf(box)) {
            return none;
        }
        return m_places[static_cast<std::size_t>(box)][static_cast<std::size_t>(place)];
    }

    int HouseBoxes::firstFree(int box) const {
        for (int at = 1; at <= placesOf(box); ++at) {
            if (place(box, at).itemId == 0) {
                return at;
            }
        }
        return 0;
    }

}
