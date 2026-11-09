#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_READ_FUR_MAIL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_READ_FUR_MAIL_WINDOW_H

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
#include "ui/Window.h"

#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief Window for reading a mail that came from a player by name or was sent to the partner; Previous/Next turn, Delete removes, Reply opens the by-name form.
    /// @details Original: ReadFurMailWindow.xml, TReadFurMailBoxForm. Shows send date, sender name, the partner's picture or the mail box icon, and the text.
    class ReadFurMailWindow {
        public:
            /// @brief Hooks into the mail box and the writing form.
            struct Hooks {
                std::function<int()> mailCount;
                std::function<void(int number)> openMail;
                std::function<void(int number)> deleteMail;
                std::function<bool()> hasMailbox;
                std::function<void(const std::string& name, core::Point position)> replyTo;
                std::function<std::optional<FurMailHandover>()> takeOverWriter;
                std::function<void()> turned;
                std::function<void(core::Point)> hidden;
            };

            /// @brief Loads ReadFurMailWindow.xml and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param friends Friend list.
            /// @param guild Guild.
            /// @param marriage Marriage state, for the partner's name and picture.
            /// @param messages Short player message line.
            /// @param hooks Hooks into the mail box and the writing form.
            ReadFurMailWindow(
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

            /// @brief Gives the number of the mail shown in the box.
            /// @details Original: FHumans[2].
            /// @return Mail number, 1-based.
            int mailNumber() const { return m_number; }

            /// @brief Shows a mail; an invitation's text loses its mark and gets the card, and the by-name form gives its place.
            /// @details Original: ShowFurMailRecord and ShowCtrl.
            /// @param number Mail number in the box, 1-based.
            /// @param mail The mail.
            void show(int number, const SavedMail& mail);

            /// @brief Hides the window and tells where it was.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Moves the window.
            /// @param position New position.
            void setPosition(core::Point position) { m_window.setPosition(position); }

            /// @brief Hides this form for the writing form and tells where it was.
            /// @details Original: ShowCtrl of TFurMailBoxForm.
            /// @return Mail box state and position, or nothing when it was not open.
            std::optional<FurMailHandover> takeOver();

        private:
            /// @brief Opens the previous or next mail.
            /// @details Original: PrevFurMail / NextFurMail.
            /// @param step -1 previous, 1 next.
            void turn(int step);

            /// @brief Opens the writing form for the sender; anyone not a friend, guild member or partner needs a mail box.
            /// @details Original: ReplyFurMail.
            void reply();

            /// @brief Deletes this mail and opens the one taking its number, or the last.
            /// @details Original: DeleteFurMail.
            void deleteMail();

            /// @brief Paints date, sender, picture, and the Previous/Next arrows where there is no mail to turn to.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Origin of the painted area.
            void paint(gfx::Canvas& canvas, core::Point at);

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
            ui::Memo& m_text;
            const ui::Spot& m_dateSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_mailboxSpot;
            const ui::Spot& m_partnerSpot;
            const ui::Spot& m_invitationSpot;
            const ui::Spot& m_textColorSpot;
            int m_number = 0;
            std::string m_senderName;
            std::string m_date;
            bool m_invitation = false;
            bool m_atMailbox = false;
    };

}

#endif
