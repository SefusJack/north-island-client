#include "client/models/guild/GoldEaglePrefixes.h"

namespace client {

    namespace {

        constexpr int kFirstTrojanMap = 10065; // map 10065: Trojan Round 1
        constexpr int kSecondTrojanMap = 10066; // map 10066: Trojan Round 2
        constexpr int kThirdTrojanMap = 10068; // map 10068: Trojan Round 3
        constexpr std::size_t kMostChars = 17; // longest prefix text kept, bytes

    }

    void GoldEaglePrefixes::list(int castle, const std::vector<Entry>& entries) {
        if (!validCastle(castle)) {
            return;
        }
        m_prefixes[static_cast<std::size_t>(castle - 1)].clear();
        update(castle, entries);
    }

    void GoldEaglePrefixes::update(int castle, const std::vector<Entry>& entries) {
        if (!validCastle(castle)) {
            return;
        }
        for (const auto& [slot, text] : entries) {
            if (slot == 1) {
                m_prefixes[static_cast<std::size_t>(castle - 1)] = text.substr(0, kMostChars);
            }
        }
    }

    void GoldEaglePrefixes::remove(int castle, int slot) {
        if (validCastle(castle) && slot == 1) {
            m_prefixes[static_cast<std::size_t>(castle - 1)].clear();
        }
    }

    std::string GoldEaglePrefixes::prefix(int mapId, int ordinal) const {
        int castle = 0;
        switch (mapId) {
            case kFirstTrojanMap: castle = 1; break; // first Trojan War map: castle 1
            case kSecondTrojanMap: castle = 2; break; // second Trojan War map: castle 2
            case kThirdTrojanMap: castle = 3; break; // third Trojan War map: castle 3
            default: return {}; // not a Trojan War map: no prefix
        }
        const std::string& text = m_prefixes[static_cast<std::size_t>(castle - 1)];
        return !text.empty() && ordinal == 1 ? text : std::string();
    }

}
