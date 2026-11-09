#include "client/viewmodels/items/MemCardWindow.h"

#include "client/data/PuppetTexts.h"
#include "gfx/TextStyle.h"
#include "protocol/PuppetRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kDigitRows = 10; // digits 0..9 stacked in the Num_White3_1 picture
        constexpr int kFaceSlot = 1; // portrait slot used when drawing the partner
        constexpr std::int64_t kStepExp = 500000; // exp capacity per card step
        constexpr int kBarUnits = 108; // exp bar divided into this many px units
        constexpr int kMessageMs = 2000; // how long a player message stays up, ms
        // job icon pictures, index = job 1..6 (index 0 unused): Assassin, Samurai, Knight, Sage, Clergyman, Prophet
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};

        /// @brief Loads MemCardWindow.xml into the screen.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MemCardWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MemCard");
        }

        /// @brief Looks up the three buttons of one kind.
        /// @param screen Screen holding the buttons.
        /// @param prefix Element name prefix; "1".."3" is appended.
        /// @return The three buttons.
        std::array<std::reference_wrapper<ui::ImageButton>, MemCardWindow::kCards> buttons(ui::Screen& screen, const std::string& prefix) {
            return {screen.get<ui::ImageButton>(prefix + "1"), screen.get<ui::ImageButton>(prefix + "2"), screen.get<ui::ImageButton>(prefix + "3")};
        }

    }

    MemCardWindow::MemCardWindow(
        GameServices& services,
        ui::Screen& screen,
        const FriendList& friends,
        const Guild& guild,
        PlayerMessages& messages,
        PuppetState& state,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_messages(messages),
          m_state(state),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_mailButtons(buttons(screen, "MemCardMail")),
          m_stopButtons(buttons(screen, "MemCardStop")),
          m_timeButtons(buttons(screen, "MemCardTime")),
          m_rowSpot(screen.get<ui::Spot>("MemCardRow")),
          m_jobSpot(screen.get<ui::Spot>("MemCardJob")),
          m_nameSpot(screen.get<ui::Spot>("MemCardPlayerName")),
          m_titleSpot(screen.get<ui::Spot>("MemCardPlayerTitle")),
          m_guildSpot(screen.get<ui::Spot>("MemCardGuildName")),
          m_barSpot(screen.get<ui::Spot>("MemCardBar")),
          m_levelWordSpot(screen.get<ui::Spot>("MemCardLevelWord")),
          m_levelDigitsSpot(screen.get<ui::Spot>("MemCardLevelDigits")),
          m_onlineSpot(screen.get<ui::Spot>("MemCardOnline")),
          m_elementSpot(screen.get<ui::Spot>("MemCardElement")),
          m_faceSpot(screen.get<ui::Spot>("MemCardPortrait")),
          m_textSpot(screen.get<ui::Spot>("MemCardText")),
          m_hintFillSpot(screen.get<ui::Spot>("MemCardHintFill")),
          m_hintBorderSpot(screen.get<ui::Spot>("MemCardHintBorder")),
          m_hintTextSpot(screen.get<ui::Spot>("MemCardHintText")) {
        screen.get<ui::DrawingArea>("MemCardPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.on("onMemCardClose", [this](const ui::EventArgs&) { close(); });
        for (int place = 0; place < kCards; ++place) {
            const std::string number = std::to_string(place + 1);
            screen.on("onMemCardMail" + number, [this, place](const ui::EventArgs&) { m_hooks.mailTo(m_cards[static_cast<std::size_t>(place)].person); });
            screen.on("onMemCardStop" + number, [this, place](const ui::EventArgs&) { askCancel(place); });
            screen.on("onMemCardTime" + number, [this, place](const ui::EventArgs&) {
                if (m_state.mode != 0) {
                    m_hooks.send(protocol::puppetTimeLeftRequest(place + 1));
                }
            });
            screen.on("onMemCardFace" + number, [this, place](const ui::EventArgs&) {
                const Card& card = m_cards[static_cast<std::size_t>(place)];
                if (card.online) {
                    m_hooks.whisper(card.person.id);
                }
            });
            showButtons(place, false);
        }
    }

    void MemCardWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void MemCardWindow::close() { m_window.setVisible(false); }

    std::optional<std::reference_wrapper<MemCardWindow::Card>> MemCardWindow::cardAt(int slot) {
        if (slot < 1 || slot > kCards) {
            return std::nullopt;
        }
        return std::ref(m_cards[static_cast<std::size_t>(slot - 1)]);
    }

    void MemCardWindow::showButtons(int place, bool shown) {
        m_stopButtons[static_cast<std::size_t>(place)].get().setVisible(shown);
        m_mailButtons[static_cast<std::size_t>(place)].get().setVisible(shown);
        m_timeButtons[static_cast<std::size_t>(place)].get().setVisible(shown);
    }

    void MemCardWindow::reset() {
        m_state.cancelRequesterId = 0;
        m_targetId = 0;
        for (int place = 0; place < kCards; ++place) {
            m_cards[static_cast<std::size_t>(place)] = Card{};
            showButtons(place, false);
        }
    }

    void MemCardWindow::setPerson(Card& card, const protocol::PuppetPerson& person) {
        card.person = person;
        card.look = CharacterLook::fromColorCodes(person.bodyType, person.head, person.colorCode1, person.colorCode5, m_services.items);
    }

    void MemCardWindow::sizeBar(Card& card) {
        card.barUnit = static_cast<int>((card.step + 1) * kStepExp / kBarUnits);
        card.barWidth = card.barUnit > 0 ? static_cast<int>(card.exp / card.barUnit) : 0;
    }

    void MemCardWindow::cardSet(const protocol::PuppetPacket& packet) {
        const auto found = cardAt(packet.slot);
        if (!found || m_hooks.ownId() != packet.id) {
            return;
        }
        Card& card = found->get();
        card.exp = packet.exp;
        card.online = packet.online;
        card.step = packet.step;
        setPerson(card, packet.person);
        sizeBar(card);
        showButtons(packet.slot - 1, true);
    }

    void MemCardWindow::cardExp(const protocol::PuppetPacket& packet) {
        const auto found = cardAt(packet.slot);
        if (!found || m_hooks.ownId() != packet.id) {
            return;
        }
        Card& card = found->get();
        if (packet.extended) {
            card.step = packet.step;
            card.barUnit = static_cast<int>((card.step + 1) * kStepExp / kBarUnits);
            if (auto person = m_hooks.worldPerson(packet.other)) {
                card.online = true;
                person->colorCode1 = packet.person.colorCode1;
                person->colorCode5 = packet.person.colorCode5;
                person->guild = packet.person.guild;
                setPerson(card, *person);
            }
        }
        card.exp = packet.exp;
        card.barWidth = card.barUnit > 0 ? static_cast<int>(card.exp / card.barUnit) : 0;
        showButtons(packet.slot - 1, true);
    }

    void MemCardWindow::cardCleared(int slot) {
        if (const auto found = cardAt(slot)) {
            found->get() = Card{};
            showButtons(slot - 1, false);
        }
    }

    void MemCardWindow::cardById(const protocol::PuppetPacket& packet) {
        const auto found = cardAt(packet.slot);
        if (!found || m_hooks.ownId() != packet.id) {
            return;
        }
        Card& card = found->get();
        card.exp = packet.exp;
        card.online = packet.online;
        card.step = packet.step;
        sizeBar(card);
        protocol::PuppetPerson person;
        if (const int friendIndex = m_friends.indexOf(packet.other); friendIndex != 0) {
            const protocol::FriendRecord& record = m_friends.at(friendIndex).record;
            person = {
                record.id,
                record.name,
                record.nickname,
                record.guild,
                record.level,
                record.job,
                record.element,
                record.bodyType,
                record.head,
                record.colorCode1,
                record.colorCode5
            };
        } else if (const int memberIndex = m_guild.indexOf(packet.other); memberIndex >= 0) {
            const protocol::GuildMemberRecord& record = m_guild.at(memberIndex).record;
            person = {
                record.id,
                record.name,
                record.nickname,
                m_guild.name(),
                record.level,
                record.job,
                record.element,
                record.bodyType,
                record.head,
                record.colorCode1,
                record.colorCode5
            };
        }
        setPerson(card, person);
        showButtons(packet.slot - 1, true);
    }

    void MemCardWindow::askCancel(int place) {
        if (m_hooks.inFight()) {
            m_messages.show("In battle, cannot pause puppet training", kMessageMs);
            return;
        }
        const Card& card = m_cards[static_cast<std::size_t>(place)];
        m_targetId = card.person.id;
        if (!m_hooks.playerKnown(m_targetId)) {
            m_targetId = 0;
            m_messages.show(card.online ? "The target is on the different branch of the server." : "The target is offline.", kMessageMs);
            return;
        }
        m_hooks.ask("Cancel play for you", [this] { cancel(); });
    }

    void MemCardWindow::cancel() {
        if (m_hooks.inFight()) {
            return;
        }
        if (!m_hooks.playerKnown(m_targetId)) {
            m_targetId = 0;
            m_messages.show("The target is offline.", kMessageMs);
            return;
        }
        m_hooks.send(protocol::puppetHelperAskStopRequest(m_targetId));
        m_targetId = 0;
    }

    void MemCardWindow::stopWanted(const protocol::PuppetPacket& packet) {
        if (m_hooks.ownId() != packet.other) {
            return;
        }
        const auto requester = m_hooks.worldPerson(packet.id);
        if (!requester) {
            m_messages.show("The player isn't online", 1500); // message display time in ms
            return;
        }
        m_state.cancelRequesterId = packet.id;
        m_hooks.request(
            requester->name + " Want to cancel play for  you",
            [this] {
                if (m_hooks.inFight()) {
                    return;
                }
                if (!m_hooks.playerKnown(m_state.cancelRequesterId)) {
                    m_messages.show("The player isn't online", 1500); // message display time in ms
                    return;
                }
                m_hooks.send(protocol::puppetHelperStopRequest(m_state.cancelRequesterId));
            },
            // refuse sends keep code 11, no answer sends keep code 13
            [this] { m_hooks.send(protocol::puppetHelperKeepRequest(11, m_state.cancelRequesterId)); },
            [this] { m_hooks.send(protocol::puppetHelperKeepRequest(13, m_state.cancelRequesterId)); }
        );
    }

    void MemCardWindow::helperStopWanted(const protocol::PuppetPacket& packet) {
        if (m_hooks.ownId() != packet.id) {
            return;
        }
        const auto helper = m_hooks.worldPerson(packet.other);
        if (!helper) {
            m_messages.show("The player isn't online", 1500); // message display time in ms
            return;
        }
        m_state.cancelRequesterId = packet.other;
        m_hooks.request(
            helper->name + " Want to cancel play for  you",
            [this] {
                if (m_hooks.inFight()) {
                    return;
                }
                if (!m_hooks.playerKnown(m_state.cancelRequesterId)) {
                    m_messages.show("The player isn't online", 1500); // message display time in ms
                    return;
                }
                m_hooks.clearPartner();
                m_hooks.send(protocol::puppetAgreeStopRequest());
            },
            // refuse sends keep code 10, no answer sends keep code 12
            [this] { m_hooks.send(protocol::puppetKeepRequest(10)); },
            [this] { m_hooks.send(protocol::puppetKeepRequest(12)); }
        );
    }

    void MemCardWindow::received(const protocol::PuppetPacket& packet) {
        switch (packet.option) {
            case 5: // 23/104 5: the requester wants to stop the helper's playing (RecvMemCardCancelRequest)
                stopWanted(packet);
                break;
            case 8: // 23/104 8: the exp of a card changed (RecvMemCardSlotExp)
                cardExp(packet);
                break;
            case 9: // 23/104 9: a card was cleared (RecvMemCardSlotCleared)
                if (m_hooks.ownId() == packet.id) {
                    cardCleared(packet.slot);
                }
                break;
            case 10: // 23/104 10: the helper wants to stop (RecvCancelRequest)
                helperStopWanted(packet);
                break;
            case 13: // 23/104 13: one card of the helper (RecvMemCardSlot)
                cardSet(packet);
                break;
            case 15: // 23/104 15: a card's player went offline (RecvTrainPartnerOffline)
                if (m_hooks.ownId() == packet.id && m_state.mode == 2) {
                    for (Card& card : m_cards) {
                        if (card.person.id == packet.other) {
                            card.online = false;
                        }
                    }
                }
                break;
            case 16: // 23/104 16: a card's player is on line at that level (RecvMemCardPartnerOnline)
                if (m_hooks.ownId() == packet.id) {
                    for (Card& card : m_cards) {
                        if (card.person.id == packet.other) {
                            card.online = true;
                            card.person.level = packet.code;
                        }
                    }
                }
                break;
            case 17: // 23/104 17: the card of a friend or guild member (RecvMemCardSlotById)
                cardById(packet);
                break;
            case 21: // 23/104 21: training time left (RecvTrainTimeLeft)
                if (m_state.mode == 2) {
                    if (const auto found = cardAt(packet.slot)) {
                        m_messages.show("[" + found->get().person.name + "]Puppet training time left:\r" + puppetDuration(packet.code), kMessageMs);
                    }
                }
                break;
            default: // other options are not for this form
                break;
        }
    }

    void MemCardWindow::refresh() {
        for (Card& card : m_cards) {
            if (card.person.id == 0) {
                continue;
            }
            if (const auto known = m_hooks.worldPerson(card.person.id)) {
                card.person.level = known->level;
                card.person.job = known->job;
                card.person.title = known->title;
            }
            if (const int friendIndex = m_friends.indexOf(card.person.id); friendIndex != 0) {
                const protocol::FriendRecord& record = m_friends.at(friendIndex).record;
                setPerson(
                    card,
                    {record.id,
                     record.name,
                     record.nickname,
                     record.guild,
                     record.level,
                     record.job,
                     record.element,
                     record.bodyType,
                     record.head,
                     record.colorCode1,
                     record.colorCode5}
                );
            } else if (const int memberIndex = m_guild.indexOf(card.person.id); memberIndex >= 0) {
                const protocol::GuildMemberRecord& record = m_guild.at(memberIndex).record;
                setPerson(
                    card,
                    {record.id,
                     record.name,
                     record.nickname,
                     m_guild.name(),
                     record.level,
                     record.job,
                     record.element,
                     record.bodyType,
                     record.head,
                     record.colorCode1,
                     record.colorCode5}
                );
            }
        }
    }

    void MemCardWindow::paintCard(gfx::Canvas& canvas, core::Point at, int place, std::uint64_t nowMs) {
        const Card& card = m_cards[static_cast<std::size_t>(place)];
        const gfx::TextStyle plain{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const int row = at.y + place * m_rowSpot.pitch().y;
        const auto draw = [&](const char* name, core::Point origin) {
            if (const auto picture = m_services.pictures.find(name)) {
                canvas.drawPicture(*picture, {at.x + origin.x, row + origin.y});
            }
        };
        if (card.person.job >= 1 && card.person.job < static_cast<int>(kJobIcons.size())) {
            draw(kJobIcons[static_cast<std::size_t>(card.person.job)], m_jobSpot.at());
        }
        canvas.drawText({at.x + m_nameSpot.at().x, row + m_nameSpot.at().y}, card.person.name, plain, m_nameSpot.rect().width, m_nameSpot.rect().height);
        canvas.drawText({at.x + m_titleSpot.at().x, row + m_titleSpot.at().y}, card.person.title, plain, m_titleSpot.rect().width, m_titleSpot.rect().height);
        canvas.drawText({at.x + m_guildSpot.at().x, row + m_guildSpot.at().y}, card.person.guild, plain, m_guildSpot.rect().width, m_guildSpot.rect().height);
        if (const auto bar = m_services.pictures.find("Color_G")) {
            canvas.drawPicture(*bar, {at.x + m_barSpot.at().x, row + m_barSpot.at().y}, core::Rect{0, 0, card.barWidth, m_barSpot.rect().height});
        }
        draw("Main_Word_Lv", m_levelWordSpot.at());
        if (const auto digits = m_services.pictures.find("Num_White3_1")) {
            const gfx::Picture& picture = m_services.pictures.picture(*digits);
            const int digitHeight = picture.height() / kDigitRows;
            const std::string level = std::to_string(card.person.level);
            for (std::size_t j = 0; j < level.size(); ++j) {
                canvas.drawPicture(
                    *digits,
                    {at.x + m_levelDigitsSpot.at().x + static_cast<int>(j) * (picture.width() - 1), row + m_levelDigitsSpot.at().y},
                    core::Rect{0, (level[j] - '0') * digitHeight, picture.width(), digitHeight}
                );
            }
        }
        draw(card.online ? "icon_online_1" : "icon_offline_1", m_onlineSpot.at());
        // elements 0..4 have an icon
        if (card.person.element >= 0 && card.person.element <= 4) {
            draw(("Icon_Element_" + std::to_string(card.person.element) + "_1").c_str(), m_elementSpot.at());
        }
        m_services.characters.drawPortrait(canvas, card.look, {at.x + m_faceSpot.at().x, row + m_faceSpot.at().y}, m_blink.frame(nowMs), kFaceSlot);
    }

    void MemCardWindow::paintHint(gfx::Canvas& canvas, core::Point at) {
        const core::Point mouse = m_screen.mouse();
        for (int place = 0; place < kCards; ++place) {
            const Card& card = m_cards[static_cast<std::size_t>(place)];
            if (card.person.id == 0) {
                continue;
            }
            const core::Rect
                bar{at.x + m_barSpot.at().x, at.y + m_barSpot.at().y + place * m_rowSpot.pitch().y, m_barSpot.rect().width, m_barSpot.rect().height};
            if (!bar.contains(mouse)) {
                continue;
            }
            const std::string text = "Exp: " + std::to_string(card.exp) + "/" + std::to_string((card.step + 1) * kStepExp);
            // hint box: x 145 in the form, 23 px above the mouse, 8 px per character plus 7 padding, 17 px high
            const core::Rect box{at.x + 145, mouse.y - 23, static_cast<int>(text.size()) * 8 + 7, 17};
            canvas.fillRectAlpha(box, m_hintFillSpot.color());
            canvas.drawRectOutline(box, m_hintBorderSpot.color());
            // text 3 px inside the box; 400 max text width, 16 line height
            canvas.drawText({box.x + 3, box.y}, text, {m_hintTextSpot.color(), gfx::TextEffect::Shadow, std::nullopt}, 400, 16);
        }
    }

    void MemCardWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        refresh();
        for (int place = 0; place < kCards; ++place) {
            if (m_cards[static_cast<std::size_t>(place)].person.id != 0) {
                paintCard(canvas, at, place, nowMs);
            }
        }
        paintHint(canvas, at);
    }

}
