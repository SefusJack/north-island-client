#include "client/viewmodels/travel/TransportWindow.h"

#include "client/data/TransportDestinations.h"
#include "client/data/VehicleKinds.h"
#include "client/rules/VehiclePlacing.h"

namespace client {

    namespace {

        constexpr int kSeaKind = 2; // teleport kind 2: submarine (kind 3 is the landing craft)
        constexpr int kFerryScene = 14; // scene type 14 (original SceneType = 14): submarine entry list differs here, see pick()
        constexpr int kFuelFloor = 500; // "Not enough fuel" unless fuel + wear left exceeds this

        /// @brief Loads TransportWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "TransportWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("TransportWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("TransportWindow");
        }

    }

    TransportWindow::TransportWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_places(screen.get<ui::ListBox>("TransportPlaces")),
          m_pictureSpot(screen.get<ui::Spot>("TransportPicture")),
          m_submarine(services.pictures.find("Icon_Submarine").value_or(gfx::PictureId{})),
          m_landingCraft(services.pictures.find("Icon_LandingCraft").value_or(gfx::PictureId{})) {
        screen.get<ui::DrawingArea>("TransportPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onTransportClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onTransportUp", [this](const ui::EventArgs&) { m_places.scrollUp(); });
        screen.on("onTransportDown", [this](const ui::EventArgs&) { m_places.scrollDown(); });
        screen.on("onTransportPick", [this](const ui::EventArgs& args) { pick(args.index); });
    }

    void TransportWindow::open(int kind) {
        m_kind = kind;
        const std::vector<std::string>& names = TransportDestinations::namesOf(kind);
        if (names.empty()) {
            return;
        }
        m_places.clear();
        for (const std::string& name : names) {
            m_places.addItem(name);
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void TransportWindow::pick(int index) {
        if (VehicleKinds::teleportKind(m_hooks.vehicleItem()) != m_kind) {
            return;
        }
        const int place = index + 1;
        const auto bagItem = m_hooks.vehicleBagItem();
        if (bagItem) {
            const auto record = m_services.items.find(bagItem->itemId);
            const int cost = record ? vehicleWearLimit(*record) - bagItem->usedAmount : 0;
            if (bagItem->extraAmount + cost <= kFuelFloor) {
                m_messages.show("Not enough fuel", 2000); // message display time in ms
            } else {
                if (m_kind == kSeaKind) {
                    const bool onFerry = m_hooks.sceneType() == kFerryScene;
                    // place 1 is the surfacing entry of the submarine list
                    if ((!onFerry && place == 1) || (onFerry && place != 1)) {
                        return;
                    }
                }
                m_hooks.setOff(place);
            }
        }
        close();
    }

    void TransportWindow::paint(gfx::Canvas& canvas, core::Point at) {
        // kind 3: landing craft
        const gfx::PictureId picture = m_kind == kSeaKind ? m_submarine : m_kind == 3 ? m_landingCraft : gfx::PictureId{};
        if (picture.isValid()) {
            canvas.drawPicture(picture, {at.x + m_pictureSpot.at().x, at.y + m_pictureSpot.at().y});
        }
    }

}
