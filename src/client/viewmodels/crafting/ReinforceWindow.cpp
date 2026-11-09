#include "client/viewmodels/crafting/ReinforceWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/Equipping.h"
#include "client/rules/FusionScrolls.h"
#include "gfx/TextStyle.h"
#include "protocol/ReinforceRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kCharWidth = 8; // width of one text character, px
        constexpr int kLastEquipSlot = 5; // equipment slots 1..5 can be forged (slot 6 cannot)
        constexpr int kMostFusion = 100; // highest fusion (forge) level
        constexpr int kMallFlag = 0x200; // item flag bit 0x200: the item can't be forged
        constexpr int kNoAttribute = 0; // attribute kind 0: none
        constexpr int kNoAttributeAlt = 250; // attribute kind 250: none worth forging (counts as no attribute)

        /// @brief Loads ReinforceWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ReinforceWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Reinforce");
        }

    }

    ReinforceWindow::ReinforceWindow(
        GameServices& services,
        ui::Screen& screen,
        Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_image(screen.get<ui::DrawingArea>("ReinforceImage")),
          m_nameSpot(screen.get<ui::Spot>("ReinforceScrollName")),
          m_levelSpot(screen.get<ui::Spot>("ReinforceLevel")),
          m_countSpot(screen.get<ui::Spot>("ReinforceCount")) {
        screen.get<ui::DrawingArea>("ReinforceImage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.get<ui::DrawingArea>("ReinforcePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            if (m_scrollSlot != 0 && isFusionScroll(m_inventory.bagItem(m_scrollSlot).itemId)) {
                if (const auto scroll = m_services.items.find(m_inventory.bagItem(m_scrollSlot).itemId)) {
                    const int width = static_cast<int>(scroll->name.size()) * kCharWidth;
                    canvas.drawText(
                        {at.x + 103 - width / 2, at.y + 60},
                        scroll->name,
                        {m_nameSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                        width,
                        12
                    ); // scroll name: centre x 103, y 60, line height 12
                }
            }
            if (!m_shown.empty()) {
                const std::string level = "+" + std::to_string(m_shown.fusionLevel);
                const int width = static_cast<int>(level.size()) * kCharWidth;
                canvas.drawText(
                    {at.x + 132 - width, at.y + 81},
                    level,
                    {m_levelSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                    width,
                    12
                ); // "+N" level: right edge x 132, y 81, line height 12
            }
        });
        screen.on("onReinforceOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onReinforceClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onReinforceTake", [this](const ui::EventArgs&) { clearImage(); });
    }

    void ReinforceWindow::release(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == kMark) {
            m_marks.setUseMark(slot, 0);
        }
    }

    void ReinforceWindow::clearImage() {
        if (m_requestSent) {
            return;
        }
        m_shown = protocol::ItemInstance{};
        release(m_bagSlot);
        m_bagSlot = 0;
        m_hooks.marksChanged();
    }

    void ReinforceWindow::reset() {
        release(m_scrollSlot);
        m_scrollSlot = 0;
        clearImage();
        m_requestSent = false;
        m_hooks.marksChanged();
    }

    void ReinforceWindow::close() {
        if (!m_requestSent) {
            reset();
        }
        m_window.setVisible(false);
    }

    void ReinforceWindow::open(int scrollSlot) {
        if (m_requestSent || scrollSlot < 1 || scrollSlot > BagMarks::kSlots) {
            return;
        }
        if (!isFusionScroll(m_inventory.bagItem(scrollSlot).itemId)) {
            close();
            return;
        }
        reset();
        m_scrollSlot = scrollSlot;
        m_marks.setUseMark(scrollSlot, kMark);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.marksChanged();
    }

    void ReinforceWindow::put(int bagSlot) {
        if (m_requestSent) {
            return;
        }
        clearImage();
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        if (record->equipSlot < 1 || record->equipSlot > kLastEquipSlot) {
            m_messages.show("Can only be used for equipment", 1000); // message display time in ms
        } else if (isMallEquipment(*record)) {
            m_messages.show("Equipment from the Item Mall cannot be used", 3000); // message display time in ms
        } else if (record->petNpcId != 0) {
            m_messages.show("Professional equipment can't be forged.", 1000); // message display time in ms
        } else if ((record->flags & kMallFlag) == kMallFlag) {
            m_messages.show("The item can't be forged", 1000); // message display time in ms
        } else if (
            (record->attributeKinds[0] == kNoAttribute || record->attributeKinds[0] == kNoAttributeAlt) &&
            (record->attributeKinds[1] == kNoAttribute || record->attributeKinds[1] == kNoAttributeAlt)
        ) {
            m_messages.show("The item can't be forged", 1000); // message display time in ms
        } else if (item.fusionLevel + 1 > kMostFusion) {
            m_messages.show("Reached the forge upper limit", 1000); // message display time in ms
        } else if (record->usePeriod != 0) {
            m_messages.show("Temporary items can't be forged", 1000); // message display time in ms
        } else if (const auto notice = m_marks.lockNotice(bagSlot, BagMarks::kCompound)) {
            m_messages.show(notice->text, notice->ms);
        } else if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 3000); // message display time in ms
        } else {
            m_bagSlot = bagSlot;
            m_shown = item;
            m_marks.setUseMark(bagSlot, kMark);
            m_hooks.marksChanged();
        }
    }

    void ReinforceWindow::confirm() {
        if (m_requestSent || m_scrollSlot == 0 || m_bagSlot == 0) {
            return;
        }
        const auto scroll = m_services.items.find(m_inventory.bagItem(m_scrollSlot).itemId);
        m_hooks.ask("Use " + (scroll ? scroll->name : std::string()) + "?", [this] { send(); });
    }

    void ReinforceWindow::send() {
        if (m_requestSent) {
            return;
        }
        m_hooks.send(protocol::reinforceRequest(m_scrollSlot, m_bagSlot));
        m_requestSent = true;
        close();
    }

    void ReinforceWindow::result(int code, int bagSlot, int newLevel) {
        // result code 1: fusion level of the bag place is now newLevel; code 2: failed
        if (code == 1 && bagSlot >= 1 && bagSlot <= Inventory::kBagSlots) {
            const int oldLevel = m_inventory.bagItem(bagSlot).fusionLevel;
            m_inventory.setFusionLevel(bagSlot, newLevel);
            const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
            const std::string name = record ? record->name : std::string();
            const std::string levels = "#R(" + std::to_string(oldLevel) + "->" + std::to_string(newLevel) + ")/#R";
            if (newLevel > oldLevel) {
                m_messages.show(name + "Fusion succeeded" + levels, 3000); // message display time in ms
                m_services.playSound("sound/SEB0300.wav");
            } else {
                m_messages.show(name + "Fusion failed" + levels, 3000); // message display time in ms
                m_services.playSound("sound/wav1607.wav");
            }
            m_requestSent = false;
            reset();
        } else if (code == 2) {
            m_messages.show("Fusion failed", 3000); // message display time in ms
            m_requestSent = false;
            reset();
        }
    }

    void ReinforceWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!isFusionScroll(m_inventory.bagItem(m_scrollSlot).itemId)) {
            close();
            return;
        }
        if (m_bagSlot != 0 && m_inventory.bagItem(m_bagSlot).empty()) {
            clearImage();
        }
        if (m_shown.empty() || m_screen.hoveredId() != m_image.id()) {
            return;
        }
        const core::Point at = m_window.bounds().position() + m_image.bounds().position();
        m_info.show(m_shown, {at.x, at.y, m_image.bounds().width, m_image.bounds().height});
    }

    void ReinforceWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_shown.empty()) {
            return;
        }
        const ui::IconCell cell = itemIconCell(m_services, m_shown, false);
        if (cell.picture.isValid()) {
            canvas.drawPicture(cell.picture, at, core::Rect{0, 0, m_image.bounds().width, m_image.bounds().height});
        }
        if (m_shown.count > 0) {
            canvas.drawText(
                at + m_countSpot.at(),
                std::to_string(m_shown.count),
                {m_countSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_image.bounds().width
            );
        }
    }

}
