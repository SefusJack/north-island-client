#include "client/viewmodels/system/PadKeyboard.h"

#include <algorithm>
#include <cctype>

#include "ui/EventArgs.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"

namespace client {

    namespace {

        constexpr int kColumns = 10; // the columns of keys
        constexpr int kCharWidth = 8; // a character of the small font, px
        constexpr int kTextHeight = 15; // the small font's height, px
        constexpr float kSlideMs = 160.0f; // the time the board takes to come up or go down

        /// @brief Loads PadKeyboard.xml into the screen and returns its board.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "PadKeyboardBoard" element.
        ui::Element& loadBoard(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PadKeyboard.xml"), services.loadContext());
            return screen.get<ui::Element>("PadKeyboardBoard");
        }

    }

    PadKeyboard::PadKeyboard(GameServices& services, ui::Screen& screen)
        : m_screen(screen),
          m_board(loadBoard(services, screen)),
          m_keys(screen.get<ui::DrawingArea>("PadKeyboardKeys")),
          m_keySkin(services.loadContext().picture("panel16"), {0, 0, 120, 21}, 4, 4, 4, 4),
          m_lineSkin(services.loadContext().picture("editorBG"), {0, 0, 50, 20}, 5, 5, 5, 5) {
        const ui::Spot& line = screen.get<ui::Spot>("PadKeyboardLine");
        const ui::Spot& key = screen.get<ui::Spot>("PadKeyboardKey");
        m_line = line.bounds();
        m_lineColor = line.color();
        m_first = key.bounds();
        m_keyColor = key.color();
        m_step = screen.get<ui::Spot>("PadKeyboardStep").at();
        m_markColor = screen.get<ui::Spot>("PadKeyboardMark").color();
        m_keys.setPainter([this](gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) { paint(canvas, origin, nowMs); });
        screen.on("onPadKeyboardUp", [this](const ui::EventArgs& args) { click(args.position); });
    }

    std::vector<std::vector<PadKeyboard::Key>> PadKeyboard::rows() const {
        static const char* const kLetters[] = {"1234567890", "qwertyuiop", "asdfghjkl-", "zxcvbnm,."};
        static const char* const kSigns[] = {"1234567890", "!@#$%^&*()", "_=+[]{};:'", "\"<>?/\\|~"};
        std::vector<std::vector<Key>> rows;
        for (int row = 0; row < 4; ++row) {
            std::vector<Key> keys;
            int column = 0;
            if (row == 3) {
                keys.push_back({Kind::Shift, "Aa", column++, 1});
            }
            for (const char* letter = (m_symbols ? kSigns : kLetters)[row]; *letter != '\0'; ++letter) {
                const char shown = m_shift && !m_symbols ? static_cast<char>(std::toupper(static_cast<unsigned char>(*letter))) : *letter;
                keys.push_back({Kind::Text, std::string(1, shown), column++, 1});
            }
            if (column < kColumns) {
                keys.push_back({Kind::Text, "`", column, kColumns - column}); // the signs' last row is a key short
            }
            rows.push_back(std::move(keys));
        }
        rows.push_back({
            {Kind::Page, m_symbols ? "abc" : "#+=", 0, 2},
            {Kind::Space, "Space", 2, 4},
            {Kind::Erase, "Erase", 6, 2},
            {Kind::Enter, "Enter", 8, 2},
        });
        return rows;
    }

    int PadKeyboard::keyAt(const std::vector<Key>& row, int column) {
        for (int index = 0; index < static_cast<int>(row.size()); ++index) {
            if (column >= row[index].column && column < row[index].column + row[index].span) {
                return index;
            }
        }
        return static_cast<int>(row.size()) - 1;
    }

    void PadKeyboard::open(int target) {
        m_target = target;
        m_open = true;
        m_shift = false;
        m_symbols = false;
        const ui::TextBox* box = m_screen.textBoxOf(target);
        // a field of digits starts on the digits, any other on the letters
        m_row = box != nullptr && !box->text().empty() && std::all_of(box->text().begin(), box->text().end(), [](unsigned char c) { return std::isdigit(c) != 0; }) ? 0 : 1;
        m_column = 0;
    }

    void PadKeyboard::close() { m_open = false; }

    void PadKeyboard::type(const std::string& text) {
        m_screen.focusId(m_target);
        m_screen.textInput(text);
    }

    void PadKeyboard::strike(ui::Key key) {
        m_screen.focusId(m_target);
        ui::KeyEvent event;
        event.key = key;
        m_screen.keyDown(event);
        m_screen.keyUp(event);
    }

    void PadKeyboard::press(const Key& key) {
        switch (key.kind) {
            case Kind::Text:
                type(key.label);
                m_shift = false;
                break;
            case Kind::Shift:
                m_shift = !m_shift;
                break;
            case Kind::Page:
                m_symbols = !m_symbols;
                m_shift = false;
                break;
            case Kind::Space:
                type(" ");
                break;
            case Kind::Erase:
                strike(ui::Key::Backspace);
                break;
            case Kind::Enter:
                close(); // first: what Enter starts may click another field, which brings the keyboard up anew
                strike(ui::Key::Enter);
                break;
        }
    }

    bool PadKeyboard::padAsk(PadAsk ask) {
        if (!m_open) {
            return false;
        }
        const auto keys = rows();
        const int last = static_cast<int>(keys.size()) - 1;
        m_row = std::clamp(m_row, 0, last);
        const int at = keyAt(keys[m_row], m_column);
        switch (ask) {
            case PadAsk::Up:
                m_row = m_row == 0 ? last : m_row - 1;
                break;
            case PadAsk::Down:
                m_row = m_row == last ? 0 : m_row + 1;
                break;
            case PadAsk::Left: {
                const auto& row = keys[m_row];
                m_column = row[at == 0 ? row.size() - 1 : at - 1].column;
                break;
            }
            case PadAsk::Right: {
                const auto& row = keys[m_row];
                m_column = row[at + 1 == static_cast<int>(row.size()) ? 0 : at + 1].column;
                break;
            }
            case PadAsk::Accept:
            case PadAsk::Interact:
                press(keys[m_row][at]);
                break;
            case PadAsk::Erase:
                strike(ui::Key::Backspace);
                break;
            case PadAsk::Space:
                type(" ");
                break;
            case PadAsk::Cancel:
                close();
                break;
            default: // Menu: nothing while typing
                break;
        }
        return true;
    }

    void PadKeyboard::click(core::Point local) {
        if (!m_open) {
            return;
        }
        const auto keys = rows();
        const int row = (local.y - m_first.y) / m_step.y;
        const int column = (local.x - m_first.x) / m_step.x;
        if (local.y < m_first.y || local.x < m_first.x || row >= static_cast<int>(keys.size()) || column >= kColumns) {
            m_screen.focusId(m_target); // beside the keys: the field keeps the keyboard
            return;
        }
        m_row = row;
        m_column = column;
        press(keys[row][keyAt(keys[row], column)]);
        if (m_open) {
            m_screen.focusId(m_target);
        }
    }

    void PadKeyboard::update(std::uint64_t nowMs) {
        const auto clicked = m_screen.takeTextClick();
        if (clicked && m_padOn && (!m_open || *clicked != m_target)) {
            open(*clicked);
        }
        if (m_open) {
            const int focused = m_screen.focusedId();
            if (!m_padOn || m_screen.textBoxOf(m_target) == nullptr) {
                close(); // the pad was put down, or the field is gone
            } else if (focused != m_target) {
                if (focused == m_keys.id() || focused == m_board.id()) {
                    m_screen.focusId(m_target); // a click on the board: the field keeps the keyboard
                } else {
                    close(); // the keyboard went to something else
                }
            }
        }
        const float passed = m_lastMs == 0 ? 0.0f : static_cast<float>(nowMs - m_lastMs);
        m_lastMs = nowMs;
        m_shown = std::clamp(m_shown + (m_open ? passed : -passed) / kSlideMs, 0.0f, 1.0f);
        m_board.setVisible(m_shown > 0.0f || m_open);
        int down = 0;
        if (m_shown < 1.0f) {
            if (const auto place = m_screen.screenRectOf("PadKeyboardBoard")) {
                const float left = 1.0f - m_shown;
                down = static_cast<int>(static_cast<float>(place->height) * left * left); // it slows as it arrives
            }
        }
        m_screen.slideLayout("PadKeyboardLayer", {0, down}, false);
    }

    void PadKeyboard::paint(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) {
        // the line being typed: its end, with stars for a password
        const core::Rect line{origin.x + m_line.x, origin.y + m_line.y, m_line.width, m_line.height};
        m_lineSkin.draw(canvas, line);
        if (const ui::TextBox* box = m_screen.textBoxOf(m_target)) {
            std::string text = box->isPassword() ? std::string(box->text().size(), '*') : box->text();
            const std::size_t fits = static_cast<std::size_t>(std::max(1, (line.width - 12) / kCharWidth - 1));
            if (text.size() > fits) {
                text = text.substr(text.size() - fits);
            }
            constexpr std::uint64_t kBlinkMs = 500; // the caret shows and hides this long
            if (nowMs / kBlinkMs % 2 == 0) {
                text += '_';
            }
            gfx::TextStyle style;
            style.color = m_lineColor;
            style.changing = true;
            canvas.drawText({line.x + 6, line.y + (line.height - kTextHeight) / 2}, text, style);
        }
        const auto keys = rows();
        const int row = std::clamp(m_row, 0, static_cast<int>(keys.size()) - 1);
        const int marked = keyAt(keys[row], m_column);
        for (int y = 0; y < static_cast<int>(keys.size()); ++y) {
            for (int index = 0; index < static_cast<int>(keys[y].size()); ++index) {
                const Key& key = keys[y][index];
                const core::Rect face{
                    origin.x + m_first.x + key.column * m_step.x,
                    origin.y + m_first.y + y * m_step.y,
                    m_first.width + (key.span - 1) * m_step.x,
                    m_first.height
                };
                m_keySkin.draw(canvas, face);
                const bool held = (key.kind == Kind::Shift && m_shift) || (key.kind == Kind::Page && m_symbols);
                if ((y == row && index == marked) || held) {
                    core::Color mark = m_markColor;
                    mark.a = y == row && index == marked ? 150 : 70;
                    canvas.fillRectAlpha(face, mark);
                }
                if (y == row && index == marked) {
                    canvas.drawRectOutline(face, m_markColor, 2);
                }
                gfx::TextStyle style;
                style.color = m_keyColor;
                canvas.drawText(
                    {face.x + (face.width - static_cast<int>(key.label.size()) * kCharWidth) / 2, face.y + (face.height - kTextHeight) / 2},
                    key.label,
                    style
                );
            }
        }
    }

}
