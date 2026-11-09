#include "client/viewmodels/travel/MagicMapWindow.h"

#include "client/data/MagicMapPlaces.h"
#include "protocol/MagicMapRequests.h"

namespace client {

    namespace {

        constexpr int kMessageMs = 2000; // message display time, ms
        constexpr int kBagSlots = 50; // number of bag slots (slots are numbered 1..50)

        /// @brief Loads MagicMapWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "MagicMapWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MagicMapWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MagicMapWindow");
        }

    }

    MagicMapWindow::MagicMapWindow(
        GameServices& services,
        ui::Screen& screen,
        BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)) {
        screen.on("onMagicMapClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMagicMapPlace", [this](const ui::EventArgs& args) { placeClicked(std::stoi(args.source.substr(std::string("MagicMapPlace").size()))); });
    }

    bool MagicMapWindow::use(int bagSlot, int mode) {
        const auto refuse = [this](const std::string& text) {
            m_messages.show(text, kMessageMs);
            return false;
        };
        if (bagSlot < 1 || bagSlot > kBagSlots) {
            return false;
        }
        if (m_hooks.inTeam()) {
            return refuse("Can't be used when you are in a team");
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 1)) {
            m_messages.show(notice->text, notice->ms);
            return false;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            return refuse("Unable to operate when the item is locked");
        }
        if (const std::string text = m_hooks.refusal(); !text.empty()) {
            return refuse(text);
        }
        if (m_hooks.inHouse() || m_hooks.cannotLeaveScene()) {
            return refuse("The scene can't be used");
        }
        m_itemSlot = bagSlot;
        m_mode = mode;
        m_marks.setUseMark(bagSlot, BagMarks::kMagicMap);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        return true;
    }

    void MagicMapWindow::close() {
        m_window.setVisible(false);
        if (m_itemSlot != 0) {
            m_marks.setUseMark(m_itemSlot, 0);
        }
        m_itemSlot = 0;
    }

    void MagicMapWindow::release(int bagSlot) {
        if (bagSlot >= 1 && bagSlot <= kBagSlots && m_marks.useMark(bagSlot) == BagMarks::kMagicMap) {
            m_marks.setUseMark(bagSlot, 0);
        }
    }

    void MagicMapWindow::placeClicked(int place) {
        m_targetPlace = place;
        if (place == 0) {
            return;
        }
        const auto refuse = [this](const std::string& text) { m_messages.show(text, kMessageMs); };
        if (m_hooks.inTeam()) {
            refuse("Can't be used when you are in a team");
            return;
        }
        if (const std::string text = m_hooks.refusal(); !text.empty()) {
            refuse(text);
            return;
        }
        if (m_hooks.inHouse()) {
            refuse("The scene can't be used");
            return;
        }
        if (MagicMapPlaces::mapId(place) == m_hooks.mapId()) {
            refuse("Can't be used in the same scene");
            return;
        }
        m_hooks.ask("Teleport to #R#B" + MagicMapPlaces::name(place) + "/#R/#B sure?", [this] { confirm(); });
    }

    void MagicMapWindow::confirm() {
        m_hooks.send(protocol::magicMapTeleportRequest(m_targetPlace, m_itemSlot, m_mode));
        m_targetPlace = 0;
        close();
    }

}
