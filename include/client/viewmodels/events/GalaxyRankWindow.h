#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_GALAXY_RANK_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_GALAXY_RANK_WINDOW_H

#include "client/services/GameServices.h"
#include "core/Bytes.h"
#include "gfx/Canvas.h"
#include "ui/DrawingArea.h"
#include "ui/ScrollBar.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief The galaxy competition's ranking window, one group (an arena and a kind) per page.
    /// @details Original: GalaxyRankWindow.xml, RE_Menu.pas TRE_GalaxyRankForm. 36/9 brings the ranking of one group, 36/10 opens the form on the first group. A group is a page with its title above a list of names and the servers they play on, five rows at a time with a big rank number beside each; the arrows turn from group to group.
    class GalaxyRankWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            GalaxyRankWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Handles a whole 36/9 (store a group's ranking) or 36/10 (open the form) message.
            /// @details Original: 36/9 RecvGalaxyRank and 36/10 ShowCtrl.
            /// @param payload The whole message.
            void received(const core::Bytes& payload);

        private:
            /// @brief One ranking line: server, name and rank.
            struct Entry {
                int server = 0;
                std::string name;
                int rank = 0;
            };

            /// @brief A ranking page of one arena and kind with its entries.
            /// @details Original: TGalaxyRankPageInfo.
            struct Group {
                int arena = 0;
                int kind = 0;
                std::string title;
                std::vector<Entry> entries;
            };

            /// @brief Parses a 36/9 message and stores or replaces its group.
            /// @details Original: RecvGalaxyRank.
            /// @param payload The whole message.
            void store(const core::Bytes& payload);

            /// @brief Builds the title of a page.
            /// @details Original: AddGalaxyRankGroup.
            /// @param arena Arena number (99 to 96 are the four arenas, else a server).
            /// @param kind Kind (1 Group P, 2 Chaos fight).
            /// @return The title text.
            static std::string titleOf(int arena, int kind);

            /// @brief Shows the form on the first group, when there is one.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Shows a group's list from its top.
            /// @details Original: ShowGalaxyRankGroup.
            /// @param number Group number (1 first).
            void showGroup(int number);

            /// @brief Turns to the next group.
            /// @details Original: NextGalaxyRankGroup.
            void nextGroup();

            /// @brief Turns to the previous group.
            /// @details Original: PrevGalaxyRankGroup.
            void previousGroup();

            /// @brief Scrolls the list to a top row (clamped).
            /// @param top Wanted top row.
            void scrollTo(int top);

            /// @brief Finds the row under the mouse in the list.
            /// @param local Mouse position relative to the list.
            /// @return Row index, or -1 for none.
            int rowAt(core::Point local) const;

            /// @brief Paints the title, the group and group count, and the rank numbers.
            /// @details Original: TRE_GalaxyRankForm.DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the form area.
            void paintForm(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the hovered row, the names and the servers.
            /// @details Original: TBK_ImgListBox.DrawControl and TBK_BiImgList.DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the list.
            void paintList(gfx::Canvas& canvas, core::Point at);

            /// @brief Draws a number from the digit picture of a number font.
            /// @details Original: DrawNumber.
            /// @param canvas Target canvas.
            /// @param at Top-left of the first digit.
            /// @param value The number.
            /// @param picture Name of the digit picture.
            /// @param digit Size of one drawn digit.
            void drawNumber(
                gfx::Canvas& canvas,
                core::Point at,
                int value,
                const char* picture,
                core::Size digit
            );

            /// @brief Counts the rows of the current group.
            /// @return Entry count, 0 when no group is shown.
            int rowCount() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::DrawingArea& m_list;
            ui::ScrollBar& m_scroll;
            const ui::Spot& m_titleSpot;
            const ui::Spot& m_pageSpot;
            const ui::Spot& m_smallDigitSpot;
            const ui::Spot& m_rankAtSpot;
            const ui::Spot& m_rankShiftSpot;
            const ui::Spot& m_bigDigitSpot;
            const ui::Spot& m_whiteSpot;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_serverSpot;
            const ui::Spot& m_hoverColorSpot;
            std::vector<Group> m_groups;
            int m_current = 0;
            int m_top = 0;
    };

}

#endif
