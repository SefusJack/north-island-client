#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/rendering/painters/InstanceGoalsPainter.h"
#include "client/models/instances/Instances.h"
#include "client/models/social/TeamMate.h"
#include "client/viewmodels/instances/InstanceCreateForm.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "core/Point.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Panel.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The instance tab of the group window: choose an instance (five a page, Join buttons, New Instance), inner info (own instance rules and goals, Enter/Leave) and members (five a page).
    /// @details Original: InstanceWindow.xml, TDupMisForm. The tabs of the group window lead back to the party and the guild.
    class InstanceWindow {
        public:
            /// @brief Callbacks into the rest of the client that the window needs.
            struct Hooks {
                std::function<bool()> inGuild;
                std::function<void(core::Point)> showParty;
                std::function<void(core::Point)> showGuild;
                std::function<std::int32_t()> ownId;
                std::function<std::optional<TeamMate>(std::int32_t)> player;
                std::function<void(std::int32_t)> whisper;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param instances Instance model.
            /// @param create The create form opened by New Instance.
            /// @param goals Painter of the instance goals.
            /// @param talk Dialog used for the confirmation questions.
            /// @param hooks Callbacks into guild, party, player lookup and whisper.
            InstanceWindow(
                GameServices& services,
                ui::Screen& screen,
                Instances& instances,
                InstanceCreateForm& create,
                InstanceGoalsPainter& goals,
                TalkDialog& talk,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Gives the window's top-left position.
            /// @return Top-left point.
            core::Point position() const { return {m_window.bounds().x, m_window.bounds().y}; }

            /// @brief Shows the window where the group window was.
            /// @details Original: ShowCtrl.
            /// @param at Top-left of the window.
            void open(core::Point at);

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

            /// @brief Asks whether to enter the current instance (the status icons ask it too).
            /// @details Original: ConfirmEnterDup.
            void askEnter();

            /// @brief Asks whether to quit the instance (the status icons ask it too).
            /// @details Original: ConfirmLeaveDup.
            void askLeave();

        private:
            /// @brief Decides which tabs and controls show and draws the picture of the page.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void layout(gfx::Canvas& canvas, core::Point at);

            /// @brief Sets the controls of a page.
            /// @details Original: SelectDupMisTab.
            /// @param tab Page selected.
            void applyTab(Instances::Tab tab);

            /// @brief Finds the row of the list the mouse is over.
            /// @details Original: FHoverRow.
            /// @param localY Mouse Y relative to the list.
            /// @return Row index, -1 for none.
            int hoverRow(int localY) const;

            /// @brief Paints the page selected plus the start number.
            /// @details Original: RenderWithChildren, DrawDupMisTab.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the rows of the choose page.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintRows(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the own instance's info page.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintInner(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the members page.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintMembers(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the creator hint beside the row under the mouse.
            /// @details Original: SetDupHintText and DrawDupHintLines.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param row Row index under the mouse.
            void paintHint(gfx::Canvas& canvas, core::Point at, int row);

            /// @brief Paints the hover bar of the row under the mouse.
            /// @param canvas Target canvas.
            /// @param at Top-left of the list.
            void paintList(gfx::Canvas& canvas, core::Point at);

            /// @brief Asks to send a member away.
            /// @details Original: ConfirmDupKickMember.
            /// @param slot Member row (1-5).
            void askSendAway(int slot);

            /// @brief Whispers to the player of the member row that was double clicked.
            /// @details Original: HandleDblClick.
            void doubleClicked();

            /// @brief Draws one line of text 288 pixels wide.
            /// @details Original: DrawDupMisText.
            /// @param canvas Target canvas.
            /// @param line Text.
            /// @param at Top-left of the text.
            void text(gfx::Canvas& canvas, const std::string& line, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            Instances& m_instances;
            InstanceCreateForm& m_create;
            TalkDialog& m_talk;
            Hooks m_hooks;
            InstanceGoalsPainter& m_goals;
            ui::Window& m_window;
            ui::DrawingArea& m_list;
            ui::Panel& m_hint;
            ui::ImageButton& m_tabGuild;
            ui::ImageButton& m_tabInstances;
            ui::ImageButton& m_pageInfo;
            ui::ImageButton& m_pageMembers;
            ui::ImageButton& m_pageChoose;
            ui::ImageButton& m_newInstance;
            ui::ImageButton& m_previous;
            ui::ImageButton& m_next;
            ui::ImageButton& m_membersPrevious;
            ui::ImageButton& m_membersNext;
            ui::ImageButton& m_enter;
            ui::ImageButton& m_leave;
            const ui::Spot& m_artSpot;
            const ui::Spot& m_orderNumbersSpot;
            const ui::Spot& m_memberNumbersSpot;
            const ui::Spot& m_startIconSpot;
            const ui::Spot& m_startNumberSpot;
            const ui::Spot& m_memberTitleSpot;
            const ui::Spot& m_memberPanelSpot;
            const ui::Spot& m_memberRowSpot;
            const ui::Spot& m_memberRectSpot;
            const ui::Spot& m_innerTextSpot;
            const ui::Spot& m_innerDescriptionSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_hoverBarSpot;
            const ui::Spot& m_hintTextSpot;
            std::vector<std::reference_wrapper<ui::ImageButton>> m_joins;
            std::vector<std::reference_wrapper<ui::ImageButton>> m_kicks;
            // five member rows per page
            std::array<std::int32_t, 5> m_slotIds{};
    };

}

#endif
