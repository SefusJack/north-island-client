#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_FUR_MAIL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_FUR_MAIL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/viewmodels/mail/FurMailHandover.h"
#include "client/models/mail/SavedMail.h"
#include "client/models/social/Marriage.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief Window for writing mail to a player by name; sends friends' and guild members' letters through the send form and the rest by name.
    /// @details Original: FurMailWindow.xml, TFurMailBoxForm. At a mail box (furniture or server 14/16 3) anyone can be written to; elsewhere only a friend, guild member or the partner. Name mail is sent as 14/6 with a copy kept; the server answers 14/16 1 (name exists, has a mail box). The mail box's Sent tab shows a kept copy read-only.
    class FurMailWindow {
        public:
            /// @brief Hooks into the rest of the mail system and the world.
            struct Hooks {
                std::function<void()> openMailBox;
                std::function<void(core::Point)> hidden;
                std::function<void(bool)> emotesToMail;
                std::function<void()> hideSendMail;
                std::function<std::optional<FurMailHandover>()> takeOverReader;
                std::function<void(const SavedMail&)> keepSent;
                std::function<void(std::int32_t friendId, const std::string& text)> sendToFriend;
                std::function<void(std::int32_t memberId, const std::string& text)> sendToGuildMember;
                std::function<std::int32_t()> ownId;
            };

            /// @brief Loads FurMailWindow.xml and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param friends Friend list, to recognise friends by name.
            /// @param guild Guild, to recognise members by name.
            /// @param marriage Marriage state, for the partner's name and picture.
            /// @param messages Short player message line.
            /// @param hooks Hooks into the rest of the mail system.
            FurMailWindow(
                GameServices& services,
                ui::Screen& screen,
                const FriendList& friends,
                const Guild& guild,
                const Marriage& marriage,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Tells whether the server said the player has a mail box.
            /// @details Original: FurMailBoxForm.FHasMailbox.
            /// @return True when a mail box is at hand.
            bool hasMailbox() const { return m_hasMailbox; }

            /// @brief Opens the form from the mail box furniture of a house.
            /// @details Original: ShowFurMailBox with 0.
            void show();

            /// @brief Opens the form standing at a mail box.
            /// @details Original: OpenFurMailBox, server 14/16 3.
            void openAtMailbox();

            /// @brief Opens the form as a reply: the name filled in, placed where the read form was.
            /// @param name Recipient name.
            /// @param position Window position.
            void openTo(const std::string& name, core::Point position);

            /// @brief Shows a sent copy read-only.
            /// @details Original: OpenSavedMail on the Sent tab.
            /// @param mail The kept copy.
            void openSent(const SavedMail& mail);

            /// @brief Hides this form for the reader and tells where it was.
            /// @details Original: ShowCtrl of TReadFurMailBoxForm.
            /// @return Mail box state and position, or nothing when it was not open.
            std::optional<FurMailHandover> takeOver();

            /// @brief Handles the server's answer to a by-name send (14/16/1, u8 code).
            /// @details Original: RecvFurMailSendResult.
            /// @param code 1 name does not exist, 2 mail box needed, 3 has a mail box, 4 has none.
            void result(int code);

            /// @brief Hides the window and tells where it was.
            /// @details Original: HideCtrl.
            void hide();

            /// @brief Moves the window.
            /// @param position New position.
            void setPosition(core::Point position) { m_window.setPosition(position); }

        private:
            /// @brief Checks the mail and sends it to a friend, guild member or by name (14/6).
            /// @details Original: SendFurMail.
            void send();

            /// @brief Opens the mail box and closes this form.
            /// @details Original: ClearFurMailText, ShowMailSaveBoxFromFurMail.
            void showMailBox();

            /// @brief Paints the partner's picture for the partner's name, else the mail box's icon.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Origin of the painted area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Makes the form editable, or read-only for a saved mail (no Send, Clear or Old Mail).
            /// @details Original: DrawControl.
            /// @param editable False for a saved mail.
            void setEditable(bool editable);

            /// @brief Gives the partner's name.
            /// @return The partner's name, or empty when unmarried.
            std::string partnerName() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const FriendList& m_friends;
            const Guild& m_guild;
            const Marriage& m_marriage;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_name;
            ui::Memo& m_text;
            const ui::Spot& m_mailboxSpot;
            const ui::Spot& m_partnerSpot;
            bool m_atMailbox = false;
            bool m_hasMailbox = false;
            bool m_fromSent = false;
            std::string m_savedText;
    };

}

#endif
