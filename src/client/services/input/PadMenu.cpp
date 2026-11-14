#include "client/services/input/PadMenu.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr float kStickStep = 0.6f; // how far the left stick is pushed before it moves the highlight
        constexpr double kFirstRepeatMs = 350.0; // a held way steps again after this
        constexpr double kNextRepeatMs = 140.0; // and then this often

        // the buttons of the bar at the bottom right (MainButtons.xml)
        constexpr const char* kBottomBar[] = {"MainRetained", "MainItem", "MainSkill", "MainCompound", "MainGroup", "MainSocial", "MainSystem"};
        // the buttons of the bar at the top right (FunctionButtons.xml)
        constexpr const char* kTopBar[] = {
            "FunctionPk",
            "FunctionJoinCombat",
            "FunctionWatchCombat",
            "FunctionTeam",
            "FunctionFriends",
            "FunctionMoney",
            "FunctionGuildInvite",
            "FunctionSchedule",
            "FunctionDraw",
            "FunctionMall",
        };

    }

    std::vector<PadMenu::Shown> PadMenu::shownOf(ui::Screen& screen, bool top) {
        std::vector<Shown> shown;
        const auto add = [&](const char* name) {
            if (const auto rect = screen.screenRectOf(name)) {
                shown.push_back({name, rect->x});
            }
        };
        if (top) {
            for (const char* name : kTopBar) {
                add(name);
            }
        } else {
            for (const char* name : kBottomBar) {
                add(name);
            }
        }
        std::stable_sort(shown.begin(), shown.end(), [](const Shown& a, const Shown& b) { return a.x < b.x; });
        return shown;
    }

    bool PadMenu::open(ui::Screen& screen) {
        std::vector<Shown> row = shownOf(screen, m_top);
        if (row.empty()) {
            m_top = !m_top;
            row = shownOf(screen, m_top);
        }
        if (row.empty()) {
            return false;
        }
        const bool kept = std::any_of(row.begin(), row.end(), [this](const Shown& button) { return button.name == m_current; });
        if (!kept) {
            // the bottom bar's first button of the six is the Item button; the furniture button before it is rarely there
            const auto item = std::find_if(row.begin(), row.end(), [](const Shown& button) { return button.name == "MainItem"; });
            m_current = item != row.end() ? item->name : row.front().name;
        }
        m_open = true;
        m_held = Way::None;
        screen.setPadFocus(m_current, m_top ? m_below : m_above);
        return true;
    }

    void PadMenu::close(ui::Screen& screen) {
        m_open = false;
        m_held = Way::None;
        screen.setPadFocus({});
    }

    void PadMenu::choose(ui::Screen& screen) {
        if (!m_open) {
            return;
        }
        screen.pressPadFocus();
        close(screen);
    }

    void PadMenu::step(Way way, ui::Screen& screen) {
        std::vector<Shown> row = shownOf(screen, m_top);
        const auto here = std::find_if(row.begin(), row.end(), [this](const Shown& button) { return button.name == m_current; });
        if (way == Way::Left || way == Way::Right) {
            if (row.empty()) {
                return;
            }
            const int count = static_cast<int>(row.size());
            const int index = here != row.end() ? static_cast<int>(here - row.begin()) : 0;
            const int next = here == row.end() ? 0 : (index + (way == Way::Right ? 1 : count - 1)) % count; // around the ends
            m_current = row[static_cast<std::size_t>(next)].name;
        } else {
            if ((way == Way::Up) == m_top) {
                return; // already on that bar
            }
            const std::vector<Shown> other = shownOf(screen, !m_top);
            if (other.empty()) {
                return;
            }
            const int x = here != row.end() ? here->x : other.back().x;
            const auto nearest =
                std::min_element(other.begin(), other.end(), [x](const Shown& a, const Shown& b) { return std::abs(a.x - x) < std::abs(b.x - x); });
            m_top = !m_top;
            m_current = nearest->name;
        }
        screen.setPadFocus(m_current, m_top ? m_below : m_above);
    }

    void PadMenu::update(double nowMs, const Gamepad& pad, ui::Screen& screen) {
        if (!m_open) {
            return;
        }
        if (!screen.screenRectOf(m_current)) {
            close(screen); // the game took the bars away (a film, a battle)
            return;
        }
        Way way = pad.isHeld(PadButton::Left)    ? Way::Left
                  : pad.isHeld(PadButton::Right) ? Way::Right
                  : pad.isHeld(PadButton::Up)    ? Way::Up
                  : pad.isHeld(PadButton::Down)  ? Way::Down
                                                 : Way::None;
        if (way == Way::None) {
            const PadStick stick = pad.leftStick();
            if (std::abs(stick.x) >= kStickStep && std::abs(stick.x) >= std::abs(stick.y)) {
                way = stick.x < 0.0f ? Way::Left : Way::Right;
            } else if (std::abs(stick.y) >= kStickStep) {
                way = stick.y < 0.0f ? Way::Up : Way::Down;
            }
        }
        if (way == Way::None) {
            m_held = Way::None;
            return;
        }
        if (way != m_held) {
            m_held = way;
            m_repeatMs = nowMs + kFirstRepeatMs;
            step(way, screen);
        } else if (nowMs >= m_repeatMs) {
            m_repeatMs = nowMs + kNextRepeatMs;
            step(way, screen);
        }
    }

}
