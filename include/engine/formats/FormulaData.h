#ifndef NORTH_ISLAND_FORMATS_FORMULA_DATA_H
#define NORTH_ISLAND_FORMATS_FORMULA_DATA_H

#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <optional>

namespace formats {

    /// @brief Growth constants of data\Formula.dat (combat factors, exp power, HP/SP constants).
    /// @details Original: data\Formula.dat, one 407-byte record (first byte 2); 0x01 holds 20 f64 combat factors,
    /// 0xF1 the exp power and HP/SP constants (f64), 0x169 i32 exp addend, 0x16D u16 HP base, 0x16F u16 SP base.
    struct FormulaData {
        std::array<double, 20> combatFactors{}; // 20 f64 values at record offset 0x01
        double expPower = 0;
        double hpPerLevel = 0;
        double hpStatScale = 0;
        double hpLevelPower = 0;
        double hpPerStat = 0;
        double spPerLevel = 0;
        double spStatScale = 0;
        double spLevelPower = 0;
        double spPerStat = 0;
        std::int32_t expAdd = 0;
        std::uint16_t hpBase = 0;
        std::uint16_t spBase = 0;

        /// @brief Parses a Formula.dat record.
        /// @param data Raw file bytes.
        /// @return The constants, or nothing when the record is too short or its first byte is not 2.
        static std::optional<FormulaData> parse(const core::Bytes& data);
    };

}

#endif
