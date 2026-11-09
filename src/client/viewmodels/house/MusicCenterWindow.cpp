#include "client/viewmodels/house/MusicCenterWindow.h"

#include "core/ByteReader.h"
#include "gfx/TextStyle.h"
#include "protocol/HouseRequests.h"
#include "ui/DrawingArea.h"

#include <filesystem>
#include <fstream>

namespace client {

    namespace {

        constexpr int kRows = 10; // song rows per page
        constexpr int kCharWidth = 8; // width of one text character, px
        constexpr int kCharHeight = 15; // text box height, px
        constexpr int kNameStart = 7; // payload offset where the song name starts (after the header, sub id and 4-byte song number)
        constexpr std::int64_t kClickGapMs = 1500; // least time between two song clicks, ms
        constexpr std::int64_t kScanGapMs = 2000; // least time between two mouse-over scans of the window, ms
        constexpr std::int64_t kPlayGapMs = 1000; // least time between two song starts from the server, ms

        /// @brief Loads MusicCenterWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MusicCenterWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MusicCenterWindow");
        }

    }

    MusicCenterWindow::MusicCenterWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_playIcon(screen.get<ui::ImageView>("MusicPlayIcon")),
          m_playButton(screen.get<ui::ImageButton>("MusicPlay")),
          m_rowSpot(screen.get<ui::Spot>("MusicRow")),
          m_textSpot(screen.get<ui::Spot>("MusicText")),
          m_playingSpot(screen.get<ui::Spot>("MusicPlaying")),
          m_songs(services.game.data("Sound")) {
        screen.get<ui::DrawingArea>("MusicPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onMusicClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMusicPreviousPage", [this](const ui::EventArgs&) {
            if (m_page > 0) {
                --m_page;
            }
        });
        screen.on("onMusicNextPage", [this](const ui::EventArgs&) {
            if ((m_songs.count() - 1) / kRows > m_page) {
                ++m_page;
            }
        });
        screen.on("onMusicPlay", [this](const ui::EventArgs&) { playClick(); });
        screen.on("onMusicStop", [this](const ui::EventArgs&) { stopClick(); });
        screen.on("onMusicNext", [this](const ui::EventArgs&) { nextClick(); });
        screen.on("onMusicPrevious", [this](const ui::EventArgs&) { previousClick(); });
        screen.on("onMusicRowUp", [this](const ui::EventArgs&) { rowUp(); });
        screen.on("onMusicRowDouble", [this](const ui::EventArgs&) {
            if (takeClick()) {
                setHouseBgm();
            }
        });
    }

    void MusicCenterWindow::open() {
        // song numbers are 1-based and number 1 is the "Set music" entry, hence the +1 on the count
        if (m_playing > 0 && m_songs.count() + 1 > m_playing) {
            m_page = (m_playing - 1) / kRows;
        } else {
            m_playing = 1;
            m_page = 0;
        }
        m_iconShown = true;
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    int MusicCenterWindow::rowAt(core::Point mouse) const {
        const core::Point origin = m_window.bounds().position();
        for (int row = 1; row <= kRows; ++row) {
            if (core::Rect{
                    origin.x + m_rowSpot.rect().x,
                    origin.y + m_rowSpot.rect().y + (row - 1) * m_rowSpot.rect().height,
                    m_rowSpot.rect().width,
                    m_rowSpot.rect().height
                }
                    .contains(mouse)) {
                return row;
            }
        }
        return 0;
    }

    void MusicCenterWindow::rowUp() {
        if (const int row = rowAt(m_screen.mouse())) {
            m_playing = m_page * kRows + row;
        }
    }

    bool MusicCenterWindow::takeClick() {
        if (m_waiting || m_nowMs - m_lastClick < kClickGapMs) {
            return false;
        }
        m_lastClick = m_nowMs;
        return true;
    }

    void MusicCenterWindow::playClick() {
        m_playHeld = true;
        if (takeClick()) {
            m_iconShown = true;
            setHouseBgm();
        }
    }

    void MusicCenterWindow::stopClick() {
        if (takeClick()) {
            m_playing = 0;
            m_iconShown = false;
            setHouseBgm();
        }
    }

    void MusicCenterWindow::nextClick() {
        if (takeClick() && m_songs.count() > m_playing) {
            ++m_playing;
            m_page = (m_playing - 1) / kRows;
            setHouseBgm();
        }
    }

    void MusicCenterWindow::previousClick() {
        if (takeClick() && m_playing > 1) {
            --m_playing;
            m_page = (m_playing - 1) / kRows;
            setHouseBgm();
        }
    }

    void MusicCenterWindow::setHouseBgm() {
        if (m_houseSong == m_playing || !m_hooks.inFloorRoom() || m_playing > m_songs.count()) {
            return;
        }
        m_waiting = true;
        m_houseSong = m_playing;
        m_hooks.send(protocol::houseMusicRequest(m_houseSong, m_songs.name(m_houseSong)));
        std::ofstream file(m_services.game.user(std::to_string(m_hooks.ownId()) + "MC.Txt"), std::ios::binary | std::ios::trunc);
        file << m_houseSong << "\r\n";
    }

    void MusicCenterWindow::receive(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 2); // payload offset where the sub id is read
        const int sub = reader.u8();
        if (sub == 1) { // sub 1: the house's song
            if (!m_hooks.inHouse()) {
                return;
            }
            m_houseSong = reader.i32();
            const std::string name = reader.failed() ? "BGM0011" : std::string(payload.begin() + kNameStart, payload.end());
            m_iconShown = true;
            m_waiting = false;
            const auto now = static_cast<std::int64_t>(nowMs);
            if (now - m_lastPlay < kPlayGapMs) {
                return;
            }
            m_lastPlay = now;
            m_services.music.stop();
            const int song = m_songs.find(name);
            if (m_houseSong == 0) {
                m_playing = 0;
            } else if (song != 0) {
                m_playing = song;
                m_houseSong = song;
                const std::filesystem::path file = m_services.game.data("Sound/" + name + ".wav");
                if (std::filesystem::exists(file)) {
                    m_services.music.playLooped(file);
                }
            } else {
                m_services.music.playLooped(m_services.game.data("Sound/BGM0011.wav"));
                m_playing = 1;
                m_houseSong = 1;
            }
        } else if (sub == 2) { // sub 2: the saved choice is asked for
            m_houseSong = 1;
            std::ifstream file(m_services.game.user(std::to_string(m_hooks.ownId()) + "MC.Txt"));
            int saved = 0;
            if (file && (file >> saved)) {
                m_houseSong = saved;
            }
            m_hooks.send(protocol::houseMusicRequest(m_houseSong, m_songs.name(m_houseSong)));
        }
    }

    void MusicCenterWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const core::Point origin = m_window.bounds().position();
        if (m_window.bounds().contains(m_screen.mouse()) && m_nowMs - m_lastScan >= kScanGapMs) {
            m_lastScan = m_nowMs;
            m_songs.scan();
        }
        if (m_playing == 0) {
            m_iconShown = false;
        }
        const bool onPlay = m_screen.hoveredId() == m_playButton.id();
        m_playHeld = m_playHeld && onPlay;
        m_playButton.setSelected(m_playHeld);
        m_playIcon.setVisible(m_iconShown && m_screen.hoveredId() != m_playButton.id());
        m_hovered = rowAt(m_screen.mouse());
        if (m_hovered != 0) {
            // +2: the hover bar is two pixels taller than the row
            canvas.fillRectAlpha(
                {origin.x + m_rowSpot.rect().x,
                 origin.y + m_rowSpot.rect().y + (m_hovered - 1) * m_rowSpot.rect().height,
                 m_rowSpot.rect().width,
                 m_rowSpot.rect().height + 2},
                m_rowSpot.color()
            );
        }
        for (int row = 1; row <= kRows; ++row) {
            const int number = m_page * kRows + row;
            if (number > m_songs.count()) {
                break;
            }
            std::string text = number == 1 ? "Set music" : m_songs.name(number);
            int centre = (m_rowSpot.rect().width - static_cast<int>(text.size()) * kCharWidth) / 2;
            if (centre < 0) {
                centre = 0;
                text = text.substr(0, 19); // longest name shown: 19 letters
            }
            const gfx::TextStyle style{number == m_playing ? m_playingSpot.color() : m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
            canvas.drawText(
                {at.x + m_textSpot.at().x + centre, at.y + m_textSpot.at().y + (row - 1) * m_rowSpot.rect().height},
                text,
                style,
                m_window.bounds().width,
                kCharHeight
            );
        }
    }

}
