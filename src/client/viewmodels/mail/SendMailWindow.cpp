#include "client/viewmodels/mail/SendMailWindow.h"

#include "client/models/character/CharacterLook.h"
#include "client/rules/GmIds.h"
#include "core/DateTime.h"
#include "protocol/FriendRequests.h"
#include "protocol/GuildRequests.h"
#include "protocol/MarriageRequests.h"
#include "protocol/NoticeRequests.h"
#include "protocol/PuppetRequests.h"
#include "ui/DrawingArea.h"

#include <vector>

namespace client {

    namespace {

        constexpr int kFaceSlot = 2; // face bank sprite slot 2: the large face
        constexpr int kGmFace = 8084; // face sprite id 8084: the GM's face
        constexpr int kNoticeItem = 39109; // item 39109: Notice Board (item type 30), its picture is drawn for a notice author
        constexpr int kSavedGuildMail = 2;
        constexpr int kSavedNoticeMail = 0;

        /// @brief Loads SendMailWindow.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "SendMail" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SendMailWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("SendMail");
        }

    }

    SendMailWindow::SendMailWindow(
        GameServices& services,
        ui::Screen& screen,
        FriendList& friends,
        Guild& guild,
        const Marriage& marriage,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_marriage(marriage),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("SendMailText")),
          m_nameSpot(screen.get<ui::Spot>("SendMailName")),
          m_faceSpot(screen.get<ui::Spot>("SendMailFace")),
          m_noticeSpot(screen.get<ui::Spot>("SendMailNotice")),
          m_nameColorSpot(screen.get<ui::Spot>("SendMailNameColor")) {
        screen.on("onSendMailSend", [this](const ui::EventArgs&) { send(); });
        screen.on("onSendMailClear", [this](const ui::EventArgs&) { m_text.clear(); });
        screen.on("onSendMailClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onSendMailHide", [this](const ui::EventArgs&) { hide(); });
        screen.on("onSendMailOldMail", [this](const ui::EventArgs&) {
            m_hooks.openMailBox();
            hide();
        });
        screen.get<ui::DrawingArea>("SendMailPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
    }

    void SendMailWindow::openTo(std::int32_t friendId) { show(Kind::Friend, friendId); }

    void SendMailWindow::openToChecked() { show(Kind::Checked, m_targetId); }

    void SendMailWindow::openToNoticeAuthor(std::int32_t authorId, const std::string& name) {
        m_noticeAuthor = name;
        show(Kind::Notice, authorId);
    }

    void SendMailWindow::openToGuildMember(std::int32_t memberId) { show(Kind::Guild, memberId); }

    void SendMailWindow::openToCheckedMembers() { show(Kind::GuildChecked, m_targetId); }

    void SendMailWindow::openToSpouse() { show(Kind::Spouse, m_marriage.card().id); }

    void SendMailWindow::openToPuppet(const protocol::PuppetPerson& person) {
        m_puppet = person;
        show(Kind::Puppet, person.id);
    }

    void SendMailWindow::show(Kind kind, std::int32_t targetId) {
        m_kind = kind;
        m_targetId = targetId;
        m_hooks.emotesToMail(true);
        if (m_window.isVisible()) {
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus("SendMailText");
    }

    void SendMailWindow::sendNamed(Kind kind, std::int32_t targetId, const std::string& text) {
        m_kind = kind;
        m_targetId = targetId;
        m_text.setText(text);
        send();
    }

    void SendMailWindow::reply(std::int32_t friendId, core::Point position) {
        m_window.setPosition(position);
        show(Kind::Friend, friendId);
        m_screen.focus("SendMailText");
    }

    void SendMailWindow::hide() {
        m_window.setVisible(false);
        m_hooks.hidden({m_window.bounds().x, m_window.bounds().y});
    }

    void SendMailWindow::close() {
        m_text.clear();
        hide();
        m_hooks.emotesToMail(false);
    }

    void SendMailWindow::send() {
        if (m_text.text().empty() || isGmId(m_targetId)) {
            return;
        }
        m_services.playSound("sound/wav0155.wav"); // wav0155: mail sent sound
        hide();
        SavedMail copy;
        copy.kind = 1; // saved mail kind 1 = ordinary sent mail
        copy.read = true;
        copy.text = m_text.text();
        copy.sendTime = core::nowAsDateTime();
        switch (m_kind) {
            case Kind::Friend: // one friend: mail request to the friend
                if (!sendToFriend(copy)) {
                    return;
                }
                break;
            case Kind::Checked: // the ticked friends
                sendToChecked();
                break;
            case Kind::Notice: // author of a public notice (82/6)
                if (!sendToNoticeAuthor(copy)) {
                    return;
                }
                break;
            case Kind::Guild: // one guild member (39/4)
                if (!sendToGuildMember(copy)) {
                    return;
                }
                break;
            case Kind::GuildChecked: // the ticked guild members (39/4)
                sendToCheckedMembers();
                copy.kind = kSavedGuildMail;
                break;
            case Kind::Spouse: // the spouse
                if (!sendToSpouse(copy)) {
                    return;
                }
                break;
            case Kind::Puppet: // Memory Card training partner (23/104 option 13)
                if (!sendToPuppet(copy)) {
                    return;
                }
                break;
        }
        m_hooks.keepSent(copy);
        m_text.clear();
        m_hooks.emotesToMail(false);
    }

    bool SendMailWindow::sendToFriend(SavedMail& copy) {
        bool known = false;
        for (int index = 0; index <= m_friends.count(); ++index) {
            known = known || m_friends.at(index).record.id == m_targetId;
        }
        if (m_targetId == 0 || !known) {
            return false;
        }
        m_services.connection.send(protocol::mailRequest({m_targetId}, m_text.text()));
        copy.targetId = m_targetId;
        copy.targetName = m_friends.at(m_friends.indexOf(m_targetId)).record.name;
        return true;
    }

    void SendMailWindow::sendToChecked() {
        std::vector<std::int32_t> ids;
        for (int index = 1; index <= m_friends.count(); ++index) {
            Friend& f = m_friends.at(index);
            if (f.checked) {
                f.checked = false;
                ids.push_back(f.record.id);
            }
        }
        if (!ids.empty()) {
            m_services.connection.send(protocol::mailRequest(ids, m_text.text()));
        }
    }

    bool SendMailWindow::sendToNoticeAuthor(SavedMail& copy) {
        if (m_targetId == 0) {
            return false;
        }
        m_services.connection.send(protocol::mailNoticeAuthorRequest(m_hooks.ownId(), m_noticeAuthor, m_text.text()));
        copy.kind = kSavedNoticeMail;
        copy.targetId = m_targetId;
        copy.targetName = m_noticeAuthor;
        return true;
    }

    bool SendMailWindow::sendToGuildMember(SavedMail& copy) {
        const int member = m_guild.indexOf(m_targetId);
        if (m_targetId == 0 || member < 0) {
            return false;
        }
        m_services.connection.send(protocol::guildMailRequest({m_targetId}, m_text.text()));
        copy.kind = kSavedGuildMail;
        copy.targetId = m_targetId;
        copy.targetName = m_guild.at(member).record.name;
        return true;
    }

    bool SendMailWindow::sendToSpouse(SavedMail& copy) {
        if (m_targetId == 0 || m_marriage.spouseId() != m_targetId) {
            return false;
        }
        m_services.connection.send(protocol::spouseMailRequest(m_hooks.ownId(), m_marriage.card().name, m_text.text()));
        copy.kind = kSavedNoticeMail;
        copy.targetId = m_targetId;
        copy.targetName = m_marriage.card().name;
        return true;
    }

    bool SendMailWindow::sendToPuppet(SavedMail& copy) {
        if (m_targetId == 0) {
            return false;
        }
        m_services.connection.send(protocol::puppetMailRequest(m_hooks.ownId(), m_puppet.name, m_text.text()));
        copy.kind = 0; // saved mail kind 0 = ordinary mail
        copy.targetId = m_targetId;
        copy.targetName = m_puppet.name;
        return true;
    }

    void SendMailWindow::sendToCheckedMembers() {
        std::vector<std::int32_t> ids;
        for (int index = 0; index < m_guild.count(); ++index) {
            GuildMember& member = m_guild.at(index);
            if (member.checked) {
                member.checked = false;
                ids.push_back(member.record.id);
            }
        }
        if (!ids.empty()) {
            m_services.connection.send(protocol::guildMailRequest(ids, m_text.text()));
        }
    }

    void SendMailWindow::paint(gfx::Canvas& canvas, core::Point at) {
        switch (m_kind) {
            case Kind::Friend: paintFriend(canvas, at); break; // friend's name and face
            case Kind::Notice: paintNotice(canvas, at); break; // notice author over the board's sign
            case Kind::Guild: paintGuildMember(canvas, at); break; // guild member's name and face
            case Kind::Spouse: paintSpouse(canvas, at); break; // spouse's name and face
            case Kind::Puppet: paintPuppet(canvas, at); break; // partner's name and face
            case Kind::Checked: // ticked friends: no preview
            case Kind::GuildChecked: break; // ticked members: no preview
        }
    }

    void SendMailWindow::paintNotice(gfx::Canvas& canvas, core::Point at) {
        if (const auto item = m_services.items.find(kNoticeItem)) {
            if (const auto icon = m_services.pictures.find(std::to_string(item->iconId))) {
                canvas.drawPicture(*icon, at + m_noticeSpot.at());
            }
        }
        canvas.drawText(
            at + m_nameSpot.at(),
            m_noticeAuthor,
            {m_nameColorSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            200,
            16
        ); // text box width 200, height 16
    }

    void SendMailWindow::paintGuildMember(gfx::Canvas& canvas, core::Point at) {
        const int member = m_guild.indexOf(m_targetId);
        if (member < 0) {
            return;
        }
        const protocol::GuildMemberRecord& record = m_guild.at(member).record;
        canvas.drawText(
            at + m_nameSpot.at(),
            record.name,
            {m_nameColorSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            200,
            16
        ); // text box width 200, height 16
        m_services.characters.drawPortrait(
            canvas,
            CharacterLook::fromColorCodes(record.bodyType, record.head, record.colorCode1, record.colorCode5, m_services.items),
            at + m_faceSpot.at(),
            0,
            kFaceSlot
        );
    }

    void SendMailWindow::paintSpouse(gfx::Canvas& canvas, core::Point at) {
        if (m_marriage.spouseId() == 0) {
            return;
        }
        canvas.drawText(
            at + m_nameSpot.at(),
            m_marriage.card().name,
            {m_nameColorSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            200,
            16
        ); // text box width 200, height 16
        m_services.characters.drawPortrait(canvas, m_marriage.card().look, at + m_faceSpot.at(), 0, kFaceSlot);
    }

    void SendMailWindow::paintPuppet(gfx::Canvas& canvas, core::Point at) {
        canvas.drawText(
            at + m_nameSpot.at(),
            m_puppet.name,
            {m_nameColorSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            200,
            16
        ); // text box width 200, height 16
        m_services.characters.drawPortrait(
            canvas,
            CharacterLook::fromColorCodes(m_puppet.bodyType, m_puppet.head, m_puppet.colorCode1, m_puppet.colorCode5, m_services.items),
            at + m_faceSpot.at(),
            0,
            kFaceSlot
        );
    }

    void SendMailWindow::paintFriend(gfx::Canvas& canvas, core::Point at) {
        const core::Point face = at + m_faceSpot.at();
        const gfx::TextStyle style{m_nameColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        if (isGmId(m_targetId)) {
            canvas.drawText(at + m_nameSpot.at(), m_friends.at(0).record.name, style, 200, 16); // text box width 200, height 16
            m_services.characters.drawFaceSprite(canvas, kGmFace, face, 0, 0);
            return;
        }
        const int index = m_friends.indexOf(m_targetId);
        if (index == 0) {
            return;
        }
        const protocol::FriendRecord& record = m_friends.at(index).record;
        canvas.drawText(at + m_nameSpot.at(), record.name, style, 200, 16); // text box width 200, height 16
        formats::ColorPattern colors;
        colors.apply(record.colorCode1, 1);
        colors.apply(record.colorCode5, 5); // colour slot 5
        m_services.characters.drawPortrait(canvas, CharacterLook::dressed(record.bodyType, record.head, colors, {}, m_services.items), face, 0, kFaceSlot);
    }

}
