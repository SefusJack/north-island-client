#include "client/viewmodels/items/InsteadEcisWindow.h"

#include "client/rules/TargetNames.h"
#include "client/data/PuppetTexts.h"
#include "gfx/TextStyle.h"
#include "protocol/PuppetRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <utility>

namespace client {

    namespace {

        constexpr int kBarUnits = 108; // full exp bar width, px
        constexpr int kDigitWidth = 8; // width of one digit of the exp number, px
        constexpr int kFaceSlot = 1; // portrait slot used when drawing the partner
        constexpr int kMessageMs = 2000; // how long a player message stays up, ms
        constexpr int kLeastLevel = 10; // lowest player level that may use puppet training
        constexpr int kBagSlots = BagMarks::kSlots;
        constexpr int kRefuses = 6; // refuse reason 6: the helper refuses (packet 23/104 option 3)
        constexpr int kNoAnswer = 7; // refuse reason 7: no answer in time
        constexpr int kBusy = 17; // refuse reason 17: helper is busy (in combat)
        constexpr int kFirstStepScrolls = 2; // Strong Scrolls needed for the first capacity step
        constexpr int kNextStepScrolls = 3; // Strong Scrolls needed for every later capacity step
        constexpr int kMostSteps = 10; // most capacity expansion steps
        constexpr std::int64_t kStepExp = 500000; // exp capacity gained per step

        /// @brief Loads InsteadEcisWindow.xml and returns its window.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("InsteadEcisWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("InsteadEcis");
        }

    }

    InsteadEcisWindow::InsteadEcisWindow(
        GameServices& services,
        ui::Screen& screen,
        const FriendList& friends,
        const Guild& guild,
        const Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        PuppetState& state,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_state(state),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_names(screen.get<ui::ListBox>("InsteadEcisNames")),
          m_name(screen.get<ui::TextBox>("InsteadEcisName")),
          m_friendsTick(screen.get<ui::CheckBox>("InsteadEcisFriends")),
          m_guildTick(screen.get<ui::CheckBox>("InsteadEcisGuild")),
          m_useButton(screen.get<ui::ImageButton>("InsteadEcisUse")),
          m_expandButton(screen.get<ui::ImageButton>("InsteadEcisExpand")),
          m_cancelButton(screen.get<ui::ImageButton>("InsteadEcisCancel")),
          m_timeButton(screen.get<ui::ImageButton>("InsteadEcisTime")),
          m_mailButton(screen.get<ui::ImageButton>("InsteadEcisMail")),
          m_barSpot(screen.get<ui::Spot>("InsteadEcisBar")),
          m_expSpot(screen.get<ui::Spot>("InsteadEcisExp")),
          m_nameSpot(screen.get<ui::Spot>("InsteadEcisPartnerName")),
          m_onlineSpot(screen.get<ui::Spot>("InsteadEcisOnline")),
          m_faceSpot(screen.get<ui::Spot>("InsteadEcisPortrait")),
          m_textSpot(screen.get<ui::Spot>("InsteadEcisText")) {
        screen.get<ui::DrawingArea>("InsteadEcisPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onInsteadEcisClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onInsteadEcisOk", [this](const ui::EventArgs&) { invite(); });
        screen.on("onInsteadEcisUp", [this](const ui::EventArgs&) { m_names.scrollUp(); });
        screen.on("onInsteadEcisDown", [this](const ui::EventArgs&) { m_names.scrollDown(); });
        screen.on("onInsteadEcisFilter", [this](const ui::EventArgs&) { fillNames(); });
        screen.on("onInsteadEcisPick", [this](const ui::EventArgs& args) { pick(args.index); });
        screen.on("onInsteadEcisTime", [this](const ui::EventArgs&) { askTimeLeft(); });
        screen.on("onInsteadEcisExpand", [this](const ui::EventArgs&) { askExpand(); });
        screen.on("onInsteadEcisUse", [this](const ui::EventArgs&) { askUseExp(); });
        screen.on("onInsteadEcisCancel", [this](const ui::EventArgs&) { askCancel(); });
        screen.on("onInsteadEcisMail", [this](const ui::EventArgs&) { m_hooks.mailTo(m_partner); });
        screen.on("onInsteadEcisFace", [this](const ui::EventArgs&) {
            if (m_state.mode != 0 && m_partnerOnline) {
                m_hooks.whisper(m_partner.id);
            }
        });
        m_mailButton.setVisible(false);
        m_cancelButton.setVisible(false);
    }

    void InsteadEcisWindow::releaseMark() {
        if (m_bagSlot >= 1 && m_bagSlot <= kBagSlots && m_marks.useMark(m_bagSlot) == kMark) {
            m_marks.setUseMark(m_bagSlot, 0);
            m_hooks.marksChanged();
        }
    }

    void InsteadEcisWindow::markBusy(int bagSlot) {
        if (bagSlot >= 1 && bagSlot <= kBagSlots && m_marks.busyKind(bagSlot) == 0) {
            m_marks.setBusyKind(bagSlot, kBusyKind);
            m_hooks.marksChanged();
        }
    }

    void InsteadEcisWindow::close() {
        m_window.setVisible(false);
        if (!m_requestSent) {
            releaseMark();
        }
    }

    void InsteadEcisWindow::setName(const std::string& name) {
        if (m_window.isVisible()) {
            m_name.setText(name);
        }
    }

    void InsteadEcisWindow::use(int bagSlot) {
        if (m_state.mode != 0 || m_window.isVisible()) {
            m_messages.show("Unable to use", kMessageMs);
            return;
        }
        if (bagSlot < 1 || bagSlot > kBagSlots) {
            return;
        }
        if (m_requestSent) {
            m_messages.show("Waiting for reactions, cannot use", kMessageMs);
            return;
        }
        m_bagSlot = bagSlot;
        fillNames();
        m_useButton.setVisible(true);
        m_expandButton.setVisible(true);
        if (m_marks.useMark(bagSlot) == 0) {
            m_marks.setUseMark(bagSlot, kMark);
            m_hooks.marksChanged();
        }
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void InsteadEcisWindow::showTraining() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.send(protocol::puppetOpenRequest());
    }

    void InsteadEcisWindow::fillNames() {
        m_names.clear();
        for (const std::string& name : targetNames(m_friends, m_guild, m_hooks.ownId(), m_friendsTick.isChecked(), m_guildTick.isChecked())) {
            m_names.addItem(name);
        }
    }

    void InsteadEcisWindow::pick(int row) {
        if (row < 0 || row >= m_names.itemCount()) {
            return;
        }
        const std::string& name = m_names.itemText(row);
        if (!m_hooks.playerNamed(name)) {
            m_messages.show("The player isn't online", 1500); // message duration in ms
            return;
        }
        m_name.setText(name);
    }

    void InsteadEcisWindow::invite() {
        if (m_hooks.ownLevel() < kLeastLevel) {
            m_messages.show("Must be Lvl 10+ to perform puppet training", kMessageMs);
            return;
        }
        if (!m_hooks.enoughOnlineTime()) {
            m_messages.show("Your time Online is not enough, cannot carry out the puppet training", kMessageMs);
            return;
        }
        if (m_state.mode != 0) {
            m_messages.show("Has the play for you treaty, can't transfer", kMessageMs);
            return;
        }
        if (m_hooks.inFight()) {
            m_messages.show("In combat now, cannot send the requirement about using puppet training", kMessageMs);
            return;
        }
        const std::string name = m_name.text();
        if (name.empty()) {
            m_messages.show("Enter the name", kMessageMs);
            return;
        }
        if (name == m_hooks.ownName()) {
            m_messages.show("You can't use it", kMessageMs);
            return;
        }
        const auto target = m_hooks.playerNamed(name);
        if (!target) {
            m_messages.show("Can't find players according to the name", kMessageMs);
            return;
        }
        if (m_exp != 0) {
            m_messages.show("You are unable to ask for help to train due to the lack of exp in the Memory Card.", kMessageMs);
            return;
        }
        if (m_inventory.bagItem(m_bagSlot).itemId != kMemoryCard) {
            return;
        }
        if (m_requestSent) {
            m_messages.show("Please send your requirements later", kMessageMs);
            return;
        }
        m_hooks.send(protocol::puppetInviteRequest(*target, m_bagSlot));
        m_requestSent = true;
    }

    void InsteadEcisWindow::invitation(const protocol::PuppetPacket& packet) {
        if (m_hooks.ownId() != packet.id) {
            return;
        }
        const std::int32_t askerId = packet.other;
        const auto refuse = [this, askerId](int reason) { m_hooks.send(protocol::puppetRefuseRequest(reason, askerId)); };
        if (m_state.mode == 0 && m_exp != 0) {
            m_messages.show("You are unable to agree to help to train due to the lack of exp in the Memory Card.", kMessageMs);
            refuse(kRefuses);
            return;
        }
        if (!m_hooks.playerKnown(askerId)) {
            m_messages.show("The player isn't online", 1500); // message duration in ms
            return;
        }
        m_hooks.invited(
            askerId,
            "Invite you to help play",
            [this, askerId, refuse] {
                if (!m_hooks.playerKnown(askerId)) {
                    m_messages.show("The player isn't online", 1500); // message duration in ms
                    return;
                }
                if (m_state.mode == 0 && m_exp != 0) {
                    m_messages.show("You are unable to agree to help to train due to the lack of exp in the Memory Card.", kMessageMs);
                    refuse(kRefuses);
                    return;
                }
                if (m_hooks.inFight()) {
                    m_messages.show("In combat now, cannot send the requirement about using puppet training", kMessageMs);
                    refuse(kBusy);
                    return;
                }
                m_hooks.send(protocol::puppetAcceptRequest(askerId));
            },
            [refuse] { refuse(kRefuses); },
            [refuse] { refuse(kNoAnswer); }
        );
    }

    void InsteadEcisWindow::setPartner(const protocol::PuppetPerson& person) {
        m_partner = person;
        m_partnerLook = CharacterLook::fromColorCodes(person.bodyType, person.head, person.colorCode1, person.colorCode5, m_services.items);
    }

    void InsteadEcisWindow::showPartnerControls(bool partnered) {
        m_mailButton.setVisible(partnered);
        m_cancelButton.setVisible(partnered);
        m_timeButton.setVisible(partnered);
    }

    void InsteadEcisWindow::partnerSet(const protocol::PuppetPacket& packet) {
        if (m_hooks.ownId() != packet.id || m_inventory.bagItem(packet.slot).itemId != kMemoryCard) {
            return;
        }
        auto person = m_hooks.worldPerson(packet.other);
        if (!person) {
            return;
        }
        person->colorCode1 = packet.person.colorCode1;
        person->colorCode5 = packet.person.colorCode5;
        setPartner(*person);
        m_partnerOnline = true;
        showPartnerControls(true);
        m_useButton.setVisible(false);
        m_expandButton.setVisible(false);
        markBusy(packet.slot);
        m_state.mode = 1;
        m_requestSent = false;
    }

    void InsteadEcisWindow::partnerInfo(const protocol::PuppetPacket& packet) {
        if (m_hooks.ownId() != packet.id) {
            return;
        }
        setPartner(packet.person);
        m_partnerOnline = packet.online;
        m_exp = packet.exp;
        showPartnerControls(true);
        m_useButton.setVisible(false);
        m_expandButton.setVisible(false);
        sizeBar();
    }

    void InsteadEcisWindow::sizeBar() {
        if (m_exp >= m_capacity) {
            m_exp = m_capacity;
            m_barWidth = kBarUnits;
        } else {
            const std::int64_t unit = m_capacity / kBarUnits;
            m_barWidth = unit > 0 ? static_cast<int>(m_exp / unit) : 0;
        }
        m_digits = static_cast<int>(std::to_string(m_exp).size());
    }

    void InsteadEcisWindow::askExpand() {
        if (m_state.capacityStep + 1 > kMostSteps) {
            m_messages.show(kPuppetSizeLimit, kMessageMs);
            return;
        }
        const int scrolls = m_state.capacityStep > 0 ? kNextStepScrolls : kFirstStepScrolls;
        m_hooks.askExpand(
            std::string(kPuppetExpandTo) + std::to_string(m_capacity + kStepExp) + kPuppetNeed + std::to_string(scrolls) + kPuppetScrollsSure,
            [this] { confirmExpand(); }
        );
    }

    void InsteadEcisWindow::confirmExpand() {
        int message = 0;
        if (m_state.mode != 0) {
            message = 1;
        }
        if (m_state.capacityStep + 1 > kMostSteps) {
            message = 2;
        }
        int count = 0;
        bool enough = false;
        for (int slot = 1; slot <= kBagSlots && !enough; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId != kStrongScroll || m_marks.useMark(slot) != 0 || !m_marks.isAvailable(slot)) {
                continue;
            }
            count += item.count;
            enough = count >= (m_state.capacityStep == 0 ? kFirstStepScrolls : kNextStepScrolls);
        }
        if (!enough) {
            message = 3; // message code: not enough Strong Scrolls
        }
        if (message == 1) {
            m_messages.show(kPuppetInTraining, kMessageMs);
        } else if (message == 2) {
            m_messages.show(kPuppetCardFull, kMessageMs);
        } else if (message == 3) { // message code: not enough Strong Scrolls
            m_messages.show(kPuppetNotEnoughScrolls, kMessageMs);
        }
        if (message == 0) {
            m_hooks.send(protocol::puppetGrowRequest());
        }
    }

    void InsteadEcisWindow::askUseExp() {
        m_hooks.ask("At present, there are #R" + std::to_string(m_exp) + "/#RExp\rWanna use?", [this] { useExp(); });
    }

    void InsteadEcisWindow::useExp() {
        if (m_exp == 0) {
            m_messages.show("Exp is 0, can't use", kMessageMs);
            return;
        }
        if (m_state.mode != 0) {
            m_messages.show("Can't use in play for you mode", kMessageMs);
            return;
        }
        m_hooks.send(protocol::puppetUseExpRequest());
    }

    void InsteadEcisWindow::askCancel() {
        m_hooks.ask("Cancel play for you", [this] { cancel(); });
    }

    void InsteadEcisWindow::cancel() {
        if (m_hooks.inFight()) {
            m_messages.show("In battle, cannot pause puppet training", kMessageMs);
            return;
        }
        if (!m_hooks.playerKnown(m_partner.id)) {
            m_messages.show(m_partnerOnline ? "The target is on the different branch of the server." : "The target is offline.", kMessageMs);
            return;
        }
        m_hooks.send(protocol::puppetAskStopRequest());
    }

    void InsteadEcisWindow::askTimeLeft() {
        if (m_state.mode != 0) {
            m_hooks.send(protocol::puppetTimeLeftRequest(1));
        }
    }

    void InsteadEcisWindow::timeLeft(int minutes) {
        m_messages.show("[" + m_partner.name + "]Puppet training time left:\r" + puppetDuration(minutes), kMessageMs);
    }

    void InsteadEcisWindow::result(int code) {
        static const char* const kTexts[] = {
            "Doesn't have memory card",
            "Trading",
            "Running a stall",
            "Name is in the blacklist",
            "Event...",
            "He is in the event",
            "He refuses to play for you",
            "Refuse to response",
            "Can't help you play",
            "The target is offline.",
            "Cancelled the play for you",
            "Cancelled the play for you",
            "Ask for play for you",
            "Ask for play for you...",
            "have past 10 days, the relation is removed.",
            "Busy...",
            "You are forced to stop your training.",
            "The target is busy",
            "Puppet training send successful",
            "Finished puppet training successfully",
            "Spouse Must be Lvl 10+ to perform puppet training",
            "Spouse Your time Online is not enough, cannot carry out the puppet training"
        };
        if (code >= 0 && code < static_cast<int>(sizeof(kTexts) / sizeof(kTexts[0]))) {
            m_messages.show(kTexts[code], kMessageMs);
        }
        m_requestSent = false;
    }

    void InsteadEcisWindow::released(int bagSlot) {
        if (bagSlot < 1 || bagSlot > kBagSlots) {
            return;
        }
        if (!m_window.isVisible() && m_marks.useMark(bagSlot) == kMark) {
            m_marks.setUseMark(bagSlot, 0);
        }
        if (m_state.mode == 0 && m_marks.busyKind(bagSlot) == kBusyKind) {
            m_marks.setBusyKind(bagSlot, 0);
        }
        m_hooks.marksChanged();
    }

    void InsteadEcisWindow::received(const protocol::PuppetPacket& packet) {
        switch (packet.option) {
            case 1: // 23/104/1: invitation to help play
                invitation(packet);
                break;
            case 2: // 23/104/2: the helper is set
                partnerSet(packet);
                break;
            case 3: // 23/104/3: training result code
                result(packet.code);
                break;
            case 4: // 23/104/4: training state (code 1 requester, 2 helper, 3 card exp)
                if (packet.code == 1 || packet.code == 2) {
                    markBusy(packet.slot);
                    m_state.mode = packet.code;
                } else if (packet.code == 3) {
                    m_exp = std::min<std::int64_t>(packet.exp, m_capacity);
                    const std::int64_t unit = m_capacity / kBarUnits;
                    m_barWidth = unit > 0 ? static_cast<int>(std::min<std::int64_t>(m_exp / unit, kBarUnits)) : 0;
                }
                break;
            case 7: // 23/104/7: reset the form
                if (m_hooks.ownId() == packet.id) {
                    reset();
                }
                break;
            case 12: // 23/104/12: the card's exp
                if (m_hooks.ownId() == packet.id) {
                    m_exp = packet.exp;
                    sizeBar();
                }
                break;
            case 14: // 23/104/14: partner and exp info (form opened from the status icon)
                partnerInfo(packet);
                break;
            case 15: // 23/104/15: the requester's partner went offline
                if (m_hooks.ownId() == packet.id && m_state.mode == 1 && m_partner.id == packet.other) {
                    m_partnerOnline = false;
                }
                break;
            case 18: // 23/104/18: the partner is online
                m_partnerOnline = true;
                showPartnerControls(true);
                break;
            case 19: // 23/104/19: the card was released
                if (m_hooks.ownId() == packet.id) {
                    released(packet.slot);
                }
                break;
            case 20: // 23/104/20: the card is in use
                markBusy(packet.slot);
                break;
            case 21: // 23/104/21: training time left in minutes (code)
                if (m_state.mode == 1) {
                    timeLeft(packet.code);
                }
                break;
            case 22: // 23/104/22: the card's size (step)
                if (m_hooks.ownId() == packet.id) {
                    m_state.capacityStep = packet.step;
                    m_capacity = (packet.step + 1) * kStepExp;
                    if (packet.code == 1) {
                        m_messages.show(std::string(kPuppetSizeIs) + std::to_string(m_capacity), kMessageMs);
                    }
                }
                break;
            default: // other options (e.g. 16): ignored
                break;
        }
    }

    void InsteadEcisWindow::end(int bagSlot) {
        m_state.mode = 0;
        m_partner = protocol::PuppetPerson{};
        m_partnerOnline = false;
        m_mailButton.setVisible(false);
        m_useButton.setVisible(true);
        m_expandButton.setVisible(true);
        m_cancelButton.setVisible(false);
        m_timeButton.setVisible(false);
        if (bagSlot >= 1 && bagSlot <= kBagSlots) {
            if (m_marks.busyKind(bagSlot) == kBusyKind) {
                m_marks.setBusyKind(bagSlot, 0);
            }
            if (m_marks.useMark(bagSlot) == kMark) {
                m_marks.setUseMark(bagSlot, 0);
            }
            m_hooks.marksChanged();
        }
        close();
    }

    void InsteadEcisWindow::clearPartner() {
        m_partner.name.clear();
        m_partner.id = 0;
        m_partnerOnline = false;
    }

    void InsteadEcisWindow::reset() {
        m_exp = 0;
        m_barWidth = 0;
        m_digits = 0;
        m_partner = protocol::PuppetPerson{};
        m_partnerOnline = false;
        showPartnerControls(false);
        m_requestSent = false;
    }

    void InsteadEcisWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const gfx::TextStyle plain{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        if (const auto bar = m_services.pictures.find("Color_G")) {
            canvas.drawPicture(*bar, {at.x + m_barSpot.at().x, at.y + m_barSpot.at().y}, core::Rect{0, 0, m_barWidth, m_barSpot.rect().height});
        }
        sizeBar();
        // text box width 80 and height 15 pixels of the exp numbers and the partner's name
        canvas.drawText({at.x + m_expSpot.at().x, at.y + m_expSpot.at().y}, std::to_string(m_exp), plain, 80, 15);
        canvas.drawText({at.x + m_expSpot.at().x + m_digits * kDigitWidth, at.y + m_expSpot.at().y}, "/" + std::to_string(m_capacity), plain, 80, 15);
        canvas.drawText({at.x + m_nameSpot.at().x, at.y + m_nameSpot.at().y}, m_partner.name, plain, 80, 15);
        if (m_state.mode != 1) {
            return;
        }
        if (const auto status = m_services.pictures.find(m_partnerOnline ? "icon_online_1" : "icon_offline_1")) {
            canvas.drawPicture(*status, {at.x + m_onlineSpot.at().x, at.y + m_onlineSpot.at().y});
        }
        if (m_partner.id != 0) {
            const int friendIndex = m_friends.indexOf(m_partner.id);
            const int memberIndex = m_guild.indexOf(m_partner.id);
            if (friendIndex != 0) {
                const protocol::FriendRecord& record = m_friends.at(friendIndex).record;
                m_partnerLook = CharacterLook::fromColorCodes(m_partner.bodyType, m_partner.head, record.colorCode1, record.colorCode5, m_services.items);
            } else if (memberIndex >= 0) {
                const protocol::GuildMemberRecord& record = m_guild.at(memberIndex).record;
                m_partnerLook = CharacterLook::fromColorCodes(m_partner.bodyType, m_partner.head, record.colorCode1, record.colorCode5, m_services.items);
            }
        }
        m_services.characters.drawPortrait(canvas, m_partnerLook, {at.x + m_faceSpot.at().x, at.y + m_faceSpot.at().y}, m_blink.frame(nowMs), kFaceSlot);
    }

}
