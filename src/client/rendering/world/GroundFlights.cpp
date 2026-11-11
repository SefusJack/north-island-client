#include "client/rendering/world/GroundFlights.h"

namespace client {

    namespace {

        constexpr double kFlightSpeed = 0.24;

    }

    void GroundFlights::start(
        std::vector<Flight>& flights,
        int thing,
        gfx::PictureId picture,
        core::Point from,
        core::Point to,
        std::uint64_t nowMs
    ) {
        LightEffect::Setup setup;
        setup.picture = picture;
        setup.frameMs = static_cast<std::uint64_t>(thing);
        setup.motion = LightEffect::Motion::Arc;
        setup.loop = false;
        setup.target = to;
        setup.speed = kFlightSpeed;
        Flight& flight = flights.emplace_back();
        flight.thing = thing;
        flight.light.start(setup, from, nowMs);
    }

    void GroundFlights::drop(
        int thing,
        gfx::PictureId picture,
        core::Point from,
        core::Point to,
        std::uint64_t nowMs
    ) {
        start(m_drops, thing, picture, from, to, nowMs);
    }

    void GroundFlights::pickUp(gfx::PictureId picture, core::Point from, core::Point to, std::uint64_t nowMs) {
        start(m_pickups, 10, picture, from, to, nowMs); // ground index given to pickup flights (they do not land)
    }

    std::vector<int> GroundFlights::update(gfx::Canvas& canvas, std::uint64_t nowMs) {
        for (std::size_t i = m_pickups.size(); i-- > 0;) {
            m_pickups[i].light.update(nowMs);
            m_pickups[i].light.draw(canvas, nowMs);
            if (!m_pickups[i].light.isActive()) {
                m_pickups.erase(m_pickups.begin() + static_cast<std::ptrdiff_t>(i));
            }
        }
        std::vector<int> landed;
        for (std::size_t i = m_drops.size(); i-- > 0;) {
            m_drops[i].light.update(nowMs);
            m_drops[i].light.draw(canvas, nowMs);
            if (!m_drops[i].light.isActive()) {
                landed.push_back(m_drops[i].thing);
                m_drops.erase(m_drops.begin() + static_cast<std::ptrdiff_t>(i));
            }
        }
        return landed;
    }

    void GroundFlights::clear() {
        m_pickups.clear();
        m_drops.clear();
    }

}
