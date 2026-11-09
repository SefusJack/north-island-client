#include "client/viewmodels/house/BedWindow.h"

#include "client/rules/FurnitureKinds.h"
#include "gfx/TextStyle.h"
#include "protocol/FurnitureBedMessages.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kCarriedRows = 4; // rows in the carried-pets list
        constexpr int kBeds = 4; // beds in the room
        constexpr int kLongName = 10; // names longer than this many characters are drawn higher
        constexpr int kLongNameRise = 11; // how far a long name is raised, px
        constexpr int kSleeperNameShift = 4; // sleeper names are shifted left by this, px
        constexpr int kBedPanelGap = 3; // gap between bed panels, px
        constexpr int kBedPanelHeight = 34; // height of one bed panel, px

        /// @brief Loads BedWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded BedWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BedWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BedWindow");
        }

        /// @brief Makes a list row with nothing of its own, shown under its small picture.
        /// @details Original: AddImgBlockItem(' ', '', '0').
        /// @return An empty block list row.
        ui::BlockList::Row blankRow() { return {}; }

    }

    BedWindow::BedWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const PetTeam& pets,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_pets(pets),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_sleeperList(screen.get<ui::BlockList>("BedSleepers")),
          m_carriedList(screen.get<ui::BlockList>("BedCarried")),
          m_carriedInfoSpot(screen.get<ui::Spot>("BedCarriedInfo")),
          m_sleeperInfoSpot(screen.get<ui::Spot>("BedSleeperInfo")),
          m_carriedElementSpot(screen.get<ui::Spot>("BedCarriedElement")),
          m_sleeperElementSpot(screen.get<ui::Spot>("BedSleeperElement")),
          m_nameOffsetSpot(screen.get<ui::Spot>("BedNameOffset")),
          m_untradableSpot(screen.get<ui::Spot>("BedUntradableName")),
          m_panelSpot(screen.get<ui::Spot>("BedPanel")),
          // pet info painter style: {150, 2} and {10, 2} / {6, 2} are layout offsets in pixels (right list / left list)
          m_carriedInfo(services, PetInfoPainter::Style{{150, 2}, true, false, {10, 2}}),
          m_sleeperInfo(services, PetInfoPainter::Style{{150, 2}, true, false, {6, 2}}) {
        screen.get<ui::DrawingArea>("BedPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintForm(canvas, at); });
        screen.get<ui::DrawingArea>("BedInfos").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paintInfos(canvas, at, nowMs); });
        screen.get<ui::DrawingArea>("BedNames").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintNames(canvas, at); });
        screen.on("onBedClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onBedStore", [this](const ui::EventArgs&) { store(); });
        screen.on("onBedRetrieve", [this](const ui::EventArgs&) { retrieve(); });
        screen.on("onBedCarriedClick", [this](const ui::EventArgs& args) { clickCarried(args.index); });
        screen.on("onBedSleeperClick", [this](const ui::EventArgs& args) { clickSleeper(args.index); });
    }

    void BedWindow::open(int furnIndex) {
        const int slot = m_house.utensils().slotOfPiece(furnIndex);
        const int kind = FurnitureKinds::bedKind(m_services.items, m_house.utensils().utensil(slot).itemId);
        if (kind < 1 || kind > kBeds) {
            return;
        }
        m_furnIndex = furnIndex;
        m_bedKind = kind;
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    const std::optional<Pet>& BedWindow::carried(int row) const { return m_pets.at(m_pets.placeByOrder(row + 1)); }

    std::optional<Pet> BedWindow::sleeper(int row) const {
        const int bed = m_house.beds().bedOfSleeper(m_furnIndex, row + 1);
        if (bed == 0) {
            return std::nullopt;
        }
        if (const auto record = m_house.beds().petIn(m_furnIndex, bed)) {
            return m_pets.fromRecord(record->get());
        }
        return std::nullopt;
    }

    void BedWindow::refresh() {
        m_carriedList.clear();
        for (int order = 1; order <= kCarriedRows; ++order) {
            if (m_pets.placeByOrder(order) != 0) {
                m_carriedList.addRow(blankRow());
            }
        }
        m_sleeperList.clear();
        for (int bed = 1; bed <= kBeds; ++bed) {
            if (m_house.beds().petIn(m_furnIndex, bed)) {
                m_sleeperList.addRow(blankRow());
            }
        }
    }

    int BedWindow::checkedCount(ui::BlockList& list, int except) const {
        int count = 0;
        for (int row = 0; row < list.rowCount(); ++row) {
            if (row != except && list.row(row).checked) {
                ++count;
            }
        }
        return count;
    }

    void BedWindow::clickSleeper(int row) {
        if (row < 0 || row >= m_sleeperList.rowCount()) {
            return;
        }
        m_sleeperList.row(row).checked = !m_sleeperList.row(row).checked;
        if (m_carriedList.rowCount() + checkedCount(m_sleeperList, row) + 1 > kCarriedRows) {
            m_messages.show("Your space is not enough", 2000); // message display time in ms
            m_sleeperList.row(row).checked = false;
        }
    }

    void BedWindow::clickCarried(int row) {
        if (row < 0 || row >= m_carriedList.rowCount()) {
            return;
        }
        m_carriedList.row(row).checked = !m_carriedList.row(row).checked;
        const int place = m_pets.placeByOrder(row + 1);
        const std::optional<Pet>& pet = m_pets.at(place);
        if (!pet) {
            return;
        }
        const auto refuse = [&](const char* text) {
            m_messages.show(text, 2000); // message display time in ms
            m_carriedList.row(row).checked = false;
        };
        if (m_pets.ridingPlace() == place) {
            refuse("Riding pet cannot be stored");
        } else if (pet->tradable != 1) {
            refuse("This NPC cannot be stored.");
        } else if (pet->info.mercenary != 0) {
            refuse("Soldiers can't be stored");
        } else {
            const int others = checkedCount(m_carriedList, row);
            const int slot = m_house.utensils().slotOfPiece(m_furnIndex);
            int freeBeds = 0;
            for (int bed = 1; bed <= m_bedKind; ++bed) {
                freeBeds += m_house.utensils().utensil(slot).contentSlots[static_cast<std::size_t>(bed)] == 0 ? 1 : 0;
            }
            if (others + 1 > m_bedKind || others + 1 > freeBeds) {
                refuse("Your store space is not enough");
            }
        }
    }

    void BedWindow::store() {
        std::vector<int> places;
        for (int row = 0; row < m_carriedList.rowCount(); ++row) {
            if (!m_carriedList.row(row).checked) {
                continue;
            }
            const int place = m_pets.placeByOrder(row + 1);
            const std::optional<Pet>& pet = m_pets.at(place);
            if (!pet) {
                return;
            }
            if (m_pets.ridingPlace() == place) {
                m_messages.show("Riding pet cannot be stored", 2000); // message display time in ms
                return;
            }
            if (m_hooks.petInStall(place)) {
                m_messages.show("Pet sest a stall\xA1Kcan't operate", 2000); // message display time in ms
                return;
            }
            if (pet->tradable != 1) {
                m_messages.show("This NPC cannot be stored.", 2000); // message display time in ms
                return;
            }
            if (pet->info.mercenary != 0) {
                m_messages.show("Soldiers can't be stored", 2000); // message display time in ms
                return;
            }
            places.push_back(place);
        }
        for (const int place : places) {
            m_hooks.send(protocol::putPetInBedRequest(m_furnIndex, place));
        }
    }

    void BedWindow::retrieve() {
        std::vector<int> beds;
        for (int row = 0; row < m_sleeperList.rowCount(); ++row) {
            if (!m_sleeperList.row(row).checked) {
                continue;
            }
            const int bed = m_house.beds().bedOfSleeper(m_furnIndex, row + 1);
            if (bed == 0) {
                return;
            }
            beds.push_back(bed);
        }
        for (const int bed : beds) {
            m_hooks.send(protocol::takePetFromBedRequest(m_furnIndex, bed));
        }
    }

    void BedWindow::paintForm(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        if (const auto panel = m_services.pictures.find("Pan_addBed_1")) {
            for (int bed = 1; bed < m_bedKind; ++bed) {
                canvas.drawPicture(*panel, {at.x + m_panelSpot.at().x, at.y + m_panelSpot.at().y + (kBedPanelGap + kBedPanelHeight) * (bed - 1)});
            }
        }
    }

    void BedWindow::paintInfos(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (!m_window.isVisible()) {
            return;
        }
        for (int row = 0; row < m_carriedList.rowCount(); ++row) {
            if (const std::optional<Pet>& pet = carried(row)) {
                m_carriedInfo
                    .paint(canvas, {at.x + m_carriedInfoSpot.at().x, at.y + m_carriedInfoSpot.at().y + row * m_carriedInfoSpot.pitch().y}, *pet, nowMs);
            }
        }
        for (int row = 0; row < m_sleeperList.rowCount(); ++row) {
            if (const std::optional<Pet> pet = sleeper(row)) {
                m_sleeperInfo
                    .paint(canvas, {at.x + m_sleeperInfoSpot.at().x, at.y + m_sleeperInfoSpot.at().y + row * m_sleeperInfoSpot.pitch().y}, *pet, nowMs);
            }
        }
    }

    void BedWindow::paintNames(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        const auto name = [&](core::Point info, int shift, const std::string& text, core::Color color) {
            const int rise = static_cast<int>(text.size()) > kLongName ? kLongNameRise : 0;
            canvas.drawText(
                {info.x + m_nameOffsetSpot.at().x - shift, info.y + m_nameOffsetSpot.at().y - rise},
                text,
                {color, gfx::TextEffect::Plain, std::nullopt},
                80,
                16
            ); // name text box size in pixels: 80 wide, 16 high
        };
        const auto element = [&](core::Point where, int value) {
            if (const auto icon = m_services.pictures.find("Icon_Element_" + std::to_string(value) + "_1")) {
                canvas.drawPicture(*icon, where);
            }
        };
        for (int row = 0; row < m_carriedList.rowCount(); ++row) {
            if (const std::optional<Pet>& pet = carried(row)) {
                name(
                    {at.x + m_carriedInfoSpot.at().x, at.y + m_carriedInfoSpot.at().y + row * m_carriedInfoSpot.pitch().y},
                    0,
                    pet->name,
                    pet->tradable != 1 ? m_untradableSpot.color() : m_carriedInfoSpot.color()
                );
                element({at.x + m_carriedElementSpot.at().x, at.y + m_carriedElementSpot.at().y + row * m_carriedInfoSpot.pitch().y}, pet->info.element);
            }
        }
        for (int row = 0; row < m_sleeperList.rowCount(); ++row) {
            if (const std::optional<Pet> pet = sleeper(row)) {
                name(
                    {at.x + m_sleeperInfoSpot.at().x, at.y + m_sleeperInfoSpot.at().y + row * m_sleeperInfoSpot.pitch().y},
                    kSleeperNameShift,
                    pet->name,
                    m_sleeperInfoSpot.color()
                );
                element({at.x + m_sleeperElementSpot.at().x, at.y + m_sleeperElementSpot.at().y + row * m_sleeperInfoSpot.pitch().y}, pet->info.element);
            }
        }
    }

}
