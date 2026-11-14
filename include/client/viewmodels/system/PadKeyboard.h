#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_PAD_KEYBOARD_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_PAD_KEYBOARD_H

#include "client/services/GameServices.h"
#include "client/services/input/PadAsk.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/TiledFrame.h"

#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief The game pad's keyboard on the screen: it comes up from the bottom edge when the pad clicks a text field, types
    /// into that field, and goes down again on Enter or when it is backed out of.
    /// @details The pad's directions step from key to key and its accept button presses the key; the keys can be clicked
    /// too. Layout: PadKeyboard.xml.
    class PadKeyboard {
        public:
            /// @brief Loads the board and wires its painter and click.
            /// @param services Shared game services.
            /// @param screen Screen the board is loaded into.
            PadKeyboard(GameServices& services, ui::Screen& screen);

            /// @brief Tells the keyboard whether a game pad is what is played with; it only comes up for the pad.
            /// @param on True in controller mode.
            void setPad(bool on) { m_padOn = on; }

            /// @brief Whether the keyboard is up.
            /// @return True while it takes the pad's buttons.
            bool isOpen() const { return m_open; }

            /// @brief Does what the pad asks: steps to a key, presses it, erases, puts a space, or goes down.
            /// @param ask What is asked.
            /// @return True: while it is up the keyboard takes everything.
            bool padAsk(PadAsk ask);

            /// @brief Runs one frame: comes up for a text field the pad clicked, goes down when that field is gone, and slides.
            /// @param nowMs The time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            /// @brief What a key does.
            enum class Kind { Text, Shift, Page, Space, Erase, Enter };

            /// @brief A key of a row.
            struct Key {
                Kind kind = Kind::Text;
                std::string label; // what the key shows; a Text key types it
                int column = 0; // the first column it stands in
                int span = 1; // how many columns wide
            };

            /// @brief Gives the rows of keys of the page shown now.
            /// @return The rows from the top.
            std::vector<std::vector<Key>> rows() const;

            /// @brief Gives the key of a row that stands in a column.
            /// @param row The row's keys.
            /// @param column The column.
            /// @return The key's place in the row.
            static int keyAt(const std::vector<Key>& row, int column);

            /// @brief Comes up for a text field.
            /// @param target The field's element id.
            void open(int target);

            /// @brief Goes down.
            void close();

            /// @brief Does what a key does to the field.
            /// @param key The key.
            void press(const Key& key);

            /// @brief Types into the field.
            /// @param text The characters.
            void type(const std::string& text);

            /// @brief Presses and lets go a key of the real keyboard on the field.
            /// @param key The key.
            void strike(ui::Key key);

            /// @brief Presses the key under a point of the keys' area.
            /// @param local The point, from the area's top left corner.
            void click(core::Point local);

            /// @brief Paints the line being typed and the keys.
            /// @param canvas Target canvas.
            /// @param origin The area's top left corner.
            /// @param nowMs The time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs);

            ui::Screen& m_screen;
            ui::Element& m_board;
            ui::DrawingArea& m_keys;
            ui::TiledFrame m_keySkin; // a key's face
            ui::TiledFrame m_lineSkin; // the line being typed
            core::Rect m_line; // the line's place in the keys' area
            core::Rect m_first; // the first key's place and a key's size
            core::Point m_step; // from a key to the next column and row
            core::Color m_lineColor;
            core::Color m_keyColor;
            core::Color m_markColor;
            bool m_padOn = false;
            bool m_open = false;
            int m_target = 0; // the text field typed into
            bool m_shift = false; // capital letters, for one key
            bool m_symbols = false; // the page of signs is shown
            int m_row = 1; // the key the pad stands on
            int m_column = 0;
            float m_shown = 0.0f; // how far up the board is, 0 to 1
            std::uint64_t m_lastMs = 0;
    };

}

#endif
