#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_MAIL_BOX_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_MAIL_BOX_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/FriendList.h"
#include "client/viewmodels/mail/FurMailHandover.h"
#include "client/models/mail/MailBox.h"
#include "client/models/mail/MailLogImporter.h"
#include "client/models/social/Marriage.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/mail/ReadFurMailWindow.h"
#include "client/viewmodels/mail/ReadMailWindow.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The mail box: received mail and copies of mail sent, ten a page, kept in the character's folder; new mail makes the input bar's envelope blink.
    /// @details Original: MailBoxWindow.xml, TMailSaveBoxForm. Received mail is read in ReadMailWindow, or ReadFurMailWindow when it came by a name.
    class MailBoxWindow {
        public:
            /// @brief Hooks into the rest of the client.
            struct Hooks {
                std::function<bool(std::int32_t, const std::string&)> blocked;
                std::function<std::string(std::int32_t)> worldName;
                std::function<void(const std::string&)> chatLine;
                std::function<void(std::int32_t friendId, core::Point position)> reply;
                std::function<void(std::int32_t memberId, core::Point position)> replyGuild;
                std::function<std::optional<CharacterLook>(std::int32_t)> worldLook;
                std::function<void(core::Point)> mailFormHidden;
                std::function<void(const std::string& name, core::Point position)> replyByName;
                std::function<bool()> hasMailbox;
                std::function<std::optional<FurMailHandover>()> takeOverWriter;
                std::function<void(const SavedMail&)> openSent;
                std::function<void()> toggleWriter;
            };

            /// @brief Loads MailBoxWindow.xml and builds the read forms.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param friends Friend list.
            /// @param guild Guild.
            /// @param marriage Marriage state.
            /// @param messages Short player message line.
            /// @param hooks Hooks into the rest of the client.
            MailBoxWindow(
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

            /// @brief Opens the window (the envelope was clicked).
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Closes the window; the files are written when the box changed.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Resets the box and loads it from a folder on entering the world.
            /// @details Original: ResetMailSaveBox and LoadMailSaveBoxFile; folder user/Server<server>/MailBoxMsgLog<player xor 121>.
            /// @param folder Character's mail folder.
            void load(const std::filesystem::path& folder);

            /// @brief Saves the box on leaving the world.
            /// @details Original: SaveMailSaveBoxFile.
            void save();

            /// @brief Imports the message logs of an older client into the box.
            /// @details Original: ImportMessageLogs; done on the friend list, the guild info and entering the world; logs lie in user/Server<id>.
            /// @param serverFolder The server's user folder.
            /// @param playerKey Key of the player in the log names.
            /// @param kind Which kind of log to import.
            void importLogs(const std::filesystem::path& serverFolder, std::int32_t playerKey, MailLogImporter::Kind kind);

            /// @brief Adds a received mail and makes the envelope blink.
            /// @details Original: AddReceivedMail, from server 14/1, 39/5 and 14/16/2; kind 1 friend, 2 guild, 0 by name.
            /// @param senderId Sender's id.
            /// @param sendTime Send time as a date-time number.
            /// @param text Mail text.
            /// @param kind 1 friend, 2 guild member, 0 by name.
            /// @param name Sender's name when the server gives one.
            void receive(
                std::int32_t senderId,
                double sendTime,
                const std::string& text,
                int kind = 1,
                const std::string& name = {}
            );

            /// @brief Keeps a copy of a mail sent.
            /// @details Original: AddBKSavedMail.
            /// @param mail The copy.
            void keepSent(const SavedMail& mail);

            /// @brief Tells whether the input bar's envelope should blink.
            /// @details Original: StartNewMailBlink / StopNewMailBlink.
            /// @return True while there is unread mail.
            bool envelopeBlinks() const { return m_blink; }

            /// @brief Puts both read forms at a position.
            /// @details Original: SetMailFormsX / SetMailFormsY.
            /// @param position New window position.
            void placeReadForm(core::Point position) {
                m_reader.setPosition(position);
                m_furReader.setPosition(position);
            }

            /// @brief Makes the by-name reader go for the writing form and tells where it was.
            /// @details Original: ShowCtrl of TFurMailBoxForm.
            /// @return Mail box state and position, or nothing when it was not open.
            std::optional<FurMailHandover> takeOverFurReader() { return m_furReader.takeOver(); }

        private:
            /// @brief Gives the tab's mail list.
            /// @return Received on tab 0, else Sent.
            MailBox::Box box() const { return m_tab == 0 ? MailBox::Box::Received : MailBox::Box::Sent; }

            /// @brief Counts the pages of the current tab.
            /// @return Page count.
            int pageCount() const;

            /// @brief Counts the rows on the current page.
            /// @return Row count.
            int rowsOnPage() const;

            /// @brief Finds the row with mail under a point of the rows' area.
            /// @details Original: FHoverRow.
            /// @param local Point relative to the rows' area.
            /// @return Row 1..10, or 0 for none.
            int rowAt(core::Point local) const;

            /// @brief Adds a mail to a box, moving the choice down with the rows.
            /// @param which Received or Sent.
            /// @param mail The mail.
            void add(MailBox::Box which, SavedMail mail);

            /// @brief Switches to a tab.
            /// @param tab 0 received, 1 sent.
            void setTab(int tab);

            /// @brief Opens a mail from the list.
            /// @details Original: OpenSavedMail.
            /// @param number Mail number, 1-based.
            void openMail(int number);

            /// @brief Deletes a mail and says so.
            /// @details Original: DeleteSavedMail / DeleteBKSavedMail.
            /// @param which Received or Sent.
            /// @param number Mail number, 1-based.
            /// @return True when deleted.
            bool deleteMail(MailBox::Box which, int number);

            /// @brief Deletes the chosen mail of this page.
            void deleteChosen();

            /// @brief Turns the page.
            /// @param step Positive for next, else previous.
            void turnPage(int step);

            /// @brief Paints the page numbers, the rows and the hover and chosen bars.
            /// @param canvas Target canvas.
            /// @param at Origin of the painted area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_titleMaskSpot;
            const ui::Spot& m_pageSpot;
            const ui::Spot& m_fullPageSpot;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_hoverBarSpot;
            const ui::Spot& m_chosenBarSpot;
            const ui::Spot& m_kindSpot;
            const ui::Spot& m_startSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_dateSpot;
            const ui::Spot& m_unreadSpot;
            const ui::Spot& m_readSpot;
            MailBox m_box;
            ReadMailWindow m_reader;
            ReadFurMailWindow m_furReader;
            std::filesystem::path m_folder;
            int m_tab = 0;
            int m_page = 1;
            int m_chosen = 0;
            int m_pressedRow = 0;
            bool m_blink = false;
    };

}

#endif
