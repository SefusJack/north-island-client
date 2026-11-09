#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_READ_MAIL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_READ_MAIL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/models/mail/SavedMail.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief Window that shows one saved mail with sender, date, text and turn/delete/reply buttons.
    /// @details Original: ReadMailWindow.xml, TSe_ReadMailForm.
    class ReadMailWindow {
        public:
            /// @brief Callbacks into the mail box, send forms and world state.
            struct Hooks {
                std::function<int()> mailCount;
                std::function<void(int number)> openMail;
                std::function<void(int number)> deleteMail;
                std::function<void(std::int32_t friendId, core::Point position)> reply;
                std::function<void(std::int32_t memberId, core::Point position)> replyGuild;
                std::function<std::optional<CharacterLook>(std::int32_t)> worldLook;
                std::function<void()> turned;
                std::function<void(core::Point)> hidden;
                std::function<void(const std::string& name, core::Point position)> replyByName;
            };

            /// @brief Loads the window layout and wires its buttons.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param friends Friend list used to resolve the sender.
            /// @param guild Guild used to resolve the sender of guild mail.
            /// @param hooks Callbacks into the surrounding forms.
            ReadMailWindow(
                GameServices& services,
                ui::Screen& screen,
                const FriendList& friends,
                const Guild& guild,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Number of the mail shown in the box.
            /// @details Original: FSavedMailIndex.
            /// @return The mail number.
            int mailNumber() const { return m_number; }

            /// @brief Shows a mail and opens the window.
            /// @details Original: ShowMailRecord and ShowCtrl.
            /// @param number Number of the mail in the box.
            /// @param mail The mail to show.
            void show(int number, const SavedMail& mail);

            /// @brief Hides the window and reports its position.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Moves the window.
            /// @param position New top-left position.
            void setPosition(core::Point position) { m_window.setPosition(position); }

        private:
            /// @brief Opens the neighbouring mail of the box.
            /// @details Original: PrevMail / NextMail.
            /// @param step -1 for previous, 1 for next.
            void turn(int step);

            /// @brief Opens the reply form for the sender.
            /// @details Original: ReplyMail.
            void reply();

            /// @brief Deletes the shown mail and shows the one taking its place.
            /// @details Original: DeleteMail.
            void deleteMail();

            /// @brief Paints the window's drawing area: sender, invitation card and disabled arrows.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the date, sender name and face.
            /// @details Original: DrawMailSenderPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paintSender(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the name and face of a guild mail sender.
            /// @details Original: DrawGuildMailSenderPreview.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paintGuildSender(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            const FriendList& m_friends;
            const Guild& m_guild;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Memo& m_text;
            const ui::Spot& m_dateSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_invitationSpot;
            const ui::Spot& m_textColorSpot;
            int m_number = 0;
            std::int32_t m_senderId = 0;
            std::string m_senderName;
            std::string m_date;
            bool m_invitation = false;
            bool m_guildMail = false;
            PortraitBlink m_blink;
    };

}

#endif
