#include "client/viewmodels/login/CreateCharacterForm.h"
#include "client/rules/ElementNames.h"

#include "client/rules/CharacterNames.h"
#include "client/data/StarterRoles.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kShortMessageMs = 2000; // how long a player message stays up, ms
        constexpr int kStartPoints = 5; // stat points to spend on a new character
        constexpr int kNeutralColor = 4; // colour digit 4 = colour unchanged
        constexpr int kMaxColor = 9; // highest colour digit (digits run 0..9)
        constexpr int kColorStepWidth = 12; // px of colour bar per digit step
        constexpr int kColorBarHeight = 15; // colour bar height, px
        constexpr int kFacingSouthWest = 11; // character action slot 11: standing, facing south-west (standing slots run 8..15)

        // first of the three colour-pattern digits of each part (the red, green, blue channels), index = part 1..4 minus 1
        constexpr std::array<std::size_t, 4> kPartFirstDigit = {3, 6, 9, 39};

    }

    CreateCharacterForm::CreateCharacterForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        PasswordForm& passwordForm,
        Callbacks callbacks
    )
        : m_services(services),
          m_messages(messages),
          m_passwordForm(passwordForm),
          m_callbacks(std::move(callbacks)),
          m_form(screen.load(services.game.layout("CreateCharacterForm.xml"), services.loadContext())),
          m_rolePage(screen.get<ui::Element>("CreatePage1")),
          m_pointsPage(screen.get<ui::Element>("CreatePage2")),
          m_pointsBonuses(screen.get<ui::Element>("CreatePage2Bonuses")),
          m_pointsControls(screen.get<ui::Element>("CreatePage2Controls")),
          m_elementControls(screen.get<ui::Element>("CreatePage3Controls")),
          m_colorPage(screen.get<ui::Element>("CreatePage4")),
          m_colorControls(screen.get<ui::Element>("CreatePage4Controls")),
          m_figure(screen.get<ui::DrawingArea>("CreateFigure")),
          m_portrait(screen.get<ui::DrawingArea>("CreatePortrait")),
          m_pointsLeftText(screen.get<ui::DigitText>("CreatePointsLeft")),
          m_description(screen.get<ui::ImageView>("CreateDescription")),
          m_elementFrame(screen.get<ui::ImageView>("CreateElementFrame")),
          m_partFrame(screen.get<ui::ImageView>("CreatePartFrame")),
          m_rolePicture(screen.get<ui::ImageView>("CreateRolePicture")),
          m_nameText(screen.get<ui::Label>("CreateName")),
          m_nameBox(screen.get<ui::TextBox>("CreateNameBox")),
          m_elementIcon(screen.get<ui::ImageView>("CreateElementIcon")),
          m_turnLeft(screen.get<ui::ImageButton>("CreateTurnLeft")),
          m_turnRight(screen.get<ui::ImageButton>("CreateTurnRight")),
          m_previousButton(screen.get<ui::ImageButton>("CreatePrevious")),
          m_nextButton(screen.get<ui::ImageButton>("CreateNext")),
          m_confirmButton(screen.get<ui::ImageButton>("CreateConfirm")),
          m_nameTextSpot(screen.get<ui::Spot>("CreateNameText")),
          m_elementDescriptionSpot(screen.get<ui::Spot>("CreateElementDescription")),
          m_elementRoleSpot(screen.get<ui::Spot>("CreateElementRole")),
          m_carousel(screen) {
        for (int page = 1; page <= 4; ++page) { // four wizard pages
            m_pageBackgrounds.push_back(screen.get<ui::Element>("CreatePage" + std::to_string(page) + "Background"));
        }
        for (int stat = 1; stat <= 5; ++stat) { // five stats: STR, CON, INT, WIS, AGI
            const std::string n = std::to_string(stat);
            m_statTexts.push_back(screen.get<ui::DigitText>("CreateStat" + n));
            m_bonusTexts.push_back(screen.get<ui::Label>("CreateStatBonus" + n));
            on(screen, "onCreateStatDown" + n, [this, stat] { changeStat(stat, -1); });
            on(screen, "onCreateStatUp" + n, [this, stat] { changeStat(stat, 1); });
        }
        for (int channel = 1; channel <= 3; ++channel) { // three colour channels (red, green, blue)
            const std::string n = std::to_string(channel);
            m_colorBars.push_back(screen.get<ui::ImageView>("CreateColorBar" + n));
            on(screen, "onCreateColorDown" + n, [this, channel] { changeColor(channel, -1); });
            on(screen, "onCreateColorUp" + n, [this, channel] { changeColor(channel, 1); });
        }
        for (int role = 1; role <= kStarterRoleCount; ++role) {
            on(screen, "onCreateRole" + std::to_string(role), [this, role] { selectRole(role); });
        }
        for (int element = 1; element <= 4; ++element) { // four elements
            on(screen, "onCreateElement" + std::to_string(element), [this, element] { selectElement(element); });
        }
        for (int part = 1; part <= 4; ++part) { // four colour parts: hair, skin, clothing, eyes
            on(screen, "onCreatePart" + std::to_string(part), [this, part] { selectPart(part); });
        }
        on(screen, "onCreateRolesBack", [this] { m_carousel.back(); });
        on(screen, "onCreateRolesForward", [this] { m_carousel.forward(); });
        on(screen, "onCreateTurnLeft", [this] { turn(-1); });
        on(screen, "onCreateTurnRight", [this] { turn(1); });
        on(screen, "onCreatePrevious", [this] { previous(); });
        on(screen, "onCreateNext", [this] { next(); });
        on(screen, "onCreateConfirm", [this] { confirm(); });
        on(screen, "onCreateDefaultColors", [this] {
            if (!m_nameCheckPending) {
                resetColors();
            }
        });
        m_figure.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paintFigure(canvas, at, nowMs); });
        m_portrait.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paintPortrait(canvas, at, nowMs); });
        reset();
    }

    void CreateCharacterForm::on(ui::Screen& screen, const std::string& event, std::function<void()> action) {
        screen.on(event, [action = std::move(action)](const ui::EventArgs&) { action(); });
    }

    void CreateCharacterForm::show(bool askCipher) {
        reset();
        m_askCipher = askCipher;
        m_blink.restart();
        m_form.setVisible(true);
        goTo(Page::Role);
    }

    void CreateCharacterForm::hide() {
        m_form.setVisible(false);
        m_passwordForm.hide();
        reset();
    }

    void CreateCharacterForm::reset() {
        m_page = Page::Closed;
        m_nameCheckPending = false;
        m_element = 1;
        m_nameBox.clear();
        selectRole(1);
        m_stats.fill(0);
        m_pointsLeft = kStartPoints;
        m_colorPart = 1;
        resetColors();
    }

    void CreateCharacterForm::resetColors() {
        m_colors = formats::ColorPattern();
        m_partColors.fill(kNeutralColor);
        refresh();
    }

    void CreateCharacterForm::goTo(Page page) {
        m_direction = kFacingSouthWest;
        m_page = page;
        switch (page) {
            case Page::Closed: // leaving the wizard: close and cancel
                hide();
                m_callbacks.cancel();
                return;
            case Page::Element: // element page starts on the first element
                selectElement(1);
                break;
            case Page::Cipher: // ask the player to set a cipher
                m_passwordForm
                    .show(PasswordForm::Mode::Set, [this](const PasswordForm::Result& result) { finish(result.cipher); }, [this] { goTo(Page::Colors); });
                break;
            default: // role, points and colour pages need no extra setup
                break;
        }
        showPage();
        refresh();
    }

    void CreateCharacterForm::showPage() {
        const bool colors = m_page == Page::Colors;
        for (std::size_t i = 0; i < m_pageBackgrounds.size(); ++i) {
            const auto page = static_cast<Page>(static_cast<int>(Page::Role) + static_cast<int>(i));
            m_pageBackgrounds[i].get().setVisible(m_page == page);
        }
        m_rolePage.setVisible(m_page == Page::Role);
        m_pointsPage.setVisible(m_page == Page::Points);
        m_pointsBonuses.setVisible(m_page == Page::Points);
        m_pointsControls.setVisible(m_page == Page::Points);
        m_elementFrame.setVisible(m_page == Page::Element);
        m_elementControls.setVisible(m_page == Page::Element);
        m_colorPage.setVisible(colors);
        m_colorControls.setVisible(colors);

        const bool character = m_page != Page::Cipher;
        m_figure.setVisible(character);
        m_portrait.setVisible(character);
        m_figure.setPosition(colors ? core::Point{549, 387} : core::Point{678, 387}); // figure position on colour page / other pages
        m_portrait.setPosition(colors ? core::Point{460, 328} : core::Point{588, 327}); // portrait position on colour page / other pages

        const bool turning = m_page == Page::Role || colors;
        m_turnLeft.setVisible(turning);
        m_turnRight.setVisible(turning);
        m_turnLeft.setPosition(colors ? core::Point{494, 369} : core::Point{622, 368}); // turn-left arrow position on colour page / other pages
        m_turnRight.setPosition(colors ? core::Point{583, 369} : core::Point{711, 368}); // turn-right arrow position on colour page / other pages

        m_previousButton.setVisible(character);
        m_previousButton.setPosition(colors ? core::Point{468, 447} : core::Point{595, 447}); // previous button position on colour page / other pages
        m_nextButton.setVisible(character && (!colors || m_askCipher));
        m_nextButton.setPosition(colors ? core::Point{570, 447} : core::Point{700, 447}); // next button position on colour page / other pages
        m_confirmButton.setVisible(colors && !m_askCipher);
        m_confirmButton.setPosition({570, 447}); // confirm button position (same slot as next on colour page)

        const bool described = m_page == Page::Points || m_page == Page::Element;
        m_description.setVisible(described);
        m_rolePicture.setVisible(described);
        m_nameText.setVisible(described || colors);
        m_elementIcon.setVisible(m_page == Page::Element || colors);
        m_elementIcon.setPosition(colors ? core::Point{574, 289} : core::Point{700, 289}); // element icon position on colour page / element page
    }

    void CreateCharacterForm::refresh() {
        const StarterRole& role = starterRole(m_role);
        gfx::PictureLibrary& pictures = m_services.pictures;
        const auto findPicture = [&](const std::string& name) { return pictures.find(name).value_or(gfx::PictureId{}); };

        m_pointsLeftText.setText(std::to_string(m_pointsLeft));
        const std::array<int, 5> bonuses = roleStatBonuses(roleIndex(role.bodyType, role.head));
        for (std::size_t i = 0; i < m_stats.size(); ++i) {
            m_statTexts[i].get().setText(std::to_string(m_stats[i]));
            m_bonusTexts[i].get().setText(bonuses[i] > 0 ? "+ " + std::to_string(bonuses[i]) : std::string());
        }

        const bool elementPage = m_page == Page::Element;
        const gfx::PictureId description = findPicture(elementPage ? "ElementInfo_" + std::to_string(m_element) : role.infoPicture);
        m_description.setPicture(description);
        if (description.isValid()) {
            const gfx::Picture& picture = pictures.picture(description);
            const core::Point at = elementPage ? m_elementDescriptionSpot.at() : role.infoPosition;
            m_description.setBounds({at.x, at.y, picture.width(), picture.height()});
        }

        const gfx::PictureId rolePicture = findPicture("Btn_RolePic_" + std::to_string(m_role));
        m_rolePicture.setPicture(rolePicture);
        if (rolePicture.isValid()) {
            const gfx::Picture& picture = pictures.picture(rolePicture);
            const int height = picture.height() / 3; // the picture holds three stacked looks
            const core::Point anchor = elementPage ? m_elementRoleSpot.at() : role.picturePosition;
            m_rolePicture.setBounds({anchor.x - picture.width() / 2, anchor.y - height, picture.width(), height});
            m_rolePicture.setSourceRect(core::Rect{0, 2 * height, picture.width(), height}); // third look (pressed) starts two looks down
        }

        m_elementFrame.setPosition({(m_element - 1) * 60 + 294, 244}); // element frame: 60 px spacing, first at x 294, y 244
        m_elementIcon.setPicture(findPicture("Icon_Element_" + std::to_string(m_element) + "_1"));
        m_elementIcon.setHint(elementName(m_element));
        m_partFrame.setPosition({(m_colorPart - 1) * 52 + 167, 262}); // part frame: 52 px spacing, first at x 167, y 262
        for (std::size_t i = 0; i < m_colorBars.size(); ++i) {
            ui::ImageView& bar = m_colorBars[i].get();
            bar.setVisible(m_partColors[i] > 0);
            bar.setSourceRect(core::Rect{0, 0, m_partColors[i] * kColorStepWidth, kColorBarHeight});
        }

        const std::string& name = m_nameBox.text();
        const int nameCentre = m_page == Page::Colors ? 550 : 643; // x centre of the name on colour page / other pages
        m_nameText.setText(name);
        m_nameText.setPosition({nameCentre - static_cast<int>(name.size()) * m_nameTextSpot.pitch().x / 2, m_nameTextSpot.at().y});
    }

    void CreateCharacterForm::next() {
        if (m_nameCheckPending) {
            return;
        }
        switch (m_page) {
            case Page::Role: // validate the name, then ask the server (9/2)
                if (const auto problem = characterNameProblem(m_nameBox.text())) {
                    m_messages.show(*problem, kShortMessageMs);
                } else {
                    m_nameCheckPending = true;
                    m_callbacks.checkName(m_nameBox.text());
                }
                break;
            case Page::Points: // all points must be given before moving on
                if (m_pointsLeft != 0) {
                    m_messages.show("Never finish distributing points.", kShortMessageMs);
                } else {
                    goTo(Page::Element);
                }
                break;
            case Page::Element: // on to the colour page
                goTo(Page::Colors);
                break;
            case Page::Colors: // on to the cipher page only when a cipher is needed
                if (m_askCipher) {
                    goTo(Page::Cipher);
                }
                break;
            default: // closed and cipher pages have no next step here
                break;
        }
    }

    void CreateCharacterForm::previous() {
        if (m_nameCheckPending) {
            return;
        }
        switch (m_page) {
            case Page::Role: // back to character select
                goTo(Page::Closed);
                break;
            case Page::Points: // back to role and name
                goTo(Page::Role);
                break;
            // back to the points page
            case Page::Element:
                selectRole(m_role);
                goTo(Page::Points);
                break;
            case Page::Colors: // back to the element page
                goTo(Page::Element);
                break;
            default: // closed and cipher pages have no previous step here
                break;
        }
    }

    void CreateCharacterForm::confirm() {
        if (!m_nameCheckPending && m_page == Page::Colors && !m_askCipher) {
            finish(std::nullopt);
        }
    }

    void CreateCharacterForm::nameChecked(NameCheckResult result) {
        if (!m_nameCheckPending) {
            return;
        }
        m_nameCheckPending = false;
        switch (result) {
            case NameCheckResult::Accepted: // name is free: on to the points page
                if (m_page == Page::Role) {
                    goTo(Page::Points);
                }
                break;
            case NameCheckResult::Unavailable: // name already taken
                m_messages.show("Unavailable name, please enter again.", kShortMessageMs);
                break;
            case NameCheckResult::Invalid: // server rejected the name
                m_messages.show("invalid name, please enter again!", kShortMessageMs);
                break;
        }
    }

    void CreateCharacterForm::finish(std::optional<std::string> cipher) {
        const StarterRole& role = starterRole(m_role);
        protocol::CreateCharacterRequest request;
        request.bodyType = role.bodyType;
        request.head = role.head;
        request.colorCode1 = m_colors.code(1); // first half of the colour pattern code
        request.colorCode2 = m_colors.code(5); // second half of the colour pattern code
        request.element = m_element;
        request.stats = m_stats;
        request.cipher = std::move(cipher);
        hide();
        m_callbacks.create(request);
    }

    void CreateCharacterForm::turn(int step) {
        if (m_nameCheckPending) {
            return;
        }
        m_direction += step;
        if (m_direction < 8) { // standing directions run 8..15
            m_direction = 15;
        } else if (m_direction > 15) {
            m_direction = 8;
        }
    }

    void CreateCharacterForm::selectRole(int role) {
        if (role < 1 || role > kStarterRoleCount) {
            return;
        }
        m_role = role;
        m_direction = kFacingSouthWest;
        m_animation.restart();
        refresh();
    }

    void CreateCharacterForm::selectElement(int element) {
        m_element = element;
        refresh();
    }

    void CreateCharacterForm::selectPart(int part) {
        m_colorPart = part;
        const std::size_t first = kPartFirstDigit[static_cast<std::size_t>(part - 1)];
        for (std::size_t i = 0; i < m_partColors.size(); ++i) {
            m_partColors[i] = m_colors.digit(first + i);
        }
        refresh();
    }

    void CreateCharacterForm::changeStat(int stat, int step) {
        int& value = m_stats[static_cast<std::size_t>(stat - 1)];
        if (step < 0 && value > 0) {
            --value;
            ++m_pointsLeft;
        } else if (step > 0 && m_pointsLeft > 0) {
            --m_pointsLeft;
            ++value;
        }
        refresh();
    }

    void CreateCharacterForm::changeColor(int channel, int step) {
        int& value = m_partColors[static_cast<std::size_t>(channel - 1)];
        value = std::clamp(value + step, 0, kMaxColor); // colour digit range 0..9
        const std::size_t first = kPartFirstDigit[static_cast<std::size_t>(m_colorPart - 1)];
        for (std::size_t i = 0; i < m_partColors.size(); ++i) {
            m_colors.setDigit(first + i, static_cast<std::uint8_t>(m_partColors[i]));
        }
        refresh();
    }

    CharacterLook CreateCharacterForm::look() const {
        const StarterRole& role = starterRole(m_role);
        const std::array<std::uint16_t, 6> equipment =
            m_page == Page::Colors ? std::array<std::uint16_t, 6>{} : starterEquipment(roleIndex(role.bodyType, role.head));
        return CharacterLook::dressed(role.bodyType, role.head, m_colors, equipment, m_services.items);
    }

    void CreateCharacterForm::paintFigure(gfx::Canvas& canvas, core::Point feet, std::uint64_t nowMs) {
        const CharacterLook character = look();
        const int frame = m_animation.frame(nowMs, m_services.characters.figureFrameCount(character, m_direction));
        m_services.characters.drawFigure(canvas, character, feet, m_direction, frame);
    }

    void CreateCharacterForm::paintPortrait(gfx::Canvas& canvas, core::Point anchor, std::uint64_t nowMs) {
        m_services.characters.drawPortrait(canvas, look(), anchor, m_blink.frame(nowMs));
    }

    void CreateCharacterForm::update(std::uint64_t nowMs) {
        if (m_page == Page::Role) {
            m_carousel.update(nowMs);
        }
    }

}
