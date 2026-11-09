#include "client/viewmodels/minigames/GameStartWindow.h"

#include "ui/DrawingArea.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        constexpr int kFormBottomLimit = 565; // a form whose bottom would pass this y is moved up, px
        constexpr int kFormBottomEdge = 575; // y the moved-up form's bottom edge sits at (above the input bar), px
        constexpr int kHitMouse = 3; // sport kind 3: whack-a-mouse game
        constexpr int kMemoryGame = 15; // sport kind 15: memory card game

        /// @brief The picture of a game's instructions and the places of its buttons.
        /// @details Original: SetupGameStartForm. A variant of -1 serves every variant of the game; the form is kept above the input bar.
        struct Instructions {
            int kind;
            int variant;
            const char* picture;
            int left;
            int width;
            int height;
            core::Point begin;
            core::Point leave;
            int closeX;
            const char* closePicture;
        };

        // per row: sport kind, variant, form picture, form left/width/height (px), begin and leave button positions (px), close button x (px), close picture
        constexpr std::array<Instructions, 20> kInstructions{{
            {1, -1, "Throw_Exp_Form", 225, 388, 507, {122, 465}, {210, 465}, 338, "btn_close_s_1"}, // sport kind 1: throwing game
            {2, -1, "MiniGame_Explan_Form", 215, 409, 525, {144, 488}, {209, 488}, 359, "btn_close_s_1"}, // sport kind 2: generic mini game explanation
            {3, 0, "HitMouse_Exp_Form", 245, 331, 483, {96, 445}, {175, 445}, 281, "btn_close_s_1"}, // sport kind 3, look 0: whack-a-mouse (mouse)
            {3, 1, "HitRabit_Exp_Form", 245, 331, 483, {96, 445}, {175, 445}, 281, "btn_close_s_1"}, // sport kind 3, look 1: whack-a-mouse (rabbit)
            {3, 2, "HitTurtle_Exp_Form", 245, 331, 483, {96, 445}, {175, 445}, 281, "btn_close_s_1"}, // sport kind 3, look 2: whack-a-mouse (turtle)
            {4, -1, "Hunt_Exp_Form", 235, 345, 483, {107, 440}, {184, 440}, 295, "btn_close_s_1"}, // sport kind 4: hunter game
            {5, 1, "Sheep_Exp_Form", 215, 409, 507, {144, 465}, {209, 465}, 359, "btn_close_s_1"}, // sport kind 5, level 1: sheep game
            {5, 0, "Pig_Exp_Form", 215, 409, 507, {144, 465}, {209, 465}, 359, "btn_close_s_1"}, // sport kind 5, other levels: pig game
            {6, -1, "TrunEgg_Exp_Form", 215, 409, 525, {119, 458}, {234, 458}, 359, "btn_close_s_1"}, // sport kind 6: fortune egg
            {7, -1, "Doll_Exp_Form", 215, 409, 525, {119, 458}, {234, 458}, 359, "btn_close_s_1"}, // sport kind 7: claw machine (dolls)
            {8, -1, "Slotmach_Exp_Form", 215, 409, 525, {119, 458}, {234, 458}, 359, "btn_close_s_1"}, // sport kind 8: slot machine
            {9, -1, "Boxing_Exp_Form", 215, 409, 525, {119, 475}, {234, 475}, 359, "btn_close_s_1"}, // sport kind 9: boxing
            {11, -1, "Mario_Exp_Form", 215, 409, 525, {119, 475}, {234, 475}, 359, "btn_close_s_1"}, // sport kind 11: jigsaw (Mario)
            {13, -1, "MiniGame_Lucky_Form", 215, 409, 525, {95, 430}, {210, 430}, 295, "btn_close_s_1"}, // sport kind 13: lucky game
            {15, -1, "MemGame_Exp", 235, 409, 525, {107, 440}, {216, 440}, 359, "btn_close_s_3"}, // sport kind 15: memory card game
            {16, -1, "MiniGame_Poke_Form", 215, 409, 525, {119, 475}, {234, 475}, 359, "btn_close_s_1"}, // sport kind 16: lucky cards (Poke)
            {17, -1, "MiniGame_pumpkin_Form", 215, 409, 525, {95, 430}, {210, 430}, 295, "btn_close_s_1"}, // sport kind 17: pumpkin game
            {18, -1, "MiniGame_Gobang_Form", 215, 409, 525, {119, 475}, {234, 475}, 359, "btn_close_s_1"}, // sport kind 18: gobang (five in a row)
            {20, -1, "MiniGame_DigHole_Form", 215, 409, 525, {119, 475}, {234, 475}, 359, "btn_close_s_1"}, // sport kind 20: dig hole game
            {21, -1, "MiniGame_TG_Form", 215, 409, 525, {120, 471}, {229, 471}, 359, "btn_close_s_3"}, // sport kind 21: lucky wheel
        }};

        /// @brief A figure a game puts on its instructions, at its place on the screen.
        /// @details Original: DrawControl of CH_GameStartForm (for the memory game the place is on the form).
        struct Figure {
            int kind;
            core::Point at;
            const char* text;
        };

        // per row: sport kind, text position (px; relative to the form for the memory game), number (points or price) printed on the instructions
        constexpr std::array<Figure, 13> kFigures{
            {{6, {522, 141}, "35"}, // fortune egg: prints "35"
             {7, {359, 131}, "45"}, // claw machine: prints "45"
             {8, {372, 139}, "10"}, // slot machine: prints "10"
             {9, {440, 171}, "75"}, // boxing: prints "75"
             {10, {376, 139}, "5"}, // small slot machine: prints "5"
             {11, {405, 119}, "25"}, // jigsaw (Mario): prints "25"
             {12, {403, 119}, "100"}, // jigsaw gem machine: prints "100"
             {14, {403, 119}, "10"}, // jigsaw: prints "10"
             {16, {380, 344}, "20"}, // lucky cards: prints "20"
             {19, {374, 139}, "20"}, // small slot machine: prints "20"
             {20, {356, 307}, "15"}, // dig hole: prints "15"
             {21, {354, 347}, "65"}, // lucky wheel: prints "65"
             {kMemoryGame, {122, 424}, "12"}} // memory card game: prints "12"
        };
        // positions (px, relative to the form) of the HitMouse_40s picture for looks 0..2 (mouse, rabbit, turtle)
        constexpr core::Point kHitMouseForty[] = {{64, 396}, {33, 398}, {135, 412}};

        /// @brief Maps a game kind to the kind whose instructions it uses.
        /// @param kind Mini game kind.
        /// @return 8 for kinds 10 and 19 (small Lucky 7s), 11 for kinds 12 and 14 (Jigsaw), else the kind itself.
        int pictureKind(int kind) {
            return kind == 10 || kind == 19 ? 8 : kind == 12 || kind == 14 ? 11 : kind;
        } // slot machine kinds 10/19 use kind 8, jigsaw kinds 12/14 use kind 11

    }

    GameStartWindow::GameStartWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_window((screen.load(services.game.layout("GameStartWindow.xml"), services.loadContext()), screen.get<ui::Element>("GameStartWindow"))),
          m_picture(screen.get<ui::ImageView>("GameStartPicture")),
          m_begin(screen.get<ui::ImageButton>("GameStartBegin")),
          m_leave(screen.get<ui::ImageButton>("GameStartLeave")),
          m_close(screen.get<ui::ImageButton>("GameStartClose")),
          m_standardSpot(screen.get<ui::Spot>("GameStartStandard")),
          m_pointsSpot(screen.get<ui::Spot>("GameStartPoints")) {
        screen.get<ui::DrawingArea>("GameStartPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onGameStartBegin", [this](const ui::EventArgs&) { m_hooks.begin(m_kind); });
        screen.on("onGameStartLeave", [this](const ui::EventArgs&) { m_hooks.leave(m_kind); });
    }

    void GameStartWindow::open(int kind, int variant) {
        m_kind = kind;
        m_variant = variant;
        int left = m_standardSpot.rect().x;
        int width = m_standardSpot.rect().width;
        int height = m_standardSpot.rect().height;
        for (const Instructions& form : kInstructions) {
            if (form.kind != pictureKind(kind) || (form.variant != -1 && form.variant != variant)) {
                continue;
            }
            left = form.left;
            width = form.width;
            height = form.height;
            m_picture.setPicture(m_services.pictures.find(form.picture).value_or(gfx::PictureId{}));
            m_begin.setPosition(form.begin);
            m_leave.setPosition(form.leave);
            m_close.setPosition({form.closeX, m_close.bounds().y});
            m_close.setPicture(m_services.pictures.find(form.closePicture).value_or(gfx::PictureId{}));
            break;
        }
        int top = m_standardSpot.rect().y;
        if (top + height > kFormBottomLimit) {
            top = kFormBottomEdge - height;
        }
        // a form that would sit less than 10 pixels from the top is put at 0
        if (top < 10 && kFormBottomEdge - height != top) {
            top = 0;
        }
        m_window.setBounds({left, top, width, height});
        m_window.setVisible(true);
    }

    void GameStartWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_kind == kHitMouse) {
            if (const auto forty = m_services.pictures.find("HitMouse_40s");
                forty && m_variant >= 0 && m_variant <= 2) { // Hit Mouse looks 0..2 (mouse, rabbit, turtle)
                canvas.drawPicture(*forty, {at.x + kHitMouseForty[m_variant].x, at.y + kHitMouseForty[m_variant].y});
            }
            return;
        }
        for (const Figure& figure : kFigures) {
            if (figure.kind == m_kind) {
                const core::Point place = m_kind == kMemoryGame ? core::Point{at.x + figure.at.x, at.y + figure.at.y} : figure.at;
                canvas.drawText(
                    place,
                    figure.text,
                    {m_pointsSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                    m_pointsSpot.size().width,
                    m_pointsSpot.size().height
                );
            }
        }
    }

}
