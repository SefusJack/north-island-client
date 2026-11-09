#include "client/viewmodels/house/ComputerWindow.h"

#include "client/rules/FurnitureKinds.h"
#include "gfx/TextStyle.h"
#include "protocol/ManufactureRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kBarHeight = 3; // progress bar fill height, px
        constexpr int kNameCharWidth = 8; // width of one name character, px

        /// @brief Loads ComputerWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded ComputerWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ComputerWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ComputerWindow");
        }

    }

    ComputerWindow::ComputerWindow(
        GameServices& services,
        ui::Screen& screen,
        MakeJobs& jobs,
        const HomeFurniture& furniture,
        const MakeMaterials& materials,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_jobs(jobs),
          m_furniture(furniture),
          m_materials(materials),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_editors{
              {screen.get<ui::TextBox>("ComputerEdit1"),
               screen.get<ui::TextBox>("ComputerEdit2"),
               screen.get<ui::TextBox>("ComputerEdit3"),
               screen.get<ui::TextBox>("ComputerEdit4"),
               screen.get<ui::TextBox>("ComputerEdit5")}
          },
          m_nameSpot(screen.get<ui::Spot>("ComputerName")),
          m_frameSpot(screen.get<ui::Spot>("ComputerFrame")),
          m_barSpot(screen.get<ui::Spot>("ComputerBar")) {
        screen.get<ui::DrawingArea>("ComputerPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onComputerOk", [this](const ui::EventArgs&) { start(); });
        screen.on("onComputerStop", [this](const ui::EventArgs&) { clear(); });
        screen.on("onComputerClose", [this](const ui::EventArgs&) { close(); });
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            setEditor(slot, "0");
        }
    }

    void ComputerWindow::setEditor(int slot, const std::string& text) {
        ui::TextBox& editor = m_editors[static_cast<std::size_t>(slot - 1)];
        if (editor.text() != text) {
            editor.setText(text);
        }
    }

    int ComputerWindow::order(int slot) const {
        const std::string& text = m_editors[static_cast<std::size_t>(slot - 1)].get().text();
        return text.empty() ? 0 : std::stoi(text);
    }

    void ComputerWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ComputerWindow::close() { m_window.setVisible(false); }

    void ComputerWindow::clear() {
        if (m_state == State::None) {
            return;
        }
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            setEditor(slot, "0");
            m_queue[static_cast<std::size_t>(slot)] = 0;
        }
        m_state = State::None;
    }

    void ComputerWindow::stopped() { clear(); }

    void ComputerWindow::ended(int slot) {
        if (m_state != State::Running || slot < 1 || slot > MakeJobs::kSlots) {
            return;
        }
        setEditor(slot, "0");
        m_queue[static_cast<std::size_t>(slot)] = 0;
        startNext(m_jobs.petSlot(), m_jobs.petId());
    }

    void ComputerWindow::start() {
        if (m_hooks.bathing()) {
            m_messages.show("Bathing, cancel the action", 1000); // message display time in ms
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        if (m_jobs.playerSlot() != 0) {
            m_messages.show("Manufacturing, cannot operate", 1000); // message display time in ms
            return;
        }
        m_state = State::Asked;
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            m_queue[static_cast<std::size_t>(slot)] = order(slot);
        }
        if (m_hooks.hasMaid()) {
            m_hooks.choosePartner([this](int petSlot, int petId) {
                startNext(petSlot, petId);
                m_state = State::Running;
                close();
                return true;
            });
            return;
        }
        close();
        m_state = State::Running;
        startNext(0, 0);
    }

    int ComputerWindow::facilityPiece(int facilityId) const {
        for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
            const FurnitureRecord& record = m_furniture.piece(piece);
            if (record.itemId == facilityId && record.retained != 1) {
                return piece;
            }
        }
        return 0;
    }

    bool ComputerWindow::blueprintFound(int blueprintItemId, int& source, int& slot) const {
        const MakeMaterials::BlueprintAt at = m_materials.blueprintAt(blueprintItemId);
        if (at.source == 0) {
            return false;
        }
        source = at.source == 4 ? 3 : at.source; // the Desk (source 4) counts as source 3, like the Bookcase
        slot = at.slot;
        return true;
    }

    void ComputerWindow::startNext(int petSlot, int petId) {
        if (m_jobs.playerSlot() != 0) {
            return;
        }
        if (m_hooks.inOwnHouse()) {
            for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
                const MakeSlot& job = m_jobs.slot(slot);
                if (job.productId != 0 && facilityPiece(m_services.compounds.at(m_services.compounds.indexOfProduct(job.productId)).facilityId) == 0) {
                    m_queue[static_cast<std::size_t>(slot)] = 0;
                    setEditor(slot, "0");
                }
            }
        }
        int lowest = 999; // larger than any queue place
        int next = 0;
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            const int place = m_queue[static_cast<std::size_t>(slot)];
            if (place != 0 && place < lowest) {
                lowest = place;
                next = slot;
            }
        }
        if (next == 0) {
            m_state = State::None;
            m_hooks.partnerLeft();
            return;
        }
        const formats::CompoundRecipe& recipe = m_services.compounds.at(m_services.compounds.indexOfProduct(m_jobs.slot(next).productId));
        int source = 0;
        int materialSlot = 0;
        if (recipe.blueprintItemId != 0 && !blueprintFound(recipe.blueprintItemId, source, materialSlot)) {
            return;
        }
        if (const int piece = facilityPiece(recipe.facilityId); piece != 0) {
            m_jobs.setPet(petSlot, petId);
            m_hooks.send(protocol::continueMakingRequest(piece, next, source, materialSlot, petSlot, petId)); // 64/2: continue making
        }
    }

    void ComputerWindow::checkEditors() {
        int under = 0;
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            under += m_jobs.slot(slot).productId != 0 ? 1 : 0;
        }
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            const MakeSlot& job = m_jobs.slot(slot);
            if (job.productId == 0) {
                setEditor(slot, "0");
                continue;
            }
            if (m_state != State::Running && !m_editors[static_cast<std::size_t>(slot - 1)].get().text().empty() && order(slot) > under) {
                setEditor(slot, std::to_string(under));
            }
            const formats::CompoundRecipe& recipe = m_services.compounds.at(m_services.compounds.indexOfProduct(job.productId));
            int source = 0;
            int materialSlot = 0;
            if (recipe.blueprintItemId != 0 && !blueprintFound(recipe.blueprintItemId, source, materialSlot)) {
                setEditor(slot, "0");
            }
            bool found = false;
            for (int piece = 1; piece <= HomeFurniture::kMost && !found; ++piece) {
                found = m_furniture.piece(piece).itemId == recipe.facilityId;
            }
            if (!found) {
                setEditor(slot, "0");
            }
        }
    }

    void ComputerWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        checkEditors();
    }

    void ComputerWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const auto frame = m_services.pictures.find("Panel_makeThingTimeForm_1");
        const auto bar = m_services.pictures.find("Panel_makeThingTime_1");
        const gfx::TextStyle style{m_nameSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            const MakeSlot& job = m_jobs.slot(slot);
            if (job.productId == 0) {
                continue;
            }
            const auto product = m_services.items.find(job.productId);
            const std::string name = product ? product->name : std::string();
            const int row = (slot - 1) * m_nameSpot.pitch().y;
            canvas.drawText(
                {at.x + m_nameSpot.at().x, at.y + m_nameSpot.at().y + row},
                name,
                style,
                static_cast<int>(name.size()) * kNameCharWidth,
                0
            ); // text box height 0 (single line)
            if (frame) {
                canvas.drawPicture(*frame, {at.x + m_frameSpot.at().x, at.y + m_frameSpot.at().y + row});
            }
            int minutes = m_services.compounds.at(m_services.compounds.indexOfProduct(job.productId)).makeMinutes * job.batches;
            if (const int flags = m_hooks.furnitureFlags(); flags != 0) {
                // furniture flags: bit 1 saves 15 percent, bit 2 saves 2, bit 8 saves 3, bit 4 saves 3; percent of the time left
                const int percent = 100 - (flags & 1) * 15 - ((flags & 2) / 2) * 2 - ((flags & 8) / 8) * 3 - ((flags & 4) / 4) * 3;
                minutes = percent * minutes < 100 ? 1 : percent * minutes / 100; // 100 = percent base
            }
            minutes = std::max(minutes, 1);
            // 1000 ms per second, 60 seconds per minute
            const std::int64_t worked = job.active ? m_jobs.workedSeconds(m_hooks.nowMs()) : static_cast<std::int64_t>(job.elapsedMs) / 1000;
            const int length = minutes * 60 >= worked ? static_cast<int>(MakeJobs::kBarLength * worked / (minutes * 60)) : MakeJobs::kBarLength;
            if (bar && length > 0) {
                canvas.drawPicture(*bar, {at.x + m_barSpot.at().x, at.y + m_barSpot.at().y + row}, core::Rect{0, 0, length, kBarHeight});
            }
        }
    }

}
