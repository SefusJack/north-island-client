#ifndef NORTH_ISLAND_CLIENT_MODELS_MAIL_MAIL_LOG_IMPORTER_H
#define NORTH_ISLAND_CLIENT_MODELS_MAIL_MAIL_LOG_IMPORTER_H

#include "client/models/mail/MailBox.h"

#include <filesystem>

namespace client {

    /// @brief Folds the message logs of an older client into the mail box and deletes them.
    /// @details Original: ImportMessageLogs (HL_Menu.pas). Friend logs (MsgFriendLog<player xor 121>, kind 1) and guild logs (MsgArmyLog..., kind 2) hold 165-byte records (the sender, the text, the time), mail box logs (MsgMailBoxLog..., kind 3) 183-byte records with the sender's name too.
    /// Every *.Dat of the folder is read (UnReadMsg.dat marks its mails unread), the folder goes when it is empty, and the mails are added newest first until 1000 are kept.
    class MailLogImporter {
        public:
            /// @brief Kind of log to import.
            enum class Kind { Friend = 1, Guild = 2, MailBox = 3 };

            /// @brief Imports the logs of one kind into the mail box.
            /// @param box Mail box to add to.
            /// @param serverFolder user/Server<id> of the character's server.
            /// @param playerKey Player key that forms the log folder name.
            /// @param kind Kind of log.
            /// @return True when a mail was added.
            static bool import(MailBox& box, const std::filesystem::path& serverFolder, std::int32_t playerKey, Kind kind);
    };

}

#endif
