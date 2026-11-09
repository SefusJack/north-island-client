#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_CHAT_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_CHAT_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/chat/ChatLog.h"
#include "client/models/chat/AnnounceBanner.h"
#include "client/models/chat/Marquee.h"
#include "client/services/settings/TalkSettings.h"
#include "core/Rect.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/ScrollBar.h"
#include "ui/Spot.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The chat lines at the bottom left of the world (ChatWindow.xml, TTalkMsgForm) with the system messages running along the bottom.
    /// @details Shows the last lines of the log in 20-pixel rows; the switch button cycles see-through, hidden and dithered.
    class ChatWindow {
        public:
            /// @brief Loads ChatWindow.xml and wires the form.
            /// @param services Game services (layout, pictures, characters, chat hook).
            /// @param screen Screen the form is loaded into.
            ChatWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Shows the chat panel.
            void show() { m_panel.setVisible(true); }

            /// @brief Hides the chat panel.
            void hide() { m_panel.setVisible(false); }

            /// @brief Adds a message to the log (original: AddTalkMessage).
            /// @param channel 0 system, 1 world, 2 local, 3 whisper, 4 GM, 5 team, 6 guild, 7 ally.
            /// @param message Message text.
            /// @param senderId Speaker id, 0 for none.
            /// @param announce False keeps a system-coloured line off the marquee along the bottom.
            void add(int channel, const std::string& message, std::int32_t senderId = 0, bool announce = true);

            /// @brief Whether one of the last count messages is this one.
            /// @param channel Channel of the message.
            /// @param message Message text.
            /// @param count How many of the latest messages to look at.
            /// @return True when found.
            bool heardRecently(int channel, const std::string& message, std::size_t count) const { return m_log.hasRecent(channel, message, count); }

            /// @brief Sets a callback told every message added.
            /// @param observer The callback.
            void setObserver(std::function<void(const std::string&)> observer) { m_observer = std::move(observer); }

            /// @brief Clears the log lines; the marquee stays.
            /// @details Original: ClearTalkLines (ClearChatList empties the list and brings the scroll bar back).
            void clear();

            /// @brief Sets how to look up a speaker's appearance for the head beside a line.
            /// @param lookOf Returns the look of a player id, or nullopt when not known.
            void setSpeakerLooks(std::function<std::optional<CharacterLook>(std::int32_t)> lookOf) { m_lookOf = std::move(lookOf); }

            /// @brief Advances the marquee and the banner.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs) {
                m_marquee.update(nowMs);
                m_bannerShown = m_banner.update(nowMs);
            }

            /// @brief Adds a prize announcement to the banner across the top.
            /// @param line Announcement text.
            /// @details Original: RecvSystemAnnounce.
            void announce(const std::string& line) { m_banner.add(line); }

            /// @brief Sets the colour of a channel's coming messages.
            /// @param channel Channel number.
            /// @param color Colour in RGB565.
            void setChannelColor(int channel, std::uint16_t color) { m_log.setChannelColor(channel, color); }

            /// @brief Whether the box is closed.
            /// @return True in look 2 (hidden).
            /// @details Original: FFormMode 2.
            bool isHidden() const { return m_look == 2; }

            /// @brief Sets the own player's id; its lines unlock the box.
            /// @param id Player id.
            void setOwnId(std::int32_t id) { m_ownId = id; }

            /// @brief Sets the hook called to whisper to a speaker.
            /// @param whisper Receives the speaker id and whether it came from a click on the head (no notice then).
            /// @details Original: SetWhisperTarget (fromHead: FWhisperFromHead).
            void setWhisperHook(std::function<void(std::int32_t, bool fromHead)> whisper) { m_whisper = std::move(whisper); }

            /// @brief Applies saved place, size (from the resize button's place) and look.
            /// @param settings Saved talk settings.
            /// @details Original: part of LoadTalkFormSettings.
            void apply(const TalkSettings& settings);

            /// @brief Current talk settings.
            /// @return Look, form rectangle, background and resize button place.
            TalkSettings settings() const;

        private:
            /// @brief Scrolls the lines back or forward.
            /// @param lines Positive to go back in the log, negative to go forward.
            void scroll(int lines);

            /// @brief Locks or unlocks the box (locked, new lines do not scroll it).
            /// @details Original: ToggleMsgLock.
            void toggleLock();

            /// @brief The log line under a point of the lines' area.
            /// @param local Point inside the lines' area.
            /// @return Index counted over the whole log, or nullopt.
            std::optional<std::size_t> lineAt(core::Point local) const;

            /// @brief Switches the look.
            /// @param look 0 dithered, 1 see-through, 2 hidden.
            /// @details Original: ApplyTalkFormMode.
            void setLook(int look);

            /// @brief Places the controls from the form's size.
            /// @details Original: ArrangeTalkControls.
            void arrange();

            /// @brief Keeps the form on the screen.
            /// @details Original: TSe_Form.RenderWithChildren.
            void keepOnScreen();

            /// @brief Sets the form's new size from where the resize button was let go.
            /// @details Original: ApplyResizeButton.
            void applyResize();

            /// @brief Number of line rows the form holds.
            /// @return Row count, at least 1.
            int rows() const;

            /// @brief Syncs the scroll bar range and thumb with the log and the scroll position.
            void syncScrollBar();

            /// @brief Paints the dithered grid or the see-through frame.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: TTalkMsgForm.DrawControl.
            void paintBackground(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the announce banner.
            /// @param canvas Target canvas.
            /// @details Original: DrawSystemAnnounce.
            void paintAnnounce(gfx::Canvas& canvas);

            /// @brief Paints the visible lines with emotes and speaker heads.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @param nowMs Current time in ms.
            void paintLines(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the resize button (Btn_Resize_1).
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            void paintResizeButton(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints a dash-dot outline of the form to be while the resize button is dragged.
            /// @param canvas Target canvas.
            /// @param at Where the panel is.
            void paintResizePreview(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            ui::Element& m_panel;
            ui::Element& m_form;
            ui::ScrollBar& m_scrollBar;
            ui::DrawingArea& m_dragArea;
            ui::DrawingArea& m_resize;
            ui::DrawingArea& m_lines;
            ui::ImageButton& m_switch;
            const ui::DrawingArea& m_marqueeArea;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_systemSpot;
            const ui::Spot& m_previewSpot;
            ChatLog m_log;
            std::function<void(const std::string&)> m_observer;
            std::function<std::optional<CharacterLook>(std::int32_t)> m_lookOf;
            Marquee m_marquee;
            AnnounceBanner m_banner;
            std::optional<AnnounceBanner::Visible> m_bannerShown;
            std::size_t m_scrolledBack = 0;
            int m_look = 1;
            int m_background = 0;
            bool m_locked = false;
            std::int32_t m_ownId = 0;
            std::int32_t m_hoveredHead = 0;
            std::function<void(std::int32_t, bool)> m_whisper;
            core::Point m_grab;
            bool m_resizing = false;
    };

}

#endif
