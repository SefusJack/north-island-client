#ifndef NORTH_ISLAND_CLIENT_SCENES_LOADING_SCENE_H
#define NORTH_ISLAND_CLIENT_SCENES_LOADING_SCENE_H

#include "client/services/GameServices.h"
#include "client/scenes/Scene.h"
#include "ui/ProgressBar.h"
#include "ui/Screen.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The loading screen: opens the picture sets in its first update (the progress bar fills at once), then hands over to the login screen.
    /// @details Original: LoadingScene.xml layout.
    class LoadingScene : public Scene {
        public:
            /// @brief Builds the loading screen and queues the loading steps.
            /// @param services The shared game services.
            explicit LoadingScene(
                GameServices& services
            );

            /// @brief Overrides Scene::update: runs every loading step and fills the progress bar.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs) override;

            /// @brief Overrides Scene::draw: draws the loading screen.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in ms.
            void draw(gfx::Canvas& canvas, std::uint64_t nowMs) override;

            /// @brief Overrides Scene::screen: gives the loading screen.
            /// @return The screen.
            ui::Screen& screen() override { return m_screen; }

            /// @brief Overrides Scene::nextScene: the login scene once every step is done.
            /// @return The login scene, or nothing while loading.
            std::optional<SceneKind> nextScene() const override;

        private:
            GameServices& m_services;
            ui::Screen m_screen;
            ui::Element& m_layout;
            ui::ProgressBar& m_progress;
            std::vector<std::function<void()>> m_steps;
            std::size_t m_nextStep = 0;
    };

}

#endif
