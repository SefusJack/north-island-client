#include "client/viewmodels/login/CharacterSelectForm.h"
#include "client/rules/ElementNames.h"

#include "client/rules/Experience.h"

#include <array>
#include <cmath>
#include <string>

namespace client {

    namespace {

        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};

    }

    CharacterSelectForm::Slot CharacterSelectForm::findSlot(ui::Screen& screen, int number) {
        const std::string prefix = "Slot" + std::to_string(number);
        return Slot{
            screen.get<ui::Element>(prefix + "Empty"),
            screen.get<ui::Element>(prefix + "Info"),
            screen.get<ui::Element>(prefix + "EmptyIcon"),
            screen.get<ui::Element>(prefix + "Details"),
            screen.get<ui::ImageView>(prefix + "Element"),
            screen.get<ui::Label>(prefix + "Name"),
            screen.get<ui::DigitText>(prefix + "Level"),
            screen.get<ui::Element>(prefix + "Career"),
            screen.get<ui::ImageView>(prefix + "Job"),
            screen.get<ui::DigitText>(prefix + "Hp"),
            screen.get<ui::DigitText>(prefix + "Sp"),
            screen.get<ui::ProgressBar>(prefix + "ExpBar"),
            screen.get<ui::DigitText>(prefix + "ExpPercent"),
            screen.get<ui::DigitText>(prefix + "Gold"),
            screen.get<ui::ImageButton>(prefix + "CreateOrDelete"),
            screen.get<ui::DrawingArea>(prefix + "Portrait"),
            screen.get<ui::DrawingArea>(prefix + "Figure")
        };
    }

    CharacterSelectForm::CharacterSelectForm(
        GameServices& services,
        ui::Screen& screen,
        Callbacks callbacks
    )
        : m_services(services),
          m_callbacks(std::move(callbacks)),
          m_form(screen.load(services.game.layout("CharacterSelectForm.xml"), services.loadContext())) {
        // the account has 2 character slots
        for (int number = 1; number <= 2; ++number) {
            m_slots.push_back(findSlot(screen, number));
            const std::string n = std::to_string(number);
            screen.on("onEnter" + n, [this, number](const ui::EventArgs&) { m_callbacks.enter(number); });
            screen.on("onCreateOrDelete" + n, [this, number](const ui::EventArgs&) { createOrDelete(number); });
            screen.on("onTurnLeft" + n, [this, number](const ui::EventArgs&) { turn(number, -1); });
            screen.on("onTurnRight" + n, [this, number](const ui::EventArgs&) { turn(number, 1); });
        }
        screen.on("onCharacterSelectPrevious", [this](const ui::EventArgs&) { m_callbacks.previous(); });
        for (std::size_t i = 0; i < m_slots.size(); ++i) {
            m_slots[i].figure.setPainter([this, i](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paintFigure(m_slots[i], canvas, at, nowMs); });
            m_slots[i].portrait.setPainter([this, i](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
                paintPortrait(m_slots[i], canvas, at, nowMs);
            });
        }
    }

    void CharacterSelectForm::show() {
        // the account has 2 character slots
        for (int number = 1; number <= 2; ++number) {
            fillSlot(m_slots[static_cast<std::size_t>(number - 1)], m_services.session.character(number));
        }
        m_form.setVisible(true);
    }

    void CharacterSelectForm::fillSlot(Slot& slot, const std::optional<protocol::CharacterSummary>& character) {
        const bool used = character.has_value();
        slot.empty.setVisible(!used);
        slot.emptyIcon.setVisible(!used);
        slot.info.setVisible(used);
        slot.details.setVisible(used);
        slot.createOrDelete.setPicture(m_services.pictures.find(used ? "Btn_DeleteCharacter_1" : "Btn_CreateCharacter").value_or(gfx::PictureId{}));
        slot.direction = 11; // 11: the default facing direction (directions run 8..15)
        slot.look.reset();
        slot.blink.restart();
        if (!used) {
            return;
        }
        const protocol::CharacterSummary& c = *character;
        slot.look = CharacterLook::fromSummary(c, m_services.items);
        slot.element.setPicture(m_services.pictures.find("Icon_Element_" + std::to_string(c.element) + "_1").value_or(gfx::PictureId{}));
        slot.element.setHint(elementName(c.element));
        slot.name.setText(c.name);
        slot.level.setText(std::to_string(c.level));
        slot.career.setVisible(c.job != 0);
        const std::size_t job = static_cast<std::size_t>(c.job);
        slot.job.setVisible(job > 0 && job < kJobIcons.size());
        if (slot.job.isVisible()) {
            slot.job.setPicture(m_services.pictures.find(kJobIcons[job]).value_or(gfx::PictureId{}));
        }
        slot.hp.setText(std::to_string(c.hp) + "/" + std::to_string(c.maxHp));
        slot.sp.setText(std::to_string(c.sp) + "/" + std::to_string(c.maxSp));
        const double progress = levelProgress(m_services.formula, c.level, c.exp, c.reborn);
        slot.expBar.setMaximum(slot.expBar.bounds().width);
        slot.expBar.setValue(static_cast<int>(std::trunc(slot.expBar.bounds().width * progress)));
        slot.expPercent.setText(std::to_string(static_cast<int>(std::trunc(progress * 100))) + "%"); // * 100 makes a percent
        slot.gold.setText(std::to_string(c.gold));
    }

    void CharacterSelectForm::turn(int slot, int step) {
        int& direction = m_slots[static_cast<std::size_t>(slot - 1)].direction;
        direction += step;
        // directions run 8..15 and wrap around
        if (direction < 8) {
            direction = 15;
        } else if (direction > 15) {
            direction = 8;
        }
    }

    void CharacterSelectForm::createOrDelete(int slot) {
        if (m_services.session.character(slot)) {
            m_callbacks.remove(slot);
        } else {
            m_callbacks.create(slot);
        }
    }

}

namespace client {

    void CharacterSelectForm::paintFigure(Slot& slot, gfx::Canvas& canvas, core::Point feet, std::uint64_t nowMs) {
        if (!slot.look) {
            return;
        }
        const int frame = slot.animation.frame(nowMs, m_services.characters.figureFrameCount(*slot.look, slot.direction));
        m_services.characters.drawFigure(canvas, *slot.look, feet, slot.direction, frame);
    }

    void CharacterSelectForm::paintPortrait(Slot& slot, gfx::Canvas& canvas, core::Point anchor, std::uint64_t nowMs) {
        if (!slot.look) {
            return;
        }
        m_services.characters.drawPortrait(canvas, *slot.look, anchor, slot.blink.frame(nowMs));
    }

}
