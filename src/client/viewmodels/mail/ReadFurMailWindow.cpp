#include "client/viewmodels/mail/ReadFurMailWindow.h"

#include "core/DateTime.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"

namespace client {

    namespace {

        constexpr int kMailboxItem = 39084; // item 39084: Mailbox (item type 30), its picture is drawn for non-partner senders
        constexpr int kFaceSlot = 2; // face bank sprite slot 2: the large face
        const std::string kInvitationMark = "^inviation^";

        /// @brief Loads ReadFurMailWindow.xml and finds its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The ReadFurMail window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ReadFurMailWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ReadFurMail");
        }

    }

    ReadFurMailWindow::ReadFurMailWindow(
        GameServices& services,
        ui::Screen& screen,
        const FriendList& friends,
        const Guild& guild,
        const Marriage& marriage,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_marriage(marriage),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("ReadFurMailText")),
          m_dateSpot(screen.get<ui::Spot>("ReadFurMailDate")),
          m_nameSpot(screen.get<ui::Spot>("ReadFurMailName")),
          m_mailboxSpot(screen.get<ui::Spot>("ReadFurMailMailbox")),
          m_partnerSpot(screen.get<ui::Spot>("ReadFurMailPartner")),
          m_invitationSpot(screen.get<ui::Spot>("ReadFurMailInvitation")),
          m_textColorSpot(screen.get<ui::Spot>("ReadFurMailTextColor")) {
        screen.on("onReadFurMailPrevious", [this](const ui::EventArgs&) { turn(-1); });
        screen.on("onReadFurMailNext", [this](const ui::EventArgs&) { turn(1); });
        screen.on("onReadFurMailDelete", [this](const ui::EventArgs&) { deleteMail(); });
        screen.on("onReadFurMailReply", [this](const ui::EventArgs&) { reply(); });
        screen.on("onReadFurMailClose", [this](const ui::EventArgs&) { close(); });
        screen.get<ui::DrawingArea>("ReadFurMailPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
    }

    std::string ReadFurMailWindow::partnerName() const { return m_marriage.spouseId() != 0 ? m_marriage.card().name : std::string(); }

    void ReadFurMailWindow::show(int number, const SavedMail& mail) {
        m_number = number;
        m_senderName = mail.targetName;
        m_invitation = mail.text.rfind(kInvitationMark, 0) == 0;
        m_text.setText((m_invitation ? mail.text.substr(kInvitationMark.size()) : mail.text) + "\r");
        m_date = core::formatDateTimeToStr(mail.sendTime);
        if (const std::optional<FurMailHandover> writer = m_hooks.takeOverWriter()) {
            m_atMailbox = m_atMailbox || writer->atMailbox;
            m_window.setPosition(writer->position);
        }
        if (!m_window.isVisible()) {
            m_window.setVisible(true);
            if (m_screen.context().bringToFront) {
                m_screen.context().bringToFront(m_window.id());
            }
        }
    }

    void ReadFurMailWindow::close() {
        m_window.setVisible(false);
        m_hooks.hidden({m_window.bounds().x, m_window.bounds().y});
    }

    std::optional<FurMailHandover> ReadFurMailWindow::takeOver() {
        if (!m_window.isVisible()) {
            return std::nullopt;
        }
        const FurMailHandover handover{m_atMailbox, {m_window.bounds().x, m_window.bounds().y}};
        close();
        return handover;
    }

    void ReadFurMailWindow::turn(int step) {
        const int number = m_number + step;
        if (number > 0 && number <= m_hooks.mailCount()) {
            close();
            m_hooks.openMail(number);
        }
        m_hooks.turned();
    }

    void ReadFurMailWindow::reply() {
        const std::string partner = partnerName();
        if (m_senderName != partner && !m_atMailbox) {
            bool known = false;
            if (!m_senderName.empty()) {
                for (int index = 0; index <= m_friends.count(); ++index) {
                    known = known || m_friends.at(index).record.name == m_senderName;
                }
                for (int index = 0; index < m_guild.count(); ++index) {
                    known = known || m_guild.at(index).record.name == m_senderName;
                }
            }
            if (!known && !m_hooks.hasMailbox()) {
                m_messages.show("You need mailbox to reply a letter", 1200); // message display time in ms
                return;
            }
        }
        m_hooks.replyTo(m_senderName, {m_window.bounds().x, m_window.bounds().y});
        close();
    }

    void ReadFurMailWindow::deleteMail() {
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

    void ReadFurMailWindow::paint(gfx::Canvas& canvas, core::Point at) {
        auto& next = m_screen.get<ui::ImageButton>("ReadFurMailNext");
        auto& previous = m_screen.get<ui::ImageButton>("ReadFurMailPrevious");
        next.setVisible(m_hooks.mailCount() > m_number);
        previous.setVisible(m_number > 1);
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        // date and name text boxes are 200 px wide, 16 px high
        canvas.drawText(at + m_dateSpot.at(), m_date, style, 200, 16);
        canvas.drawText(at + m_nameSpot.at(), m_senderName, style, 200, 16);
        const std::string partner = partnerName();
        if (!partner.empty() && m_senderName == partner) {
            m_services.characters.drawPortrait(canvas, m_marriage.card().look, at + m_partnerSpot.at(), 0, kFaceSlot);
        } else if (const auto item = m_services.items.find(kMailboxItem)) {
            if (const auto icon = m_services.pictures.find(std::to_string(item->iconId))) {
                canvas.drawPicture(*icon, at + m_mailboxSpot.at());
            }
        }
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

}
