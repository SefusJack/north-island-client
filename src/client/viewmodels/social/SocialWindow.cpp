#include "client/viewmodels/social/SocialWindow.h"

#include "client/models/character/CharacterLook.h"
#include "client/rendering/hud/GameNumbers.h"
#include "client/rendering/hud/PageNumbers.h"
#include "client/rules/GmIds.h"
#include "client/rules/SunSigns.h"
#include "client/rules/TextFilter.h"
#include "core/Log.h"
#include "protocol/FriendRequests.h"
#include "protocol/ProfileRequests.h"

#include <array>
#include <ctime>
#include <string>

namespace client {

    namespace {

        constexpr int kFaceSlot = 1; // portrait slot passed to drawPortrait (the friend-list face size)
        constexpr int kGmFace = 8084; // face sprite id (RoleShow sprite) used for GM friends
        constexpr int kOfflineState = 3; // online state 3: offline (index into kOnlineIcons: online, busy, inline, offline)
        const std::array<const char*, 4> kOnlineIcons = {"Icon_online_1", "Icon_Busy_1", "Icon_inline_1", "icon_offline_1"};
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};
        const std::array<const char*, 7> kJobNames = {"", "Killer ", "Warrior ", "Knight ", "Wit ", "Priest ", "Seer "};
        // x of emote hotkey slots 1..8 on the emote page, px (index 0 unused)
        const std::array<int, 9> kSlotX = {0, 36, 71, 106, 141, 175, 211, 246, 281};
        constexpr int kSlotY = 403; // y of the emote hotkey slots, px
        constexpr int kEmotions = 24; // number of emotion/action buttons (EmoteEmotion1..24, EmoteAction1..24)
        constexpr int kEmotionBase = 100; // emotion number + 100 gives the picture name "E1xx"

        /// @brief Tells whether a job id has an icon and a name.
        /// @param job Job id of a friend record.
        /// @return True for 1..6.
        bool validJob(int job) { return job >= 1 && job < static_cast<int>(kJobIcons.size()); }

        /// @brief Loads SocialWindow.xml into the screen.
        /// @param services Shared game services (layout path and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The SocialWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SocialWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("SocialWindow");
        }

        /// @brief Reads the number a control name ends in when it starts with the prefix.
        /// @param name Control name.
        /// @param prefix Required leading part of the name.
        /// @return The trailing number, or 0 when the prefix does not match.
        int numberAfter(const std::string& name, const std::string& prefix) { return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0; }

    }

    SocialWindow::SocialWindow(
        GameServices& services,
        ui::Screen& screen,
        FriendList& friends,
        PlayerMessages& messages,
        TalkDialog& talk,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_friends(friends),
          m_messages(messages),
          m_talk(talk),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_nickname(screen.get<ui::TextBox>("SetupNickname")),
          m_rowSpot(screen.get<ui::Spot>("FriendRow")),
          m_faceSpot(screen.get<ui::Spot>("FriendFace")),
          m_levelSpot(screen.get<ui::Spot>("FriendLevel")),
          m_elementSpot(screen.get<ui::Spot>("FriendElement")),
          m_onlineSpot(screen.get<ui::Spot>("FriendOnline")),
          m_onlineAreaSpot(screen.get<ui::Spot>("FriendOnlineArea")),
          m_faceHitSpot(screen.get<ui::Spot>("FriendFaceHit")),
          m_pageSpot(screen.get<ui::Spot>("FriendPageNumbers")) {
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const std::string n = std::to_string(row);
            m_rows.push_back(
                {screen.get<ui::TextBox>("FriendName" + n),
                 screen.get<ui::TextBox>("FriendNickname" + n),
                 screen.get<ui::TextBox>("FriendGuild" + n),
                 screen.get<ui::DrawingArea>("FriendCareer" + n),
                 screen.get<ui::ImageButton>("FriendMail" + n),
                 screen.get<ui::Element>("FriendMailGray" + n),
                 screen.get<ui::ImageButton>("FriendCheck" + n)}
            );
            m_rows.back().career.get().setPainter([this, row](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                const int index = firstIndex() + row - 1;
                if (index > m_friends.count()) {
                    return;
                }
                const int job = m_friends.at(index).record.job;
                if (!validJob(job)) {
                    return;
                }
                if (const auto icon = m_services.pictures.find(kJobIcons[static_cast<std::size_t>(job)])) {
                    canvas.drawPicture(*icon, at);
                }
            });
        }
        for (int i = 1; i <= kEmotions; ++i) {
            m_emoteControls.push_back(screen.get<ui::Element>("EmoteEmotion" + std::to_string(i)));
            m_emoteControls.push_back(screen.get<ui::Element>("EmoteAction" + std::to_string(i)));
        }
        for (int slot = 1; slot <= EmoteHotkeys::kSlots; ++slot) {
            m_slots.push_back(screen.get<ui::AnimatedIcon>("EmoteSlot" + std::to_string(slot)));
            m_emoteControls.push_back(m_slots.back().get());
        }
        for (int i = 1; i <= EmoteHotkeys::kPhrases; ++i) {
            m_phrases.push_back(screen.get<ui::TextBox>("EmotePhrase" + std::to_string(i)));
            m_emoteControls.push_back(m_phrases.back().get());
            m_emoteControls.push_back(screen.get<ui::Element>("EmotePhraseButton" + std::to_string(i)));
        }
        for (const char* name : {"SetupBirthYear", "SetupBirthMonth", "SetupBirthDay"}) {
            m_birthday.push_back(screen.get<ui::TextBox>(name));
            m_setupControls.push_back(m_birthday.back().get());
        }
        for (int i = 1; i <= 4; ++i) { // four blood types
            m_bloods.push_back(screen.get<ui::ImageButton>("SetupBlood" + std::to_string(i)));
            m_setupControls.push_back(m_bloods.back().get());
        }
        for (const char* name : {"SetupNickname", "SetupOnline", "SetupBusy", "SetupHeartRadar", "SetupSunSign"}) {
            m_setupControls.push_back(screen.get<ui::Element>(name));
        }
        screen.get<ui::DrawingArea>("FriendPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            m_screen.get<ui::ImageButton>("SocialTabMarry").setVisible(m_hooks.married());
            layoutProfileButtons();
            if (m_tab == Tab::Friend) {
                layoutPage();
                paintPage(canvas, at);
            }
        });
        screen.get<ui::DrawingArea>("HeartRadar").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintHeartRadar(canvas, at, nowMs);
        });
        screen.on("onSocialTabSetup", [this](const ui::EventArgs&) { showSetupTab(); });
        screen.on("onSocialTabMarry", [this](const ui::EventArgs&) {
            const core::Rect bounds = m_window.bounds();
            close();
            m_hooks.showMarry({bounds.x, bounds.y});
        });
        screen.on("onSetupOnline", [this](const ui::EventArgs&) { setOnlineStatus(false); });
        screen.on("onSetupBusy", [this](const ui::EventArgs&) { setOnlineStatus(true); });
        screen.on("onSetupBirthday", [this](const ui::EventArgs&) {
            clampBirthday();
            m_edited = true;
        });
        screen.on("onSetupBlood", [this](const ui::EventArgs& args) { selectBlood(numberAfter(args.source, "SetupBlood")); });
        screen.on("onSetupHeartRadar", [this](const ui::EventArgs&) { toggleHeartRadar(); });
        screen.on("onSetupOk", [this](const ui::EventArgs&) { askSubmitProfile(); });
        screen.on("onSetupCancel", [this](const ui::EventArgs&) { revertProfile(); });
        screen.on("onSocialClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onSocialTabFriend", [this](const ui::EventArgs&) { showFriendTab(); });
        screen.on("onSocialTabEmote", [this](const ui::EventArgs&) { showEmoteTab(); });
        screen.on("onEmoteEmotion", [this](const ui::EventArgs& args) { pickEmotion(numberAfter(args.source, "EmoteEmotion")); });
        screen.on("onEmoteAction", [this](const ui::EventArgs& args) { pickAction(numberAfter(args.source, "EmoteAction")); });
        screen.on("onEmoteSlot", [this](const ui::EventArgs& args) { clickSlot(numberAfter(args.source, "EmoteSlot")); });
        screen.on("onEmotePhraseButton", [this](const ui::EventArgs& args) { usePhrase(numberAfter(args.source, "EmotePhraseButton")); });
        screen.on("onEmotePhraseChanged", [this](const ui::EventArgs& args) {
            const int index = numberAfter(args.source, "EmotePhrase");
            m_hotkeys.setPhrase(index, m_phrases[static_cast<std::size_t>(index - 1)].get().text());
            saveHotkeys();
        });
        screen.on("onSocialDoubleClick", [this](const ui::EventArgs&) { whisperAtMouse(); });
        screen.on("onFriendPreviousPage", [this](const ui::EventArgs&) { turnPage(-1); });
        screen.on("onFriendNextPage", [this](const ui::EventArgs&) { turnPage(1); });
        screen.on("onFriendCheck", [this](const ui::EventArgs& args) { toggleCheck(numberAfter(args.source, "FriendCheck")); });
        screen.on("onFriendMail", [this](const ui::EventArgs& args) { mailRow(numberAfter(args.source, "FriendMail")); });
        screen.on("onFriendSendMail", [this](const ui::EventArgs&) { mailChecked(); });
        screen.on("onFriendDelete", [this](const ui::EventArgs&) { deleteChecked(); });
    }

    void SocialWindow::openAt(core::Point at) {
        const core::Rect bounds = m_window.bounds();
        m_window.setBounds({at.x, at.y, bounds.width, bounds.height});
        open();
    }

    void SocialWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void SocialWindow::close() {
        m_window.setVisible(false);
        m_edited = false;
        if (m_nickname.text() != m_savedNickname) {
            m_nickname.setText(m_savedNickname);
        }
        for (std::size_t i = 0; i < m_birthday.size(); ++i) {
            if (m_birthday[i].get().text() != m_savedBirthday[i]) {
                m_birthday[i].get().setText(m_savedBirthday[i]);
            }
        }
        if (m_blood != m_savedBlood) {
            m_blood = m_savedBlood;
            showBlood();
        }
    }

    void SocialWindow::toggle() {
        if (isOpen()) {
            close();
        } else {
            open();
        }
    }

    int SocialWindow::lastIndex() const { return m_page < pageCount() ? firstIndex() + kRowsPerPage - 1 : m_friends.count(); }

    int SocialWindow::onlineState(int index) const {
        const std::int32_t id = m_friends.at(index).record.id;
        if (index == 0) {
            const int state = m_hooks.worldState(id);
            return state != 3 ? state : m_hooks.worldState(101); // 3 is offline; 101 is the second GM id
        }
        return m_services.session.serverGrouped ? m_friends.at(index).onlineState : m_hooks.worldState(id);
    }

    void SocialWindow::layoutPage() {
        for (const Row& row : m_rows) {
            row.name.get().setVisible(false);
            row.nickname.get().setVisible(false);
            row.guild.get().setVisible(false);
            row.career.get().setVisible(false);
            row.career.get().setHint("");
            row.check.get().setVisible(false);
            row.mail.get().setVisible(false);
            row.mailGray.get().setVisible(true);
        }
        for (int index = firstIndex(); index <= lastIndex(); ++index) {
            const Row& row = m_rows[static_cast<std::size_t>(index - firstIndex())];
            const Friend& f = m_friends.at(index);
            row.mail.get().setVisible(true);
            if (index != 0) {
                row.check.get().setVisible(true);
                row.check.get().setPicture(m_services.pictures.find(f.checked ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}));
            }
            row.nickname.get().setText(f.record.nickname);
            row.guild.get().setText(f.record.guild);
            row.nickname.get().setVisible(true);
            row.guild.get().setVisible(true);
            row.name.get().setText(f.record.name);
            row.name.get().setVisible(true);
            if (validJob(f.record.job)) {
                row.career.get().setVisible(true);
                row.career.get().setHint(kJobNames[static_cast<std::size_t>(f.record.job)]);
            }
        }
    }

    void SocialWindow::paintPage(gfx::Canvas& canvas, core::Point at) {
        const auto draw = [&](const std::string& name, core::Point where) {
            if (const auto id = m_services.pictures.find(name)) {
                canvas.drawPicture(*id, where);
            }
        };
        for (int index = firstIndex(); index <= lastIndex(); ++index) {
            const Friend& f = m_friends.at(index);
            const int row = index - firstIndex();
            const int state = onlineState(index);
            if (state >= 0 && state < static_cast<int>(kOnlineIcons.size())) {
                if (const auto icon = m_services.pictures.find(kOnlineIcons[static_cast<std::size_t>(state)])) {
                    canvas.drawPicture(*icon, {at.x + m_onlineSpot.at().x, at.y + m_onlineSpot.at().y + row * m_onlineSpot.pitch().y}, m_onlineAreaSpot.rect());
                }
            }
            if (f.record.element >= 0 && f.record.element <= 4) { // element icons 0..4
                draw(
                    "Icon_Element_" + std::to_string(f.record.element) + "_1",
                    {at.x + m_elementSpot.at().x, at.y + m_elementSpot.at().y + row * m_elementSpot.pitch().y}
                );
            }
            const core::Point origin{at.x + m_rowSpot.at().x, at.y + m_rowSpot.at().y + row * m_rowSpot.pitch().y};
            const core::Point face = origin + m_faceSpot.at();
            if (f.record.bodyType >= 1 && f.record.bodyType <= 4) { // body types 1..4 are players
                const CharacterLook look =
                    CharacterLook::fromColorCodes(f.record.bodyType, f.record.head, f.record.colorCode1, f.record.colorCode5, m_services.items);
                m_services.characters.drawPortrait(canvas, look, face, 0, kFaceSlot);
            } else if (f.record.bodyType == 0 && isGmId(f.record.id)) { // body type 0 is the GM
                m_services.characters.drawFaceSprite(canvas, kGmFace, face, 0, 0);
            }
            drawLevelDigits(canvas, f.record.level, origin + m_levelSpot.at());
        }
        drawPageNumbers(canvas, {at.x + m_pageSpot.at().x, at.y + m_pageSpot.at().y}, m_page, pageCount());
    }

    void SocialWindow::turnPage(int step) {
        const int page = m_page + step;
        if (page >= 1 && page <= pageCount()) {
            m_page = page;
        }
    }

    void SocialWindow::whisperAtMouse() {
        if (m_tab != Tab::Friend) {
            return;
        }
        const core::Point local = m_screen.mouse() - m_window.bounds().position();
        for (int row = 0; row < kRowsPerPage; ++row) {
            if (!core::Rect{m_faceHitSpot.rect().x, row * m_rowSpot.pitch().y + m_faceHitSpot.rect().y, m_faceHitSpot.rect().width, m_faceHitSpot.rect().height}
                     .contains(local)) {
                continue;
            }
            const int index = firstIndex() + row;
            if (index <= lastIndex() && onlineState(index) != kOfflineState) {
                m_hooks.whisper(m_friends.at(index).record.id);
            }
            break;
        }
    }

    void SocialWindow::toggleCheck(int row) {
        const int index = firstIndex() + row - 1;
        if (row < 1 || index < 1 || index > m_friends.count()) {
            return;
        }
        Friend& f = m_friends.at(index);
        f.checked = !f.checked;
    }

    void SocialWindow::mailRow(int row) {
        const int index = firstIndex() + row - 1;
        if (row < 1 || index > m_friends.count() || isGmId(m_friends.at(index).record.id)) {
            return;
        }
        m_friends.at(index).checked = false;
        if (m_hooks.mailFriend) {
            m_hooks.mailFriend(m_friends.at(index).record.id);
        }
    }

    void SocialWindow::mailChecked() {
        for (int index = 1; index <= m_friends.count(); ++index) {
            if (m_friends.at(index).checked) {
                if (m_hooks.mailChecked) {
                    m_hooks.mailChecked();
                }
                return;
            }
        }
        m_messages.show("Please choose friends To mail a letter", 1200); // message time in ms
    }

    void SocialWindow::deleteChecked() {
        std::vector<int> checked;
        for (int index = 1; index <= m_friends.count(); ++index) {
            if (m_friends.at(index).checked) {
                checked.push_back(index);
            }
        }
        if (checked.empty()) {
            m_messages.show("Please choose friends To delete", 1200); // message time in ms
            return;
        }
        if (checked.size() > 1) {
            std::vector<std::int32_t> ids;
            for (int index : checked) {
                ids.push_back(m_friends.at(index).record.id);
            }
            m_talk.ask(
                "Are you sure you want to remove these friends?",
                std::nullopt,
                [this, ids] {
                    for (std::int32_t id : ids) {
                        m_services.connection.send(protocol::deleteFriendRequest(id));
                    }
                },
                nullptr
            );
            return;
        }
        Friend& f = m_friends.at(checked.front());
        f.checked = false;
        const std::int32_t id = f.record.id;
        m_talk.ask(
            "Are you sure you want to delete this friend? " + f.record.name,
            std::nullopt,
            [this, id] { m_services.connection.send(protocol::deleteFriendRequest(id)); },
            nullptr
        );
    }

    void SocialWindow::hideFriendControls() {
        for (const Row& row : m_rows) {
            row.name.get().setVisible(false);
            row.nickname.get().setVisible(false);
            row.guild.get().setVisible(false);
            row.career.get().setVisible(false);
            row.check.get().setVisible(false);
            row.mail.get().setVisible(false);
            row.mailGray.get().setVisible(false);
        }
    }

    void SocialWindow::setTabControls(Tab tab) {
        m_tab = tab;
        const bool friends = tab == Tab::Friend;
        m_screen.get<ui::ImageButton>("SocialTabFriend").setSelected(friends);
        m_screen.get<ui::ImageButton>("SocialTabEmote").setSelected(tab == Tab::Emote);
        m_screen.get<ui::ImageButton>("SocialTabSetup").setSelected(tab == Tab::Setup);
        for (const char* name : {"FriendSendMail", "FriendDelete", "FriendPreviousPage", "FriendNextPage"}) {
            m_screen.get<ui::Element>(name).setVisible(friends);
        }
        for (ui::Element& control : m_emoteControls) {
            control.setVisible(tab == Tab::Emote);
        }
        for (ui::Element& control : m_setupControls) {
            control.setVisible(tab == Tab::Setup);
        }
        if (!friends) {
            hideFriendControls();
        }
        m_window.setPicture(friends ? "Form_Friend_2" : tab == Tab::Emote ? "Form_Emotion_1" : "Form_setup_1", m_services.pictures);
    }

    void SocialWindow::showFriendTab() {
        m_edited = false;
        setTabControls(Tab::Friend);
        m_page = 1;
    }

    void SocialWindow::showEmoteTab() {
        m_edited = false;
        setTabControls(Tab::Emote);
    }

    void SocialWindow::showSetupTab() {
        setTabControls(Tab::Setup);
        m_savedNickname = m_nickname.text();
        for (std::size_t i = 0; i < m_birthday.size(); ++i) {
            m_savedBirthday[i] = m_birthday[i].get().text();
        }
        m_savedBlood = m_blood;
        m_edited = false;
        for (std::size_t i = 1; i < m_birthday.size(); ++i) {
            if (m_birthday[i].get().text() == "0") {
                m_birthday[i].get().setText("");
            }
        }
    }

    void SocialWindow::loadHotkeys(const std::filesystem::path& file) {
        m_hotkeyFile = file;
        const bool fromFile = m_hotkeys.load(file);
        for (int slot = 1; slot <= EmoteHotkeys::kSlots; ++slot) {
            showSlot(slot);
        }
        for (int i = 1; i <= EmoteHotkeys::kPhrases; ++i) {
            m_phrases[static_cast<std::size_t>(i - 1)].get().setText(m_hotkeys.phrase(i));
        }
        if (fromFile) {
            showHeartRadar();
        }
    }

    void SocialWindow::saveHotkeys() {
        if (!m_hotkeyFile.empty()) {
            m_hotkeys.save(m_hotkeyFile);
        }
    }

    void SocialWindow::showSlot(int slot) {
        ui::AnimatedIcon& icon = m_slots[static_cast<std::size_t>(slot - 1)];
        const EmoteSlot& content = m_hotkeys.slot(slot);
        const int x = kSlotX[static_cast<std::size_t>(slot)];
        std::string picture;
        if (content.kind == EmoteSlot::Kind::Emotion) {
            picture = "E" + std::to_string(content.number + kEmotionBase);
            icon.setPosition({x, kSlotY});
        } else {
            picture = content.kind == EmoteSlot::Kind::Action ? "btn_Action_" + std::to_string(content.number) : "";
            icon.setPosition({x + 1, kSlotY + 1});
        }
        icon.setPicture(picture.empty() ? gfx::PictureId{} : m_services.pictures.find(picture).value_or(gfx::PictureId{}));
    }

    void SocialWindow::pickEmotion(int number) {
        m_picked = {EmoteSlot::Kind::Emotion, number};
        m_hooks.playEmotion(number);
        saveHotkeys();
    }

    void SocialWindow::pickAction(int number) {
        m_picked = {EmoteSlot::Kind::Action, number};
        m_hooks.playAction(number);
        saveHotkeys();
    }

    void SocialWindow::clickSlot(int slot) {
        if (m_picked.kind != EmoteSlot::Kind::Empty) {
            m_hotkeys.setSlot(slot, m_picked);
            m_picked = {};
            showSlot(slot);
        }
        saveHotkeys();
        useSlot(slot);
    }

    void SocialWindow::useSlot(int slot) {
        const EmoteSlot& content = m_hotkeys.slot(slot);
        if (content.kind == EmoteSlot::Kind::Emotion) {
            m_hooks.playEmotion(content.number);
        } else if (content.kind == EmoteSlot::Kind::Action) {
            m_hooks.playAction(content.number);
        }
    }

    void SocialWindow::usePhrase(int index) {
        const std::string& text = m_phrases[static_cast<std::size_t>(index - 1)].get().text();
        if (!text.empty()) {
            m_hooks.putPhrase(text);
        }
    }

    void SocialWindow::loadProfile(const protocol::OwnPlayerRecord& record) {
        m_playerId = record.playerId;
        m_nameLength = record.name.size();
        m_birthYear = record.birthYear;
        m_birthMonth = record.birthMonth;
        m_birthDay = record.birthDay;
        m_profileChoice = record.profileChoice;
        m_savedNickname = record.nickname;
        m_nickname.setText(record.nickname);
        m_birthday[0].get().setText(
            std::to_string(record.birthYear != 0 ? record.birthYear + 1900 : 1900)
        ); // the record stores the year minus 1900; 1900 is the year shown when unset
        m_birthday[1].get().setText(record.birthMonth != 0 ? std::to_string(record.birthMonth) : "");
        m_birthday[2].get().setText(record.birthDay != 0 ? std::to_string(record.birthDay) : "");
        m_blood = record.profileChoice % 5; // the blood type is the profile choice modulo 5 (0 none, 1..4)
        showBlood();
        showSunSign();
        setOnlineStatus(false);
    }

    void SocialWindow::layoutProfileButtons() {
        if (m_nickname.text() != m_savedNickname) {
            m_edited = true;
        }
        m_screen.get<ui::Element>("SetupOk").setVisible(m_edited);
        m_screen.get<ui::Element>("SetupCancel").setVisible(m_edited);
        m_screen.get<ui::Element>("SocialClose").setVisible(!m_edited);
    }

    void SocialWindow::setOnlineStatus(bool busy) {
        m_busy = busy;
        const auto picture = [this](bool checked) { return m_services.pictures.find(checked ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}); };
        m_screen.get<ui::ImageButton>("SetupOnline").setPicture(picture(!busy));
        m_screen.get<ui::ImageButton>("SetupBusy").setPicture(picture(busy));
        m_services.connection.send(protocol::onlineStatusRequest(busy ? 1 : 0));
    }

    int SocialWindow::birthdayField(int index) const {
        const std::string& text = m_birthday[static_cast<std::size_t>(index)].get().text();
        return text.empty() ? -1 : std::stoi(text);
    }

    void SocialWindow::clampBirthday() {
        const std::time_t now = std::time(nullptr);
        std::tm today{};
#ifdef _WIN32
        localtime_s(&today, &now);
#else
        localtime_r(&now, &today);
#endif
        const int thisYear = today.tm_year + 1900; // tm_year counts from 1900
        const int thisMonth = today.tm_mon + 1;
        const int thisDay = today.tm_mday;
        const auto set = [this](int index, int value) { m_birthday[static_cast<std::size_t>(index)].get().setText(std::to_string(value)); };
        const int year = birthdayField(0);
        if (year > thisYear) {
            set(0, thisYear);
        }
        // a four-digit year (from 1000) below 1900 becomes 1900; the month goes up to 12
        if (birthdayField(0) >= 1000 && birthdayField(0) < 1900) {
            set(0, 1900);
        }
        if (birthdayField(1) > 12) {
            set(1, 12);
        }
        const int month = birthdayField(1);
        const int day = birthdayField(2);
        if (month >= 0 && day >= 0) {
            // month numbers with 31 days, with 30 days, and February (29 days in a leap year, 28 otherwise)
            const bool longMonth = month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12;
            const bool shortMonth = month == 4 || month == 6 || month == 9 || month == 11;
            if (longMonth && day > 31) {
                set(2, 31);
            }
            if (shortMonth && day > 30) {
                set(2, 30);
            }
            if (month == 2 && birthdayField(0) >= 0) {
                const int most = birthdayField(0) % 4 == 0 ? 29 : 28; // days in February: 29 in a year divisible by 4, else 28
                if (birthdayField(2) > most) {
                    set(2, most);
                }
            }
        }
        if (birthdayField(0) == thisYear && birthdayField(1) >= 0) {
            if (birthdayField(1) > thisMonth) {
                set(1, thisMonth);
            }
            if (birthdayField(1) == thisMonth && birthdayField(2) > thisDay) {
                set(2, thisDay);
            }
        }
    }

    void SocialWindow::showSunSign() {
        m_sunSign = birthdayField(0) > 0 && birthdayField(1) > 0 && birthdayField(2) > 0 ? sunSign(birthdayField(0), birthdayField(1), birthdayField(2)) : 0;
        const std::string picture = m_sunSign != 0 ? "SunSign_" + std::to_string(m_sunSign) + "_1" : "";
        m_screen.get<ui::ImageView>("SetupSunSign")
            .setPicture(picture.empty() ? gfx::PictureId{} : m_services.pictures.find(picture).value_or(gfx::PictureId{}));
    }

    void SocialWindow::selectBlood(int index) {
        m_blood = m_blood == index ? 0 : index;
        showBlood();
        m_edited = true;
    }

    void SocialWindow::showBlood() {
        for (int i = 1; i <= static_cast<int>(m_bloods.size()); ++i) {
            m_bloods[static_cast<std::size_t>(i - 1)].get().setPicture(
                m_services.pictures.find(i == m_blood ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{})
            );
        }
    }

    void SocialWindow::toggleHeartRadar() {
        m_hotkeys.setHeartRadar(!m_hotkeys.heartRadar());
        showHeartRadar();
        saveHotkeys();
    }

    void SocialWindow::showHeartRadar() {
        m_screen.get<ui::ImageButton>("SetupHeartRadar")
            .setPicture(m_services.pictures.find(m_hotkeys.heartRadar() ? "Btn_on_1" : "Btn_off_1").value_or(gfx::PictureId{}));
    }

    bool SocialWindow::profileComplete() const { return m_blood != 0 && birthdayField(0) >= 0 && birthdayField(1) >= 0 && birthdayField(2) >= 0; }

    void SocialWindow::askSubmitProfile() {
        // box options: lines of 32 characters
        m_talk.ask(
            "check your information again, systme will offer you a gift, you should make sure there is a available case.",
            std::nullopt,
            [this] { submitProfile(); },
            nullptr,
            {32, 0, nullptr, false}
        );
    }

    void SocialWindow::submitProfile() {
        for (ui::TextBox& field : m_birthday) {
            if (field.text().empty()) {
                field.setText("0");
            }
        }
        if (!m_nickname.text().empty() && !isNameClean(m_nickname.text(), isGmId(m_playerId))) {
            m_messages.show(kIllegalWords, 2000); // message time in ms (2000 = 2 s)
            return;
        }
        if (m_nickname.text() != m_savedNickname) {
            m_savedNickname = m_nickname.text();
            m_services.connection.send(protocol::nicknameRequest(m_savedNickname));
        }
        for (std::size_t i = 0; i < m_birthday.size(); ++i) {
            m_savedBirthday[i] = m_birthday[i].get().text();
        }
        m_savedBlood = m_blood;
        if (!profileComplete()) {
            m_messages.show("You haven't finished the form", 2000); // message time in ms (2000 = 2 s)
            return;
        }
        // 1900 is the earliest year; 255 is the heart number of a year before it
        m_heartNumber =
            birthdayField(0) >= 1900 ? heartRadarNumber(m_birthYear, m_birthMonth, m_birthDay, m_sunSign, m_profileChoice, m_playerId, m_nameLength) : 255;
        m_profileChoice = m_blood;
        m_birthMonth = birthdayField(1);
        m_birthDay = birthdayField(2);
        if (birthdayField(0) >= 1900) {
            m_birthYear = birthdayField(0) - 1900; // the year is sent minus 1900
            m_services.connection.send(protocol::profileRequest(m_heartNumber, m_profileChoice, m_birthYear, m_birthMonth, m_birthDay));
        }
        showSunSign();
        m_edited = false;
        m_services.connection.send(protocol::profileDoneRequest());
    }

    void SocialWindow::revertProfile() {
        m_nickname.setText(m_savedNickname);
        for (std::size_t i = 0; i < m_birthday.size(); ++i) {
            m_birthday[i].get().setText(m_savedBirthday[i]);
        }
        m_blood = m_savedBlood;
        showBlood();
        m_edited = false;
    }

    void SocialWindow::paintHeartRadar(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (!m_hotkeys.heartRadar()) {
            return;
        }
        if (!profileComplete()) {
            m_hotkeys.setHeartRadar(false);
            showHeartRadar();
            return;
        }
        // 1900 is the earliest year; 255 is the heart number of a year before it
        const int radar =
            birthdayField(0) >= 1900 ? heartRadarNumber(m_birthYear, m_birthMonth, m_birthDay, m_sunSign, m_profileChoice, m_playerId, m_nameLength) : 255;
        const auto distance = m_hooks.nearestHeart(radar);
        if (!distance) {
            return;
        }
        // the heart stays down 40 ms; the jump interval is half the distance plus 40 ms
        if (m_heartFrame == 1 && nowMs - m_heartMs >= 40) {
            m_heartFrame = 0;
        }
        if (nowMs - m_heartMs >= static_cast<std::uint64_t>(*distance / 2 + 40)) {
            m_heartMs = nowMs;
            m_heartFrame = 1;
        }
        if (core::Rect{715, 550, 30, 45}.contains(m_screen.mouse())) { // screen rect (x, y, width, height) where the heart hides under the mouse
            return;
        }
        if (const auto heart = m_services.pictures.find("icon_Jumpheart_1")) {
            const gfx::Picture& picture = m_services.pictures.picture(*heart);
            const int height = picture.height() / 2; // the picture holds two frames, up and down
            canvas.drawPicture(*heart, at, core::Rect{0, m_heartFrame * height, picture.width(), height});
        }
    }

}
