#include "client/viewmodels/house/PianoWindow.h"

#include "protocol/HouseRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr std::array<ui::Key, PianoWindow::kNotes> kNoteKeys = {ui::Key::A, ui::Key::S, ui::Key::D, ui::Key::F, ui::Key::G, ui::Key::H, ui::Key::J};
        constexpr const char* kKeyPictures[PianoWindow::kNotes] = {"Piano_do", "Piano_re", "Piano_mi", "Piano_fa", "Piano_sol", "Piano_la", "Piano_ci"};

        constexpr int kWhiteGap = 1; // gap between white keys, px
        constexpr int kWhiteWidth = 42; // white key width, px
        constexpr int kWhiteHeight = 189; // white key height, px
        constexpr int kBlackWidth = 28; // black key width, px
        constexpr int kBlackHeight = 118; // black key height, px
        constexpr int kBlackGap = 13; // gap between two neighbouring black keys, px
        constexpr int kKeysTop = 4; // top of the keys in the window, px
        constexpr int kWhiteLeft = 8; // left of the first white key, px

        // left edge of each pressed-key picture (do, re, mi, fa, sol, la, ci), px: 43 px apart, the last four 1 px closer
        constexpr std::array<int, PianoWindow::kNotes> kPressedLeft = {9, 9 + 43, 9 + 86, 9 + 129 - 1, 9 + 172 - 1, 9 + 215 - 1, 9 + 258 - 1};

        /// @brief Tells whether an x coordinate lies on one of the five black keys.
        /// @param x X relative to the form.
        /// @return True when on a black key (PianoMouseKey leaves them without a note).
        bool onBlackKey(int x) {
            // left edges of the black keys: the first two start at 36, the last three at 165
            const int lefts[] = {36, 36 + kBlackGap + kBlackWidth, 165, 165 + kBlackGap + kBlackWidth, 165 + kBlackGap * 2 + kBlackWidth * 2};
            // five black keys; index 4 is the last one
            for (std::size_t key = 0; key < 5; ++key) {
                const int width = key == 4 ? kBlackWidth + 2 : kBlackWidth;
                if (x >= lefts[key] && x <= lefts[key] + width) {
                    return true;
                }
            }
            return false;
        }

        /// @brief Loads PianoWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PianoWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Piano");
        }

    }

    PianoWindow::PianoWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)) {
        for (std::size_t note = 0; note < m_keyPictures.size(); ++note) {
            m_keyPictures[note] = services.pictures.find(kKeyPictures[note]).value_or(gfx::PictureId{});
        }
        const core::Rect area = m_window.bounds();
        // 800x600 = screen size; the close button sits 8 px in from the right edge and 5 px down
        m_window.setPosition({(800 - area.width) / 2, (600 - area.height) / 2});
        screen.get<ui::Element>("PianoClose").setPosition({area.width - 8, 5});
        screen.get<ui::DrawingArea>("PianoPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintKey(canvas, at); });
        screen.on("onPianoClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onPianoEscape", [this](const ui::EventArgs&) { hide(); });
        screen.on("onPianoMouseDown", [this](const ui::EventArgs& args) { mouseDown(args.position); });
        screen.on("onPianoMouseUp", [this](const ui::EventArgs&) {
            m_mouseHeld = false;
            m_pressed = false;
        });
    }

    void PianoWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void PianoWindow::close() {
        m_hooks.sceneMusic();
        m_pressed = false;
        hide();
    }

    void PianoWindow::playNote(int note) const {
        if (note >= 1 && note <= kNotes) {
            // note n plays WAV99(n+3): notes 1..6 are WAV9904..WAV9909, the 7th is WAV9910
            m_services.playSound("sound/WAV99" + std::string(note == kNotes ? "10" : "0" + std::to_string(note + 3)) + ".wav");
        }
    }

    bool PianoWindow::keyDown(const ui::KeyEvent& key) {
        for (int note = 1; note <= kNotes; ++note) {
            if (key.key == kNoteKeys[static_cast<std::size_t>(note - 1)]) {
                if (!m_pressed) {
                    m_pressed = true;
                    strike(note);
                }
                return true;
            }
        }
        return false;
    }

    void PianoWindow::update() {
        if (!m_pressed || m_mouseHeld) {
            return;
        }
        for (const ui::Key key : kNoteKeys) {
            if (m_screen.isKeyHeld(key)) {
                return;
            }
        }
        m_pressed = false;
    }

    void PianoWindow::mouseDown(core::Point local) {
        if (!m_hooks.seated()) {
            return;
        }
        m_mouseHeld = true;
        m_pressed = true;
        m_note = 0;
        if (const int note = noteAt(local)) {
            strike(note);
        }
    }

    void PianoWindow::strike(int note) {
        m_note = note;
        playNote(note);
        m_services.connection.send(protocol::pianoKeyRequest(note));
    }

    int PianoWindow::noteAt(core::Point local) {
        if (local.x < 0 || local.y < 0) {
            return 0;
        }
        if (local.y <= kBlackHeight && onBlackKey(local.x)) {
            return 0;
        }
        if (local.y < kKeysTop || local.y > kKeysTop + kWhiteHeight) {
            return 0;
        }
        for (int note = 1; note <= kNotes; ++note) {
            const int left = kWhiteLeft + (kWhiteGap + kWhiteWidth) * (note - 1);
            if (local.x >= left && local.x <= left + kWhiteWidth) {
                return note;
            }
        }
        return 0;
    }

    void PianoWindow::paintKey(gfx::Canvas& canvas, core::Point at) const {
        if (!m_pressed || m_note < 1 || m_note > kNotes) {
            return;
        }
        const gfx::PictureId picture = m_keyPictures[static_cast<std::size_t>(m_note - 1)];
        if (picture.isValid()) {
            canvas.drawPicture(picture, {at.x + kPressedLeft[static_cast<std::size_t>(m_note - 1)], at.y});
        }
    }

}
