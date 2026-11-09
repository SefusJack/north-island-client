#include "client/viewmodels/hud/MainStatus.h"
#include "client/rules/ElementNames.h"

#include "client/rules/Experience.h"
#include "ui/Tooltip.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace client {

    namespace {

        // job icon picture per job number (index 0 = no job): 1 Assassin, 2 Samurai, 3 Knight, 4 Sage, 5 Clergyman, 6 Prophet
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn_2", "Icon_Samurai_2", "Icon_Knight_2", "Icon_Sage_2", "Icon_Clergyman_2", "Icon_Prophet_2"};
        constexpr int kStatusPortraitSlot = 0; // face slot 0: the small face of the status panel
        constexpr int kSlideWidth = 241 + 2; // panel width (241) plus 2 px, the distance it slides, px
        constexpr int kRestX = 2; // resting x offset of the panel, px
        constexpr std::uint64_t kSlideMs = 500; // slide in/out duration, ms

    }

    MainStatus::MainStatus(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        const CharacterLook& look,
        const PetTeam& pets
    )
        : m_services(services),
          m_screen(screen),
          m_status(status),
          m_look(look),
          m_pets(pets),
          m_panel(screen.load(services.game.layout("MainStatus.xml"), services.loadContext())),
          m_element(screen.get<ui::ImageView>("StatusElement")),
          m_job(screen.get<ui::ImageView>("StatusJob")),
          m_level(screen.get<ui::DigitText>("StatusLevel")),
          m_hp(screen.get<ui::DigitText>("StatusHp")),
          m_sp(screen.get<ui::DigitText>("StatusSp")),
          m_expBar(screen.get<ui::ProgressBar>("StatusExpBar")),
          m_expPercent(screen.get<ui::DigitText>("StatusExpPercent")),
          m_gold(screen.get<ui::DigitText>("StatusGold")),
          m_hpArea(screen.get<ui::DrawingArea>("StatusHpGauge")),
          m_spArea(screen.get<ui::DrawingArea>("StatusSpGauge")),
          m_petArea(screen.get<ui::DrawingArea>("StatusPet")),
          m_petPartsSpot(screen.get<ui::Spot>("StatusPetParts")),
          m_hpGauge(services.pictures, "Main_Hp"),
          m_spGauge(services.pictures, "Main_Sp"),
          m_petInfo(services, PetInfoPainter::Style{m_petPartsSpot.at(), false, true}),
          m_recovery(screen.get<ui::Element>("StatusRecovery")) {
        m_hpArea.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { m_hpGauge.paint(canvas, at, m_status.hp, m_status.maxHp); });
        m_spArea.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { m_spGauge.paint(canvas, at, m_status.sp, m_status.maxSp); });
        screen.get<ui::DrawingArea>("StatusPortrait").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            m_services.characters.drawPortrait(canvas, m_look, at, m_blink.frame(nowMs), kStatusPortraitSlot);
        });
        m_petArea.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paintPet(canvas, at, nowMs); });
        screen.get<ui::DrawingArea>("StatusVehicle").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintVehicle(canvas, at); });
        screen.get<ui::DrawingArea>("StatusHints").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintHints(canvas, at); });
    }

    void MainStatus::paintHints(gfx::Canvas& canvas, core::Point at) {
        const core::Point mouse = m_screen.mouseIn(m_panel);
        const auto& pet = m_pets.at(m_pets.activePlace());
        // the pet's hints are drawn here, over the player's gauges, not with the pet that is drawn under them
        const core::Point petAt{at.x + m_petArea.bounds().x, at.y + m_petArea.bounds().y};
        const core::Rect petHp = m_petInfo.hpBar(petAt);
        const core::Rect petSp = m_petInfo.spBar(petAt);
        const bool onPetBar = pet && (petHp.contains(mouse) || petSp.contains(mouse));
        if (pet) {
            if (petHp.contains(mouse)) {
                ui::drawTooltipAt(canvas, {petHp.x, petHp.y + petHp.height}, "Hp:" + std::to_string(pet->hp) + "/" + std::to_string(pet->maxHp));
            } else if (petSp.contains(mouse)) {
                ui::drawTooltipAt(canvas, {petSp.x, petSp.y + petSp.height}, "Sp:" + std::to_string(pet->sp) + "/" + std::to_string(pet->maxSp));
            }
            m_petInfo.paintExpHint(canvas, petAt, *pet, mouse);
        }
        const auto onGauge = [&](const ui::DrawingArea& area, const char* picture) {
            const core::Rect rect{at.x + area.bounds().x, at.y + area.bounds().y, area.bounds().width, area.bounds().height};
            const auto found = m_services.pictures.find(picture);
            return rect.contains(mouse) && found && m_services.pictures.picture(*found).isOpaque(mouse.x - rect.x, mouse.y - rect.y);
        };
        const auto tip = [&](const ui::DrawingArea& area, const std::string& text) {
            ui::drawTooltipAt(canvas, {at.x + area.bounds().x, at.y + area.bounds().y + area.bounds().height}, text);
        };
        if (!onPetBar) {
            if (onGauge(m_hpArea, "Main_Hp")) {
                tip(m_hpArea, "Hp:" + std::to_string(m_status.hp) + "/" + std::to_string(m_status.maxHp));
            } else if (onGauge(m_spArea, "Main_Sp")) {
                tip(m_spArea, "Sp:" + std::to_string(m_status.sp) + "/" + std::to_string(m_status.maxSp));
            }
        }
        const core::Rect expBar{at.x + 41, at.y + 168, 47, 8}; // experience bar rect in the panel: x, y, width, height
        if (expBar.contains(mouse)) {
            const std::int64_t reached = totalLevelExp(m_services.formula, m_status.level, m_status.reborn);
            const std::int64_t needed = levelUpExp(m_services.formula, m_status.level + 1, m_status.reborn);
            const std::string text = "Exp: " + std::to_string(m_status.exp - reached) + "/" + std::to_string(needed);
            // hint box: offset from the mouse (-1, -23), 8 px per character + 7 padding, 17 px high
            const core::Rect box{mouse.x - 1, mouse.y - 23, static_cast<int>(text.size()) * 8 + 7, 17};
            canvas.fillRectAlpha(box, {61, 138, 249, 200}); // fill colour RGBA (blue, alpha 200)
            canvas.drawRectOutline(box, {0, 0, 128, 255}); // outline colour RGBA (dark blue)
            // text: 3 px inset, white RGBA, font weight 400, height 16
            canvas.drawText({box.x + 3, box.y}, text, {{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt}, 400, 16);
        }
    }

    void MainStatus::paintVehicle(gfx::Canvas& canvas, core::Point at) {
        const auto gauge = m_vehicleGauge ? m_vehicleGauge() : std::nullopt;
        if (!gauge) {
            return;
        }
        const core::Point origin{at.x + 5, at.y + 5}; // vehicle icon offset (5, 5) in the drawing area
        if (const auto icon = m_services.pictures.find(gauge->icon)) {
            if (gauge->capsule) {
                canvas.drawPicture(*icon, origin);
            } else {
                // icon scaled to 10 x 10 cells
                canvas.drawPictureScaled(*icon, {origin.x, origin.y, gauge->cellWidth * 10, gauge->cellHeight * 10});
            }
        }
        // rails sit right of the icon (10 cells wide) plus 5 px, at half its height (5 cells) minus 5 px
        const core::Point rail{origin.x + gauge->cellWidth * 10 + 5, origin.y + gauge->cellHeight * 5 - 5};
        const auto strip = [&](const char* name, core::Point where, double part) {
            if (const auto picture = m_services.pictures.find(name)) {
                const gfx::Picture& data = m_services.pictures.picture(*picture);
                canvas.drawPicture(*picture, where, core::Rect{0, 0, static_cast<int>(std::lround(data.width() * part)), data.height()});
            }
        };
        if (const auto rails = m_services.pictures.find("icon_rail2")) {
            canvas.drawPicture(*rails, rail);
            if (gauge->hasFuel) {
                canvas.drawPicture(*rails, {rail.x, rail.y + 5}); // fuel rail is 5 px below the wear rail
            }
        }
        strip("icon_HP2", rail, gauge->durability);
        if (gauge->hasFuel) {
            strip("icon_SP2", {rail.x, rail.y + 5}, gauge->fuel); // fuel strip is 5 px below the wear strip
        }
    }

    void MainStatus::paintPet(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const auto& pet = m_pets.at(m_pets.activePlace());
        if (!pet) {
            return;
        }
        m_petInfo.paint(canvas, at, *pet, nowMs);
    }

    void MainStatus::slideOut(std::uint64_t nowMs, std::function<void()> onGone) {
        m_slide = Slide::Out;
        m_slideMs = nowMs;
        m_onGone = std::move(onGone);
        m_recovery.setVisible(false);
    }

    void MainStatus::slideIn(std::uint64_t nowMs) {
        m_slide = Slide::In;
        m_slideMs = nowMs;
        placeAt(-kSlideWidth - kRestX);
        m_panel.setVisible(true);
        m_recovery.setVisible(true);
    }

    void MainStatus::update(std::uint64_t nowMs) {
        if (m_slide == Slide::None) {
            return;
        }
        const int offset = static_cast<int>(std::ceil(static_cast<double>(nowMs - m_slideMs) * kSlideWidth / kSlideMs));
        if (m_slide == Slide::Out) {
            if (offset >= kSlideWidth) {
                m_slide = Slide::None;
                m_panel.setVisible(false);
                placeAt(0);
                if (m_onGone) {
                    m_onGone();
                }
                return;
            }
            placeAt(-offset - kRestX);
        } else {
            if (offset >= kSlideWidth) {
                m_slide = Slide::None;
                placeAt(0);
                return;
            }
            placeAt(-kSlideWidth - kRestX + offset);
        }
    }

    void MainStatus::placeAt(int x) {
        const core::Point shift = m_panel.viewShift();
        m_panel.setPosition({shift.x + x, shift.y});
    }

    void MainStatus::refresh() {
        const PlayerStatus& status = m_status;
        m_element.setPicture(m_services.pictures.find("Icon_Element_" + std::to_string(status.element) + "_1").value_or(gfx::PictureId{}));
        m_element.setHint(elementName(status.element));
        const std::size_t job = static_cast<std::size_t>(status.job);
        m_job.setVisible(job >= 1 && job < kJobIcons.size());
        if (m_job.isVisible()) {
            m_job.setPicture(m_services.pictures.find(kJobIcons[job]).value_or(gfx::PictureId{}));
        }
        m_level.setText(std::to_string(status.level));
        m_hp.setText(std::to_string(status.hp) + "/" + std::to_string(status.maxHp));
        m_sp.setText(std::to_string(status.sp) + "/" + std::to_string(status.maxSp));
        const double progress = levelProgress(m_services.formula, status.level, status.exp, status.reborn);
        m_expBar.setMaximum(m_expBar.bounds().width);
        m_expBar.setValue(static_cast<int>(std::trunc(m_expBar.bounds().width * progress)));
        m_expPercent.setText(std::to_string(static_cast<int>(std::trunc(progress * 100))) + "%"); // 100: fraction to percent
        m_gold.setText(std::to_string(status.gold));
    }

}
