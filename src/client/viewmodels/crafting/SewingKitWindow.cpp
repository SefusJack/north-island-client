#include "client/viewmodels/crafting/SewingKitWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "gfx/TextStyle.h"
#include "protocol/SewingKitRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kMostBagCells = 50; // bag cells 1..50
        constexpr int kRoleExclusiveSubType = 149; // item sub type 149: role-exclusive gear
        constexpr int kWeddingItems[] = {
            21207, // Groom's Wear
            24164, // Groom's Boots
            25610, // Diamond Band
            22211, // Bride's Veil
            21208, // Bride's Wear
            18070, // Bouquets
            24165, // Bride's Boots
            25609 // Wedding Band
        };
        constexpr int kValueBar = 18071; // item 18071: Lover's Bouquet (the "value bar" in the original)
        constexpr int kGunType = 11; // item type 11: guns (Arquebus, Bazooka, ...)
        constexpr int kBowType = 4; // item type 4: bows (Crescent Bow, Blue Moon Bow, ...)
        constexpr int kWeaponSlot = 3; // equipment slot 3: weapon

        /// @brief Tells whether an item is a wedding item.
        /// @details Original: TFItem.IsWeddingItem.
        /// @param itemId Item id.
        /// @return True for a wedding item.
        bool isWedding(int itemId) {
            for (const int wedding : kWeddingItems) {
                if (wedding == itemId) {
                    return true;
                }
            }
            return false;
        }

        /// @brief Loads SewingKitWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SewingKitWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("SewingKit");
        }

    }

    SewingKitWindow::SewingKitWindow(
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
          m_cells{
              screen.get<ui::DrawingArea>("SewingKitPlace1"),
              screen.get<ui::DrawingArea>("SewingKitPlace2"),
              screen.get<ui::DrawingArea>("SewingKitPlace3")
          },
          m_iconSpot(screen.get<ui::Spot>("SewingKitIcon")),
          m_countSpot(screen.get<ui::Spot>("SewingKitCount")) {
        for (int place = 1; place <= 3; ++place) { // the form has 3 places (look, attributes, result)
            screen.get<ui::DrawingArea>("SewingKitPlace" + std::to_string(place)).setPainter([this, place](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paint(canvas, at, place);
            });
        }
        screen.on("onSewingKitConfirm", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onSewingKitClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onSewingKitTake1", [this](const ui::EventArgs&) { clearPlace(1); });
        screen.on("onSewingKitTake2", [this](const ui::EventArgs&) { clearPlace(2); });
    }

    void SewingKitWindow::release(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == kMark) {
            m_marks.setUseMark(slot, 0);
        }
    }

    std::string SewingKitWindow::kitName() const {
        const auto kit =
            m_services.items.find(m_kitKind == 1 ? kSewingTools : kWeaponModel); // kit kind 1: Sewing Tools (item 34167), 2: Weapon Model (item 34170)
        return kit ? kit->name : std::string();
    }

    void SewingKitWindow::cannotUse(bool withName) {
        m_messages.show("The item can't be used " + (withName ? kitName() : std::string()), 3000);
    } // message display time in ms

    void SewingKitWindow::clearPlace(int place) {
        if (m_sending) {
            return;
        }
        const int before = shown(place).itemId;
        m_images[static_cast<std::size_t>(place - 1)] = protocol::ItemInstance{};
        if (place == 1) {
            release(m_poseSlot);
            m_poseSlot = 0;
        } else if (place == 2) {
            release(m_equipSlot);
            m_equipSlot = 0;
        }
        if (!shown(3).empty()) {
            m_images[2] = protocol::ItemInstance{}; // index of the result place (place 3)
            m_previewIcon = 0;
        }
        if (shown(place).itemId != before) {
            m_services.playSound("sound/wav0154.wav");
        }
        m_hooks.marksChanged();
    }

    void SewingKitWindow::reset() {
        for (int place = 1; place <= 3; ++place) { // the form has 3 places
            clearPlace(place);
        }
        release(m_kitSlot);
        m_kitSlot = 0;
        m_kitKind = 0;
        m_sending = false;
        m_hooks.marksChanged();
    }

    void SewingKitWindow::close() {
        m_window.setVisible(false);
        if (!m_sending) {
            reset();
        }
    }

    void SewingKitWindow::refreshPlaces() {
        for (int round = 1; round <= 3; ++round) { // one round per place (3 places)
            if (m_poseSlot != 0) {
                setPlace(m_poseSlot, 1, false);
            } else if (m_equipSlot != 0) {
                setPlace(m_equipSlot, 2, false);
            } else {
                clearPlace(round);
            }
        }
    }

    void SewingKitWindow::open(int kitSlot) {
        if (m_sending || kitSlot < 1 || kitSlot > kMostBagCells) {
            return;
        }
        const int itemId = m_inventory.bagItem(kitSlot).itemId;
        if (itemId != kSewingTools && itemId != kWeaponModel) {
            close();
            return;
        }
        reset();
        m_kitKind = itemId == kSewingTools ? 1 : 2; // kit kind 1: Sewing Tools (armour), 2: Weapon Model (weapons)
        m_window.setPicture(m_kitKind == 1 ? "Form_SewingKit_1" : "Form_WeaponMold_1", m_services.pictures);
        release(m_kitSlot);
        m_kitSlot = kitSlot;
        m_marks.setUseMark(kitSlot, kMark);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        refreshPlaces();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.marksChanged();
    }

    void SewingKitWindow::put(int bagSlot, int place) {
        if (!m_sending) {
            setPlace(bagSlot, place, place == 1);
        }
    }

    void SewingKitWindow::setPlace(int bagSlot, int place, bool confirmFirst) {
        if (m_sending || place < 1 || place > 2) { // only places 1 and 2 take a piece
            return;
        }
        clearPlace(place);
        if (bagSlot < 1 || bagSlot > kMostBagCells || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        const std::string notSame = "They are not the same type, and can't be mixed.";
        const auto typeOf = [this](int slot) {
            const auto other = m_services.items.find(m_inventory.bagItem(slot).itemId);
            return other ? other->type : 0;
        };
        const auto slotOf = [this](int slot) {
            const auto other = m_services.items.find(m_inventory.bagItem(slot).itemId);
            return other ? other->equipSlot : 0;
        };
        const auto exclusive = [this](int slot) {
            const auto other = m_services.items.find(m_inventory.bagItem(slot).itemId);
            return other && other->subType == kRoleExclusiveSubType;
        };
        if (m_kitKind == 1) {
            // armour equip slots 1, 2, 4 and 5 (the weapon slot 3 and slot 6 are excluded)
            const bool armour = record->equipSlot == 1 || record->equipSlot == 2 || record->equipSlot == 4 || record->equipSlot == 5;
            if (!armour) {
                cannotUse(true);
                return;
            }
            if (isWedding(record->id)) {
                cannotUse(false);
                return;
            }
            if (record->subType == kRoleExclusiveSubType) {
                cannotUse(true);
                return;
            }
        } else if (m_kitKind == 2) {
            if (record->equipSlot != kWeaponSlot || record->type == kGunType) {
                cannotUse(true);
                return;
            }
            // the refusal messages from here to the usableBy switch show for 3000 ms
            if (place == 2 && record->id == kValueBar) {
                m_messages.show(record->name + "Cannot be put into value bar", 3000);
                return;
            }
            if (m_poseSlot != 0) {
                if (m_inventory.bagItem(m_poseSlot).itemId == kValueBar) {
                    if (record->type == kBowType || record->type == kGunType) {
                        m_messages.show(notSame, 3000);
                        return;
                    }
                } else if (typeOf(m_poseSlot) != record->type) {
                    m_messages.show(notSame, 3000);
                    return;
                }
            }
            if (m_equipSlot != 0) {
                if (record->id == kValueBar) {
                    if (typeOf(m_equipSlot) == kBowType || typeOf(m_equipSlot) == kGunType) {
                        m_messages.show(notSame, 3000);
                        return;
                    }
                } else if (typeOf(m_equipSlot) != record->type) {
                    m_messages.show(notSame, 3000);
                    return;
                }
            }
            if (isWedding(record->id)) {
                cannotUse(false);
                return;
            }
            if (exclusive(m_poseSlot) || exclusive(m_equipSlot)) {
                m_messages.show(notSame, 3000);
                return;
            }
        } else {
            return;
        }
        const int bodyType = m_hooks.bodyType();
        const std::string notSuit = "Doesn't suit players";
        // usableBy restricts the wearer by body type; the notSuit messages show for 3000 ms
        switch (record->usableBy) {
            case 0: // usableBy 0: no body type restriction
            case 3: // usableBy 3: no body type restriction
            case 8: // usableBy 8: no body type restriction
            case 9: // usableBy 9: no body type restriction
            case 10: // usableBy 10: no body type restriction
            case 11: // usableBy 11: no body type restriction
            case 12: // usableBy 12: no body type restriction
            case 13: break; // usableBy 13: no body type restriction
            case 1: // usableBy 1: body types 1 and 3 only
                if (bodyType != 1 && bodyType != 3) {
                    m_messages.show(notSuit, 3000);
                    return;
                }
                break;
            case 2: // usableBy 2: body types 2 and 4 only
                if (bodyType != 2 && bodyType != 4) {
                    m_messages.show(notSuit, 3000);
                    return;
                }
                break;
            case 4: // usableBy 4: body type 1 only (usableBy - 3)
            case 5: // usableBy 5: body type 2 only (usableBy - 3)
            case 6: // usableBy 6: body type 3 only (usableBy - 3)
            case 7: // usableBy 7: body type 4 only (usableBy - 3)
                if (bodyType != record->usableBy - 3) {
                    m_messages.show(notSuit, 3000);
                    return;
                }
                break;
            default: return; // any other usableBy value: refused silently
        }
        if (record->rank <= 1 || record->petNpcId != 0) { // rank 1 or lower: basic gear is refused; pet (petNpcId) gear is refused
            cannotUse(true);
            return;
        }
        // the notSame messages below show for 3000 ms
        if (m_poseSlot != 0 && slotOf(m_poseSlot) != record->equipSlot) {
            m_messages.show(notSame, 3000);
            return;
        }
        if (m_equipSlot != 0 && slotOf(m_equipSlot) != record->equipSlot) {
            m_messages.show(notSame, 3000);
            return;
        }
        if (bagSlot != m_equipSlot && bagSlot != m_poseSlot) {
            if (const auto notice = m_marks.lockNotice(bagSlot, BagMarks::kCompound)) {
                m_messages.show(notice->text, notice->ms);
                return;
            }
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 3000); // message display time in ms
            return;
        }
        if (place == 1) {
            m_poseSlot = bagSlot;
            if (bagSlot == m_equipSlot) {
                clearPlace(2);
                m_equipSlot = 0;
            }
        } else {
            m_equipSlot = bagSlot;
            if (bagSlot == m_poseSlot) {
                clearPlace(1);
                m_poseSlot = 0;
            }
        }
        if (confirmFirst) {
            m_pendingSlot = bagSlot;
            m_hooks.ask("Being the equipment, forging value , and additional item of pose, will #Rdisappear after using/#R\rSure? ", [this] {
                setPlace(m_pendingSlot, 1, false);
            });
            return;
        }
        m_images[static_cast<std::size_t>(place - 1)] = item;
        m_marks.setUseMark(bagSlot, kMark);
        m_services.playSound("sound/wav0154.wav");
        if (m_poseSlot != 0 && m_equipSlot != 0) {
            const protocol::ItemInstance& attribute = m_inventory.bagItem(m_equipSlot);
            const protocol::ItemInstance& shape = m_inventory.bagItem(m_poseSlot);
            // index 2 is the result place (place 3)
            m_images[2] = attribute;
            m_images[2].lookItemId = shape.lookItemId == 0 ? shape.itemId : shape.lookItemId;
            m_previewIcon = shape.itemId;
        }
        m_hooks.marksChanged();
    }

    void SewingKitWindow::confirm() {
        if (m_sending || (m_kitKind != 1 && m_kitKind != 2)) {
            return;
        }
        m_hooks.ask("Had Pose\rCan't be transfered\rConfirm " + kitName() + "?", [this] { send(); });
    }

    void SewingKitWindow::send() {
        if (m_sending) {
            return;
        }
        m_hooks.send(protocol::sewingKitRequest(m_kitSlot, m_equipSlot, m_poseSlot));
        m_sending = true;
    }

    void SewingKitWindow::result(int code, int bagSlot, int look) {
        if (code == 1) { // result code 1: the pose was made (code 2: failed)
            if (bagSlot >= 1 && bagSlot <= kMostBagCells) {
                m_inventory.setLookItem(bagSlot, look);
            }
            m_sending = false;
            close();
            m_services.playSound("sound/wav0152.wav");
            m_messages.show("Pose designed successfully", 2000); // message display time in ms
        } else if (code == 2) {
            m_sending = false;
            close();
            m_messages.show("Pose designed failed", 2000); // message display time in ms
        }
    }

    void SewingKitWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        const int wanted = m_kitKind == 1 ? kSewingTools : m_kitKind == 2 ? kWeaponModel : 0;
        if (wanted == 0 || m_inventory.bagItem(m_kitSlot).itemId != wanted) {
            close();
            return;
        }
        if (m_poseSlot != 0 && m_inventory.bagItem(m_poseSlot).itemId != shown(1).itemId) {
            m_poseSlot = 0;
            refreshPlaces();
        }
        if (m_equipSlot != 0 && m_inventory.bagItem(m_equipSlot).itemId != shown(2).itemId) {
            m_equipSlot = 0;
            refreshPlaces();
        }
        for (int place = 1; place <= 3; ++place) { // the form has 3 places
            ui::Element& cell = m_cells[static_cast<std::size_t>(place - 1)];
            if (shown(place).empty() || m_screen.hoveredId() != cell.id()) {
                continue;
            }
            const core::Point at = m_window.bounds().position() + cell.bounds().position();
            m_info.show(shown(place), {at.x, at.y, m_iconSpot.size().width, m_iconSpot.size().height});
        }
    }

    void SewingKitWindow::paint(gfx::Canvas& canvas, core::Point at, int place) {
        const protocol::ItemInstance& item = shown(place);
        if (item.empty()) {
            return;
        }
        protocol::ItemInstance drawn = item;
        // place 3 is the result place and takes the preview look
        drawn.lookItemId = place == 3 ? m_previewIcon : 0;
        const ui::IconCell cell = itemIconCell(m_services, drawn, false);
        if (cell.picture.isValid()) {
            canvas.drawPicture(cell.picture, at, core::Rect{0, 0, m_iconSpot.size().width, m_iconSpot.size().height});
        }
        if (item.count > 0) {
            canvas.drawText(
                at + m_countSpot.at(),
                std::to_string(item.count),
                {m_countSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_iconSpot.size().width
            );
        }
    }

}
