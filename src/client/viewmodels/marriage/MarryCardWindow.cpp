#include "client/viewmodels/marriage/MarryCardWindow.h"

#include "client/rendering/hud/GameNumbers.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"

#include <array>

namespace client {

    namespace {

        constexpr int kFigureSlot = 17; // action slot of the spouse's full figure
        constexpr int kFaceSlot = 2; // face slot 2: the large face
        constexpr int kFaceReach = 30; // double-click on the face counts within 30 px of its anchor
        constexpr int kAttributesX = 60; // x of the 5 attribute values
        constexpr std::array<int, 5> kAttributesY = {187, 204, 220, 235, 250}; // y of each attribute value, in px
        constexpr int kGlyphWidth = 8; // width of one text character, px
        // online state 0..3: online, busy, inline (away), offline
        const std::array<const char*, 4> kOnlineIcons = {"Icon_online_1", "Icon_Busy_1", "Icon_inline_1", "icon_offline_1"};
        // job 0 none, 1 Assassin, 2 Samurai, 3 Knight, 4 Sage, 5 Clergyman, 6 Prophet
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};

        /// @brief Loads MarryCardWindow.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "MarryCardWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MarryCardWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MarryCardWindow");
        }

    }

    MarryCardWindow::MarryCardWindow(
        GameServices& services,
        ui::Screen& screen,
        Marriage& marriage,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_marriage(marriage),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_figureSpot(screen.get<ui::Spot>("MarryCardFigure")),
          m_faceSpot(screen.get<ui::Spot>("MarryCardFace")),
          m_elementSpot(screen.get<ui::Spot>("MarryCardElement")),
          m_onlineSpot(screen.get<ui::Spot>("MarryCardOnline")),
          m_onlinePartSpot(screen.get<ui::Spot>("MarryCardOnlinePart")),
          m_levelSpot(screen.get<ui::Spot>("MarryCardLevel")),
          m_jobSpot(screen.get<ui::Spot>("MarryCardJob")),
          m_nameSpot(screen.get<ui::Spot>("MarryCardName")),
          m_guildSpot(screen.get<ui::Spot>("MarryCardGuild")),
          m_nicknameSpot(screen.get<ui::Spot>("MarryCardNickname")),
          m_locationSpot(screen.get<ui::Spot>("MarryCardLocation")),
          m_statSpot(screen.get<ui::Spot>("MarryCardStat")) {
        screen.get<ui::ImageButton>("MarryCardTabMarry").setSelected(true);
        screen.get<ui::DrawingArea>("MarryCardPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.on("onMarryCardClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMarryCardMail", [this](const ui::EventArgs&) { m_hooks.mailSpouse(); });
        screen.on("onMarryCardTabFriend", [this](const ui::EventArgs&) { backTo(m_hooks.showFriends); });
        screen.on("onMarryCardTabEmote", [this](const ui::EventArgs&) { backTo(m_hooks.showEmotions); });
        screen.on("onMarryCardTabSetup", [this](const ui::EventArgs&) { backTo(m_hooks.showSetup); });
        screen.on("onMarryCardDoubleClick", [this](const ui::EventArgs&) { doubleClicked(); });
    }

    void MarryCardWindow::open(core::Point at) {
        const core::Rect bounds = m_window.bounds();
        m_window.setBounds({at.x, at.y, bounds.width, bounds.height});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void MarryCardWindow::backTo(const std::function<void(core::Point)>& show) {
        const core::Rect bounds = m_window.bounds();
        close();
        show({bounds.x, bounds.y});
    }

    void MarryCardWindow::doubleClicked() {
        const core::Point mouse = m_screen.mouse();
        const core::Rect bounds = m_window.bounds();
        const core::Point face{bounds.x + m_faceSpot.at().x, bounds.y + m_faceSpot.at().y};
        if (m_marriage.spouseId() != 0 && mouse.x > face.x - kFaceReach && mouse.x < face.x + kFaceReach && mouse.y > face.y - kFaceReach &&
            mouse.y < face.y + kFaceReach) {
            m_hooks.whisper(m_marriage.card().id);
        }
    }

    void MarryCardWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const core::Rect bounds = m_window.bounds();
        if (m_marriage.spouseId() == 0) {
            close();
            m_hooks.showFriends({bounds.x, bounds.y});
            return;
        }
        const SpouseCard& card = m_marriage.card();
        const auto draw = [&](const std::string& name, core::Point where, const std::optional<core::Rect>& source = std::nullopt) {
            if (const auto icon = m_services.pictures.find(name)) {
                canvas.drawPicture(*icon, where, source);
            }
        };
        const gfx::TextStyle text{m_statSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const int frame = m_animation.frame(nowMs, m_services.characters.figureFrameCount(card.look, kFigureSlot));
        m_services.characters.drawFigure(canvas, card.look, at + m_figureSpot.at(), kFigureSlot, frame);
        m_services.characters.drawPortrait(canvas, card.look, at + m_faceSpot.at(), 0, kFaceSlot); // animation frame 0
        draw("Icon_Element_" + std::to_string(card.element) + "_1", at + m_elementSpot.at());
        draw(kOnlineIcons[static_cast<std::size_t>(card.onlineState)], at + m_onlineSpot.at(), m_onlinePartSpot.rect());
        drawLevelDigits(canvas, card.level, at + m_levelSpot.at());
        if (card.job >= 1 && card.job < static_cast<int>(kJobIcons.size())) {
            draw(kJobIcons[static_cast<std::size_t>(card.job)], at + m_jobSpot.at());
        }
        // 5 attributes: strength, constitution, intelligence, wisdom, agility
        const std::array<int, 5> attributes = {card.strength, card.constitution, card.intelligence, card.wisdom, card.agility};
        for (std::size_t i = 0; i < attributes.size(); ++i) {
            canvas.drawText(
                {at.x + kAttributesX, at.y + kAttributesY[i]},
                std::to_string(attributes[i]),
                text,
                m_statSpot.rect().width,
                m_statSpot.rect().height
            );
        }
        const auto line = [&](const std::string& value, core::Point where) {
            canvas.drawText({at.x + where.x, at.y + where.y}, value, text, static_cast<int>(value.size()) * kGlyphWidth, m_statSpot.rect().height);
        };
        line(card.location, m_locationSpot.at());
        line(card.name, m_nameSpot.at());
        line(card.guild, m_guildSpot.at());
        line(card.nickname, m_nicknameSpot.at());
    }

}
