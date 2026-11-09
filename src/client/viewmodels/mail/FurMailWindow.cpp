#include "client/viewmodels/mail/FurMailWindow.h"

#include "core/DateTime.h"
#include "protocol/FriendRequests.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"

namespace client {

    namespace {

        constexpr int kMailboxItem = 39084; // item 39084: Mailbox (item type 30), its picture is drawn when there is no partner
        constexpr int kFaceSlot = 2; // face bank sprite slot 2: the large face
        constexpr int kShortMs = 1200; // message display time, ms
        const char* const kGmCenter = "GM center";

        /// @brief Loads FurMailWindow.xml and finds its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The FurMail window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("FurMailWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("FurMail");
        }

        /// @brief Cuts control characters and spaces off both ends of a text.
        /// @details Original: Trim.
        /// @param text Text to trim.
        /// @return The trimmed text.
        std::string trimmed(const std::string& text) {
            std::size_t first = 0;
            std::size_t last = text.size();
            while (first < last && static_cast<unsigned char>(text[first]) <= ' ') {
                ++first;
            }
            while (last > first && static_cast<unsigned char>(text[last - 1]) <= ' ') {
                --last;
            }
            return text.substr(first, last - first);
        }

    }

    FurMailWindow::FurMailWindow(
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
          m_name(screen.get<ui::TextBox>("FurMailName")),
          m_text(screen.get<ui::Memo>("FurMailText")),
          m_mailboxSpot(screen.get<ui::Spot>("FurMailMailbox")),
          m_partnerSpot(screen.get<ui::Spot>("FurMailPartner")) {
        screen.on("onFurMailSend", [this](const ui::EventArgs&) { send(); });
        screen.on("onFurMailClear", [this](const ui::EventArgs&) { m_text.clear(); });
        screen.on("onFurMailClose", [this](const ui::EventArgs&) { hide(); });
        screen.on("onFurMailHide", [this](const ui::EventArgs&) { hide(); });
        screen.on("onFurMailOldMail", [this](const ui::EventArgs&) { showMailBox(); });
        screen.get<ui::DrawingArea>("FurMailPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
    }

    std::string FurMailWindow::partnerName() const { return m_marriage.spouseId() != 0 ? m_marriage.card().name : std::string(); }

    void FurMailWindow::show() {
        m_hooks.hideSendMail();
        m_hooks.emotesToMail(true);
        if (m_window.isVisible()) {
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus("FurMailText");
        if (const std::optional<FurMailHandover> reader = m_hooks.takeOverReader()) {
            m_atMailbox = m_atMailbox || reader->atMailbox;
            m_window.setPosition(reader->position);
        }
    }

    std::optional<FurMailHandover> FurMailWindow::takeOver() {
        if (!m_window.isVisible()) {
            return std::nullopt;
        }
        const FurMailHandover handover{m_atMailbox, {m_window.bounds().x, m_window.bounds().y}};
        hide();
        return handover;
    }

    void FurMailWindow::openAtMailbox() {
        m_atMailbox = true;
        show();
    }

    void FurMailWindow::openTo(const std::string& name, core::Point position) {
        m_name.setText(name);
        m_window.setPosition(position);
        show();
        m_screen.focus("FurMailText");
    }

    void FurMailWindow::openSent(const SavedMail& mail) {
        m_name.setTextFromStart(mail.targetName);
        m_text.clear();
        m_text.appendLine(mail.text);
        m_fromSent = true;
        setEditable(false);
        show();
    }

    void FurMailWindow::setEditable(bool editable) {
        m_name.setReadOnly(!editable);
        m_text.setReadOnly(!editable);
        for (const char* name : {"FurMailSend", "FurMailClear", "FurMailOldMail"}) {
            m_screen.get<ui::ImageButton>(name).setVisible(editable);
        }
    }

    void FurMailWindow::hide() {
        m_window.setVisible(false);
        m_atMailbox = false;
        m_hooks.hidden({m_window.bounds().x, m_window.bounds().y});
        if (m_fromSent) {
            m_name.clear();
            m_text.clear();
            setEditable(true);
            m_fromSent = false;
        }
    }

    void FurMailWindow::showMailBox() {
        m_hooks.openMailBox();
        hide();
    }

    void FurMailWindow::send() {
        const std::string name = m_name.text();
        const std::string partner = partnerName();
        int kind = 0; // 0: not found, 1: a friend, 2: a guild member
        std::int32_t targetId = 0;
        if (name != partner && !m_atMailbox) {
            if (!name.empty()) {
                for (int index = 0; index <= m_friends.count() && kind == 0; ++index) {
                    if (m_friends.at(index).record.name == name) {
                        kind = 1;
                        targetId = m_friends.at(index).record.id;
                    }
                }
                for (int index = 0; index < m_guild.count() && kind == 0; ++index) {
                    if (m_guild.at(index).record.name == name) {
                        kind = 2; // kind 2: a guild member
                        targetId = m_guild.at(index).record.id;
                    }
                }
            }
            if (kind == 0 && !m_hasMailbox) {
                m_messages.show("You need mailbox to reply a letter", kShortMs);
                return;
            }
        }
        if (name.empty()) {
            m_messages.show("Hasn't written players' names.", kShortMs);
            return;
        }
        if (m_text.text().empty()) {
            m_messages.show("Hasn't written words", kShortMs);
            return;
        }
        if (trimmed(name) == kGmCenter) {
            m_messages.show("The function can't be sent to GM", kShortMs);
            return;
        }
        const std::string text = m_text.text();
        hide();
        if (kind == 1) {
            m_hooks.sendToFriend(targetId, text);
        } else if (kind == 2) { // kind 2: a guild member
            m_hooks.sendToGuildMember(targetId, text);
        } else {
            m_services.playSound("sound/wav0155.wav"); // wav0155: mail sent sound
            m_services.connection.send(protocol::furMailRequest(m_hooks.ownId(), name, text));
            SavedMail copy;
            copy.kind = name == partner ? 4 : 0; // saved mail kind 4: sent to the partner, 0: by name
            copy.read = true;
            copy.targetId = name == partner ? m_marriage.spouseId() : 0;
            copy.targetName = name;
            copy.text = text;
            copy.sendTime = core::nowAsDateTime();
            m_hooks.keepSent(copy);
        }
        m_savedText = m_text.currentLine();
        m_text.clear();
    }

    void FurMailWindow::result(int code) {
        switch (code) {
            case 1: // result code 1 (from 14/16/1, u8 code): the name does not exist; the line comes back into the text
                m_messages.show("Player doesn't exsit", kShortMs);
                m_text.appendLine(m_savedText);
                break;
            case 2: // result code 2: a mail box is needed; mail box state dropped
                m_messages.show("You need mailbox to reply a letter", kShortMs);
                m_atMailbox = false;
                m_hasMailbox = false;
                break;
            case 3: // result code 3: the player has a mail box
                m_hasMailbox = true;
                break;
            case 4: // result code 4: the player has no mail box
                m_hasMailbox = false;
                break;
            default: // unknown result code: ignored
                break;
        }
    }

    void FurMailWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const std::string partner = partnerName();
        if (!partner.empty() && m_name.text() == partner) {
            m_services.characters.drawPortrait(canvas, m_marriage.card().look, at + m_partnerSpot.at(), 0, kFaceSlot);
        } else if (const auto item = m_services.items.find(kMailboxItem)) {
            if (const auto icon = m_services.pictures.find(std::to_string(item->iconId))) {
                canvas.drawPicture(*icon, at + m_mailboxSpot.at());
            }
        }
    }

}
