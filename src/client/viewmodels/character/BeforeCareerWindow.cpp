#include "client/viewmodels/character/BeforeCareerWindow.h"

#include "client/data/CareerLooks.h"
#include "client/data/CareerTexts.h"
#include "ui/DrawingArea.h"
#include "ui/WrappedTooltip.h"

namespace client {

    namespace {

        constexpr int kModelCount = 2; // career models shown: 1 soldier, 2 mage
        // click areas of the two models in the window, {x, y, width, height} px
        constexpr std::array<core::Rect, kModelCount> kModelRects = {{{28, 193, 120, 115}, {170, 193, 120, 115}}};
        // highlight frames of the chosen model, {x, y, width, height} px
        constexpr std::array<core::Rect, kModelCount> kChosenFrames = {{{28, 193, 122, 115}, {170, 193, 121, 115}}};
        // feet positions of the two career models in the window, px
        constexpr std::array<core::Point, kModelCount> kModelFeet = {{{90, 285}, {230, 285}}};
        constexpr int kTipLineHeight = 8; // tooltip line height, px
        constexpr int kMessageMs = 3000; // message display time, ms

        const char* const kSoldier =
            "    We called them soldiers\xA1"
            "Athey think strength is the truth of the world    They are good at a close battlesEven if they aren't good at fighting with spells\xA1"
            "Aonce they have a weaponthey will fight withmonsters stronger than themand never be scared";
        const char* const kMage =
            "    They control the elements around themand split or combine the elementsto suit their chemical needsWe call them Mages      Though "
            "they aren't good atclose battlesthey can defeat strong army ifthey unleash a powerful incantation";

        /// @brief Loads BeforeCareerWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "BeforeCareerWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BeforeCareerWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BeforeCareerWindow");
        }

    }

    BeforeCareerWindow::BeforeCareerWindow(
        GameServices& services,
        ui::Screen& screen,
        CareerHooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_chosenSpot(screen.get<ui::Spot>("BeforeCareerChosen")),
          m_selfSpot(screen.get<ui::Spot>("BeforeCareerSelfFeet")),
          m_descriptionSpot(screen.get<ui::Spot>("BeforeCareerDescription")) {
        screen.get<ui::DrawingArea>("BeforeCareerPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onBeforeCareerDown", [this](const ui::EventArgs& args) { pressed(args); });
        screen.on("onBeforeCareerRightDown", [this](const ui::EventArgs& args) { rightPressed(args); });
        screen.on("onBeforeCareerOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onBeforeCareerCancel", [this](const ui::EventArgs&) { cancel(); });
    }

    void BeforeCareerWindow::open(int changeMode) {
        m_hover = 0;
        m_selected = 0;
        m_changeMode = changeMode;
        const CharacterLook look = m_hooks.look();
        m_self = CareerModel{};
        m_self.look = look;
        for (std::size_t i = 0; i < m_models.size(); ++i) {
            m_models[i] = makeBeforeCareerModel(look, m_services.items, static_cast<int>(i) + 1);
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    int BeforeCareerWindow::slotAt(core::Point local) const {
        for (int slot = 1; slot <= kModelCount; ++slot) {
            const core::Rect rect = kModelRects[static_cast<std::size_t>(slot - 1)];
            if (local.x >= rect.x && local.x < rect.x + rect.width && local.y >= rect.y && local.y < rect.y + rect.height) {
                return slot;
            }
        }
        return 0;
    }

    void BeforeCareerWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (m_hooks.fighting()) {
            close();
            return;
        }
        m_hooks.closeOthers();
        const core::Rect bounds = m_window.bounds();
        const core::Point mouse = m_screen.mouse();
        m_hover = slotAt({mouse.x - bounds.x, mouse.y - bounds.y});
        for (CareerModel& model : m_models) {
            model.pose = kCareerRestPose;
        }
        const int shown = m_selected != 0 ? m_selected : m_hover;
        if (shown >= 1 && shown <= kModelCount) {
            CareerModel& model = m_models[static_cast<std::size_t>(shown - 1)];
            if (m_selected != 0) {
                model.pose = careerChosenPose(model.career);
                const core::Rect frame = kChosenFrames[static_cast<std::size_t>(shown - 1)];
                canvas.fillRectAlpha({at.x + frame.x, at.y + frame.y, frame.width, frame.height}, m_chosenSpot.color());
            } else {
                model.pose = kCareerHoverPose;
            }
            canvas.drawText(
                {at.x + m_descriptionSpot.at().x, at.y + m_descriptionSpot.at().y},
                shown == 1 ? kSoldier : kMage,
                {m_descriptionSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_descriptionSpot.rect().width,
                m_descriptionSpot.rect().height
            );
        }
        if (m_hover >= 1 && m_hover <= kModelCount) {
            const CareerModel& model = m_models[static_cast<std::size_t>(m_hover - 1)];
            const core::Point feet = kModelFeet[static_cast<std::size_t>(m_hover - 1)];
            // tooltip sits 20 px left of and 11 px below the model's feet; colours (RGBA): text white, background (61,138,249,200), border blue
            ui::drawWrappedTooltip(
                canvas,
                {at.x + feet.x - 20, at.y + feet.y + 11},
                model.name,
                static_cast<int>(model.name.size()),
                kTipLineHeight,
                {255, 255, 255, 255},
                {61, 138, 249, 200},
                {0, 0, 255, 255}
            );
        }
        drawModel(canvas, m_self, {at.x + m_selfSpot.at().x, at.y + m_selfSpot.at().y}, nowMs);
        for (std::size_t i = 0; i < m_models.size(); ++i) {
            drawModel(canvas, m_models[i], {at.x + kModelFeet[i].x, at.y + kModelFeet[i].y}, nowMs);
        }
    }

    void BeforeCareerWindow::drawModel(gfx::Canvas& canvas, CareerModel& model, core::Point feet, std::uint64_t nowMs) {
        const int frame = model.animation.frame(nowMs, m_services.characters.figureFrameCount(model.look, model.pose));
        m_services.characters.drawFigure(canvas, model.look, feet, model.pose, frame);
    }

    void BeforeCareerWindow::pressed(const ui::EventArgs& args) {
        const int slot = slotAt(args.position);
        if (slot != 0) {
            m_selected = slot == m_selected ? 0 : slot;
        }
    }

    void BeforeCareerWindow::rightPressed(const ui::EventArgs& args) {
        if (m_selected != 0 && slotAt(args.position) == m_selected) {
            m_selected = 0;
        }
    }

    void BeforeCareerWindow::cancel() {
        m_hooks.eventGoesOn();
        close();
    }

    void BeforeCareerWindow::confirm() {
        std::string noClass = "Haven't chosen class system";
        std::string statPointsMessage = "Stat points should be distributed before choosing class";
        if (m_changeMode != 0) {
            noClass = "Haven't choosen the subclass";
            statPointsMessage = "Before changing your class,you need to distribute your ability points.";
            if (m_hooks.cloakSlot() == 0) {
                m_hooks.message("Class Cloak should be in Item Frame if you want to change your class.", kMessageMs);
                return;
            }
        }
        if (m_selected == 0) {
            m_hooks.message(noClass, kMessageMs);
            return;
        }
        if (m_hooks.statPoints() != 0) {
            m_hooks.message(statPointsMessage, kMessageMs);
            return;
        }
        const int line = m_selected;
        const int changeMode = m_changeMode;
        close();
        m_hooks.showCareer(line, changeMode);
    }

}
