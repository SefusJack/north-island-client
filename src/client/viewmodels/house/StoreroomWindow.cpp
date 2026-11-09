#include "client/viewmodels/house/StoreroomWindow.h"

#include "client/rules/BoxItems.h"
#include "client/rendering/painters/ItemCells.h"
#include "protocol/ItemRequests.h"
#include "protocol/StoreroomRequests.h"

#include <cstdlib>
#include <utility>

namespace client {

    namespace {

        constexpr int kRows = 10; // rows of the storeroom grid
        constexpr int kColumns = Inventory::kColumns; // columns of the storeroom grid (same as the bag)
        constexpr int kCells = kRows * kColumns; // cells in the storeroom
        constexpr int kMostStack = Inventory::kMaxStack; // most items of one kind in one stack
        constexpr int kBagRows = Inventory::kRows; // rows of the bag grid
        constexpr int kUnstorable = 39111; // item 39111: Sweet Door, cannot be put into the warehouse
        constexpr int kTentSubType = 104; // item sub type 104: tents (original IsTentItem), cannot be placed in the warehouse
        constexpr int kNoticeMs = 2000; // notice display time, ms
        constexpr int kObtainMs = 1500; // "Obtain" notice display time, ms

        /// @brief Loads StoreroomWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("StoreroomWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("StoreroomWindow");
        }

    }

    StoreroomWindow::StoreroomWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& bag,
        const BagMarks& marks,
        PlayerMessages& messages,
        CountBox& countBox,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_bag(bag),
          m_marks(marks),
          m_messages(messages),
          m_countBox(countBox),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_cells(screen.get<ui::IconGrid>("StoreroomCells")),
          m_iconSpot(screen.get<ui::Spot>("StoreroomIcon")),
          m_grid(services.items, kRows) {
        const gfx::PictureId icon = services.pictures.find("Icon_WareHouse_1").value_or(gfx::PictureId{});
        screen.get<ui::DrawingArea>("StoreroomPage").setPainter([this, icon](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            if (icon.isValid()) {
                canvas.drawPicture(icon, {at.x + m_iconSpot.at().x, at.y + m_iconSpot.at().y});
            }
        });
        screen.on("onStoreroomClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onStoreroomSort", [this](const ui::EventArgs&) {
            if (m_services.serverSorts) {
                m_services.connection.send(protocol::sortRequest(protocol::SortGrid::Storeroom));
                m_sorter.startOnServer(m_screen.context().nowMs);
            } else {
                m_sorter.start();
            }
        });
        screen.on("onStoreroomDrop", [this](const ui::EventArgs& args) { gridItemDropped(args); });
    }

    void StoreroomWindow::open(int piece) {
        m_piece = piece;
        open();
    }

    void StoreroomWindow::open() {
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    void StoreroomWindow::close() { m_window.setVisible(false); }

    void StoreroomWindow::setList(const protocol::StoreroomList& list) {
        for (int cell = 1; cell <= kCells; ++cell) {
            m_grid.takeFromBag(cell, kMostStack);
        }
        for (const protocol::BagItemRecord& record : list.records) {
            if (record.slot >= 1 && record.slot <= kCells) {
                m_grid.addToBag(record.slot, record.item);
            }
        }
        m_piece = list.piece;
        refresh();
    }

    void StoreroomWindow::add(const protocol::StoreroomAdd& add) {
        int cell = add.record.slot;
        const protocol::ItemInstance& item = add.record.item;
        if (cell == 0) {
            cell = m_grid.freeSlotFor(item.itemId);
        }
        if (cell < 1 || cell > kCells || m_grid.fitCount(cell, item.itemId, item.count, 0) != item.count) {
            return;
        }
        m_grid.addToBag(cell, item);
        if (add.showMessage) {
            const auto record = m_services.items.find(item.itemId);
            m_messages.show("Obtain " + std::to_string(item.count) + " Quantity " + (record ? record->name : std::string()), kObtainMs);
        }
        refresh();
    }

    void StoreroomWindow::remove(const protocol::StoreroomRemove& remove) {
        int cell = remove.cell;
        if (cell == 0) {
            for (int at = 1; at <= kCells && cell == 0; ++at) {
                if (m_grid.bagItem(at).itemId == remove.itemId) {
                    cell = at;
                }
            }
        }
        if (cell < 1 || cell > kCells) {
            return;
        }
        m_grid.takeFromBag(cell, remove.count);
        refresh();
    }

    void StoreroomWindow::refresh() {
        m_cells.clearCells();
        for (int cell = 1; cell <= kCells; ++cell) {
            const protocol::ItemInstance& item = m_grid.bagItem(cell);
            if (!item.empty()) {
                m_cells.setCell(cell, itemIconCell(m_services, item, true));
            }
        }
    }

    int StoreroomWindow::neighbour(int slot, int rows, int direction) {
        // direction 1..9 lies on a 3 x 3 block around the place (3 per row)
        const int row = (slot - 1) / kColumns + (direction - 1) / 3 - 1;
        const int column = (slot - 1) % kColumns + (direction - 1) % 3 - 1;
        if (row < 0 || row >= rows || column < 0 || column >= kColumns) {
            return 0;
        }
        return row * kColumns + column + 1;
    }

    bool StoreroomWindow::pieceGone() {
        // 130 = highest furniture piece number of a house
        if (m_piece > 0 && m_piece <= 130 && m_hooks.pieceElsewhere && m_hooks.pieceElsewhere(m_piece)) {
            close();
            return true;
        }
        return false;
    }

    void StoreroomWindow::requestPut(int bagSlot, int cell, int count, bool exact) {
        const protocol::ItemInstance& item = m_bag.bagItem(bagSlot);
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || item.empty() || cell < 1 || cell > kCells) {
            return;
        }
        if (m_grid.fitCount(cell, item.itemId, count, 0) != count) {
            if (!exact) {
                return;
            }
            bool found = false;
            // 9 = the nine neighbour directions of a place
            for (int direction = 1; direction <= 9 && !found; ++direction) {
                const int near = neighbour(cell, kRows, direction);
                if (near != 0 && near != cell && m_grid.fitCount(near, item.itemId, count, 0) == count) {
                    cell = near;
                    found = true;
                }
            }
            if (!found) {
                return;
            }
        }
        m_services.connection.send(protocol::storeroomPutRequest(bagSlot, cell, count, m_piece));
    }

    void StoreroomWindow::requestMove(int from, int to, int count, bool exact) {
        const protocol::ItemInstance& item = m_grid.bagItem(from);
        if (from < 1 || from > kCells || item.empty() || to < 1 || to > kCells) {
            return;
        }
        if (m_grid.fitCount(to, item.itemId, count, from) != count) {
            if (!exact) {
                return;
            }
            bool found = false;
            // 9 = the nine neighbour directions of a place
            for (int direction = 1; direction <= 9 && !found; ++direction) {
                const int near = neighbour(to, kRows, direction);
                if (near != 0 && near != to && m_grid.fitCount(near, item.itemId, count, from) == count) {
                    to = near;
                    found = true;
                }
            }
            if (!found) {
                return;
            }
        }
        m_services.connection.send(protocol::storeroomMoveRequest(from, to, count, m_piece));
    }

    void StoreroomWindow::requestTake(int cell, int bagSlot, int count, bool exact) {
        const protocol::ItemInstance& item = m_grid.bagItem(cell);
        if (cell < 1 || cell > kCells || item.empty() || bagSlot < 1 || bagSlot > Inventory::kBagSlots) {
            return;
        }
        if (m_bag.fitCount(bagSlot, item.itemId, count, 0) != count) {
            if (!exact) {
                return;
            }
            // 9 = the nine neighbour directions of a place
            for (int direction = 1; direction <= 9; ++direction) {
                const int near = neighbour(bagSlot, kBagRows, direction);
                if (near != 0 && near != bagSlot && m_bag.fitCount(near, item.itemId, count, 0) == count) {
                    bagSlot = near;
                    break;
                }
            }
        }
        m_services.connection.send(protocol::storeroomTakeRequest(cell, bagSlot, count, m_piece));
    }

    void StoreroomWindow::gridItemDropped(const ui::EventArgs& args) {
        const int source = args.index;
        if (source < 1 || source > kCells || m_grid.bagItem(source).empty()) {
            return;
        }
        const int itemId = m_grid.bagItem(source).itemId;
        const int target = args.targetIndex;
        const int grabCells = args.grab.y / m_cells.pitch().y * kColumns + args.grab.x / m_cells.pitch().x;
        if (args.target == "ItemBag") {
            if (pieceGone() || target < 1 || target > Inventory::kBagSlots) {
                return;
            }
            if (args.ctrl) {
                const int left = m_bag.bagItem(target).count;
                const int count = m_grid.bagItem(source).count + left > kMostStack ? kMostStack - left : m_grid.bagItem(source).count;
                m_countBox.ask(count, [this, source, target](int asked) {
                    if (asked > 0) {
                        requestTake(source, target, asked, false);
                    }
                    return true;
                });
            } else if (target > grabCells) {
                requestTake(source, target - grabCells, 1, true);
            }
        } else if (args.target == "StoreroomCells") {
            if (pieceGone() || target < 1 || target > kCells) {
                return;
            }
            int destination = target;
            if (source == destination) {
                const core::Point origin = m_window.bounds().position() + m_cells.bounds().position() + m_cells.cellPosition(source);
                const int mouseColumn = std::abs(origin.x - args.position.x) / m_cells.pitch().x;
                const int mouseRow = std::abs(origin.y - args.position.y) / m_cells.pitch().y;
                destination = source + mouseRow * kColumns + mouseColumn;
            }
            if (destination <= grabCells) {
                return;
            }
            destination -= grabCells;
            if (destination < 1 || destination > kCells || destination == source) {
                return;
            }
            if (args.ctrl) {
                const int left = m_grid.bagItem(destination).count;
                const int count = left + m_grid.bagItem(source).count > kMostStack ? kMostStack - left : m_grid.bagItem(source).count;
                m_countBox.ask(count, [this, source, destination](int asked) {
                    if (asked > 0) {
                        requestMove(source, destination, asked, false);
                    }
                    return true;
                });
                return;
            }
            requestMove(source, destination, 1, true);
        } else if (args.target.empty() || args.target == "WorldView") {
            if (m_hooks.dropOnWorld) {
                m_hooks.dropOnWorld(source, itemId, 1, m_piece);
            }
        }
    }

    void StoreroomWindow::bagItemDropped(const ui::EventArgs& args) {
        const int source = args.index;
        const int target = args.targetIndex;
        if (source < 1 || source > Inventory::kBagSlots || m_bag.bagItem(source).empty() || target < 1 || target > kCells) {
            return;
        }
        if (pieceGone()) {
            return;
        }
        if (m_marks.isPriced(source) || m_marks.isInFurnitureStall(source) || m_marks.useMark(source) != 0 || m_marks.busyKind(source) != 0 ||
            m_marks.vehicleState(source) != 0) {
            return;
        }
        const int itemId = m_bag.bagItem(source).itemId;
        const auto record = m_services.items.find(itemId);
        if (record && record->subType == kTentSubType) {
            m_messages.show("The tent cannot be placed in warehouse", kNoticeMs);
            return;
        }
        if (itemId == BoxItems::kSuperHandbag || itemId == kUnstorable) {
            m_messages.show((record ? record->name : std::string()) + " Cannot put it into warehouse", kNoticeMs);
            return;
        }
        const int grabCells = args.grab.y / m_cells.pitch().y * kColumns + args.grab.x / m_cells.pitch().x;
        if (target <= grabCells) {
            return;
        }
        const int destination = target - grabCells;
        if (args.ctrl) {
            const int left = m_grid.bagItem(destination).count;
            const int count = left + m_bag.bagItem(source).count > kMostStack ? kMostStack - left : m_bag.bagItem(source).count;
            m_countBox.ask(count, [this, source, destination](int asked) {
                if (asked > 0) {
                    requestPut(source, destination, asked, false);
                }
                return true;
            });
            return;
        }
        requestPut(source, destination, 1, true);
    }

    void StoreroomWindow::updateSort(std::uint64_t nowMs) {
        m_screen.get<ui::Element>("StoreroomSort").setVisible(m_services.videoSettings().sortButtons);
        if (!isOpen()) {
            m_sorter.stop();
            return;
        }
        if (!m_sorter.isRunning()) {
            return;
        }
        if (const auto said = m_sorter.step(sortCellsOf(m_grid, m_services.items), nowMs, [this](const SortMove& move) {
                m_services.connection.send(protocol::storeroomMoveRequest(move.from, move.to, move.count, m_piece));
            })) {
            m_messages.show(*said);
        }
    }

}
