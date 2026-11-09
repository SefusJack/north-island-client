#include "client/models/mail/MailBox.h"

#include "core/FileSystem.h"

#include <algorithm>
#include <system_error>

namespace client {

    namespace {

        constexpr int kRoom = 1000; // most mails kept per box
        constexpr int kSystemKind = 3; // mail kind 3: system mail (shown as from the GM center)
        const char* const kReceivedFile = "MailData.dat";
        const char* const kSentFile = "BKMailData.dat";

        /// @brief Tells whether two mails are the same one.
        /// @param a First mail.
        /// @param b Second mail.
        /// @return True when time, target, text, name and kind match.
        bool sameMail(const SavedMail& a, const SavedMail& b) {
            return a.sendTime == b.sendTime && a.targetId == b.targetId && a.text == b.text && a.targetName == b.targetName && a.kind == b.kind;
        }

        /// @brief Reads the fixed-size mail records of a file.
        /// @param file File to read.
        /// @return The mails; none for a missing file.
        std::vector<SavedMail> readMails(const std::filesystem::path& file) {
            std::vector<SavedMail> mails;
            if (const auto data = core::readFile(file)) {
                for (std::size_t offset = 0; offset + SavedMail::kRecordSize <= data->size(); offset += SavedMail::kRecordSize) {
                    mails.push_back(SavedMail::decode(*data, offset));
                }
            }
            return mails;
        }

        /// @brief Rewrites a mail file; no file is left when there is no mail.
        /// @param folder Folder of the file.
        /// @param name File name.
        /// @param mails Mails to write.
        void writeMails(const std::filesystem::path& folder, const char* name, const std::vector<SavedMail>& mails) {
            std::error_code error;
            std::filesystem::remove(folder / name, error);
            if (mails.empty()) {
                return;
            }
            std::filesystem::create_directories(folder, error);
            core::Bytes data;
            for (const SavedMail& mail : mails) {
                const core::Bytes record = mail.encode();
                data.insert(data.end(), record.begin(), record.end());
            }
            core::writeFile(folder / name, data);
        }

    }

    bool MailBox::add(Box box, SavedMail mail, bool fromFile) {
        // target ids 100 and 101 are the GM center
        if (mail.targetId == 100 || mail.targetId == 101) {
            mail.targetName = "GM center";
            mail.kind = kSystemKind;
        }
        if (mail.targetName.empty()) {
            if (const int index = m_friends.indexOf(mail.targetId)) {
                mail.targetName = m_friends.at(index).record.name;
            }
        }
        mail.targetName = mail.targetName.substr(0, SavedMail::kLongestName);
        mail.text = mail.text.substr(0, SavedMail::kLongestText);
        if (box == Box::Sent) {
            mail.read = true;
        }
        std::vector<SavedMail>& mails = list(box);
        std::size_t position = 0;
        for (std::size_t i = 0; i < mails.size(); ++i) {
            if (sameMail(mails[i], mail)) {
                return false;
            }
            if (mails[i].sendTime > mail.sendTime) {
                position = i + 1;
            }
        }
        if (count(box) + 1 >= kRoom) {
            remove(box, count(box));
            position = std::min(position, mails.size());
        }
        if (box == Box::Received && !mail.read) {
            ++m_unread;
        }
        mails.insert(mails.begin() + static_cast<std::ptrdiff_t>(position), std::move(mail));
        m_dirty = m_dirty || !fromFile;
        return true;
    }

    bool MailBox::remove(Box box, int number) {
        std::vector<SavedMail>& mails = list(box);
        if (number < 1 || number > count(box)) {
            return false;
        }
        const auto at = mails.begin() + (number - 1);
        if (box == Box::Received && !at->read && m_unread > 0) {
            --m_unread;
        }
        mails.erase(at);
        m_dirty = true;
        return true;
    }

    void MailBox::markRead(int number) {
        if (number < 1 || number > count(Box::Received)) {
            return;
        }
        SavedMail& mail = m_received[static_cast<std::size_t>(number - 1)];
        if (!mail.read) {
            mail.read = true;
            if (m_unread > 0) {
                --m_unread;
            }
            m_dirty = true;
        }
    }

    void MailBox::clear() {
        m_received.clear();
        m_sent.clear();
        m_unread = 0;
        m_dirty = false;
    }

    void MailBox::load(const std::filesystem::path& folder) {
        for (SavedMail& mail : readMails(folder / kReceivedFile)) {
            add(Box::Received, std::move(mail), true);
        }
        for (SavedMail& mail : readMails(folder / kSentFile)) {
            add(Box::Sent, std::move(mail), true);
        }
    }

    void MailBox::save(const std::filesystem::path& folder) {
        writeMails(folder, kReceivedFile, m_received);
        writeMails(folder, kSentFile, m_sent);
        m_dirty = false;
    }

}
