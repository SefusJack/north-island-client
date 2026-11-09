#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_MATE_VITALS_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_MATE_VITALS_H

#include "formats/FormulaData.h"

#include <cstdint>

namespace client {

    /// @brief What the server's 8/3 tells of a teammate for the team bar; the maxima are computed here as the group window does.
    /// @details Original: TWorldPlayer FHp, FSp, FCon, FWis, FMaxHpBonus, FMaxSpBonus, FMaxHpExtra, FMaxSpExtra, FMaxHpExtra2; CalcMaxHp, CalcMaxSp of the total level.
    class MateVitals {
        public:
            /// @brief Takes in one attribute of packet 8/3 (original TWorldPlayer.SetWorldAttr).
            /// @details 25 HP (0 up to the largest integer), 26 SP, 29 CON, 33 WIS, 207 and 208 the maxima of the worn items, 48 and 49 the extras, 55 the third HP extra.
            /// @param attribute Attribute number.
            /// @param value Attribute value.
            /// @return False for any other attribute.
            bool set(int attribute, std::int32_t value);

            /// @brief Gets the current HP.
            /// @return The HP.
            std::int32_t hp() const { return m_hp; }

            /// @brief Gets the current SP.
            /// @return The SP.
            int sp() const { return m_sp; }

            /// @brief Computes the most HP (original CalcMaxHp).
            /// @param formula Formula data.
            /// @param totalLevel Total level of the teammate.
            /// @return The most HP.
            int maxHp(const formats::FormulaData& formula, int totalLevel) const;

            /// @brief Computes the most SP (original CalcMaxSp).
            /// @param formula Formula data.
            /// @param totalLevel Total level of the teammate.
            /// @return The most SP.
            int maxSp(const formats::FormulaData& formula, int totalLevel) const;

        private:
            std::int32_t m_hp = 0;
            int m_sp = 0;
            int m_con = 0;
            int m_wis = 0;
            int m_maxHpBonus = 0;
            int m_maxSpBonus = 0;
            int m_maxHpExtra = 0;
            int m_maxSpExtra = 0;
            int m_maxHpExtra2 = 0;
    };

}

#endif
