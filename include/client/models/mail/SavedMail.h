#ifndef NORTH_ISLAND_CLIENT_MODELS_MAIL_SAVED_MAIL_H
#define NORTH_ISLAND_CLIENT_MODELS_MAIL_SAVED_MAIL_H

#include "core/Bytes.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace client {

    /// @brief A mail kept in the mail box: kind, read flag, sender (or recipient), text and time.
    /// @details Original: TMailRec, 185 bytes in MailData.dat / BKMailData.dat. Kind: 1 friend, 2 guild, 3 system, 4 partner, others common. Text rows are ended by carriage returns.
    struct SavedMail {
        static constexpr std::size_t kRecordSize = 185; // bytes of one TMailRec in MailData.dat
        static constexpr std::size_t kLongestName = 17; // most characters of the sender / recipient name
        static constexpr std::size_t kLongestText = 152; // most characters of the mail text

        int kind = 0;
        bool read = false;
        std::int32_t targetId = 0;
        std::string targetName;
        std::string text;
        double sendTime = 0.0;

        /// @brief Decodes the record at an offset of a file's bytes.
        /// @param data File bytes.
        /// @param offset Offset of the record.
        /// @return The mail.
        static SavedMail decode(const core::Bytes& data, std::size_t offset);

        /// @brief Encodes the mail as a 185-byte record.
        /// @return The record bytes.
        core::Bytes encode() const;
    };

}

#endif
