#include "client/services/ServerStatusChannel.h"

namespace client {

    void ServerStatusChannel::query(const std::string& address) {
        m_buffer.clear();
        m_socket.connect(address, kPort);
    }

    bool ServerStatusChannel::update() {
        m_socket.poll();
        const core::Bytes received = m_socket.receive();
        if (received.empty()) {
            return false;
        }
        m_buffer.insert(m_buffer.end(), received.begin(), received.end());
        m_statuses.clear();
        std::size_t position = 0;
        // a record is 3 bytes: u16 little-endian server id, then u8 status
        for (; position + 3 <= m_buffer.size(); position += 3) {
            const int id = m_buffer[position] | (m_buffer[position + 1] << 8); // high byte of the u16 id
            const int status = m_buffer[position + 2]; // third byte is the status
            m_statuses[id] = status <= 3 ? status : 0; // valid statuses are 0..3, anything else is 0
        }
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(position));
        return true;
    }

    int ServerStatusChannel::status(int serverId) const {
        const auto found = m_statuses.find(serverId);
        return found == m_statuses.end() ? 0 : found->second;
    }

}
