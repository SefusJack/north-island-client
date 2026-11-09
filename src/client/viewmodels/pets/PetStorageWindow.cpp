#include "client/viewmodels/pets/PetStorageWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/ShopRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kLongName = 10; // names longer than this many characters are drawn higher
        constexpr int kLongNameRise = 11; // px a long name is lifted
        constexpr int kPreviewSlot = 11; // NPC action slot of the preview pose

        /// @brief Loads PetStorageWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The PetStorage window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PetStorageWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PetStorage");
        }

        /// @brief Makes a list row with nothing of its own, under its small picture.
        /// @details Original AddImgBlockItem(' ', '', '0').
        /// @return The blank row.
        ui::BlockList::Row blankRow() { return {}; }

    }

    PetStorageWindow::PetStorageWindow(
        GameServices& services,
        ui::Screen& screen,
        PetTeam& pets,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_pets(pets),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_restrain(screen.get<ui::Element>("PetStorageRestrain")),
          m_storedList(screen.get<ui::BlockList>("PetStorageStored")),
          m_carriedList(screen.get<ui::BlockList>("PetStorageCarried")),
          m_previewSpot(screen.get<ui::Spot>("PetStoragePreview")),
          m_textColorSpot(screen.get<ui::Spot>("PetStorageTextColor")),
          m_carriedInfoSpot(screen.get<ui::Spot>("PetStorageCarriedInfo")),
          m_storedInfoSpot(screen.get<ui::Spot>("PetStorageStoredInfo")),
          m_nameOffsetSpot(screen.get<ui::Spot>("PetStorageNameOffset")),
          m_carriedElementSpot(screen.get<ui::Spot>("PetStorageCarriedElement")),
          m_storedElementSpot(screen.get<ui::Spot>("PetStorageStoredElement")),
          m_carriedNameSpot(screen.get<ui::Spot>("PetStorageCarriedName")),
          m_untradableNameSpot(screen.get<ui::Spot>("PetStorageUntradableName")),
          m_storedNameSpot(screen.get<ui::Spot>("PetStorageStoredName")),
          // PetInfoPainter style numbers: {150, 2} and {6, 2} are layout sizes/offsets of the info block in pixels
          m_info(services, PetInfoPainter::Style{{150, 2}, true, false, {6, 2}}) {
        screen.get<ui::DrawingArea>("PetStoragePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintForm(canvas, at, nowMs);
        });
        screen.get<ui::DrawingArea>("PetStorageInfos").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintInfos(canvas, at, nowMs);
        });
        screen.get<ui::DrawingArea>("PetStorageNames").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintNames(canvas, at); });
        screen.on("onPetStorageCarriedClick", [this](const ui::EventArgs& args) { clickCarried(args.index); });
        screen.on("onPetStorageStoredClick", [this](const ui::EventArgs& args) { clickStored(args.index); });
        screen.on("onPetStorageStore", [this](const ui::EventArgs&) { store(); });
        screen.on("onPetStorageTake", [this](const ui::EventArgs&) { take(); });
        screen.on("onPetStorageClose", [this](const ui::EventArgs&) { close(); });
    }

    void PetStorageWindow::replaceWornIds(int first, int last, int to) {
        for (auto& pet : m_stored) {
            if (!pet) {
                continue;
            }
            for (auto& item : pet->worn) {
                if (item.itemId >= first && item.itemId <= last) {
                    item.itemId = to;
                }
            }
        }
    }

    void PetStorageWindow::show(std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void PetStorageWindow::close() {
        m_window.setVisible(false);
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    void PetStorageWindow::setStored(const std::vector<protocol::PetRecord>& records) {
        for (const protocol::PetRecord& record : records) {
            if (record.slot >= 1 && record.slot <= kPlaces) {
                m_stored[static_cast<std::size_t>(record.slot)] = m_pets.fromRecord(record);
            }
        }
        refresh();
    }

    void PetStorageWindow::stored(int place, int petSlot) {
        if (place >= 1 && place <= kPlaces && m_pets.at(petSlot)) {
            m_stored[static_cast<std::size_t>(place)] = m_pets.at(petSlot);
            refresh();
        }
    }

    void PetStorageWindow::taken(int place, int petSlot) {
        if (place < 1 || place > kPlaces || petSlot < 1 || petSlot > PetTeam::kPlaces) {
            return;
        }
        const std::optional<Pet>& from = m_asRequested[static_cast<std::size_t>(place)];
        std::optional<Pet>& to = m_pets.at(petSlot);
        if (!from || !to) {
            return;
        }
        const int order = to->order;
        *to = *from;
        to->order = order;
        m_pets.recalc(*to);
        refresh();
    }

    void PetStorageWindow::swapped(int petSlot, int place) {
        if (place < 1 || place > kPlaces || petSlot < 1 || petSlot > PetTeam::kPlaces) {
            return;
        }
        std::optional<Pet>& carried = m_pets.at(petSlot);
        std::optional<Pet>& stored = m_stored[static_cast<std::size_t>(place)];
        if (!carried || !stored) {
            return;
        }
        const int order = carried->order;
        std::swap(*carried, *stored);
        carried->order = order;
        m_pets.recalc(*carried);
        refresh();
    }

    void PetStorageWindow::cleared(int place) {
        if (place >= 1 && place <= kPlaces) {
            m_stored[static_cast<std::size_t>(place)].reset();
            refresh();
        }
    }

    int PetStorageWindow::wornBy(int place, int slot) const {
        if (place < 1 || place > kPlaces || slot < 1 || slot > 6 || !m_stored[static_cast<std::size_t>(place)]) { // a pet wears 6 slots
            return 0;
        }
        return m_stored[static_cast<std::size_t>(place)]->worn[static_cast<std::size_t>(slot)].itemId;
    }

    std::string PetStorageWindow::nameOf(int place) const {
        return place >= 1 && place <= kPlaces && m_stored[static_cast<std::size_t>(place)] ? m_stored[static_cast<std::size_t>(place)]->name : std::string();
    }

    void PetStorageWindow::replaceWorn(int place, int slot, int itemId) {
        if (place >= 1 && place <= kPlaces && slot >= 1 && slot <= 6 && m_stored[static_cast<std::size_t>(place)]) { // a pet wears 6 slots
            m_stored[static_cast<std::size_t>(place)]->worn[static_cast<std::size_t>(slot)] = {};
            m_stored[static_cast<std::size_t>(place)]->worn[static_cast<std::size_t>(slot)].itemId = itemId;
        }
    }

    void PetStorageWindow::done() {
        m_waiting = false;
        refresh();
    }

    void PetStorageWindow::refresh() {
        m_carriedList.clear();
        for (int order = 1; order <= kCarried; ++order) {
            if (m_pets.placeByOrder(order) != 0) {
                m_carriedList.addRow(blankRow());
            }
        }
        m_storedList.clear();
        for (int row = 0; row < storedCount(); ++row) {
            m_storedList.addRow(blankRow());
        }
    }

    int PetStorageWindow::storedCount() const {
        int count = 0;
        for (int place = 1; place <= kPlaces; ++place) {
            count += m_stored[static_cast<std::size_t>(place)] ? 1 : 0;
        }
        return count;
    }

    int PetStorageWindow::storedPlace(int row) const {
        int seen = 0;
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_stored[static_cast<std::size_t>(place)] && ++seen == row) {
                return place;
            }
        }
        return 0;
    }

    void PetStorageWindow::clickCarried(int row) {
        if (row < 0 || row >= m_carriedList.rowCount()) {
            return;
        }
        const std::optional<Pet>& pet = m_pets.at(m_pets.placeByOrder(row + 1));
        if (!pet) {
            return;
        }
        if (m_pets.ridingPlace() == m_pets.placeByOrder(row + 1)) {
            m_messages.show("Riding pet cannot be stored", 2000); // message display time in ms
            m_carriedList.row(row).checked = false;
            return;
        }
        if (pet->tradable != 1) {
            m_messages.show("This NPC cannot be stored.", 2000); // message display time in ms
            m_carriedList.row(row).checked = false;
            return;
        }
        if (pet->info.mercenary != 0) {
            m_messages.show("Soldiers can't be stored", 2000); // message display time in ms
            m_carriedList.row(row).checked = false;
            return;
        }
        m_carriedList.row(row).checked = !m_carriedList.row(row).checked;
    }

    void PetStorageWindow::clickStored(int row) {
        if (row >= 0 && row < m_storedList.rowCount()) {
            m_storedList.row(row).checked = !m_storedList.row(row).checked;
        }
    }

    std::optional<std::vector<int>> PetStorageWindow::checkedCarried() {
        std::vector<int> slots;
        for (int row = 0; row < m_carriedList.rowCount(); ++row) {
            if (!m_carriedList.row(row).checked) {
                continue;
            }
            const int slot = m_pets.placeByOrder(row + 1);
            const std::optional<Pet>& pet = m_pets.at(slot);
            if (slot == 0 || !pet) {
                return std::nullopt;
            }
            if (m_pets.ridingPlace() == slot) {
                m_messages.show("Riding pet cannot be stored", 2000); // message display time in ms
                return std::nullopt;
            }
            if (pet->tradable != 1) {
                m_messages.show("This NPC cannot be stored.", 2000); // message display time in ms
                return std::nullopt;
            }
            if (pet->info.mercenary != 0) {
                m_messages.show("Soldiers can't be stored", 2000); // message display time in ms
                return std::nullopt;
            }
            slots.push_back(slot);
        }
        return slots;
    }

    std::vector<int> PetStorageWindow::checkedStored() const {
        std::vector<int> places;
        for (int row = 0; row < m_storedList.rowCount(); ++row) {
            if (m_storedList.row(row).checked) {
                places.push_back(storedPlace(row + 1));
            }
        }
        return places;
    }

    void PetStorageWindow::send(const core::Bytes& request) {
        m_asRequested = m_stored;
        m_services.connection.send(request);
        m_waiting = true;
    }

    void PetStorageWindow::store() {
        if (m_waiting) {
            return;
        }
        const auto slots = checkedCarried();
        if (!slots || slots->empty()) {
            return;
        }
        const std::vector<int> places = checkedStored();
        const int storing = static_cast<int>(slots->size());
        const int taking = static_cast<int>(places.size());
        if (storing > taking && kPlaces - storedCount() < storing - taking) {
            m_messages.show("Your store space is not enough", 1500); // message display time in ms
            return;
        }
        if (taking > storing && kCarried - m_carriedList.rowCount() < taking - storing) {
            m_messages.show("Your space is not enough", 1500); // message display time in ms
            return;
        }
        send(taking == 0 ? protocol::petStorageStoreRequest(*slots) : protocol::petStorageSwapRequest(*slots, places));
    }

    void PetStorageWindow::take() {
        if (m_waiting) {
            return;
        }
        const std::vector<int> places = checkedStored();
        if (places.empty()) {
            return;
        }
        const auto slots = checkedCarried();
        if (!slots) {
            return;
        }
        const int storing = static_cast<int>(slots->size());
        const int taking = static_cast<int>(places.size());
        if (storing > taking && kPlaces - storedCount() < storing - taking) {
            m_messages.show("Your store space is not enough", 1500); // message display time in ms
            return;
        }
        if (taking > storing && kCarried - m_carriedList.rowCount() < taking - storing) {
            m_messages.show("Your space is not enough", 1500); // message display time in ms
            return;
        }
        send(storing == 0 ? protocol::petStorageTakeRequest(places) : protocol::petStorageSwapRequest(*slots, places));
    }

    void PetStorageWindow::paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (const auto friendPicture = m_services.pictures.find("Bmp_Friend_1")) {
            canvas.drawPicture(*friendPicture, {at.x + 253, at.y + 408}); // pixel position of the friend picture in the form
        }
        if (const auto icon = m_services.pictures.find("Icon_Inn_1")) {
            const int width = m_services.pictures.picture(*icon).width();
            canvas.drawPicture(*icon, {at.x + (m_window.bounds().width - width) / 2, at.y + 11}); // top margin of the inn icon in pixels
        }
        const core::Point mouse = m_screen.mouse();
        const auto rowUnder = [&](const ui::BlockList& list) {
            const core::Point local = mouse - at - list.bounds().position();
            return local.x >= 0 && local.x < list.bounds().width ? list.rowAt(local) : -1;
        };
        std::optional<Pet> shown;
        const int carriedRow = rowUnder(m_carriedList);
        const int storedRow = rowUnder(m_storedList);
        if (carriedRow >= 0) {
            shown = m_pets.at(m_pets.placeByOrder(carriedRow + 1));
        } else if (storedRow >= 0) {
            if (const int place = storedPlace(storedRow + 1)) {
                shown = m_stored[static_cast<std::size_t>(place)];
            }
        }
        if (!shown) {
            return;
        }
        const int slot = m_services.characters.npcSlot(shown->info.spriteId, shown->info.directionKind, kPreviewSlot);
        const core::Size size = m_services.characters.npcFrameSize(shown->info.spriteId, slot);
        const int rise = shown->info.spriteSize == 0 ? 16 : 50; // pixels the figure is lifted by: small sprite 16, large sprite 50
        const int frame = m_animation.frame(nowMs, m_services.characters.npcFrameCount(shown->info.spriteId, slot));
        m_services.characters.drawNpc(canvas, shown->info, {at.x + m_previewSpot.at().x, at.y + m_previewSpot.at().y + size.height / 2 - rise}, slot, frame);
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto line = [&](core::Point where, const std::string& text) {
            canvas.drawText({at.x + where.x, at.y + where.y}, text, style, static_cast<int>(text.size()) * 8, 12); // 8 pixels per character, line height 12
        };
        // pixel positions of the name, level, HP and amity lines in the form
        line({253, 355}, shown->name);
        line({273, 373}, std::to_string(shown->level));
        line({273, 390}, std::to_string(shown->hp) + "/" + std::to_string(shown->maxHp));
        line({298, 407}, std::to_string(shown->amity));
    }

    void PetStorageWindow::paintInfos(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        for (int order = 1; order <= kCarried; ++order) {
            const std::optional<Pet>& pet = m_pets.at(m_pets.placeByOrder(order));
            if (!pet) {
                break;
            }
            m_info.paint(canvas, {at.x + m_carriedInfoSpot.at().x, at.y + m_carriedInfoSpot.at().y + (order - 1) * m_carriedInfoSpot.pitch().y}, *pet, nowMs);
        }
        for (int row = 1; row <= storedCount(); ++row) {
            const std::optional<Pet>& pet = m_stored[static_cast<std::size_t>(storedPlace(row))];
            m_info.paint(canvas, {at.x + m_storedInfoSpot.at().x, at.y + m_storedInfoSpot.at().y + (row - 1) * m_carriedInfoSpot.pitch().y}, *pet, nowMs);
        }
    }

    void PetStorageWindow::paintNames(gfx::Canvas& canvas, core::Point at) {
        const auto name = [&](core::Point info, const std::string& text, core::Color color) {
            const int rise = static_cast<int>(text.size()) > kLongName ? kLongNameRise : 0;
            canvas.drawText(
                {info.x + m_nameOffsetSpot.at().x, info.y + m_nameOffsetSpot.at().y - rise},
                text,
                {color, gfx::TextEffect::Plain, std::nullopt},
                80,
                16
            ); // name text width and line height in pixels
        };
        const auto element = [&](core::Point where, int value) {
            if (const auto icon = m_services.pictures.find("Icon_Element_" + std::to_string(value) + "_1")) {
                canvas.drawPicture(*icon, where);
            }
        };
        for (int order = 1; order <= kCarried; ++order) {
            const std::optional<Pet>& pet = m_pets.at(m_pets.placeByOrder(order));
            if (!pet) {
                break;
            }
            const core::Point info{at.x + m_carriedInfoSpot.at().x, at.y + m_carriedInfoSpot.at().y + (order - 1) * m_carriedInfoSpot.pitch().y};
            name(info, pet->name, pet->tradable != 1 ? m_untradableNameSpot.color() : m_carriedNameSpot.color()); // tradable == 1: the pet can be traded
            element({at.x + m_carriedElementSpot.at().x, at.y + m_carriedElementSpot.at().y + (order - 1) * m_carriedInfoSpot.pitch().y}, pet->info.element);
        }
        for (int row = 1; row <= storedCount(); ++row) {
            const std::optional<Pet>& pet = m_stored[static_cast<std::size_t>(storedPlace(row))];
            const core::Point info{
                at.x + m_storedInfoSpot.at().x - 2,
                at.y + m_storedInfoSpot.at().y + (row - 1) * m_carriedInfoSpot.pitch().y
            }; // stored names sit 2 pixels left of the info block
            name(info, pet->name, m_storedNameSpot.color());
            element({at.x + m_storedElementSpot.at().x, at.y + m_storedElementSpot.at().y + (row - 1) * m_carriedInfoSpot.pitch().y}, pet->info.element);
        }
    }

}
