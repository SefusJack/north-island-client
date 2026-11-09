#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_INPUT_BAR_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_INPUT_BAR_H

#include "client/services/GameServices.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The chat input along the bottom of the world (InputBar.xml, TSe_InputBar): channel button, typed line, whisper name field, emotes and the mail envelope.
    /// @details The channel is world, local, whisper, team or guild; the last ten lines are kept for Up and Down.
    class InputBar {
        public:
            /// @brief What became of a line: kept in the field (refused), dropped (refused, the field cleared) or sent (cleared and remembered).
            enum class Sent { Kept, Dropped, Sent };

            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<Sent(int channel, const std::string& text)> send;
                std::function<bool(int channel)> channelAllowed;
                std::function<void()> openMailBox;
                std::function<std::optional<std::int32_t>(const std::string&)> playerNamed;
                std::function<std::int32_t()> ownId;
                std::function<std::string(std::int32_t)> playerName;
                std::function<void(const std::string&, std::int32_t)> notice;
                std::function<void(const std::string&)> emoteToMail;
                std::function<void()> quitViewing;
                std::function<bool(std::int32_t)> targetChosen;
            };

            /// @brief Loads InputBar.xml and wires the bar.
            /// @param services Game services (layout, pictures).
            /// @param screen Screen the bar is loaded into.
            /// @param hooks Callbacks into the client.
            InputBar(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Whether the bar is shown.
            /// @return True when visible.
            bool isVisible() const { return m_panel.isVisible(); }

            /// @brief Shows the bar.
            void show() { m_panel.setVisible(true); }

            /// @brief Hides the bar.
            void hide() { m_panel.setVisible(false); }

            /// @brief Puts a line into the text field without sending it.
            /// @param text The line.
            /// @details Original: UseQuickPhrase.
            void setText(const std::string& text) { m_text.setText(text); }

            /// @brief Sets the channel; team and guild need one (else local), whisper shows the name field.
            /// @param channel 1 world, 2 local, 3 whisper, 4 team, 5 guild.
            /// @details Original: SetChatChannel.
            void setChannel(int channel);

            /// @brief Per-frame update of the mail envelope, channel frame, whisper names and Quit viewing button.
            /// @param nowMs Current time in ms.
            /// @param newMail Whether new mail waits (the envelope blinks).
            /// @param watching Whether a battle is being watched (shows Quit viewing).
            /// @details Original: DrawNewMailIcon (two frames take turns every 400 ms) and DrawControl (the channel frame shows while the mouse is on the button or in the frame).
            void update(std::uint64_t nowMs, bool newMail, bool watching);

            /// @brief The whisper target.
            /// @return Player id, 0 for none.
            /// @details Original: FWhisperTargetId.
            std::int32_t whisperTarget() const { return m_whisperTarget; }

            /// @brief Text of the whisper name field.
            /// @return The name typed.
            const std::string& whisperName() const { return m_whisperName.text(); }

            /// @brief Forgets the whisper target.
            void dropWhisperTarget() { m_whisperTarget = 0; }

            /// @brief Sets the whisper target as it is and leaves the name field ("/msg").
            /// @param playerId Player id.
            void aimWhisper(std::int32_t playerId) { m_whisperTarget = playerId; }

            /// @brief Makes a player the one whispered to (not the own player nor a GM).
            /// @param playerId Player id.
            /// @param fromHead True for a click on a chat line's head (no notice).
            /// @details Original: SetWhisperTarget (fromHead is FWhisperFromHead).
            void setWhisperTarget(std::int32_t playerId, bool fromHead = false);

            /// @brief Adds a name to the whisper names list.
            /// @param name Player name.
            /// @param toEnd True puts the newest at the end, else before the last.
            /// @details Original: AddWhisperName.
            void addWhisperName(const std::string& name, bool toEnd);

            /// @brief Removes a name from the whisper names list.
            /// @param name Player name.
            /// @details Original: RemoveWhisperName.
            void removeWhisperName(const std::string& name);

            /// @brief Chooses where picked emotes go.
            /// @param toMail True for the mail being written, false for the line.
            /// @details Original: FEmoteTarget (1 mail, 2 line).
            void setEmotesToMail(bool toMail) { m_emotesToMail = toMail; }

            /// @brief The current channel.
            /// @return Channel number.
            /// @details Original: FChatChannel.
            int channel() const { return m_channel; }

        private:
            /// @brief The mouse in the panel's places (the areas are the 800 x 600 ones).
            /// @return Mouse position relative to the panel.
            core::Point panelMouse() const { return m_screen.mouseIn(m_panel) - m_panel.bounds().position(); }

            /// @brief Turns the button through world, local, whisper, team and round again.
            /// @details Original: CycleChatChannel.
            void cycleChannel();

            /// @brief Takes the channel offered in a row of the frame.
            /// @param row Row 1 .. 4.
            /// @details Original: SelectChannelFromList.
            void pickChannel(int row);

            /// @brief Sends the typed line on the channel and remembers it (ten at most).
            /// @details Original: SendChatInput.
            void sendLine();

            /// @brief Recalls a line from the history.
            /// @param step Negative for the previous line, else the next.
            /// @details Original: RecallPrevChatLine / RecallNextChatLine.
            void recall(int step);

            /// @brief Looks the typed name up among the players the world knows and sets the target.
            /// @details Original: ResolveWhisperTarget.
            void resolveWhisperTarget();

            /// @brief Takes a name from the whisper names list.
            /// @param index List index.
            /// @details Original: SelectWhisperName.
            void pickWhisperName(int index);

            /// @brief Shows or hides the whisper names list.
            /// @details Original: ToggleWhisperNameList.
            void toggleWhisperNames();

            /// @brief Resolves the whisper target when the text field is clicked.
            /// @details Original: ResolveWhisperOnEditClick.
            void textClicked();

            /// @brief Places the whisper names list above the bar, a row a name.
            void placeWhisperNames();

            /// @brief Shows or hides the emote panel.
            /// @param shown True to show.
            /// @details Original: ToggleEmotePanel / CloseEmotePanel.
            void showEmotes(bool shown);

            /// @brief Adds an emote code to the line (while two more characters fit) or to the mail.
            /// @param number Emote number.
            /// @details Original: InsertEmoteAtPoint.
            void pickEmote(int number);

            /// @brief Paints the emotes of the line over their codes.
            /// @param canvas Target canvas.
            /// @param origin Where the bar's layout (0, 0) is on the screen (it moves with the bottom left of a larger window).
            /// @param nowMs Current time in ms.
            /// @details Original: DrawInputEmotes.
            void paintTextEmotes(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs);

            static constexpr int kPickRows = 4;

            GameServices& m_services;
            ui::Screen& m_screen;
            Hooks m_hooks;
            ui::Element& m_panel;
            ui::ImageView& m_channelMark;
            ui::ImageView& m_mail;
            ui::ImageButton& m_channelButton;
            ui::ImageButton& m_quitViewing;
            ui::Element& m_channelFrame;
            ui::TextBox& m_text;
            ui::TextBox& m_whisperName;
            ui::ListBox& m_whisperNames;
            const ui::Spot& m_mailFrameSpot;
            const ui::Spot& m_buttonSpot;
            const ui::Spot& m_frameSpot;
            const ui::Spot& m_nameFieldSpot;
            const ui::Spot& m_chosenNameSpot;
            const ui::Spot& m_namesPanelSpot;
            const ui::Spot& m_namesInPanelSpot;
            const ui::Spot& m_lineEmotesSpot;
            int m_channel = 2;
            bool m_emotesToMail = false;
            std::int32_t m_whisperTarget = 0;
            std::array<int, kPickRows + 1> m_pickChannels{};
            std::vector<std::string> m_history;
            int m_historyCount = 0;
            int m_historyAt = 0;
            int m_mailFrame = 0;
            std::uint64_t m_mailFrameMs = 0;
    };

}

#endif
