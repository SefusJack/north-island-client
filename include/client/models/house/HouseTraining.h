#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_TRAINING_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_TRAINING_H

#include <cstdint>

namespace client {

    /// @brief What the Treadmill (39105) and Bicep Exerciser (39100) show while the owner trains: the piece he is at, the percent bar, and the Bicep's moving-part phase.
    /// @details Original: DZ_Thing.pas DrawExhibitItem's Bicep branch and DZ_Human.pas DrawSceneObjects' Treadmill branch.
    class HouseTraining {
        public:
            static constexpr int kPhases = 19; // phases of the Bicep's moving part (0..18)
            static constexpr std::uint64_t kStepMs = 5; // ms between phase steps

            /// @brief Sets the piece the owner is at.
            /// @details Original: the owner's FUsedUtensil.
            /// @param furnIndex The piece number, 0 for none.
            void setOwnerUsed(int furnIndex) { m_ownerUsed = furnIndex; }

            /// @brief The piece the owner is at.
            /// @return The piece number, 0 for none.
            int ownerUsed() const { return m_ownerUsed; }

            /// @brief Sets the percent of the bar over the piece, for the owner himself.
            /// @param percent The percent, -1 for none.
            void setBar(int percent) { m_bar = percent; }

            /// @brief The percent of the bar over the piece.
            /// @return The percent, -1 when a visitor sees none.
            int bar() const { return m_bar; }

            /// @brief Advances the Bicep's moving parts one step when enough time has passed; at rest when not in use.
            /// @details Original: DrawExhibitItem (picture index 16, flag 68, one step a draw once 5 ms have passed).
            /// @param nowMs The current time in milliseconds.
            /// @param inUse True while the owner is at a Bicep Exerciser.
            void step(std::uint64_t nowMs, bool inUse) {
                if (!inUse) {
                    m_phase = 0;
                    return;
                }
                if (nowMs - m_tickMs < kStepMs) {
                    return;
                }
                m_tickMs = nowMs;
                m_phase += m_back ? -1 : 1;
                if (m_phase > kPhases - 1) {
                    m_back = true;
                }
                if (m_phase <= 0) {
                    m_back = false;
                }
            }

            /// @brief The current phase of the moving parts.
            /// @return Phase 0..18.
            int phase() const { return m_phase; }

        private:
            int m_ownerUsed = 0;
            int m_bar = -1;
            int m_phase = 0;
            bool m_back = false;
            std::uint64_t m_tickMs = 0;
    };

}

#endif
