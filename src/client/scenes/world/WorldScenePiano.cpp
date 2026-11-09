#include "client/scenes/world/WorldScene.h"

#include "client/rules/FurnitureKinds.h"
#include "client/rules/PianoSeat.h"
#include "client/rules/CharacterActions.h"
#include "protocol/HouseRequests.h"

namespace client {

    namespace {

        constexpr int kSeatNoticeMs = 1500; // message display time, ms

    }

    bool WorldScene::pieceInUse(int furnIndex) const {
        if (m_player.usedUtensil == furnIndex) {
            return true;
        }
        for (const auto& [id, other] : m_others) {
            if (other.usedUtensil == furnIndex) {
                return true;
            }
        }
        return false;
    }

    void WorldScene::usePiano(int furnIndex, bool owner) {
        if (!owner) {
            if (m_pianoSeated) {
                m_services.music.stop();
                m_piano.show();
            }
            if (m_player.riding) {
                m_messages.show("You can't use piano when you are riding pet.", kSeatNoticeMs);
                return;
            }
        }
        if (!PianoSeat::isFree(m_house.furniture(), m_services.items, furnIndex)) {
            m_messages.show("Not enough seats", kSeatNoticeMs);
            return;
        }
        if (m_player.riding || m_teams.isMember(m_player.id) || m_player.walking || pieceInUse(furnIndex)) {
            return;
        }
        if (useUtensil(m_player, furnIndex, 1)) { // 1: seat number to sit on
            m_pianoSeated = true;
            m_services.music.stop();
            m_services.connection.send(protocol::pianoSitRequest(furnIndex));
            m_piano.show();
        }
    }

    void WorldScene::closeHouseForms() {
        if (m_pianoSeated) {
            m_pianoSeated = false;
            m_piano.close();
        }
        m_piano.hide();
        m_fishTank.hide();
        m_fishAnnotate.hide();
        m_player.usedUtensil = 0;
    }

}
