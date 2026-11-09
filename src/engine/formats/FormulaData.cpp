#include "formats/FormulaData.h"

#include "core/ByteReader.h"

namespace formats {

    std::optional<FormulaData> FormulaData::parse(const core::Bytes& data) {
        if (data.size() < 407 || data[0] != 2) { // record size 407 bytes; first byte (record tag) must be 2
            return std::nullopt;
        }
        core::ByteReader reader(data, 1);
        FormulaData formula;
        for (double& factor : formula.combatFactors) {
            factor = reader.f64();
        }
        reader.seek(0xF1); // offset of the exp power f64
        formula.expPower = reader.f64();
        formula.hpPerLevel = reader.f64();
        formula.hpStatScale = reader.f64();
        formula.hpLevelPower = reader.f64();
        formula.hpPerStat = reader.f64();
        formula.spPerLevel = reader.f64();
        formula.spStatScale = reader.f64();
        formula.spLevelPower = reader.f64();
        formula.spPerStat = reader.f64();
        reader.seek(0x169); // offset of the exp addend i32, HP base u16, SP base u16
        formula.expAdd = reader.i32();
        formula.hpBase = reader.u16();
        formula.spBase = reader.u16();
        return formula;
    }

}
