#include "client/viewmodels/fish/FishTankWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/HouseRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kShortMs = 1000; // notice display time, ms

        /// @brief Loads FishTankWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "FishTank" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("FishTankWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("FishTank");
        }

    }

    FishTankWindow::FishTankWindow(
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
          m_textSpot(screen.get<ui::Spot>("FishTankText")),
          m_iconSpot(screen.get<ui::Spot>("FishTankIcon")),
          m_foodIconSpot(screen.get<ui::Spot>("FishTankFoodIcon")),
          m_foodCountSpot(screen.get<ui::Spot>("FishTankFoodCount")),
          m_coinIconSpot(screen.get<ui::Spot>("FishTankCoinIcon")),
          m_coinCountSpot(screen.get<ui::Spot>("FishTankCoinCount")),
          // tank request codes: 1 for a clicked fish, 6 for the second callback
          m_tank(
              services.pictures,
              {[this](int fish) { requestKind(1, fish); }, [this] { m_services.connection.send(protocol::fishTankKindRequest(6, m_kind)); }}
          ),
          m_coinIcon(services.pictures.find("GdMoney2").value_or(gfx::PictureId{})),
          m_foodIcon(services.pictures.find("2821").value_or(gfx::PictureId{})) { // picture 2821: fish food icon
        const core::Rect area = m_window.bounds();
        m_window.setPosition({(800 - area.width) / 2, (600 - area.height) / 2}); // 800 by 600: screen size
        screen.get<ui::DrawingArea>("FishTankPaint").setBounds({0, 0, area.width, area.height});
        screen.get<ui::DrawingArea>("FishTankPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.get<ui::Element>("FishTankLeave").setPosition({area.width - 17, 5}); // Leave button: 17 px from the right edge, 5 px from the top
        screen.on("onFishTankLeave", [this](const ui::EventArgs&) { hide(); });
        screen.on("onFishTankEscape", [this](const ui::EventArgs&) { hide(); });
        screen.on("onFishTankMouseDown", [this](const ui::EventArgs& args) { mouseDown(args.position); });
        screen.on("onFishTankBuy", [this](const ui::EventArgs&) { requestBuy(); });
        screen.on("onFishTankDiscard", [this](const ui::EventArgs&) { requestDiscard(); });
        screen.on("onFishTankInstructions", [this](const ui::EventArgs&) { m_hooks.instructions(); });
    }

    void FishTankWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void FishTankWindow::showToVisitor() {
        m_tank.clearFood();
        show();
    }

    void FishTankWindow::mouseDown(core::Point local) {
        if (!m_canFeed) {
            return;
        }
        m_dropX = local.x;
        m_services.connection.send(protocol::fishTankRequest(2)); // 23/97 2: ask for a grain of fish food
    }

    void FishTankWindow::requestKind(int request, int kind) {
        m_kind = kind;
        m_services.connection.send(protocol::fishTankKindRequest(request, kind));
    }

    void FishTankWindow::requestBuy() {
        m_services.connection.send(protocol::fishTankKindRequest(3, m_kind)); // tank request code 3: buy form
    }

    void FishTankWindow::requestDiscard() {
        m_services.connection.send(protocol::fishTankKindRequest(4, m_kind)); // tank request code 4: discard form
    }

    void FishTankWindow::receive(const core::Bytes& payload) {
        if (payload.size() < 3) { // action byte, 160 and the sub code
            return;
        }
        const int sub = payload[2]; // sub code of 23/160
        const int first = payload.size() > 3 ? payload[3] : 0; // first value byte
        const int second = payload.size() > 4 ? payload[4] : 0; // second value byte
        switch (sub) {
            case 1: // 23/160 1: a click may feed the fish
                m_canFeed = true;
                break;
            case 2: // 23/160 2: a grain falls (u8 fish food left)
                m_food = first;
                m_tank.dropFood(m_dropX);
                m_services.playSound("sound/9911.wav");
                break;
            case 3: // 23/160 3: clicks may no longer feed
                m_canFeed = false;
                break;
            case 4: // 23/160 4: fish food cannot be used
                m_messages.show("Can't use fish food", kShortMs);
                break;
            case 5: // 23/160 5: fish food put into the tank (u8 food left), clicks may feed
                m_canFeed = true;
                m_food = first;
                m_messages.show("Fish food has been put into the fish tank", kShortMs);
                break;
            case 6: // 23/160 6: count beside the coin (second value byte)
                m_count = second;
                break;
            case 7: // 23/160 7: count beside the coin (first value byte)
                m_count = first;
                break;
            case 8: // 23/160 8: open the buy form (mode 1)
            case 10: // 23/160 10: open the discard form (mode 2)
                m_hooks.buyForm(sub == 8 ? 1 : 2); // form mode: 1 buy, 2 discard
                break;
            case 9: { // 23/160 9: the tank's list of fish kinds
                std::vector<int> kinds;
                // the list starts at byte 3 and holds at most 8 kinds
                for (std::size_t at = 3; at < payload.size() && kinds.size() < 8; ++at) {
                    kinds.push_back(payload[at]);
                }
                m_tank.setTankFish(kinds);
                break;
            }
            case 11: // 23/160 11: answer to the owner's 62/56, opens the form (u8 count, u8 food)
                m_count = first;
                m_food = second;
                show();
                break;
            case 12: // 23/160 12: the tank is full
                m_messages.show("The capacity of fish tank is full", kShortMs);
                break;
            default: // other sub codes are ignored
                break;
        }
    }

    void FishTankWindow::update(std::uint64_t nowMs) {
        if (!m_window.isVisible()) {
            return;
        }
        const bool owner = m_hooks.ownerHere();
        m_screen.get<ui::Element>("FishTankBuy").setVisible(owner);
        m_screen.get<ui::Element>("FishTankDiscard").setVisible(owner);
        m_tank.update(nowMs);
    }

    void FishTankWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const core::Rect area = m_window.bounds();
        m_tank.draw(canvas, at, {area.width, area.height}, nowMs);
        if (!m_hooks.ownerHere()) {
            return;
        }
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const core::Rect icon{0, 0, m_iconSpot.size().width, m_iconSpot.size().height};
        if (m_food > 0 && m_foodIcon.isValid()) {
            canvas.drawPicture(m_foodIcon, at + m_foodIconSpot.at(), icon);
            canvas.drawText(at + m_foodCountSpot.at(), std::to_string(m_food), style, area.width, 15); // 15: text box height
        }
        if (m_coinIcon.isValid()) {
            canvas.drawPicture(m_coinIcon, at + m_coinIconSpot.at(), icon);
        }
        canvas.drawText(at + m_coinCountSpot.at(), std::to_string(m_count), style, area.width, 15); // 15: text box height
    }

}
