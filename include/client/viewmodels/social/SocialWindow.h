#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_SOCIAL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_SOCIAL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/EmoteHotkeys.h"
#include "client/models/social/EmoteSlot.h"
#include "client/models/social/FriendList.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "protocol/OwnPlayerRecord.h"
#include "ui/AnimatedIcon.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Social window with the friend, emote and setup tabs.
    /// @details Original: SocialWindow.xml, TSe_EmotiomForm. Friend tab: five friends a page, the GM row first (DrawFriendListPage); ticked friends can be mailed or taken off the list. Where the server does not group its ids the online states come from the players the world knows (Hooks::worldState), otherwise from the server's 14/9. Emote tab: the Ctrl+1..5 phrases (put into the input bar), the emotions and actions, and the Alt+1..8 slots, kept in the character's hotkey file. Setup tab: the profile (nickname, birthday, blood type), online or busy, and the heart radar, which shows a jumping heart over the Social button while a player whose heart number matches the own one is near.
    class SocialWindow {
        public:
            /// @brief Callbacks into the game the window needs.
            /// @details worldState gives a player's online state from the world; playEmotion / playAction play an emotion (1..24) or action (1..24) of the own character; putPhrase puts a phrase into the input bar; nearestHeart gives how far (map pixels across plus down) the nearest player of the other body parity is whose heart number added to this one makes 9; mailFriend / mailChecked open the mail form to a friend or to the ticked ones; married / showMarry drive the Marry tab, which shows for a married player and takes the window's place (the card window opens where this one was); whisper is SetWhisperTarget on the input bar.
            struct Hooks {
                std::function<int(std::int32_t)> worldState;
                std::function<void(int)> playEmotion;
                std::function<void(int)> playAction;
                std::function<void(const std::string&)> putPhrase;
                std::function<std::optional<int>(int heartNumber)> nearestHeart;
                std::function<void(std::int32_t)> mailFriend;
                std::function<void()> mailChecked;
                std::function<bool()> married;
                std::function<void(core::Point)> showMarry;
                std::function<void(std::int32_t)> whisper;
            };

            /// @brief Loads SocialWindow.xml, wires the painters and events.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param friends The friend list.
            /// @param messages Message line for hints and refusals.
            /// @param talk Dialog used for confirmations.
            /// @param hooks Callbacks into the game.
            SocialWindow(
                GameServices& services,
                ui::Screen& screen,
                FriendList& friends,
                PlayerMessages& messages,
                TalkDialog& talk,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True while the window is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window at a position, where another tab of the family was.
            /// @details Original: ShowCtrl (ShowFriendTabFromCard and the others).
            /// @param at New top-left corner of the window.
            void openAt(core::Point at);

            /// @brief Gives the online status as the setup tab leaves it.
            /// @details Original: Player.FIcqStatus.
            /// @return 0 online, 1 busy.
            int onlineStatus() const { return m_busy ? 1 : 0; }

            /// @brief Shows the window and brings it to the front.
            void open();

            /// @brief Hides the window; what was not handed in is taken back.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Opens the window or closes it.
            void toggle();

            /// @brief Shows the friend tab on its first page.
            /// @details Original: ShowFriendTab (the tab the game shows after entering the world).
            void showFriendTab();

            /// @brief Shows the emote tab.
            /// @details Original: ShowEmotionTab.
            void showEmoteTab();

            /// @brief Shows the setup tab; what is shown counts as handed in.
            /// @details Original: ShowSetupTab.
            void showSetupTab();

            /// @brief Loads the profile from the own player record and sets the online status, as the game does on entering the world.
            /// @details Original: LoadProfileFields and SelectOnlineStatus.
            /// @param record The own player record.
            void loadProfile(const protocol::OwnPlayerRecord& record);

            /// @brief Loads the character's emote slots and phrases from its hotkey file.
            /// @details Original: ClearEmotionHotkeys and LoadHotkeyFile.
            /// @param file Path of the hotkey file.
            void loadHotkeys(const std::filesystem::path& file);

            /// @brief Plays what an emote slot holds.
            /// @details Original: UseEmotionHotkey (Alt+1..8).
            /// @param slot Slot number 1..8.
            void useSlot(int slot);

            /// @brief Puts a quick phrase into the input bar.
            /// @details Original: UseQuickPhrase (Ctrl+1..5).
            /// @param index Phrase number 1..5.
            void usePhrase(int index);

        private:
            /// @brief The three tabs of the window.
            enum class Tab { Friend, Emote, Setup };

            /// @brief The controls of one friend row.
            struct Row {
                std::reference_wrapper<ui::TextBox> name;
                std::reference_wrapper<ui::TextBox> nickname;
                std::reference_wrapper<ui::TextBox> guild;
                std::reference_wrapper<ui::DrawingArea> career;
                std::reference_wrapper<ui::ImageButton> mail;
                std::reference_wrapper<ui::Element> mailGray;
                std::reference_wrapper<ui::ImageButton> check;
            };

            /// @brief Gives the list index of the page's first friend.
            /// @return Index into the friend list (rows 1..5 at 0..4 of the page).
            int firstIndex() const { return (m_page - 1) * kRowsPerPage; }

            /// @brief Gives the list index of the page's last friend.
            /// @return Index into the friend list.
            int lastIndex() const;

            /// @brief Gives the number of friend pages.
            /// @return Page count.
            int pageCount() const { return m_friends.count() / kRowsPerPage + 1; }

            /// @brief Gives a friend's online state.
            /// @details Original: DrawFriendListPage; the GM row is online while a GM (100 or 101) is in the world.
            /// @param index Index into the friend list.
            /// @return Online state 0..3 (3 offline).
            int onlineState(int index) const;

            /// @brief Shows or hides each row's controls and fills their texts.
            /// @details Original: DrawFriendListPage's controls.
            void layoutPage();

            /// @brief Paints the page's faces, levels, icons and page numbers.
            /// @details Original: DrawFriendListPage.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paintPage(gfx::Canvas& canvas, core::Point at);

            /// @brief Turns the friend page.
            /// @details Original: PrevFriendPage / NextFriendPage.
            /// @param step -1 for the previous page, 1 for the next.
            void turnPage(int step);

            /// @brief Flips the tick of a friend row.
            /// @details Original: ToggleFriendCheck.
            /// @param row Row number 1..5.
            void toggleCheck(int row);

            /// @brief Makes the friend whose face is under the mouse the whisper target, unless he is offline.
            /// @details Original: HandleDblClick and WhisperFriendAtRow; a double click on a friend's face (the friend tab).
            void whisperAtMouse();

            /// @brief Opens the mail form to the friend of a row.
            /// @details Original: MailFriendAtRow.
            /// @param row Row number 1..5.
            void mailRow(int row);

            /// @brief Opens the mail form to the ticked friends.
            /// @details Original: MailCheckedFriends.
            void mailChecked();

            /// @brief Asks about removing the ticked friends and sends the removal on confirmation.
            /// @details Original: DeleteFriendClick.
            void deleteChecked();

            /// @brief Hides all the friend rows' controls.
            void hideFriendControls();

            /// @brief Switches the tab and shows its controls and form picture.
            /// @param tab The tab to show.
            void setTabControls(Tab tab);

            /// @brief Plays an emotion and remembers it for the next slot click.
            /// @details Original: PlayEmotionButton.
            /// @param number Emotion number 1..24.
            void pickEmotion(int number);

            /// @brief Plays an action and remembers it for the next slot click.
            /// @details Original: PlayActionButton.
            /// @param number Action number 1..24.
            void pickAction(int number);

            /// @brief Puts the remembered emotion or action into a slot, then plays what it holds.
            /// @details Original: the slots' clicks (TAnimateImage.HandleMouseUp).
            /// @param slot Slot number 1..8.
            void clickSlot(int slot);

            /// @brief Shows a slot's picture and position from its content.
            /// @param slot Slot number 1..8.
            void showSlot(int slot);

            /// @brief Writes the hotkey file if one is set.
            void saveHotkeys();

            /// @brief Shows OK and Cancel in place of Close while the profile is changed.
            /// @details Original: DrawControl's per-frame part.
            void layoutProfileButtons();

            /// @brief Sets online or busy, shows the ticks and tells the server.
            /// @details Original: SelectOnlineStatus / SelectBusyStatus (5/7); SetOnlineStatus 1 (online) or 2 (busy), the server hears 0 or 1.
            /// @param busy True for busy.
            void setOnlineStatus(bool busy);

            /// @brief Clamps the birthday fields to valid dates.
            /// @details Original: ClampBirthdayFields.
            void clampBirthday();

            /// @brief Reads a birthday field.
            /// @param index 0 year, 1 month, 2 day.
            /// @return The number, or -1 when the field is empty.
            int birthdayField(int index) const;

            /// @brief Shows the sun sign of the birthday fields.
            /// @details Original: CalcSunSign.
            void showSunSign();

            /// @brief Selects a blood type, or none when it is picked again.
            /// @details Original: SelectProfileCheckBox.
            /// @param index Blood type 1..4.
            void selectBlood(int index);

            /// @brief Shows the blood type ticks.
            void showBlood();

            /// @brief Switches the heart radar on or off and saves it.
            void toggleHeartRadar();

            /// @brief Shows the heart radar switch picture.
            void showHeartRadar();

            /// @brief Asks to confirm the profile before submitting.
            /// @details Original: AskSubmitProfile (Se_TalkMsgFormPlus_4, a box that leaves the windows up).
            void askSubmitProfile();

            /// @brief Hands the profile in.
            /// @details Original: SubmitProfile; empty date fields become 0; a whole form is handed in (10/1 for a new nickname, 10/2 for a year from 1900 on, then 10/3).
            void submitProfile();

            /// @brief Takes back the profile edits.
            /// @details Original: RevertProfileEdits.
            void revertProfile();

            /// @brief Tells whether the profile form is whole.
            /// @details Original: UpdateHeartRadar.
            /// @return True with a blood type and the three date fields.
            bool profileComplete() const;

            /// @brief Paints the jumping heart of the radar.
            /// @details Original: UpdateHeartRadar; an unfinished form switches the radar off; the heart jumps the faster the nearer the player is, and hides under the mouse.
            /// @param canvas Target canvas.
            /// @param at Origin on the canvas.
            /// @param nowMs Current time in milliseconds.
            void paintHeartRadar(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            static constexpr int kRowsPerPage = 5;

            GameServices& m_services;
            ui::Screen& m_screen;
            FriendList& m_friends;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            Hooks m_hooks;
            ui::Window& m_window;
            std::vector<Row> m_rows;
            std::vector<std::reference_wrapper<ui::AnimatedIcon>> m_slots;
            std::vector<std::reference_wrapper<ui::TextBox>> m_phrases;
            std::vector<std::reference_wrapper<ui::Element>> m_emoteControls;
            std::vector<std::reference_wrapper<ui::Element>> m_setupControls;
            ui::TextBox& m_nickname;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_levelSpot;
            const ui::Spot& m_elementSpot;
            const ui::Spot& m_onlineSpot;
            const ui::Spot& m_onlineAreaSpot;
            const ui::Spot& m_faceHitSpot;
            const ui::Spot& m_pageSpot;
            std::vector<std::reference_wrapper<ui::TextBox>> m_birthday;
            std::vector<std::reference_wrapper<ui::ImageButton>> m_bloods;
            Tab m_tab = Tab::Friend;
            int m_page = 1;
            EmoteHotkeys m_hotkeys;
            std::filesystem::path m_hotkeyFile;
            EmoteSlot m_picked;
            std::string m_savedNickname;
            std::vector<std::string> m_savedBirthday = std::vector<std::string>(3);
            int m_savedBlood = 0;
            int m_blood = 0;
            bool m_edited = false;
            bool m_busy = false;
            int m_sunSign = 0;
            int m_birthYear = 0;
            int m_birthMonth = 0;
            int m_birthDay = 0;
            int m_profileChoice = 0;
            int m_heartNumber = 0;
            std::int32_t m_playerId = 0;
            std::size_t m_nameLength = 0;
            int m_heartFrame = 0;
            std::uint64_t m_heartMs = 0;
    };

}

#endif
