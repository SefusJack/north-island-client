#ifndef NORTH_ISLAND_FORMATS_TALK_TABLE_H
#define NORTH_ISLAND_FORMATS_TALK_TABLE_H

#include "core/Bytes.h"

#include <map>
#include <optional>
#include <string>

namespace formats {

    /// @brief The talk lines of Data/Talk.Dat.
    /// @details Original: TFTalk; 292-byte records after a header record whose id must be 3. A record is a u16 id stored
    /// (v xor 60650) - 5, then a string[254] stored reversed. A "#P n /#P" in a line becomes the number n.
    class TalkTable {
        public:
            /// @brief Decodes Talk.Dat.
            /// @param data The raw file bytes.
            /// @return The table, or nothing when the file is too short or the header id is not 3.
            static std::optional<TalkTable> parse(const core::Bytes& data);

            /// @brief The text of a talk.
            /// @details Original: TalkTable.FTalks[GetTalkIndex(id)].Text.
            /// @param talkId The talk id.
            /// @return The line, or an empty string for an id not there.
            const std::string& text(int talkId) const;

            /// @brief Replaces a talk line from a text pack (the qol client's mods); "#P n /#P" becomes n as in the file.
            /// @param talkId The talk id.
            /// @param text The new line.
            /// @return Whether the id is in the table.
            bool setText(int talkId, const std::string& text);

        private:
            std::map<int, std::string> m_talks;
    };

}

#endif
