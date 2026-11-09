#include "client/viewmodels/minigames/LuckyCardsWindow.h"

#include "core/Between.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // screen width, px
        constexpr int kScreenHeight = 600; // screen height, px
        constexpr int kCardsPerRow = 4; // cards per row (cards are numbered row by row)
        constexpr int kHoverStrength = 15; // light strength added to the card under the mouse
        constexpr std::uint64_t kHintAfterMs = 2000; // idle time before the hint shows again, ms

        /// @brief Loads LuckyCardsWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "LuckyCardsForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LuckyCardsWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("LuckyCardsForm");
        }

    }

    LuckyCardsWindow::LuckyCardsWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen
    )
        : m_services(services),
          m_screen(screen),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_cards(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_exit(screen.get<ui::ImageButton>("LuckyCardsExit")),
          m_cardSpot(screen.get<ui::Spot>("LuckyCardsCard")),
          m_faceSpot(screen.get<ui::Spot>("LuckyCardsFace")),
          m_resultSpot(screen.get<ui::Spot>("LuckyCardsResult")),
          m_hintSpot(screen.get<ui::Spot>("LuckyCardsHint")) {
        screen.get<ui::DrawingArea>("LuckyCardsPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onLuckyCardsDown", [this](const ui::EventArgs& args) { clicked(args); });
        screen.on("onLuckyCardsQuit", [this](const ui::EventArgs&) { m_cards.quitClicked(); });
    }

    void LuckyCardsWindow::open(std::uint64_t nowMs) {
        m_cards.open(nowMs);
        m_exit.setVisible(false);
        m_form.setVisible(true);
    }

    void LuckyCardsWindow::begin() {
        m_cards.begin();
        m_exit.setVisible(true);
    }

    void LuckyCardsWindow::close() { m_form.setVisible(false); }

    void LuckyCardsWindow::update(std::uint64_t nowMs) {
        if (isOpen()) {
            m_cards.update(nowMs);
        }
    }

    core::Point LuckyCardsWindow::cardAt(int card) const {
        const int column = (card - 1) % kCardsPerRow;
        const int row = (card - 1) / kCardsPerRow;
        return {column * m_cardSpot.pitch().x + m_cardSpot.at().x, row * m_cardSpot.pitch().y + m_cardSpot.at().y};
    }

    int LuckyCardsWindow::cardAtPoint(core::Point point) const {
        for (int card = 1; card <= LuckyCards::kCards; ++card) {
            const core::Point at = cardAt(card);
            if (m_cards.cardVisible(card) && point.x >= at.x && point.x < at.x + m_cardSpot.rect().width && point.y >= at.y &&
                point.y < at.y + m_cardSpot.rect().height) {
                return card;
            }
        }
        return 0;
    }

    void LuckyCardsWindow::clicked(const ui::EventArgs& args) {
        if (m_instructionsOpen()) {
            return;
        }
        if (const int card = cardAtPoint(args.position)) {
            m_cards.cardClicked(card);
        } else {
            m_cards.tableClicked();
        }
    }

    void LuckyCardsWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        (void)at;
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto background = pictures.find("PokeGameBG")) {
            canvas.drawPicture(*background, {0, 0});
        }
        const bool instructions = m_instructionsOpen();
        const auto hint = pictures.find("Poke_Text_Tc");
        if (hint && !m_cards.playing() && !instructions && !m_cards.mouseDownSeen()) {
            const gfx::Picture& picture = pictures.picture(*hint);
            canvas.drawPicture(*hint, {(kScreenWidth - picture.width()) / 2, (kScreenHeight - picture.height()) / 2});
        }
        if (hint && nowMs - m_cards.flipTickMs() > kHintAfterMs && m_cards.flippedCount() == 0 && !m_cards.flipPending() && !m_cards.playing() &&
            !instructions && m_cards.flipTickMs() != 0 && m_cards.mouseDownSeen()) {
            canvas.drawPicture(*hint, m_hintSpot.at());
        }
        const auto back = pictures.find("PokeCardBack");
        const int hovered = cardAtPoint(m_screen.mouse());
        int offset = m_cards.flipOffset();
        if (const auto amount = canvas.betweenBeats(); amount && m_cards.updateMs() >= canvas.beatMs()) {
            offset = core::between(m_cards.previousFlipOffset(), offset, *amount);
        }
        if (!instructions && back) {
            for (int card = 1; card <= m_cards.cardCount(); ++card) {
                const core::Point place = cardAt(card);
                const bool turning = m_cards.flippingCard() == card;
                if (turning) {
                    const core::Rect back2{place.x + offset, place.y, m_cardSpot.rect().width - 2 * offset, m_cardSpot.rect().height};
                    if (back2.width > 0) {
                        canvas.drawPictureScaled(*back, back2);
                    }
                } else {
                    canvas.drawPicture(*back, place);
                }
                const int value = m_cards.cardValue(card);
                if (!m_cards.cardsDealt() && m_cards.cardCount() == LuckyCards::kCards && !m_cards.cardVisible(card) && value != 0) {
                    if (const auto face = pictures.find("PokeCard_" + std::to_string(value))) {
                        if (turning) {
                            const core::Rect face2{
                                place.x + m_faceSpot.at().x + offset,
                                place.y + m_faceSpot.at().y,
                                m_faceSpot.rect().width - 2 * offset,
                                m_faceSpot.rect().height
                            };
                            if (face2.width > 0) {
                                canvas.drawPictureScaled(*face, face2);
                            }
                        } else {
                            canvas.drawPicture(*face, {place.x + m_faceSpot.at().x, place.y + m_faceSpot.at().y});
                        }
                    }
                }
                if (hovered == card) {
                    canvas.drawPictureLight(*back, place, kHoverStrength);
                }
            }
        }
        if (m_cards.flipPending()) {
            if (const auto sign = pictures.find(m_cards.pairMatched() ? "Icon_Currect_1" : "Icon_Miss_1")) {
                canvas.drawPicture(*sign, m_resultSpot.at());
            }
            m_cards.soundResult();
        }
    }

}
