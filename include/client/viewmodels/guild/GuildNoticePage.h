#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_NOTICE_PAGE_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_NOTICE_PAGE_H

#include "client/services/GameServices.h"
#include "client/models/events/NoticeBoard.h"
#include "client/models/guild/Guild.h"
#include "client/viewmodels/events/NoticeReadForm.h"
#include "core/Point.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

namespace client {

    /// @brief The Message page of the guild tab: the guild's notice board, five notices a page.
    /// @details Original: DrawOrgNoticePage, DrawOrgNoticeList (controls in GuildWindow.xml).
    class GuildNoticePage {
        public:
            /// @brief Wires the page's controls.
            /// @param services Shared game services.
            /// @param screen Screen holding the controls.
            /// @param guild The guild model, used to find the authors.
            /// @param board The guild notice board model.
            /// @param read Form that shows a picked notice.
            GuildNoticePage(
                GameServices& services,
                ui::Screen& screen,
                const Guild& guild,
                NoticeBoard& board,
                NoticeReadForm& read
            );

            /// @brief Asks for the first page of notices.
            /// @details Original: ShowOrgNoticePage.
            void show() { m_board.askFirstPage(); }

            /// @brief Shows or hides the page's buttons.
            /// @param shown True when the page is the current tab.
            void layout(bool shown);

            /// @brief Paints the authors' faces, the texts and the hover bar.
            /// @param canvas Target canvas.
            /// @param at Top left of the window on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Opens the notice under the mouse in the read form.
            /// @details Original: HandleDblClick.
            /// @param windowAt Top left of the window.
            void doubleClicked(core::Point windowAt);

        private:
            GameServices& m_services;
            ui::Screen& m_screen;
            const Guild& m_guild;
            NoticeBoard& m_board;
            NoticeReadForm& m_read;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_senderSpot;
            const ui::Spot& m_subjectSpot;
            const ui::Spot& m_dateSpot;
            const ui::Spot& m_pageSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_barSpot;
    };

}

#endif
