#include "client/viewmodels/crafting/DyeWindow.h"

#include "protocol/DyeRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <array>
#include <functional>
#include <string>

namespace client {

    namespace {

        // pattern digit index of the red digit (green and blue follow) per part: [0] unused, hair 3, skin 6, eyes 39, clothes 9
        constexpr std::array<std::size_t, 5> kFirstDigit = {0, 3, 6, 39, 9};
        constexpr const char* kPartIcons[] = {"", "Icon_hair", "Icon_Skin", "icon_eye", "Icon_Part_3"};
        constexpr const char* kBars[] = {"", "Color_R", "Color_G", "Color_B"};
        constexpr int kMost = 9; // largest value of a colour digit (digits run 0..9)
        constexpr int kStandingSlot = 1; // slot of the standing portrait
        constexpr int kProfileSlot = 11; // body action slot of the animated figure: standing (8..15), direction 3 (south-west)
        constexpr int kSubTypeHair = 56; // item sub type 56: Hair Dye
        constexpr int kSubTypeSkin = 57; // item sub type 57: Color Mask (skin dye)
        constexpr int kSubTypeEyes = 102; // item sub type 102: Strange Eye-drops
        constexpr int kSubTypeClothes = 148; // item sub type 148: Clothes Dye

        /// @brief Loads DyeWindow.xml and returns the window.
        /// @param services Game services providing the layout and load context.
        /// @param screen Screen the layout is loaded into.
        /// @return The Dye window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("DyeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Dye");
        }

    }

    DyeWindow::DyeWindow(
        GameServices& services,
        ui::Screen& screen,
        const CharacterLook& ownLook
    )
        : m_services(services),
          m_screen(screen),
          m_ownLook(ownLook),
          m_window(loadWindow(services, screen)),
          m_standingSpot(screen.get<ui::Spot>("DyeStanding")),
          m_profileSpot(screen.get<ui::Spot>("DyeProfile")),
          m_iconSpot(screen.get<ui::Spot>("DyeIcon")),
          m_barSpot(screen.get<ui::Spot>("DyeBar")) {
        screen.get<ui::DrawingArea>("DyePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        for (int channel = 1; channel <= 3; ++channel) { // colour channels 1..3: red, green, blue
            screen.on("onDyeLess" + std::to_string(channel), [this, channel](const ui::EventArgs&) { step(channel, -1); });
            screen.on("onDyeMore" + std::to_string(channel), [this, channel](const ui::EventArgs&) { step(channel, 1); });
        }
        screen.on("onDyePrimal", [this](const ui::EventArgs&) { restore(); });
        screen.on("onDyeOk", [this](const ui::EventArgs&) { send(); });
        screen.on("onDyeClose", [this](const ui::EventArgs&) { hide(); });
    }

    int DyeWindow::partOf(int itemSubType) {
        switch (itemSubType) {
            case kSubTypeHair: return kHair; // item sub type 56: hair dye
            case kSubTypeSkin: return kSkin; // item sub type 57: skin dye
            case kSubTypeEyes: return kEyes; // item sub type 102: eye dye
            case kSubTypeClothes: return kClothes; // item sub type 148: clothes dye
            default: return 0; // not a dye
        }
    }

    void DyeWindow::toggle(int bagSlot, int part) {
        m_bagSlot = bagSlot;
        m_part = part;
        if (m_window.isVisible()) {
            hide();
        } else {
            show();
        }
    }

    void DyeWindow::show() {
        m_standing = m_ownLook;
        m_profile = m_ownLook;
        if (m_part == kClothes) {
            m_profile.equipmentLooks.fill(0);
        }
        for (int channel = 1; channel <= 3; ++channel) { // colour channels 1..3: red, green, blue
            const int value = m_standing.basePattern.digit(kFirstDigit[static_cast<std::size_t>(m_part)] + static_cast<std::size_t>(channel - 1));
            m_original[static_cast<std::size_t>(channel)] = value;
            m_colors[static_cast<std::size_t>(channel)] = value;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void DyeWindow::setDigits(int channel, int value) {
        const std::size_t digit = kFirstDigit[static_cast<std::size_t>(m_part)] + static_cast<std::size_t>(channel - 1);
        for (CharacterLook& look : std::array<std::reference_wrapper<CharacterLook>, 2>{m_standing, m_profile}) {
            look.basePattern.setDigit(digit, static_cast<std::uint8_t>(value));
            look.pattern.setDigit(digit, static_cast<std::uint8_t>(value));
        }
        m_colors[static_cast<std::size_t>(channel)] = value;
    }

    void DyeWindow::step(int channel, int by) {
        const int value = m_colors[static_cast<std::size_t>(channel)];
        if ((by < 0 && value == 0) || (by > 0 && value == kMost)) {
            return;
        }
        setDigits(channel, value + by);
    }

    void DyeWindow::restore() {
        for (int channel = 1; channel <= 3; ++channel) { // colour channels 1..3: red, green, blue
            setDigits(channel, m_original[static_cast<std::size_t>(channel)]);
        }
    }

    void DyeWindow::send() {
        m_services.connection.send(protocol::dyeRequest(m_bagSlot, m_colors[1], m_colors[2], m_colors[3]));
        hide();
    }

    void DyeWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        m_services.characters.drawPortrait(canvas, m_standing, at + m_standingSpot.at(), 0, kStandingSlot); // 0: frame 0 of the portrait
        const int profileFrame = m_animation.frame(nowMs, m_services.characters.figureFrameCount(m_profile, kProfileSlot));
        m_services.characters.drawFigure(canvas, m_profile, at + m_profileSpot.at(), kProfileSlot, profileFrame);
        if (const auto icon = m_services.pictures.find(kPartIcons[m_part])) {
            canvas.drawPicture(*icon, at + m_iconSpot.at());
        }
        for (int channel = 1; channel <= 3; ++channel) { // colour channels 1..3: red, green, blue
            if (const auto bar = m_services.pictures.find(kBars[channel])) {
                canvas.drawPicture(
                    *bar,
                    {at.x + m_barSpot.at().x, at.y + m_barSpot.at().y + (channel - 1) * m_barSpot.pitch().y},
                    core::Rect{0, 0, m_colors[static_cast<std::size_t>(channel)] * m_barSpot.rect().width, m_barSpot.rect().height}
                );
            }
        }
    }

}
