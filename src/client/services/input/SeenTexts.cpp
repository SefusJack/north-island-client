#include "client/services/input/SeenTexts.h"

#include <algorithm>

namespace client {

    void SeenTexts::add(const std::string& text) {
        m_texts.push_back(text);
        ++m_count;
        if (m_texts.size() > kKept) {
            m_texts.pop_front();
        }
    }

    bool SeenTexts::containsSince(std::uint64_t mark, const std::string& text) const {
        const std::uint64_t firstKept = m_count - m_texts.size();
        for (std::uint64_t number = std::max(mark, firstKept); number < m_count; ++number) {
            if (m_texts[static_cast<std::size_t>(number - firstKept)].find(text) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

}
