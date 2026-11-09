#include "client/viewmodels/mail/MailBoxWindow.h"

#include "core/DateTime.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        constexpr int kRowsPerPage = 10; // mails listed per page
        constexpr int kNearlyFull = 900; // mail count above which the box counts as nearly full (90% of 1000)
        constexpr std::size_t kShortStart = 20; // longest first row shown whole, characters
        constexpr std::size_t kStartShown = 16; // characters kept of a longer first row
        const std::array<const char*, 5> kKinds = {"Common", "Good friend", "Organization", "System", "Partner"};

        /// @brief Loads MailBoxWindow.xml and finds its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The MailBox window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MailBoxWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MailBox");
        }

        /// @brief Gives the start of a mail for the list: its first row, shortened when long.
        /// @param text Full mail text.
        /// @return The first row, or its first 16 characters and "..." when longer than 20.
        std::string mailStart(const std::string& text) {
            const std::string first = text.substr(0, text.find('\r'));
            return first.size() <= kShortStart ? first : text.substr(0, kStartShown) + "...";
        }

    }

    MailBoxWindow::MailBoxWindow(
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
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_titleMaskSpot(screen.get<ui::Spot>("MailBoxTitleMask")),
          m_pageSpot(screen.get<ui::Spot>("MailBoxPageNumbers")),
          m_fullPageSpot(screen.get<ui::Spot>("MailBoxPageNumbersFull")),
          m_rowSpot(screen.get<ui::Spot>("MailBoxRow")),
          m_hoverBarSpot(screen.get<ui::Spot>("MailBoxRowHover")),
          m_chosenBarSpot(screen.get<ui::Spot>("MailBoxRowChosen")),
          m_kindSpot(screen.get<ui::Spot>("MailBoxKind")),
          m_startSpot(screen.get<ui::Spot>("MailBoxStart")),
          m_nameSpot(screen.get<ui::Spot>("MailBoxName")),
          m_dateSpot(screen.get<ui::Spot>("MailBoxDate")),
          m_unreadSpot(screen.get<ui::Spot>("MailBoxUnread")),
          m_readSpot(screen.get<ui::Spot>("MailBoxRead")),
          m_box(friends),
          m_reader(
              services,
              screen,
              friends,
              guild,
              {[this] { return m_box.count(MailBox::Box::Received); },
               [this](int number) { openMail(number); },
               [this](int number) { deleteMail(MailBox::Box::Received, number); },
               m_hooks.reply,
               m_hooks.replyGuild,
               m_hooks.worldLook,
               [this] {
                   if (m_box.unread() == 0) {
                       m_blink = false;
                   }
               },
               m_hooks.mailFormHidden,
               m_hooks.replyByName}
          ),
          m_furReader(
              services,
              screen,
              friends,
              guild,
              marriage,
              messages,
              {[this] { return m_box.count(MailBox::Box::Received); },
               [this](int number) { openMail(number); },
               [this](int number) { deleteMail(MailBox::Box::Received, number); },
               m_hooks.hasMailbox,
               m_hooks.replyByName,
               m_hooks.takeOverWriter,
               [this] {
                   if (m_box.unread() == 0) {
                       m_blink = false;
                   }
               },
               m_hooks.mailFormHidden}
          ) {
        screen.on("onMailBoxTab", [this](const ui::EventArgs& args) { setTab(args.source == "MailBoxTab2" ? 1 : 0); });
        screen.on("onMailBoxNextPage", [this](const ui::EventArgs&) { turnPage(1); });
        screen.on("onMailBoxPreviousPage", [this](const ui::EventArgs&) { turnPage(-1); });
        screen.on("onMailBoxDelete", [this](const ui::EventArgs&) { deleteChosen(); });
        screen.on("onMailBoxClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMailBoxSendMail", [this](const ui::EventArgs&) { m_hooks.toggleWriter(); });
        screen.on("onMailBoxPress", [this](const ui::EventArgs& args) { m_pressedRow = rowAt(args.position); });
        screen.on("onMailBoxRelease", [this](const ui::EventArgs& args) {
            if (m_pressedRow != 0 && m_pressedRow == rowAt(args.position)) {
                m_chosen = (m_page - 1) * kRowsPerPage + m_pressedRow;
                openMail(m_chosen);
            }
        });
        screen.get<ui::DrawingArea>("MailBoxPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        setTab(0);
    }

    void MailBoxWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_chosen = 0;
    }

    void MailBoxWindow::close() {
        if (m_box.isDirty()) {
            save();
        }
        m_window.setVisible(false);
    }

    void MailBoxWindow::load(const std::filesystem::path& folder) {
        m_folder = folder;
        m_box.clear();
        m_tab = 0;
        m_page = 1;
        m_chosen = 0;
        m_pressedRow = 0;
        setTab(0);
        m_box.load(folder);
        m_blink = m_box.unread() != 0;
    }

    void MailBoxWindow::save() {
        if (!m_folder.empty()) {
            m_box.save(m_folder);
        }
    }

    void MailBoxWindow::importLogs(const std::filesystem::path& serverFolder, std::int32_t playerKey, MailLogImporter::Kind kind) {
        if (MailLogImporter::import(m_box, serverFolder, playerKey, kind)) {
            m_blink = m_box.unread() != 0;
        }
    }

    void MailBoxWindow::receive(
        std::int32_t senderId,
        double sendTime,
        const std::string& text,
        int kind,
        const std::string& name
    ) {
        SavedMail mail;
        mail.kind = kind;
        mail.targetId = senderId == 101 ? 100 : senderId; // sender id 101 is shown as id 100 (system sender)
        mail.targetName = name.empty() ? m_hooks.worldName(mail.targetId) : name;
        if (!m_hooks.blocked(mail.targetId, mail.targetName)) {
            mail.text = text;
            mail.sendTime = sendTime;
            add(MailBox::Box::Received, mail);
        }
        m_blink = true;
    }

    void MailBoxWindow::keepSent(const SavedMail& mail) { add(MailBox::Box::Sent, mail); }

    void MailBoxWindow::add(MailBox::Box which, SavedMail mail) {
        if (!m_box.add(which, std::move(mail))) {
            return;
        }
        if (which == MailBox::Box::Received) {
            m_blink = m_box.unread() != 0;
        }
        if (which == box() && m_chosen != 0) {
            ++m_chosen;
        }
        if (m_box.count(which) > kNearlyFull) {
            m_hooks.chatLine(
                "When the mail box is full, the system will automatically delete the earliest letter. Now, your mail "
                "box is 90% full"
            );
        }
    }

    void MailBoxWindow::setTab(int tab) {
        m_tab = tab;
        m_chosen = 0;
        m_screen.get<ui::ImageButton>("MailBoxTab1").setSelected(tab == 0);
        m_screen.get<ui::ImageButton>("MailBoxTab2").setSelected(tab == 1);
    }

    int MailBoxWindow::pageCount() const { return (m_box.count(box()) + kRowsPerPage - 1) / kRowsPerPage; }

    int MailBoxWindow::rowsOnPage() const {
        const int pages = std::max(1, pageCount());
        return m_page < pages ? kRowsPerPage : m_box.count(box()) - (m_page - 1) * kRowsPerPage;
    }

    int MailBoxWindow::rowAt(core::Point local) const {
        const int row = local.y / m_rowSpot.pitch().y + 1;
        return local.y >= 0 && row <= rowsOnPage() ? row : 0;
    }

    void MailBoxWindow::openMail(int number) {
        if (m_tab != 0) {
            if (number >= 1 && number <= m_box.count(MailBox::Box::Sent)) {
                m_chosen = number;
                m_hooks.openSent(m_box.mails(MailBox::Box::Sent)[static_cast<std::size_t>(number - 1)]);
            }
            return;
        }
        if (m_reader.isOpen() && m_reader.mailNumber() == m_chosen) {
            m_reader.close();
            m_chosen = 0;
            return;
        }
        if (m_furReader.isOpen() && m_furReader.mailNumber() == m_chosen) {
            m_furReader.close();
            m_chosen = 0;
            return;
        }
        if (number < 1 || number > m_box.count(MailBox::Box::Received)) {
            return;
        }
        m_box.markRead(number);
        m_chosen = number;
        const SavedMail& mail = m_box.mails(MailBox::Box::Received)[static_cast<std::size_t>(number - 1)];
        if (mail.kind == 1 || mail.kind == 2 || mail.kind == 3) { // mail kinds 1 friend, 2 guild, 3 system: read in the plain read form
            if (m_furReader.isOpen()) {
                m_furReader.close();
            }
            m_reader.show(number, mail);
        } else {
            if (m_reader.isOpen()) {
                m_reader.close();
            }
            m_furReader.show(number, mail);
        }
        if (m_box.unread() == 0) {
            m_blink = false;
        }
    }

    bool MailBoxWindow::deleteMail(MailBox::Box which, int number) {
        if (m_box.count(which) == 0) {
            m_messages.show("There are no letters in your mail box", 2000); // message display time in ms
            return false;
        }
        if (!m_box.remove(which, number)) {
            return false;
        }
        if (m_chosen > number) {
            --m_chosen;
        }
        m_page = std::max(1, std::min(m_page, (m_box.count(which) + kRowsPerPage - 1) / kRowsPerPage));
        m_messages.show("The letter was successfully deleted", 2000); // message display time in ms
        if (m_box.count(which) == 0) {
            m_messages.show("The mail box has been cleared", 2000); // message display time in ms
        }
        return true;
    }

    void MailBoxWindow::deleteChosen() {
        if (m_chosen == 0) {
            m_messages.show("Haven't chosen a letter", 2000); // message display time in ms
            return;
        }
        if ((m_chosen + kRowsPerPage - 1) / kRowsPerPage != m_page) {
            return;
        }
        deleteMail(box(), m_chosen);
        if (m_tab == 0) {
            m_reader.close();
            if (m_furReader.isOpen()) {
                m_furReader.close();
            }
        }
        if (m_chosen != 0) {
            m_chosen = std::min(m_chosen, m_box.count(box()));
            if (m_tab == 0) {
                openMail(m_chosen);
            }
        }
    }

    void MailBoxWindow::turnPage(int step) {
        if (step > 0) {
            if (m_page < pageCount()) {
                ++m_page;
            } else {
                m_messages.show("Last Page", 2000); // message display time in ms
            }
        } else if (m_page > 1) {
            --m_page;
        }
    }

    void MailBoxWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const MailBox::Box which = box();
        const auto& mails = m_box.mails(which);
        const int count = m_box.count(which);
        if (m_tab == 0) {
            if (const auto mask = m_services.pictures.find("Icon_MailBoxTitleMash_1")) {
                canvas.drawPicture(*mask, at + m_titleMaskSpot.at());
            }
        }
        if (count == 0) {
            m_chosen = 0;
        }
        int pages = pageCount();
        if (pages == 0) {
            m_page = 1;
            pages = 1;
        }
        const int rowsHere = rowsOnPage();
        const core::Point mouse = m_screen.mouse();
        int hoverRow = 0;
        core::Rect hoverBar;
        for (int row = 1; row <= rowsHere; ++row) {
            const core::Rect
                bar{at.x + m_rowSpot.rect().x, at.y + m_rowSpot.rect().y + (row - 1) * m_rowSpot.pitch().y, m_rowSpot.rect().width, m_rowSpot.rect().height};
            if (bar.contains(mouse)) {
                hoverRow = row;
                hoverBar = bar;
                break;
            }
        }
        const gfx::TextStyle pageStyle{count > kNearlyFull ? m_fullPageSpot.color() : m_pageSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const std::string page = std::to_string(m_page) + " ";
        const int pageWidth = static_cast<int>(page.size()) * 8; // 8 px per character
        canvas.drawText({at.x + m_pageSpot.at().x - pageWidth, at.y + m_pageSpot.at().y}, page, pageStyle, pageWidth, 16); // text box height 16 px
        const std::string total = "/ " + std::to_string(pages);
        canvas.drawText(at + m_pageSpot.at(), total, pageStyle, static_cast<int>(total.size()) * 8,
                        16); // 8 px per character, text box height 16 px
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const int number = (m_page - 1) * kRowsPerPage + row;
            if (number < 1 || number > count) {
                break;
            }
            const SavedMail& mail = mails[static_cast<std::size_t>(number - 1)];
            const gfx::TextStyle style{mail.read ? m_readSpot.color() : m_unreadSpot.color(), gfx::TextEffect::Plain, std::nullopt};
            const int y = at.y + m_kindSpot.at().y + (row - 1) * m_rowSpot.pitch().y;
            const auto text = [&](int x, const std::string& value) {
                canvas.drawText({at.x + x, y}, value, style, static_cast<int>(value.size()) * 8, 16); // 8 px per character, text box height 16 px
            };
            text(
                m_kindSpot.at().x,
                mail.kind >= 1 && mail.kind <= 4 ? kKinds[static_cast<std::size_t>(mail.kind)] : kKinds[0]
            ); // kinds 1..4 have names, others show "Common"
            text(m_startSpot.at().x, mailStart(mail.text));
            text(m_nameSpot.at().x, mail.targetName.empty() ? "Unclear" : mail.targetName);
            text(m_dateSpot.at().x, core::formatDateTime(mail.sendTime));
        }
        if (hoverRow != 0) {
            canvas.fillRectAlpha(hoverBar, m_hoverBarSpot.color());
        }
        if (m_chosen > 0 && (m_chosen + kRowsPerPage - 1) / kRowsPerPage == m_page) {
            const int row = m_chosen - (m_page - 1) * kRowsPerPage;
            canvas.fillRectAlpha(
                {at.x + m_rowSpot.rect().x, at.y + m_rowSpot.rect().y + (row - 1) * m_rowSpot.pitch().y, m_rowSpot.rect().width, m_rowSpot.rect().height},
                m_chosenBarSpot.color()
            );
        }
    }

}
