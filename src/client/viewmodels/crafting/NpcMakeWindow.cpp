#include "client/viewmodels/crafting/NpcMakeWindow.h"

#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kPlayerNameRise = 38; // the player's name is drawn this many px higher than a pet's name
        constexpr int kNoticeMs = 2000; // notice display time, ms

        /// @brief Loads NpcMakeWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NpcMakeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("NpcMakeWindow");
        }

    }

    NpcMakeWindow::NpcMakeWindow(
        GameServices& services,
        ui::Screen& screen,
        const PetTeam& pets,
        const MapCharacter& player,
        PlayerMessages& messages,
        std::function<void()> closed
    )
        : m_services(services),
          m_screen(screen),
          m_pets(pets),
          m_player(player),
          m_messages(messages),
          m_closed(std::move(closed)),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("NpcMakeRows")),
          m_firstPetSpot(screen.get<ui::Spot>("NpcMakeFirstPet")),
          m_playerFaceSpot(screen.get<ui::Spot>("NpcMakePlayerFace")),
          m_nameSpot(screen.get<ui::Spot>("NpcMakeName")),
          m_playerNameSpot(screen.get<ui::Spot>("NpcMakePlayerName")),
          m_untradableNameSpot(screen.get<ui::Spot>("NpcMakeUntradableName")),
          m_face(services, PetInfoPainter::Style{{0, 0}, false, false, {10, 2}}) { // FPreviewOffset: figure offset (10, 2)
        screen.get<ui::DrawingArea>("NpcMakeFaces").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintFaces(canvas, at, nowMs);
        });
        screen.get<ui::DrawingArea>("NpcMakeNames").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintNames(canvas, at); });
        screen.on("onNpcMakeClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onNpcMakeRow", [this](const ui::EventArgs& args) { pick(args.index); });
    }

    void NpcMakeWindow::open(Picked picked) {
        m_picked = std::move(picked);
        m_places.clear();
        m_list.clear();
        m_list.addRow({});
        for (int order = 1; order <= PetTeam::kPlaces; ++order) {
            const int place = m_pets.placeByOrder(order);
            if (place == 0) {
                continue;
            }
            const std::optional<Pet>& pet = m_pets.at(place);
            if (!pet) {
                break;
            }
            if (pet->info.directionKind == 0) { // 0: standard direction kind, only these pets can be offered
                m_places.push_back(place);
                m_list.addRow({});
            }
        }
        if (m_places.empty()) {
            pick(0);
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void NpcMakeWindow::close() {
        m_window.setVisible(false);
        m_closed();
    }

    void NpcMakeWindow::pick(int row) {
        int place = 0;
        int id = 0;
        if (row > 0) {
            if (row > static_cast<int>(m_places.size())) {
                return;
            }
            place = m_places[static_cast<std::size_t>(row - 1)];
            const std::optional<Pet>& pet = m_pets.at(place);
            if (!pet) {
                return;
            }
            id = pet->info.npcId;
            if (m_pets.activePlace() != 0 && m_pets.activePlace() == place) {
                m_messages.show("Pet is fighting\xA1I", kNoticeMs);
                return;
            }
            if (m_pets.ridingPlace() == place) {
                m_messages.show("Riding pet\xA1I", kNoticeMs);
                return;
            }
            if (pet->info.mercenary != 0) { // non-zero: the pet is a soldier (mercenary)
                m_messages.show("Soilders can't be made", kNoticeMs);
                return;
            }
        }
        if (m_picked && m_picked(place, id)) {
            close();
        }
    }

    void NpcMakeWindow::paintFaces(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (!m_window.isVisible()) {
            return;
        }
        for (std::size_t row = 0; row < m_places.size(); ++row) {
            if (const std::optional<Pet>& pet = m_pets.at(m_places[row])) {
                m_face.paintFigure(
                    canvas,
                    {at.x + m_firstPetSpot.at().x, at.y + m_firstPetSpot.at().y + static_cast<int>(row) * m_firstPetSpot.pitch().y},
                    *pet,
                    nowMs
                );
            }
        }
    }

    void NpcMakeWindow::paintNames(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        m_services.characters.drawLifeFace(
            canvas,
            m_player.look,
            {at.x + m_firstPetSpot.at().x + m_playerFaceSpot.at().x, at.y + m_firstPetSpot.at().y + m_playerFaceSpot.at().y}
        );
        const auto name = [&](int row, const std::string& text, core::Color color, int rise) {
            canvas.drawText(
                {at.x + m_firstPetSpot.at().x + m_nameSpot.at().x,
                 at.y + m_firstPetSpot.at().y + (row - 1) * m_firstPetSpot.pitch().y + m_nameSpot.at().y - rise},
                text,
                {color, gfx::TextEffect::Plain, std::nullopt},
                m_nameSpot.rect().width,
                m_nameSpot.rect().height
            );
        };
        name(1, m_player.name, m_playerNameSpot.color(), kPlayerNameRise);
        for (std::size_t row = 0; row < m_places.size(); ++row) {
            if (const std::optional<Pet>& pet = m_pets.at(m_places[row])) {
                name(
                    static_cast<int>(row) + 1,
                    pet->name,
                    pet->tradable != 1 ? m_untradableNameSpot.color() : m_playerNameSpot.color(),
                    0
                ); // tradable 1: blue name, otherwise red; 0: no rise
            }
        }
    }

}
