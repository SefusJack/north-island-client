#include "client/viewmodels/login/ReloginScreen.h"

namespace client {

    namespace {

        constexpr int kMostProgress = 30; // progress bar stops at 30 (one step per update call)

    }

    ReloginScreen::ReloginScreen(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_layer(screen.load(services.game.layout("ReloginScreen.xml"), services.loadContext())),
          m_progress(screen.get<ui::ProgressBar>("ReloginProgress")) {}

    void ReloginScreen::show(bool pvp) {
        for (const char* name : {"ReloginBackground", "ReloginBarBack", "ReloginProgress", "ReloginWarning"}) {
            m_screen.get<ui::Element>(name).setVisible(!pvp);
        }
        for (const char* name : {"ReloginPvpBackground", "ReloginMark"}) {
            m_screen.get<ui::Element>(name).setVisible(pvp);
        }
        m_progress.setValue(0);
        m_layer.setVisible(true);
    }

    void ReloginScreen::hide() { m_layer.setVisible(false); }

    void ReloginScreen::update() {
        if (m_layer.isVisible() && m_progress.value() < kMostProgress) {
            m_progress.setValue(m_progress.value() + 1);
        }
    }

}
