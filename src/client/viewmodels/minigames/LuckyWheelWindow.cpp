#include "client/viewmodels/minigames/LuckyWheelWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game screen width, px
        constexpr int kScreenHeight = 600; // game screen height, px
        constexpr int kPrizeSlotSize = 32; // item cell size of a prize slot in the side panel, px
        constexpr int kHighlightStrength = 15; // light-blend strength of the panel row highlight
        constexpr int kLightStrength = 30; // light-blend strength of the win light (S40003)

        /// @brief Adds the frame-effect callback to the wheel's hooks.
        /// @param hooks Hooks to extend.
        /// @param effect Callback that starts a frame effect (slot, prize position, tier).
        /// @return The hooks with frameEffect set.
        LuckyWheel::Hooks withFrames(LuckyWheel::Hooks hooks, std::function<void(int, core::Point, int)> effect) {
            hooks.frameEffect = std::move(effect);
            return hooks;
        }

        /// @brief Loads LuckyWheelWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "LuckyWheelForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LuckyWheelWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("LuckyWheelForm");
        }

    }

    LuckyWheelWindow::LuckyWheelWindow(
        GameServices& services,
        ui::Screen& screen,
        ItemInfoPopup& info,
        LuckyWheel::Hooks hooks,
        std::function<int()> imPoints
    )
        : m_services(services),
          m_screen(screen),
          m_info(info),
          m_imPoints(std::move(imPoints)),
          m_wheel(services.items, withFrames(std::move(hooks), [this](int slot, core::Point at, int tier) { startFrameEffect(slot, at, tier); })),
          m_form(loadForm(services, screen)),
          m_panel(screen.get<ui::IconGrid>("LuckyWheelPrizes")),
          m_refresh(screen.get<ui::ImageButton>("LuckyWheelRefresh")),
          m_double(screen.get<ui::ImageButton>("LuckyWheelDouble")),
          m_start(screen.get<ui::ImageButton>("LuckyWheelStart")),
          m_exit(screen.get<ui::ImageButton>("LuckyWheelExit")),
          m_prizeBoxSpot(screen.get<ui::Spot>("LuckyWheelPrizeBox")),
          m_rowTextSpot(screen.get<ui::Spot>("LuckyWheelRowText")),
          m_pointsSpot(screen.get<ui::Spot>("LuckyWheelPoints")),
          m_countSpot(screen.get<ui::Spot>("LuckyWheelCount")),
          m_doubleMarkSpot(screen.get<ui::Spot>("LuckyWheelDoubleMark")),
          m_frameSpot(screen.get<ui::Spot>("LuckyWheelFrame")),
          m_getPrizeSpot(screen.get<ui::Spot>("LuckyWheelGetPrize")),
          m_lightSpot(screen.get<ui::Spot>("LuckyWheelLight")),
          m_textSpot(screen.get<ui::Spot>("LuckyWheelText")),
          m_tipTextSpot(screen.get<ui::Spot>("LuckyWheelTipText")),
          m_tipFillSpot(screen.get<ui::Spot>("LuckyWheelTipFill")),
          m_tipBorderSpot(screen.get<ui::Spot>("LuckyWheelTipBorder")) {
        if (const auto picture = services.pictures.find("Form_TurnGame")) {
            const gfx::Picture& data = services.pictures.picture(*picture);
            m_form.setBounds({(kScreenWidth - data.width()) / 2, (kScreenHeight - data.height()) / 2, data.width(), data.height()});
        }
        const core::Rect area = m_form.bounds();
        m_wheel.setFormOrigin({area.x, area.y});
        screen.get<ui::DrawingArea>("LuckyWheelPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onLuckyWheelRefresh", [this](const ui::EventArgs&) { m_wheel.refreshClicked(); });
        screen.on("onLuckyWheelDouble", [this](const ui::EventArgs&) { m_wheel.doubleClicked(); });
        screen.on("onLuckyWheelStart", [this](const ui::EventArgs&) { m_wheel.startClicked(); });
        screen.on("onLuckyWheelExit", [this](const ui::EventArgs&) { m_wheel.exitClicked(); });
    }

    void LuckyWheelWindow::open(std::uint64_t nowMs) {
        m_wheel.open(nowMs);
        for (FrameEffect& effect : m_effects) {
            effect = {};
        }
        m_panel.clearCells();
        showButtons();
        m_form.setVisible(true);
    }

    void LuckyWheelWindow::close() {
        m_form.setVisible(false);
        m_info.hide();
    }

    void LuckyWheelWindow::showButtons() {
        m_refresh.setVisible(m_wheel.refreshShown());
        m_double.setVisible(m_wheel.doubleShown());
        m_start.setVisible(m_wheel.startShown());
        m_exit.setVisible(m_wheel.exitShown());
    }

    void LuckyWheelWindow::showPanel() {
        m_panel.clearCells();
        for (int slot = 1; slot <= LuckyWheel::kPanelSlots; ++slot) {
            const LuckyWheel::TopPrize& top = m_wheel.panel()[static_cast<std::size_t>(slot)];
            if (top.itemId == 0) {
                continue;
            }
            protocol::ItemInstance item;
            item.itemId = top.itemId;
            item.count = top.count;
            ui::IconCell cell = itemIconCell(m_services, item, top.count > 0);
            cell.hint.clear();
            m_panel.setCell(slot, cell);
        }
    }

    void LuckyWheelWindow::startFrameEffect(int slot, core::Point prizePosition, int tier) {
        // tier 1..3 picks frame picture A, B or C (index 0 is unused)
        static const char* const kPictures[] = {"", "Icon_TGPrize_A", "Icon_TGPrize_B", "Icon_TGPrize_C"};
        const auto picture = m_services.pictures.find(kPictures[tier]);
        if (!picture) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*picture);
        m_effects[static_cast<std::size_t>(slot)].start(
            *picture,
            {prizePosition.x - m_frameSpot.at().x, prizePosition.y - m_frameSpot.at().y},
            {data.width(), data.height()},
            m_frameSpot.size().width,
            m_frameSpot.size().height
        );
    }

    gfx::PictureId LuckyWheelWindow::iconOf(int itemId) {
        const auto record = m_services.items.find(itemId);
        return record ? m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{}) : gfx::PictureId{};
    }

    void LuckyWheelWindow::update() {
        if (!isOpen()) {
            return;
        }
        showButtons();
        showPanel();
        const core::Point mouse = m_screen.mouse();
        const core::Rect form = m_form.bounds();
        m_hoverPrize = 0;
        m_hoverSlot = 0;
        if (!m_wheel.prizes()[1].name.empty()) {
            for (int i = 1; i <= LuckyWheel::kPrizes; ++i) {
                const core::Point at = m_wheel.prizes()[static_cast<std::size_t>(i)].position;
                if (mouse.x >= at.x && mouse.x <= at.x + m_prizeBoxSpot.size().width && mouse.y >= at.y && mouse.y <= at.y + m_prizeBoxSpot.size().height) {
                    m_hoverPrize = i;
                }
            }
            for (int i = 1; i <= LuckyWheel::kPanelSlots; ++i) {
                const core::Rect box{form.x + m_panel.bounds().x, form.y + m_panel.bounds().y + (i - 1) * m_panel.pitch().y, kPrizeSlotSize, kPrizeSlotSize};
                if (mouse.x >= box.x && mouse.x <= box.x + box.width && mouse.y >= box.y && mouse.y <= box.y + box.height) {
                    m_hoverSlot = i;
                }
            }
        }
        // wheel state 0 = prizes not known yet; the popup needs state 1 or more
        const bool shown = m_wheel.state() >= 1 && m_hoverSlot != 0 && m_wheel.panel()[static_cast<std::size_t>(m_hoverSlot)].itemId != 0;
        if (shown) {
            const LuckyWheel::TopPrize& top = m_wheel.panel()[static_cast<std::size_t>(m_hoverSlot)];
            protocol::ItemInstance item;
            item.itemId = top.itemId;
            item.count = top.count;
            m_info.show(
                item,
                {form.x + m_panel.bounds().x, form.y + m_panel.bounds().y + (m_hoverSlot - 1) * m_panel.pitch().y, kPrizeSlotSize, kPrizeSlotSize}
            );
        } else {
            m_info.hide();
        }
    }

    void LuckyWheelWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        (void)at;
        const core::Rect form = m_form.bounds();
        if (const auto picture = m_services.pictures.find("Form_TurnGame")) {
            canvas.drawPicture(*picture, {form.x, form.y});
        }
        const gfx::PictureId unknown = m_services.pictures.find("Icon_UnKnow").value_or(gfx::PictureId{});
        for (int i = 1; i <= LuckyWheel::kPrizes; ++i) {
            const LuckyWheel::Prize& prize = m_wheel.prizes()[static_cast<std::size_t>(i)];
            const bool known = m_wheel.state() != 0;
            const gfx::PictureId icon = known ? iconOf(prize.itemId) : unknown;
            if (icon.isValid()) {
                canvas.drawPicture(
                    icon,
                    {prize.position.x + (known ? 4 : 0), prize.position.y + (known ? 4 : 0)}
                ); // known icons sit 4 pixels in from the prize place
            }
        }
        if (m_wheel.state() != 0) {
            const auto gray = [&](const char* name, const ui::ImageButton& button, bool shown) {
                if (!shown) {
                    if (const auto picture = m_services.pictures.find(name)) {
                        canvas.drawPicture(*picture, {form.x + button.bounds().x, form.y + button.bounds().y});
                    }
                }
            };
            gray("Icon_TGFresh_G", m_refresh, m_wheel.refreshShown());
            gray("Icon_TGDouble_G", m_double, m_wheel.doubleShown());
            gray("Icon_TGStart_G", m_start, m_wheel.startShown());
            const int row = m_wheel.highlightRow();
            // six panel rows (6); the highlight shows in wheel state 1 on blink phase 0; rows are 46 pixels apart
            if (row >= 1 && row <= 6 && m_wheel.state() == 1 && m_wheel.blinkPhase() == 0) {
                if (const auto picture = m_services.pictures.find("Form_TurnGame")) {
                    // source rect of the row highlight in the form picture: x 428, y 87 + 46 per row, 206 x 40
                    const core::Rect source{428, (row - 1) * 46 + 87, 206, 40};
                    // drawn at x 440 - 12 and y 97 - 10 (+ 46 per row) from the form origin
                    canvas.drawPictureLight(*picture, {form.x + 440 - 12, (row - 1) * 46 + 97 + form.y - 10}, kHighlightStrength, source);
                }
            }
            for (FrameEffect& effect : m_effects) {
                effect.draw(canvas);
            }
            m_wheel.spinStep(nowMs);
            const auto getPrize = m_services.pictures.find("Icon_TGGetPrize");
            const core::Point spin = m_wheel.prizes()[static_cast<std::size_t>(m_wheel.spinPosition())].position;
            // the spin box shows in wheel state 2, and in state 1 on the visible blink phase
            if (getPrize && (m_wheel.state() == 2 || (m_wheel.state() == 1 && m_wheel.blinkPhase() == 0))) {
                canvas.drawPicture(
                    *getPrize,
                    {spin.x - m_getPrizeSpot.at().x, spin.y - m_getPrizeSpot.at().y},
                    core::Rect{0, 0, m_getPrizeSpot.size().width, m_getPrizeSpot.size().height}
                );
            }
            m_wheel.blinkStep(nowMs);
            if (m_wheel.doublePending() && m_wheel.blinkPhase() == 0) {
                if (const auto mark = m_services.pictures.find("Icon_X2")) {
                    canvas.drawPicture(*mark, m_doubleMarkSpot.at());
                }
            }
            m_wheel.lightsStep(nowMs);
            if (m_wheel.lightsActive()) {
                if (const auto light = m_services.pictures.find("S40003")) {
                    const gfx::Picture& data = m_services.pictures.picture(*light);
                    const int frameHeight = data.height() / LuckyWheel::kLightFrames;
                    for (std::size_t i = 0; i < m_wheel.lights().size(); ++i) {
                        const LuckyWheel::WinLight& winLight = m_wheel.lights()[i];
                        if ((m_wheel.lightMask() & (1 << i)) != 0) { // bit i of the mask marks light i as lit
                            canvas.drawPictureLight(
                                *light,
                                {winLight.position.x - data.width() / 2, winLight.position.y - frameHeight + m_lightSpot.at().y},
                                kLightStrength,
                                core::Rect{0, winLight.frame * frameHeight, data.width(), frameHeight}
                            );
                            m_wheel.advanceLight(static_cast<int>(i), nowMs);
                        }
                    }
                    m_wheel.endLightsWhenFinished();
                }
            }
        }
        paintInfo(canvas, at);
    }

    void LuckyWheelWindow::paintInfo(gfx::Canvas& canvas, core::Point at) {
        (void)at;
        if (m_wheel.state() < 1) {
            return;
        }
        const core::Rect form = m_form.bounds();
        const gfx::TextStyle text{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        for (int i = 1; i <= LuckyWheel::kPanelSlots; ++i) {
            canvas.drawText(
                {form.x + m_rowTextSpot.at().x, (i - 1) * m_panel.pitch().y + m_rowTextSpot.at().y + form.y},
                m_wheel.panel()[static_cast<std::size_t>(i)].text,
                text,
                m_rowTextSpot.rect().width,
                m_rowTextSpot.rect().height
            );
        }
        canvas.drawText(
            {form.x + m_pointsSpot.at().x, form.y + m_pointsSpot.at().y},
            std::to_string(m_imPoints()),
            text,
            m_pointsSpot.size().width,
            m_pointsSpot.size().height
        );
        for (int i = 1; i <= LuckyWheel::kPrizes; ++i) {
            const LuckyWheel::Prize& prize = m_wheel.prizes()[static_cast<std::size_t>(i)];
            canvas.drawText(
                {prize.position.x, prize.position.y + m_countSpot.at().y},
                std::to_string(prize.count),
                text,
                m_countSpot.size().width,
                m_countSpot.size().height
            );
        }
        if (m_hoverPrize != 0) {
            const LuckyWheel::Prize& prize = m_wheel.prizes()[static_cast<std::size_t>(m_hoverPrize)];
            // tooltip box: 8 pixels per character plus 20, 25 high
            const core::Rect box{prize.position.x + m_prizeBoxSpot.size().width, prize.position.y, static_cast<int>(prize.name.size()) * 8 + 20, 25};
            canvas.fillRectAlpha(box, m_tipFillSpot.color());
            canvas.drawRectOutline(box, m_tipBorderSpot.color());
            canvas.drawText(
                {box.x + 10, prize.position.y + 5},
                prize.name,
                {m_tipTextSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                160,
                15
            ); // text offset 10 x 5 in the box; text box 160 x 15
        }
    }

}
