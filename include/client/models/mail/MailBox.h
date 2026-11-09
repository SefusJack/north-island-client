#ifndef NORTH_ISLAND_CLIENT_MODELS_MAIL_MAIL_BOX_H
#define NORTH_ISLAND_CLIENT_MODELS_MAIL_MAIL_BOX_H

#include "client/models/social/FriendList.h"
#include "client/models/mail/SavedMail.h"

#include <filesystem>
#include <vector>

namespace client {

    /// @brief The mail kept on this computer: mail received and copies of mail sent, each newest first, at most 999, numbered from 1.
    /// @details Original: TMailSaveBoxForm's FMails and FBKMails, kept in MailData.dat and BKMailData.dat of the character's folder.
    class MailBox {
        public:
            /// @brief Which of the two mail lists.
            enum class Box { Received, Sent };

            /// @brief Creates an empty mail box.
            /// @param friends Friend list giving names for mail without one.
            explicit MailBox(
                const FriendList& friends
            )
                : m_friends(friends) {}

            /// @brief The mail of one list, newest first.
            /// @param box Which list.
            /// @return The mails.
            const std::vector<SavedMail>& mails(Box box) const { return box == Box::Received ? m_received : m_sent; }

            /// @brief Number of mails in a list.
            /// @param box Which list.
            /// @return The count.
            int count(Box box) const { return static_cast<int>(mails(box).size()); }

            /// @brief Received mail not read yet.
            /// @details Original: FSavedCount.
            /// @return The count.
            int unread() const { return m_unread; }

            /// @brief Whether the mail changed since the files were written.
            /// @details Original: FLogsImported.
            /// @return True when changed.
            bool isDirty() const { return m_dirty; }

            /// @brief Adds a mail: the GM center's name, a friend's name when it has none; the oldest mail makes room when 999 are kept.
            /// @details Original: AddSavedMail / AddBKSavedMail.
            /// @param box Which list.
            /// @param mail The mail.
            /// @param fromFile True when read from the files (does not make the box dirty).
            /// @return False when the same mail is kept already.
            bool add(Box box, SavedMail mail, bool fromFile = false);

            /// @brief Deletes a mail.
            /// @details Original: DeleteSavedMail / DeleteBKSavedMail.
            /// @param box Which list.
            /// @param number Mail number 1..count.
            /// @return True when deleted.
            bool remove(Box box, int number);

            /// @brief Marks a received mail as read.
            /// @details Original: OpenSavedMail.
            /// @param number Mail number 1..count.
            void markRead(int number);

            /// @brief Empties both lists.
            /// @details Original: ResetMailSaveBox.
            void clear();

            /// @brief Loads both lists from the files of a folder.
            /// @details Original: LoadMailSaveBoxFile.
            /// @param folder The character's folder.
            void load(const std::filesystem::path& folder);

            /// @brief Saves both lists to the files of a folder.
            /// @details Original: SaveMailSaveBoxFile.
            /// @param folder The character's folder.
            void save(const std::filesystem::path& folder);

        private:
            /// @brief Writable access to one list.
            /// @param box Which list.
            /// @return The list.
            std::vector<SavedMail>& list(Box box) { return box == Box::Received ? m_received : m_sent; }

            const FriendList& m_friends;
            std::vector<SavedMail> m_received;
            std::vector<SavedMail> m_sent;
            int m_unread = 0;
            bool m_dirty = false;
    };

}

#endif
