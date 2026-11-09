#include "client/rendering/hud/PartyPanels.h"

#include "ui/Tooltip.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr core::Point kHead{12, 17}; // px: chat-head position within a row
        constexpr core::Point kParts{26, 2}; // px: origin of the level/HP/SP parts within a row
        constexpr core::Point kLevelWord{0, 2}; // px: Lv word, relative to the parts origin
        constexpr core::Point kLevelDigits{17, 2}; // px: level digits, relative to the parts origin
        constexpr core::Point kHpWord{0, 10}; // px: HP word, relative to the parts origin
        constexpr core::Point kHpFrame{13, 11}; // px: HP bar frame, relative to the parts origin
        constexpr core::Point kHpBar{14, 12}; // px: HP bar fill, relative to the parts origin
        constexpr core::Point kSpWord{0, 18}; // px: SP word, relative to the parts origin
        constexpr core::Point kSpFrame{13, 19}; // px: SP bar frame, relative to the parts origin
        constexpr core::Point kSpBar{14, 20}; // px: SP bar fill, relative to the parts origin
        constexpr int kDigitStep = 5; // px: x advance per level digit
        constexpr int kDigitPitch = 6; // px: height of one digit row in the digit picture
        const core::Color kCommanding{255, 230, 90, 255}; // yellow outline of the commanding member
        const core::Color kSelected{120, 200, 255, 255}; // light blue window outline
        const core::Color kGridBlue{74, 130, 165, 255}; // blue outline of a selected member row
        // the member's name and state lines in the chat's and the hints' look: shadowed text
        const gfx::TextStyle kState{{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt}; // white text with shadow
        const gfx::TextStyle kGone{{255, 120, 120, 255}, gfx::TextEffect::Shadow, std::nullopt}; // red text with shadow

        /// @brief Fill ratio of a pet-style bar: Ceil(value / most * 100) %, at most 100.
        /// @param value Current value.
        /// @param most Maximum value.
        /// @return Ratio from 0 to 1.
        double barRatio(double value, double most) {
            if (most <= 0) {
                return 0;
            }
            return std::clamp(std::ceil(value / most * 100.0), 0.0, 100.0) / 100.0; // ratio to percent and back
        }

        /// @brief Tells whether a member is in the world or in a battle (its pet-style row can show).
        /// @param member The member.
        /// @return True when in the world or in battle.
        bool inWorld(const PartyMemberView& member) {
            return member.state == PartyMemberView::State::InWorld || member.state == PartyMemberView::State::InBattle;
        }

    }

    std::vector<int> PartyPanels::rows(const std::vector<PartyMemberView>& members, const std::vector<std::int32_t>& battleIds) {
        std::vector<int> rows;
        int next = 0;
        for (const PartyMemberView& member : members) {
            const bool fights = member.id != 0 && std::find(battleIds.begin(), battleIds.end(), member.id) != battleIds.end();
            rows.push_back(fights ? -1 : next++);
        }
        return rows;
    }

    core::Point PartyPanels::toRows(core::Point point) const {
        return {static_cast<int>(std::floor((point.x - m_pivot.x) / m_scale)), static_cast<int>(std::floor((point.y - m_pivot.y) / m_scale))};
    }

    int PartyPanels::memberAt(core::Point point) const {
        const core::Point at = toRows(point);
        for (std::size_t index = 0; index < m_rows.size(); ++index) {
            if (m_rows[index] >= 0 &&
                core::Rect{kLeft, kTop + m_rows[index] * kRowHeight, kRowWidth, kRowHeight - 2}.contains(at)) { // 2 pixel gap between rows
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    void PartyPanels::drawGrid(gfx::Canvas& canvas, const core::Rect& area) {
        const auto grid = m_pictures.find("32X32Grid");
        if (!grid) {
            return;
        }
        const gfx::Picture& picture = m_pictures.picture(*grid);
        for (int y = 0; y < area.height; y += picture.height()) {
            for (int x = 0; x < area.width; x += picture.width()) {
                canvas.drawPicture(
                    *grid,
                    {area.x + x, area.y + y},
                    core::Rect{0, 0, std::min(picture.width(), area.width - x), std::min(picture.height(), area.height - y)}
                );
            }
        }
    }

    void PartyPanels::drawRow(gfx::Canvas& canvas, const PartyMemberView& member, core::Point at) {
        const auto draw = [&](const char* name, core::Point offset) {
            if (const auto id = m_pictures.find(name)) {
                canvas.drawPicture(*id, at + kParts + offset);
            }
        };
        const auto bar = [&](const char* name, core::Point offset, double ratio) {
            const auto id = m_pictures.find(name);
            if (!id) {
                return;
            }
            const int width = static_cast<int>(m_pictures.picture(*id).width() * ratio);
            if (width > 0) {
                canvas.drawPicture(*id, at + kParts + offset, core::Rect{0, 0, width, m_pictures.picture(*id).height()});
            }
        };
        if (!inWorld(member)) {
            const bool loggingIn = member.state == PartyMemberView::State::LoggingIn;
            canvas.drawText({at.x, at.y + 2}, member.name.empty() ? member.account : member.name, kState); // name line, 2 pixels down
            canvas.drawText({at.x, at.y + 16}, loggingIn ? "logging in..." : member.reason, loggingIn ? kState : kGone); // state line, 16 pixels down
            return;
        }
        if (member.look) {
            m_characters.drawChatHead(canvas, *member.look, at + kHead, true);
        }
        draw("Word_Lv_1", kLevelWord);
        if (const auto digits = m_pictures.find("Pet_Num_Lv_White_1")) {
            const gfx::Picture& picture = m_pictures.picture(*digits);
            const std::string text = std::to_string(member.level);
            for (std::size_t i = 0; i < text.size(); ++i) {
                const core::Point place = at + kParts + kLevelDigits;
                canvas.drawPicture(
                    *digits,
                    {place.x + static_cast<int>(i) * kDigitStep, place.y},
                    core::Rect{0, (text[i] - '0') * kDigitPitch, picture.width(), picture.height() / 10}
                ); // the picture is a column of ten digits
            }
        }
        draw("Word_Hp_1", kHpWord);
        draw("Pet_Frame_Hp", kHpFrame);
        bar("Pet_Bmp_Hp", kHpBar, barRatio(member.hp, member.maxHp));
        draw("Word_Sp_1", kSpWord);
        draw("Pet_Frame_Sp", kSpFrame);
        bar("Pet_Bmp_Sp", kSpBar, barRatio(member.sp, member.maxSp));
    }

    void PartyPanels::draw(
        gfx::Canvas& canvas,
        const std::vector<PartyMemberView>& members,
        const std::vector<std::int32_t>& battleIds,
        const core::Rect& view,
        float hudScale,
        std::optional<core::Point> mouse
    ) {
        m_pivot = {view.x, view.y};
        m_scale = hudScale > 0 ? hudScale : 1.0f;
        m_rows = rows(members, battleIds);
        canvas.beginScale(m_pivot, m_scale);
        for (std::size_t index = 0; index < members.size(); ++index) {
            if (m_rows[index] < 0) {
                continue;
            }
            const PartyMemberView& member = members[index];
            const core::Point at = m_pivot + core::Point{kLeft, kTop + m_rows[index] * kRowHeight};
            const core::Rect
                area{at.x - 2, at.y - 1, kRowWidth, kRowHeight - 2}; // row box: 2 left and 1 up of the row origin, 2 pixel shorter than the row pitch
            if (member.selected) {
                drawGrid(canvas, area);
                canvas.drawRectOutline(area, kGridBlue, 1);
            }
            drawRow(canvas, member, at);
            if (member.commanding) {
                canvas.drawRectOutline(area, kCommanding, 1);
            }
        }
        canvas.endScale();
        const int hovered = mouse ? memberAt(*mouse) : -1;
        if (hovered >= 0 && inWorld(members[static_cast<std::size_t>(hovered)])) {
            ui::drawHintBox(canvas, *mouse, members[static_cast<std::size_t>(hovered)].name);
        }
    }

    void PartyPanels::drawWindowTag(gfx::Canvas& canvas, const core::Rect& window, const std::string& name) {
        canvas.drawRectOutline(window, kSelected, 2); // outline thickness in pixels
        ui::drawHintBox(canvas, window.position(), name);
    }

}
