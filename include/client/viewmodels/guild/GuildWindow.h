#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/events/NoticeBoard.h"
#include "client/models/guild/Guild.h"
#include "client/viewmodels/guild/GuildEmblemPage.h"
#include "client/viewmodels/guild/GuildNoticePage.h"
#include "client/viewmodels/events/NoticeReadForm.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "core/Point.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The guild tab of the group window (GuildWindow.xml, TSe_TeamForm with its guild tab chosen): info, members, notice board and badge pages.
    class GuildWindow {
        public:
            /// @brief Callbacks into the rest of the client that the guild tab needs.
            struct Hooks {
                std::function<std::int32_t()> ownId;
                std::function<int(std::int32_t)> onlineState;
                std::function<TalkDialog::Speaker()> ownSpeaker;
                std::function<std::string()> ownNickname;
                std::function<int()> ownLevel;
                std::function<int()> ownJob;
                std::function<std::string()> castleOwner;
                std::function<void()> showParty;
                std::function<void()> inviteCursor;
                std::function<void(core::Point)> editRules;
                std::function<void()> toggleMyAuthority;
                std::function<void(int)> toggleAuthority;
                std::function<bool()> authorityOpen;
                std::function<void(std::int32_t)> mailMember;
                std::function<void()> mailChecked;
                std::function<void()> showInstances;
                std::function<std::optional<formats::EmblemBlock>()> ownBadge;
                std::function<void()> castleTeleport;
            };

            /// @brief Loads the window layout, builds the member rows and wires all buttons.
            /// @param services Game services (layouts, pictures, connection).
            /// @param screen Screen the window is loaded into.
            /// @param guild The own guild's data.
            /// @param messages Player message line for notices.
            /// @param talk Dialog used for confirmations.
            /// @param board Notice board shown on the Message page.
            /// @param read Form that reads one notice.
            /// @param hooks Callbacks into the rest of the client.
            GuildWindow(
                GameServices& services,
                ui::Screen& screen,
                Guild& guild,
                PlayerMessages& messages,
                TalkDialog& talk,
                NoticeBoard& board,
                NoticeReadForm& read,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Gives the window's top-left corner.
            /// @return Window position on the screen.
            core::Point position() const { return {m_window.bounds().x, m_window.bounds().y}; }

            /// @brief Opens the window on the page it was left on, at the place the group window had.
            /// @param at Top-left corner to place the window at.
            void open(core::Point at);

            /// @brief Hides the window.
            void close();

            /// @brief Called when the group form was opened; loads the badge samples.
            /// @details Original: TSe_TeamForm.ShowCtrl.
            void formShown() { m_emblem.loadSamples(); }

            /// @brief Called when the own badge changed; the samples are made again.
            /// @details Original: RecvOwnEnsignUpdate.
            void badgeChanged() { m_emblem.loadSamples(); }

            /// @brief Reloads the display after the guild changed (rules text, members).
            void refresh();

            /// @brief Shows the rules text the lord wrote in the rules memo.
            /// @details Original: SetOrgRulesText.
            /// @param rules The rules text.
            void showRules(const std::string& rules);

        private:
            /// @brief The pages of the guild tab.
            enum class Page { Info, Members, Insignia, Message };

            /// @brief The controls of one member row.
            struct Row {
                std::reference_wrapper<ui::TextBox> name;
                std::reference_wrapper<ui::TextBox> nickname;
                std::reference_wrapper<ui::TextBox> rank;
                std::reference_wrapper<ui::DrawingArea> career;
                std::reference_wrapper<ui::ImageButton> mail;
                std::reference_wrapper<ui::Element> mailGray;
                std::reference_wrapper<ui::ImageButton> check;
            };

            /// @brief Switches to a page: its background picture and the pressed tab.
            /// @details Original: ShowGroupInfoPage / ShowGroupMemberPage.
            /// @param page Page to show.
            void showPage(Page page);

            /// @brief Shows or hides the info page controls (rules, End Guild and Edit Rule for the lord, castle button).
            /// @details Original: DrawGuildInfoPage.
            void layoutInfo();

            /// @brief Shows or hides the member page controls and fills the member rows.
            /// @details Original: DrawOrgMemberPage.
            void layoutMembers();

            /// @brief Paints the info page: lord name, member count, badge and castle mark.
            /// @details Original: DrawGuildInfoPage.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paintInfo(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the member page: per row figure, level, element, online state, title label, and the page numbers.
            /// @details Original: DrawOrgMemberPage.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paintMembers(gfx::Canvas& canvas, core::Point at);

            /// @brief Counts the member pages.
            /// @return Number of pages of five members.
            int pageCount() const;

            /// @brief Finds the member shown in a row of the current page.
            /// @param row Row number (1..5).
            /// @return Index into the guild's member list, -1 for none.
            int memberOfRow(int row) const;

            /// @brief Sets the member page buttons for the lord or a plain member (declared, not defined).
            /// @param lord True when the viewer is the guild lord.
            void setButtons(bool lord);

            /// @brief Toggles the tick of a member row (not the own row, not while the authority form is up).
            /// @details Original: ToggleMemberCheck.
            /// @param row Row number (1..5).
            void toggleCheck(int row);

            /// @brief Opens the mail form to the member of a row.
            /// @details Original: MailGuildMember.
            /// @param row Row number (1..5).
            void mailMember(int row);

            /// @brief Opens the mail form to the ticked members; shows a notice when none is ticked.
            /// @details Original: MailCheckedMembers.
            void mailChecked();

            /// @brief Asks whether to leave the guild and sends the leave request on yes.
            /// @details Original: ConfirmLeaveGuild.
            void askLeave();

            /// @brief Asks whether to disband the guild and sends the disband request on yes.
            /// @details Original: ConfirmDisbandGuild.
            void askDisband();

            /// @brief Offers the lordship to the one ticked member who is online.
            /// @details Original: AbdicateGuildLord.
            void abdicate();

            /// @brief Asks to remove the ticked members; several are removed together.
            /// @details Original: FireCheckedMembers.
            void fire();

            /// @brief Sends the request that removes one member from the guild.
            /// @param memberId Id of the member to remove.
            void sendKick(std::int32_t memberId);

            /// @brief Clears the tick of every member.
            void uncheckAll();

            /// @brief Opens the authority form for the ticked members after checking the rights.
            /// @details Original: SetMemberAuthority and ShowMyAuthority.
            void setAuthority();

            GameServices& m_services;
            ui::Screen& m_screen;
            Guild& m_guild;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Memo& m_rules;
            const ui::Spot& m_badgeSpot;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_levelSpot;
            const ui::Spot& m_careerSpot;
            const ui::Spot& m_elementSpot;
            const ui::Spot& m_stateSpot;
            const ui::Spot& m_stateAreaSpot;
            const ui::Spot& m_titleSpot;
            const ui::Spot& m_pageSpot;
            const ui::Spot& m_textSpot;
            std::vector<Row> m_rows;
            Page m_page = Page::Info;
            int m_memberPage = 1;
            std::array<std::reference_wrapper<ui::ImageButton>, 4> m_pageButtons;
            GuildNoticePage m_notices;
            GuildEmblemPage m_emblem;
    };

}

#endif
