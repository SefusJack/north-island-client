#include "client/models/social/MateVitals.h"

#include "client/rules/Vitality.h"

#include <algorithm>

namespace client {

    bool MateVitals::set(int attribute, std::int32_t value) {
        switch (attribute) {
            case 25: // 8/3 attribute 25: HP
                m_hp = std::max(value, 0);
                return true;
            case 26: // 8/3 attribute 26: SP
                m_sp = value;
                return true;
            case 29: // 8/3 attribute 29: CON
                m_con = value;
                return true;
            case 33: // 8/3 attribute 33: WIS
                m_wis = value;
                return true;
            case 207: // 8/3 attribute 207: most HP from worn items
                m_maxHpBonus = value;
                return true;
            case 208: // 8/3 attribute 208: most SP from worn items
                m_maxSpBonus = value;
                return true;
            case 48: // 8/3 attribute 48: most HP extra
                m_maxHpExtra = value;
                return true;
            case 49: // 8/3 attribute 49: most SP extra
                m_maxSpExtra = value;
                return true;
            case 55: // 8/3 attribute 55: third most HP extra
                m_maxHpExtra2 = value;
                return true;
            default: // other attributes are not kept
                return false;
        }
    }

    int MateVitals::maxHp(const formats::FormulaData& formula, int totalLevel) const {
        return client::maxHp(formula, totalLevel, m_con, m_maxHpBonus + m_maxHpExtra + m_maxHpExtra2);
    }

    int MateVitals::maxSp(const formats::FormulaData& formula, int totalLevel) const {
        return client::maxSp(formula, totalLevel, m_wis, m_maxSpBonus + m_maxSpExtra);
    }

}
