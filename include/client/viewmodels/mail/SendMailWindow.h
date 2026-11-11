#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_SEND_MAIL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_SEND_MAIL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/models/mail/SavedMail.h"
#include "client/models/social/Marriage.h"
#include "protocol/PuppetPerson.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Window for writing mail to friends, guild members, notice authors, the spouse or a puppet partner.
    /// @details Original: SendMailWindow.xml, TSe_SendMailForm. Send hands the text to the server (14/1).
    /// Notice authors are written to with 82/6 and guild members with 39/4.
    class SendMailWindow {
        public:
            /// @brief Callbacks into the mail box and other forms.
            struct Hooks {
                std::function<void(const SavedMail&)> keepSent;
                std::function<void()> openMailBox;
                std::function<void(core::Point)> hidden;
                std::function<void(bool)> emotesToMail;
                std::function<std::int32_t()> ownId;
            };

            /// @brief Loads the window layout and wires its buttons.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param friends Friend list (names, faces, ticked friends).
            /// @param guild Guild (members, ticked members).
            /// @param marriage Marriage state, for mail to the spouse.
            /// @param hooks Callbacks into the surrounding forms.
            SendMailWindow(
                GameServices& services,
                ui::Screen& screen,
                FriendList& friends,
                Guild& guild,
                const Marriage& marriage,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form to write to one friend.
            /// @details Original: MailFriendAtRow.
            /// @param friendId Id of the friend.
            void openTo(std::int32_t friendId);

            /// @brief Opens the form to write to the ticked friends.
            /// @details Original: MailCheckedFriends.
            void openToChecked();

            /// @brief Opens the form to write to the author of a public notice.
            /// @details Original: MailNoticeAuthor (FMailKind 4).
            /// @param authorId Id of the author.
            /// @param name Author name as the notice list gave it.
            void openToNoticeAuthor(std::int32_t authorId, const std::string& name);

            /// @brief Opens the form to write to one guild member.
            /// @details Original: MailGuildMember and the guild notice's MailNoticeAuthor.
            /// @param memberId Id of the guild member.
            void openToGuildMember(std::int32_t memberId);

            /// @brief Opens the form to write to the ticked guild members.
            /// @details Original: MailCheckedMembers.
            void openToCheckedMembers();

            /// @brief Opens the form to write to the spouse.
            /// @details Original: MailSpouse.
            void openToSpouse();

            /// @brief Opens the form to write to a Memory Card training partner.
            /// @details Original: MailToTrainPartner and MemCardMailClick (FMailKind 6): to the one who plays for the
            /// player or whom he plays for, sent through the Memory Card (23/104 option 13).
            /// @param person The partner to write to.
            void openToPuppet(const protocol::PuppetPerson& person);

            /// @brief Opens the form as a reply to a friend at the read form's position, text focused.
            /// @details Original: ReplyMail.
            /// @param friendId Id of the friend.
            /// @param position Position of the read form.
            void reply(std::int32_t friendId, core::Point position);

            /// @brief Sends a ready-made text to a friend through the normal send path.
            /// @details Original: SendFurMail for a name that is a friend's, going through SendMailClick.
            /// @param friendId Id of the friend.
            /// @param text Mail text.
            void sendToFriendNamed(std::int32_t friendId, const std::string& text) { sendNamed(Kind::Friend, friendId, text); }

            /// @brief Sends a ready-made text to a guild member through the normal send path.
            /// @details Original: SendFurMail for a name that is a guild member's, going through SendMailClick.
            /// @param memberId Id of the guild member.
            /// @param text Mail text.
            void sendToMemberNamed(std::int32_t memberId, const std::string& text) { sendNamed(Kind::Guild, memberId, text); }

            /// @brief Clears the text, hides the window and sends emotes back to the chat line.
            /// @details Original: CloseSendMail.
            void close();

            /// @brief Hides the window and reports its position, keeping the text.
            /// @details Original: HideCtrl.
            void hide();

            /// @brief Moves the window.
            /// @param position New top-left position.
            void setPosition(core::Point position) { m_window.setPosition(position); }

            /// @brief Types an emote code into the mail text.
            /// @details Original: InsertEmoteAtPoint, FEmoteTarget 1.
            /// @param code Emote code.
            void typeEmote(const std::string& code) { m_text.type(code); }

        private:
            /// @brief Kind of mail being written.
            enum class Kind { Friend, Checked, Notice, Guild, GuildChecked, Spouse, Puppet };

            /// @brief Shows the window for a kind and target, focusing the text when it was closed.
            /// @details Original: ShowCtrl.
            /// @param kind Kind of mail.
            /// @param targetId Id of the target player.
            void show(Kind kind, std::int32_t targetId);

            /// @brief Sets kind, target and text, then sends right away.
            /// @param kind Kind of mail.
            /// @param targetId Id of the target player.
            /// @param text Mail text.
            void sendNamed(Kind kind, std::int32_t targetId, const std::string& text);

            /// @brief Sends the mail according to its kind and keeps a copy in the mail box.
            /// @details Original: SendMailClick. Nothing is sent and the text stays when the target is invalid.
            void send();

            /// @brief Sends to one friend still on the list (the GM row counts).
            /// @param copy Saved copy, filled with the target.
            /// @return False when the mail goes nowhere.
            bool sendToFriend(SavedMail& copy);

            /// @brief Sends to the ticked friends and unticks them.
            void sendToChecked();

            /// @brief Sends to the author of a public notice.
            /// @details Original: FMailKind 4, packet 82/6.
            /// @param copy Saved copy, filled with the target.
            /// @return False when the mail goes nowhere.
            bool sendToNoticeAuthor(SavedMail& copy);

            /// @brief Sends to a member of the own guild.
            /// @details Original: FMailKind 1, packet 39/4.
            /// @param copy Saved copy, filled with the target.
            /// @return False when the mail goes nowhere.
            bool sendToGuildMember(SavedMail& copy);

            /// @brief Sends to the spouse.
            /// @details Original: FMailKind 5.
            /// @param copy Saved copy, filled with the target.
            /// @return False when the mail goes nowhere.
            bool sendToSpouse(SavedMail& copy);

            /// @brief Sends to the Memory Card training partner.
            /// @details Original: FMailKind 6, through the card (23/104 option 13).
            /// @param copy Saved copy, filled with the target.
            /// @return False when the mail goes nowhere.
            bool sendToPuppet(SavedMail& copy);

            /// @brief Sends to the ticked guild members and unticks them.
            /// @details Original: FMailKind 3, packet 39/4.
            void sendToCheckedMembers();

            /// @brief Paints the target preview for the current kind.
            /// @details Original: DrawMailTargetPreview, DrawNoticeMailPreview, DrawGuildMailTargetPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the friend's name and face (the GM's own face for a GM).
            /// @details Original: DrawMailTargetPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintFriend(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the notice author's name over the notice board's sign.
            /// @details Original: DrawNoticeMailPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintNotice(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the guild member's name and face.
            /// @details Original: DrawGuildMailTargetPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintGuildMember(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the spouse's name and face.
            /// @details Original: DrawSpouseMailPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintSpouse(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the Memory Card partner's name and face.
            /// @details Original: DrawPuppetMailPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintPuppet(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            FriendList& m_friends;
            Guild& m_guild;
            const Marriage& m_marriage;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Memo& m_text;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_noticeSpot;
            const ui::Spot& m_nameColorSpot;
            Kind m_kind = Kind::Friend;
            std::int32_t m_targetId = 0;
            std::string m_noticeAuthor;
            protocol::PuppetPerson m_puppet;
    };

}

#endif
