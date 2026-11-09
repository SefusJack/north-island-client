#include "client/viewmodels/character/CareerWindow.h"

#include "client/data/CareerLooks.h"
#include "client/data/CareerTexts.h"
#include "protocol/CareerRequests.h"
#include "ui/DrawingArea.h"
#include "ui/WrappedTooltip.h"

namespace client {

    namespace {

        constexpr int kModelCount = 3; // career models offered per line
        constexpr int kModelTop = 193; // top of the model rectangles, px from the window top
        constexpr int kModelWidth = 120; // width of a model rectangle, px
        constexpr int kModelHeight = 115; // height of a model rectangle, px
        // highlight frames of the chosen model, one per slot: {x, y, width, height} px
        constexpr std::array<core::Rect, kModelCount> kChosenFrames = {{{21, 193, 121, 115}, {146, 193, 122, 115}, {273, 193, 121, 115}}};
        // feet positions of the three career models, px
        constexpr std::array<core::Point, kModelCount> kModelFeet = {{{80, 285}, {210, 285}, {335, 285}}};
        constexpr int kTipLineHeight = 8; // tooltip line height, px
        constexpr int kMessageMs = 3000; // message display time, ms

        /// @brief Loads CareerWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "CareerWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CareerWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CareerWindow");
        }

        /// @brief Gives the rectangle of a model slot on the form.
        /// @details Original: CareerModelRects.
        /// @param slot Model slot, 1..3.
        /// @return The rectangle relative to the window.
        core::Rect modelRect(int slot) {
            const int x = (slot - 1) * kModelWidth + 20 + (slot != 1 ? 7 : 0); // first rect at x 20; the others get 7 px more
            return {x, kModelTop, kModelWidth, kModelHeight};
        }

    }

    CareerWindow::CareerWindow(
        GameServices& services,
        ui::Screen& screen,
        CareerHooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_chosenSpot(screen.get<ui::Spot>("CareerChosen")),
          m_selfSpot(screen.get<ui::Spot>("CareerSelfFeet")),
          m_warTotemSpot(screen.get<ui::Spot>("CareerWarTotem")),
          m_magicTotemSpot(screen.get<ui::Spot>("CareerMagicTotem")),
          m_descriptionSpot(screen.get<ui::Spot>("CareerDescription")),
          m_bonusSpot(screen.get<ui::Spot>("CareerBonus")) {
        screen.get<ui::DrawingArea>("CareerPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.on("onCareerDown", [this](const ui::EventArgs& args) { pressed(args); });
        screen.on("onCareerRightDown", [this](const ui::EventArgs& args) { rightPressed(args); });
        screen.on("onCareerOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onCareerClose", [this](const ui::EventArgs&) { close(); });
    }

    void CareerWindow::open(int line, int changeMode) {
        m_hover = 0;
        m_selected = 0;
        m_chosen = 0;
        m_line = line;
        m_changeMode = changeMode;
        const CharacterLook look = m_hooks.look();
        m_self = CareerModel{};
        m_self.look = look;
        // career ids: line 1 = 1,2,3 (killer, warrior, knight); line 2 = 4,5,6 (wit, priest, seer)
        const std::array<int, kModelCount> careers = line == 1 ? std::array<int, kModelCount>{1, 2, 3} : std::array<int, kModelCount>{4, 5, 6};
        for (std::size_t i = 0; i < m_models.size(); ++i) {
            m_models[i] = makeCareerClassModel(look, m_services.items, careers[i]);
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    int CareerWindow::slotAt(core::Point local) const {
        for (int slot = 1; slot <= kModelCount; ++slot) {
            const core::Rect rect = modelRect(slot);
            if (local.x >= rect.x && local.x < rect.x + rect.width && local.y >= rect.y && local.y < rect.y + rect.height) {
                return slot;
            }
        }
        return 0;
    }

    void CareerWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
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
        if (const auto totem = m_services.pictures.find(m_line == 1 ? "Icon_WarTotem" : "Icon_MagicTotem"); totem && (m_line == 1 || m_line == 2)) {
            const core::Point where = m_line == 1 ? m_warTotemSpot.at() : m_magicTotemSpot.at();
            canvas.drawPicture(*totem, {at.x + where.x, at.y + where.y});
        }
        const int shown = m_selected != 0 ? m_selected : m_hover;
        if (shown >= 1 && shown <= kModelCount) {
            CareerModel& model = m_models[static_cast<std::size_t>(shown - 1)];
            if (m_selected != 0) {
                model.pose = careerChosenPose(model.career);
                m_chosen = model.career;
                const core::Rect frame = kChosenFrames[static_cast<std::size_t>(shown - 1)];
                canvas.fillRectAlpha({at.x + frame.x, at.y + frame.y, frame.width, frame.height}, m_chosenSpot.color());
            } else {
                model.pose = kCareerHoverPose;
            }
            canvas.drawText(
                {at.x + m_descriptionSpot.at().x, at.y + m_descriptionSpot.at().y},
                careerDescription(model.career),
                {m_descriptionSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_descriptionSpot.rect().width,
                m_descriptionSpot.rect().height
            );
            canvas.drawText(
                {at.x + m_bonusSpot.at().x, at.y + m_bonusSpot.at().y},
                careerBonusText(model.career),
                {m_bonusSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_bonusSpot.rect().width,
                m_bonusSpot.rect().height
            );
        }
        if (m_selected == 0) {
            m_chosen = 0;
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

    void CareerWindow::drawModel(gfx::Canvas& canvas, CareerModel& model, core::Point feet, std::uint64_t nowMs) {
        const int frame = model.animation.frame(nowMs, m_services.characters.figureFrameCount(model.look, model.pose));
        m_services.characters.drawFigure(canvas, model.look, feet, model.pose, frame);
    }

    void CareerWindow::pressed(const ui::EventArgs& args) {
        const int slot = slotAt(args.position);
        if (slot == 0) {
            return;
        }
        if (slot == m_selected) {
            m_selected = 0;
            return;
        }
        const int career = m_models[static_cast<std::size_t>(slot - 1)].career;
        if (m_changeMode != 0 && career == m_hooks.job()) {
            m_hooks.message("You are " + careerName(m_hooks.job()) + "!", 1000); // message display time in ms
            return;
        }
        m_selected = slot;
    }

    void CareerWindow::rightPressed(const ui::EventArgs& args) {
        if (m_selected != 0 && slotAt(args.position) == m_selected) {
            m_selected = 0;
            m_chosen = 0;
        }
    }

    void CareerWindow::confirm() {
        std::string noClass = "Haven't chosen class";
        std::string statPointsMessage = "Stat points should be distributed before choosing class";
        int cloak = 0;
        int mirror = 0;
        if (m_changeMode != 0) {
            noClass = "Haven't choosen the subclass";
            statPointsMessage = "Before changing your class,you need to distribute your ability points.";
            cloak = m_hooks.cloakSlot();
            mirror = m_hooks.mirrorSlot();
        }
        if (m_changeMode != 0 && cloak == 0) {
            m_hooks.message("Class Cloak should be in Item Frame if you want to change your class.", kMessageMs);
            return;
        }
        if (m_changeMode != 0 && mirror == 0) {
            m_hooks.message("The Transfer Mirror should be in the Item Frame", kMessageMs);
            return;
        }
        if (m_chosen == 0) {
            m_hooks.message(noClass, kMessageMs);
            return;
        }
        if (m_chosen < 1 || m_chosen > 6) { // valid career ids are 1..6
            return;
        }
        if (m_hooks.statPoints() != 0) {
            m_hooks.message(statPointsMessage, kMessageMs);
            return;
        }
        const std::string name = careerName(m_chosen);
        const std::string levelText =
            "Become " + name + " The level will be " + std::to_string(m_hooks.level() % 100 + 1); // level after the change: level wraps at 100, plus 1
        std::string text = levelText + "\r" + "Class cannot be changed" + "\r" + "To choose " + name;
        if (m_changeMode != 0) {
            text = "The current class is " + careerName(m_hooks.job()) + "\r" + "Are you sure that you want to change your class into " + name;
        }
        text += "Yes or No?";
        m_hooks.ask(text, [this] { m_changeMode != 0 ? sendChange() : sendChoose(); }, [] {});
    }

    void CareerWindow::sendChoose() {
        m_hooks.send(protocol::chooseCareerRequest(m_chosen));
        close();
    }

    void CareerWindow::sendChange() {
        const int cloak = m_hooks.cloakSlot();
        const int mirror = m_hooks.mirrorSlot();
        if (mirror == 0) {
            m_hooks.message("The Transfer Mirror should be in the Item Frame", kMessageMs);
        } else if (cloak == 0) {
            m_hooks.message("Class Cloak should be in Item Frame if you want to change your class.", kMessageMs);
        } else {
            m_hooks.send(protocol::changeCareerRequest(m_chosen, mirror, cloak));
            close();
        }
    }

}
