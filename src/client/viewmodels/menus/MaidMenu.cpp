#include "client/viewmodels/menus/MaidMenu.h"

#include "client/rules/FurnitureKinds.h"
#include "gfx/TextStyle.h"
#include "protocol/MaidRequests.h"
#include "ui/TextWrap.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::size_t kWrapChars = 45; // talk text wraps at 45 characters
        constexpr int kWitMaid = 39096; // item 39096: Wit Maid (the default maid piece)
        constexpr int kNextIconStepMs = 250; // "next" arrow frame time, ms

        /// @brief A suit of the bag and the piece of furniture it makes of her.
        struct Costume {
            int suit;
            int piece;
        };

        /// @brief Looks up the costume of a menu choice.
        /// @details Original: ChangeMaidCostume's table.
        /// @param choice Menu choice counted across pages.
        /// @return The suit and piece, or nothing when the choice has no costume.
        std::optional<Costume> costumeOf(int choice) {
            // table: suit item id and the maid piece item id it makes (suits are type 25 items, maid pieces type 30)
            switch (choice) {
                case 0:
                    return Costume{
                        34175,
                        39096
                    }; // choice 0: first-page costume (reserved Season Costume-Winter), the default maid piece; Seasonal Suit -> Wit Maid
                case 1: return Costume{34175, 39117}; // choice 1: Season Costume-Summer; Seasonal Suit -> Wit Maid-Summer
                case 2: return Costume{34176, 39118}; // choice 2: Rabbit Costume; Rabbit Suit -> Wit Maid-Rabbit
                case 3: return Costume{34177, 39119}; // choice 3: Catwoman Costume; Cat Suit -> Wit Maid-Cat
                case 5: return Costume{34411, 39223}; // choice 5: Pumpkin Maid Paper Pack (second page); Pumpkin Suit -> Wit Maid-Pumpkin
                case 6: return Costume{34412, 39224}; // choice 6: Succubus Maid Paper Pack (second page); Succubus Suit -> Wit Maid-Succubus
                case 7: return Costume{34417, 39243}; // choice 7: Maid Qi Pao (second page); Maid Qi Pao -> Wit Maid-Qi Pao
                default: return std::nullopt; // no costume for this choice (the page arrows and the rest)
            }
        }

        /// @brief Loads MaidMenu.xml into the screen and returns its content area.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "MaidContent" drawing area.
        ui::DrawingArea& loadContent(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MaidMenu.xml"), services.loadContext());
            return screen.get<ui::DrawingArea>("MaidContent");
        }

    }

    MaidMenu::MaidMenu(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_content(loadContent(services, screen)),
          m_choices(screen.get<ui::ListBox>("MaidChoices")),
          m_textSpot(screen.get<ui::Spot>("MaidText")),
          m_figureSpot(screen.get<ui::Spot>("MaidFigure")) {
        m_content.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.on("onMaidClicked", [this](const ui::EventArgs&) { clicked(); });
        screen.on("onMaidChoice", [this](const ui::EventArgs& args) {
            if (m_onChoice) {
                const std::function<void(int)> handler = m_onChoice;
                hide();
                m_speaker = false;
                handler(args.index);
            }
        });
    }

    void MaidMenu::open(int furnIndex) {
        m_furn = furnIndex;
        m_active = true;
        showPage(1);
    }

    void MaidMenu::showPage(int page) {
        if (!m_furn || !m_active) {
            return;
        }
        const int item = m_hooks.pieceItem(*m_furn);
        m_figureItem = FurnitureKinds::maidKind(m_services.items, item) <= 1 ? kWitMaid : item;
        m_speaker = true;
        switch (page) {
            case 1: // page 1: greeting, then page 2
                m_page = 1;
                say("Master, what can I do for you?", [this] {
                    if (m_furn && m_active) {
                        showPage(2);
                    }
                });
                break;
            case 2: // page 2: main choices (item bank, pets, clothes, ...)
                choose(
                    {"Deposit and withdraw of Item bank", "Deposit and withdraw of Pets", "Change clothes", "something...", "nothing to do..."},
                    [this](int choice) { handleChoice(choice); }
                );
                break;
            case 3: // page 3: clothes question, then page 4
                say("Master, what kind of clothes do you want me to wear?", [this] {
                    if (m_furn && m_active) {
                        showPage(4);
                    }
                });
                break;
            default: // page 4: clothes choices of the current costume page (m_page 1 or 2)
                if (m_page == 2) {
                    choose({"Pumpkin Maid Paper Pack", "Succubus Maid Paper Pack", "Maid Qi Pao", "Prev page", "No clothes to wear"}, [this](int choice) {
                        changeCostume(choice);
                    });
                } else {
                    m_page = 1;
                    choose(
                        {"Season Costume-Winter (reserved costume)", "Season Costume-Summer", "Rabbit Costume", "Catwoman Costume", "Next page"},
                        [this](int choice) { changeCostume(choice); }
                    );
                }
                break;
        }
    }

    void MaidMenu::handleChoice(int choice) {
        if (!m_furn || !m_active) {
            return;
        }
        const auto leave = [this] {
            m_active = false;
            m_furn.reset();
        };
        switch (choice) {
            case 0: // choice 0: item bank, sends 76/1
                m_costume.clear();
                m_hooks.send(protocol::maidRequest(1, *m_furn)); // sub id 1: item bank
                break;
            case 1: // choice 1: pets' inn, sends 76/2
                m_costume.clear();
                m_hooks.send(protocol::maidRequest(2, *m_furn)); // sub id 2: inn
                break;
            case 2: // choice 2: change clothes (page 3)
                showPage(3);
                break;
            case 3: // choice 3: "something...", she jokes and the menu ends
                m_speaker = true;
                say("Master, you are really a joker, haha~ (Blush)", nullptr);
                leave();
                break;
            case 4: // choice 4: "nothing to do...", she says goodbye and the menu ends
                m_speaker = true;
                say("Master, goodbye~ (smile)", nullptr);
                leave();
                break;
            default: // any other choice: close the box and end the menu
                hide();
                leave();
                break;
        }
    }

    void MaidMenu::changeCostume(int choice) {
        constexpr int kPerPage = 5; // choices per costume page
        constexpr int kNextPage = 4; // choice index of "Next page" (first page)
        constexpr int kPrevPage = 8; // choice index of "Prev page" (second page, 5 + 3)
        m_costume.clear();
        choice += (m_page - 1) * kPerPage;
        const auto leaveWith = [this](const std::string& words) {
            m_speaker = true;
            say(words, nullptr);
            m_active = false;
            m_furn.reset();
        };
        if (choice == kNextPage || choice == kPrevPage) {
            m_page = choice == kNextPage ? std::min(m_page + 1, 2) : std::max(m_page - 1, 1); // costume pages 1..2
            showPage(3);
            return;
        }
        const std::optional<Costume> costume = costumeOf(choice);
        if (!costume) {
            leaveWith(std::string("Master\xA1") + "KI will feel embarrassed...(Shy)");
            return;
        }
        const int slot = m_hooks.suitSlot(costume->suit);
        if (slot == 0) {
            leaveWith("Master, you don't have the costume~");
            return;
        }
        if (m_hooks.pieceItem(*m_furn) == costume->piece) {
            leaveWith("Master, I am wearing the costume now~ (Smile)");
            return;
        }
        m_costume = {static_cast<std::uint8_t>(choice), static_cast<std::uint8_t>(slot)};
        m_speaker = true;
        say("Master, wait... (Blush)", [this] { sendCostumeChange(); });
    }

    void MaidMenu::sendCostumeChange() {
        if (!m_costume.empty() && m_furn) {
            m_hooks.send(protocol::maidRequest(3, *m_furn, m_costume)); // sub id 3: costume change (choice, bag place)
        }
    }

    void MaidMenu::received(const core::Bytes& payload) {
        if (!m_furn || !m_active || payload.size() < 2) { // the sub id is byte 1
            return;
        }
        switch (payload[1]) {
            case 1: // 76/1: open the item bank
                m_hooks.openBank();
                return;
            case 2: // 76/2: open the pets' inn
                m_hooks.openInn();
                return;
            case 3: { // 76/3: new costume (u8 piece number, u16 item id)
                // needs 5 bytes: sub id, piece number, item id as little-endian u16 at bytes 3..4
                if (payload.size() >= 5 && payload[2] == *m_furn) {
                    const int item = payload[3] | (payload[4] << 8); // little-endian u16: high byte shifted by 8
                    m_hooks.changePiece(*m_furn, item);
                    m_figureItem = item;
                    m_services.playSound("sound/Wav0152.wav");
                    m_speaker = true;
                    say("Master, do I look pretty? (Shy)", nullptr);
                }
                break;
            }
            default: // 76/250 and any other sub id: close the box
                hide();
                break;
        }
        m_page = 1;
        m_active = false;
        m_furn.reset();
        m_costume.clear();
    }

    void MaidMenu::formClosed() {
        if (m_active) {
            m_hooks.send(protocol::maidRequest(250, m_furn.value_or(0), m_costume)); // sub id 250: the form closed, end the menu
        }
    }

    void MaidMenu::reset() {
        m_page = 1;
        m_active = false;
        m_furn.reset();
        m_costume.clear();
        hide();
    }

    void MaidMenu::say(const std::string& text, std::function<void()> onClosed) {
        m_lines = ui::wrapText(text, kWrapChars);
        m_onClosed = std::move(onClosed);
        m_onChoice = nullptr;
        m_choices.clear();
        m_choices.setVisible(false);
        m_visible = true;
        m_content.setVisible(true);
    }

    void MaidMenu::choose(const std::vector<std::string>& choices, std::function<void(int)> onChoice) {
        m_lines.clear();
        m_onClosed = nullptr;
        m_onChoice = std::move(onChoice);
        m_choices.clear();
        for (const std::string& choice : choices) {
            m_choices.insertItem(m_choices.itemCount(), choice);
        }
        m_choices.setTopIndex(0);
        m_choices.setVisible(true);
        m_visible = true;
        m_content.setVisible(true);
    }

    void MaidMenu::clicked() {
        if (!m_visible) {
            return;
        }
        const std::function<void()> onClosed = std::move(m_onClosed);
        hide();
        if (onClosed) {
            onClosed();
        }
    }

    void MaidMenu::hide() {
        m_visible = false;
        m_onClosed = nullptr;
        m_onChoice = nullptr;
        m_content.setVisible(false);
        m_choices.setVisible(false);
    }

    void MaidMenu::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (const auto frame = m_services.pictures.find("Form_Talk_1")) {
            canvas.drawPicture(*frame, at);
        }
        for (std::size_t row = 0; row < m_lines.size(); ++row) {
            const std::string& line = m_lines[row];
            const int x = (m_content.bounds().width - static_cast<int>(line.size()) * 8) / 2 + at.x - 3; // line centred (8 px per character), shifted 3 px left
            const int y = at.y + m_textSpot.rect().y + 2 + m_textSpot.pitch().y * static_cast<int>(row); // 2 px below the top margin
            canvas.drawText(
                {x, y},
                line,
                {m_textSpot.color(), gfx::TextEffect::Shadow, std::nullopt},
                m_textSpot.size().width,
                m_textSpot.size().height
            ); // text box height 20
        }
        if (m_speaker) {
            paintFigure(canvas);
            if (!m_onChoice) {
                if (const auto icon = m_services.pictures.find("Icon_Nextone_1")) {
                    const gfx::Picture& picture = m_services.pictures.picture(*icon);
                    const int height = picture.height() / 2;
                    const int frame = static_cast<int>(nowMs / kNextIconStepMs % 2);
                    // arrow at x 22, 25 px (less 7) above the box bottom; the picture holds two frames
                    canvas.drawPicture(
                        *icon,
                        {at.x + 22, at.y + m_content.bounds().height - 25 - height + 7},
                        core::Rect{0, frame * height, picture.width(), height}
                    );
                }
            }
        }
    }

    void MaidMenu::paintFigure(gfx::Canvas& canvas) {
        constexpr int kTypeOfWit = 9011; // sprite 9011 (bank 009): the Wit Maid figure
        int sprite = kTypeOfWit;
        switch (FurnitureKinds::maidKind(m_services.items, m_figureItem)) {
            case 3: sprite = 9054; break; // maid kind 3 (Wit Maid-Summer): sprite 9054
            case 4: sprite = 9053; break; // maid kind 4 (Wit Maid-Rabbit): sprite 9053
            case 5: sprite = 9055; break; // maid kind 5 (Wit Maid-Cat): sprite 9055
            case 26: sprite = 9088; break; // maid kind 26 (Wit Maid-Pumpkin): sprite 9088
            case 27: sprite = 9089; break; // maid kind 27 (Wit Maid-Succubus): sprite 9089
            case 28: sprite = 9095; break; // maid kind 28 (Wit Maid-Qi Pao): sprite 9095
            default: break; // other kinds keep the Wit Maid sprite
        }
        // sprites below 9088 are in the 009_b01 bank first, 9088 and up in 009_c01, the Wit Maid's own in 009
        const std::vector<std::string> banks = sprite == kTypeOfWit ? std::vector<std::string>{"009"}
                                               : sprite < 9088      ? std::vector<std::string>{"009_b01", "009"}
                                                                    : std::vector<std::string>{"009_c01"};
        for (const std::string& bank : banks) {
            if (m_services.characters.hasSprite(bank, sprite)) {
                m_services.characters.drawCampSprite(canvas, bank, sprite, m_figureSpot.at());
                return;
            }
        }
    }

}
