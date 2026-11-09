#include "client/viewmodels/hud/MainButtons.h"

namespace client {

    void MainButtons::alertItem() {
        if (m_alerting) {
            clearItemAlert();
        }
        m_item.setBounds({m_itemBounds.x + m_alertSpot.at().x, m_itemBounds.y + m_alertSpot.at().y, m_alertSpot.size().width, m_alertSpot.size().height});
        m_item.setAnimating(true);
        m_item.setRectHit(true);
        m_alerting = true;
    }

    void MainButtons::clearItemAlert() {
        if (!m_alerting) {
            return;
        }
        m_item.setBounds(m_itemBounds);
        m_item.setAnimating(false);
        m_item.setRectHit(false);
        m_alerting = false;
    }

}
