#include "client/models/mail/MailLogImporter.h"

#include "core/ByteReader.h"
#include "core/FileSystem.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <system_error>
#include <vector>

namespace client {

    namespace {

        constexpr std::size_t kFriendRecord = 165; // bytes per record of a friend / guild log
        constexpr std::size_t kMailBoxRecord = 183; // bytes per record of a mail box log (adds the sender's name)
        constexpr std::size_t kLongestName = 17; // name field capacity, characters
        constexpr std::size_t kLongestText = 152; // text field capacity, characters
        constexpr int kRoom = 1000; // most mails kept per box
        constexpr int kSystemKind = 3; // mail kind 3: system mail (shown as from the GM center)
        const char* const kUnreadFile = "UnReadMsg.dat";

        /// @brief Lower-cases a string.
        /// @param text Text to convert.
        /// @return The lower-case text.
        std::string lower(std::string text) {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return text;
        }

        /// @brief Reads a Delphi short string of a fixed size (length byte plus room for size characters).
        /// @param reader Reader positioned at the length byte.
        /// @param size Capacity of the string field.
        /// @return The text.
        std::string shortString(core::ByteReader& reader, std::size_t size) {
            const std::size_t length = std::min<std::size_t>(reader.u8(), size);
            std::string text = reader.text(length);
            reader.skip(size - length);
            return text;
        }

        /// @brief Lists the *.Dat files of a folder in name order.
        /// @param folder Folder to scan.
        /// @return The file paths.
        std::vector<std::filesystem::path> logFiles(const std::filesystem::path& folder) {
            std::vector<std::filesystem::path> files;
            std::error_code error;
            for (std::filesystem::directory_iterator it(folder, error), end; !error && it != end; it.increment(error)) {
                if (lower(it->path().extension().string()) == ".dat") {
                    files.push_back(it->path());
                }
            }
            std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) { return lower(a.filename().string()) < lower(b.filename().string()); });
            return files;
        }

        /// @brief Reads the records of every log file of a folder (a record seen twice is kept once), deletes the files, removes the folder when nothing is left.
        /// @param folder Log folder.
        /// @param withName True for mail box logs, whose records carry the sender's name.
        /// @return The mails read.
        std::vector<SavedMail> readLogs(const std::filesystem::path& folder, bool withName) {
            std::vector<SavedMail> mails;
            std::error_code error;
            if (!std::filesystem::is_directory(folder, error)) {
                return mails;
            }
            const std::size_t size = withName ? kMailBoxRecord : kFriendRecord;
            for (const std::filesystem::path& file : logFiles(folder)) {
                if (const auto data = core::readFile(file)) {
                    const bool unread = lower(file.filename().string()) == lower(kUnreadFile);
                    for (std::size_t offset = 0; offset + size <= data->size(); offset += size) {
                        core::ByteReader reader(*data, offset);
                        SavedMail mail;
                        mail.read = !unread;
                        mail.targetId = reader.i32();
                        if (withName) {
                            mail.targetName = shortString(reader, kLongestName);
                        }
                        mail.text = shortString(reader, kLongestText);
                        mail.sendTime = reader.f64();
                        const bool duplicate = std::any_of(mails.begin(), mails.end(), [&](const SavedMail& other) {
                            return other.sendTime == mail.sendTime && other.targetId == mail.targetId && other.text == mail.text &&
                                   (!withName || other.targetName == mail.text);
                        });
                        if (!duplicate) {
                            mails.push_back(mail);
                        }
                    }
                }
                std::filesystem::remove(file, error);
            }
            if (logFiles(folder).empty()) {
                std::filesystem::remove(folder, error);
            }
            return mails;
        }

    }

    bool MailLogImporter::import(MailBox& box, const std::filesystem::path& serverFolder, std::int32_t playerKey, Kind kind) {
        const char* const prefix = kind == Kind::Friend ? "MsgFriendLog" : kind == Kind::Guild ? "MsgArmyLog" : "MsgMailBoxLog";
        std::vector<SavedMail> mails = readLogs(serverFolder / (prefix + std::to_string(playerKey)), kind == Kind::MailBox);
        std::vector<std::size_t> order(mails.size());
        for (std::size_t i = 0; i < order.size(); ++i) {
            order[i] = i;
        }
        // 86400 seconds per day (a TDateTime is in days); 0.0005 is a rounding epsilon
        const auto second = [&](std::size_t i) { return std::floor(mails[i].sendTime * 86400.0 + 0.0005); };
        std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return second(a) < second(b); });
        bool added = false;
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            if (box.count(MailBox::Box::Received) >= kRoom) {
                break;
            }
            SavedMail mail = mails[*it];
            // kind 3 (mail box log) becomes mail kind 0 (common)
            mail.kind = static_cast<int>(kind) == 3 ? 0 : static_cast<int>(kind);
            // target ids 100 and 101 are the GM center
            if (mail.targetId == 100 || mail.targetId == 101) {
                mail.kind = kSystemKind;
            }
            added = box.add(MailBox::Box::Received, mail) || added;
        }
        return added;
    }

}
