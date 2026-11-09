#include "client/viewmodels/mail/ReadMailWindow.h"

#include "client/rules/GmIds.h"
#include "core/DateTime.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"

namespace client {

    namespace {

        constexpr int kFaceSlot = 2; // face bank sprite slot 2: the large face
        constexpr int kGmFace = 8084; // face sprite id 8084: the GM's face
        const std::string kInvitationMark = "^inviation^";

        /// @brief Loads ReadMailWindow.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "ReadMail" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ReadMailWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ReadMail");
        }

    }

    ReadMailWindow::ReadMailWindow(
        GameServices& services,
        ui::Screen& screen,
        const FriendList& friends,
        const Guild& guild,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("ReadMailText")),
          m_dateSpot(screen.get<ui::Spot>("ReadMailDate")),
          m_nameSpot(screen.get<ui::Spot>("ReadMailName")),
          m_faceSpot(screen.get<ui::Spot>("ReadMailFace")),
          m_invitationSpot(screen.get<ui::Spot>("ReadMailInvitation")),
          m_textColorSpot(screen.get<ui::Spot>("ReadMailTextColor")) {
        screen.on("onReadMailPrevious", [this](const ui::EventArgs&) { turn(-1); });
        screen.on("onReadMailNext", [this](const ui::EventArgs&) { turn(1); });
        screen.on("onReadMailDelete", [this](const ui::EventArgs&) { deleteMail(); });
        screen.on("onReadMailReply", [this](const ui::EventArgs&) { reply(); });
        screen.on("onReadMailClose", [this](const ui::EventArgs&) { close(); });
        screen.get<ui::DrawingArea>("ReadMailPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
    }

    void ReadMailWindow::show(int number, const SavedMail& mail) {
        m_number = number;
        m_senderId = mail.targetId;
        m_senderName = mail.targetName;
        m_guildMail = mail.kind == 2; // saved mail kind 2 = guild mail
        m_invitation = mail.text.rfind(kInvitationMark, 0) == 0;
        m_text.setText((m_invitation ? mail.text.substr(kInvitationMark.size()) : mail.text) + "\r");
        m_date = core::formatDateTime(mail.sendTime);
        if (!m_window.isVisible()) {
            m_window.setVisible(true);
        }
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ReadMailWindow::close() {
        m_window.setVisible(false);
        if (m_hooks.hidden) {
            m_hooks.hidden({m_window.bounds().x, m_window.bounds().y});
        }
    }

    void ReadMailWindow::turn(int step) {
        const int number = m_number + step;
        if (number > 0 && number <= m_hooks.mailCount()) {
            close();
            m_hooks.openMail(number);
        }
        m_hooks.turned();
    }

    void ReadMailWindow::reply() {
        bool known = false;
        if (m_guildMail) {
            known = m_guild.indexOf(m_senderId) >= 0;
        } else {
            for (int index = 0; index <= m_friends.count(); ++index) {
                known = known || m_friends.at(index).record.id == m_senderId;
            }
        }
        const core::Point position{m_window.bounds().x, m_window.bounds().y};
        if (known) {
            if (m_guildMail) {
                m_hooks.replyGuild(m_senderId, position);
            } else {
                m_hooks.reply(m_senderId, position);
            }
        } else {
            m_hooks.replyByName(m_senderName, position);
        }
        close();
    }

    void ReadMailWindow::deleteMail() {
        m_hooks.deleteMail(m_number);
        close();
        const int count = m_hooks.mailCount();
        if (count != 0) {
            if (count < m_number) {
                --m_number;
            }
            m_hooks.openMail(m_number);
        }
    }

    void ReadMailWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        auto& next = m_screen.get<ui::ImageButton>("ReadMailNext");
        auto& previous = m_screen.get<ui::ImageButton>("ReadMailPrevious");
        next.setVisible(m_hooks.mailCount() > m_number);
        previous.setVisible(m_number > 1);
        paintSender(canvas, at, nowMs);
        const auto draw = [&](const char* name, core::Point offset) {
            if (const auto picture = m_services.pictures.find(name)) {
                canvas.drawPicture(*picture, {at.x + offset.x, at.y + offset.y});
            }
        };
        if (m_invitation) {
            draw("form_invitation2", m_invitationSpot.at());
        }
        if (!next.isVisible()) {
            draw("icon_NextMail_1", {next.bounds().x, next.bounds().y});
        }
        if (!previous.isVisible()) {
            draw("icon_PrevMail_1", {previous.bounds().x, previous.bounds().y});
        }
    }

    void ReadMailWindow::paintSender(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        canvas.drawText(at + m_dateSpot.at(), m_date, style, 200, 16); // text box width 200, height 16
        if (m_guildMail) {
            paintGuildSender(canvas, at, nowMs);
            return;
        }
        const core::Point face = at + m_faceSpot.at();
        const int frame = m_blink.frame(nowMs);
        if (isGmId(m_senderId)) {
            if (m_senderName.empty()) {
                m_senderName = m_friends.at(0).record.name;
            }
            canvas.drawText(at + m_nameSpot.at(), m_senderName, style, 200, 16); // text box width 200, height 16
            m_services.characters.drawFaceSprite(canvas, kGmFace, face, 0, 0);
            return;
        }
        if (m_senderName.empty()) {
            m_senderName = "Unclear";
        }
        canvas.drawText(at + m_nameSpot.at(), m_senderName, style, 200, 16); // text box width 200, height 16
        std::optional<CharacterLook> look = m_hooks.worldLook(m_senderId);
        if (!look) {
            if (const int index = m_friends.indexOf(m_senderId)) {
                const protocol::FriendRecord& record = m_friends.at(index).record;
                formats::ColorPattern colors;
                colors.apply(record.colorCode1, 1);
                colors.apply(record.colorCode5, 5); // colour slot 5
                look = CharacterLook::dressed(record.bodyType, record.head, colors, {}, m_services.items);
            }
        }
        if (look && look->bodyType != 0) {
            m_services.characters.drawPortrait(canvas, *look, face, frame, kFaceSlot);
        }
    }

    void ReadMailWindow::paintGuildSender(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const core::Point face = at + m_faceSpot.at();
        std::optional<CharacterLook> look = m_hooks.worldLook(m_senderId);
        std::string name;
        if (look) {
            name = m_senderName;
        } else if (const int index = m_guild.indexOf(m_senderId); index >= 0) {
            const protocol::GuildMemberRecord& record = m_guild.at(index).record;
            name = m_senderName.empty() ? record.name : m_senderName;
            formats::ColorPattern colors;
            colors.apply(record.colorCode1, 1);
            colors.apply(record.colorCode5, 5); // colour slot 5
            look = CharacterLook::dressed(record.bodyType, record.head, colors, {}, m_services.items);
        }
        if (!look) {
            return;
        }
        if (m_senderName.empty()) {
            m_senderName = name;
        }
        canvas.drawText(at + m_nameSpot.at(), m_senderName, style, 200, 16); // text box width 200, height 16
        if (look->bodyType != 0) {
            m_services.characters.drawPortrait(canvas, *look, face, m_blink.frame(nowMs), kFaceSlot);
        }
    }

}
