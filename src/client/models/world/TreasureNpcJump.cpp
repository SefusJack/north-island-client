#include "client/models/world/TreasureNpcJump.h"

namespace client {

    namespace {

        constexpr std::uint64_t kStepMs = 30; // time per jump step, ms
        constexpr int kRisingSteps = 5; // steps going up before the fall starts
        constexpr int kPixelsPerStep = 10; // px moved per step (per step number when falling)

    }

    void TreasureNpcJump::start(int npcIndex, std::uint64_t nowMs) {
        if (m_active) {
            return;
        }
        m_active = true;
        m_npcIndex = npcIndex;
        m_step = 0;
        m_offsetY = 0;
        m_stepMs = nowMs;
    }

    std::optional<TreasureNpcJump::Step> TreasureNpcJump::update(std::uint64_t nowMs) {
        if (!m_active || nowMs - m_stepMs < kStepMs) {
            return std::nullopt;
        }
        m_stepMs = nowMs;
        Step step;
        step.npcIndex = m_npcIndex;
        ++m_step;
        if (m_step <= kRisingSteps) {
            m_offsetY = m_step * -kPixelsPerStep;
        } else if (m_offsetY + (m_step - kRisingSteps) * kPixelsPerStep > 0) {
            m_offsetY = 0;
            step.landed = true;
            m_active = false;
        } else {
            m_offsetY += (m_step - kRisingSteps) * kPixelsPerStep;
        }
        step.offsetY = m_offsetY;
        return step;
    }

}
