#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_TIME_EFFECT_ICONS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_TIME_EFFECT_ICONS_H

#include "client/services/GameServices.h"
#include "client/models/character/StatusEffects.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"

#include <array>
#include <functional>
#include <string>

namespace client {

    /// @brief The icons above the bottom right buttons that show what is going on with the player (Remote Control, Memory Card, instance enter/quit, effect icons).
    /// @details Original: TimeEffectIcons.xml, TTimeEffectStatusForm. An icon takes the lowest free place of 17 when added (AddTimeEffectIcon); they are laid out from place 17 down, from the right, 50 pixels apart. Instance icons are not shown in a battle.
    class TimeEffectIcons {
        public:
            /// @brief Callbacks into the rest of the client that the icons need.
            struct Hooks {
                std::function<int()> instanceState;
                std::function<bool()> inInstanceMap;
                std::function<bool()> fighting;
                std::function<void()> enterInstance;
                std::function<void()> quitInstance;
                std::function<bool()> remoteOn;
                std::function<void()> stopRemote;
                std::function<int()> puppetMode;
                std::function<void()> openPuppet;
                std::function<void(int tag)> askTimeLeft;
                std::function<bool()> inTrainingMap;
                std::function<bool()> warLeaveShown;
                std::function<void()> leaveWar;
                std::function<bool()> sandglassOn;
                std::function<void()> showSandglass;
                std::function<bool()> inHouseRoom;
                std::function<void()> openRemote;
            };

            /// @brief Loads the icons' layout and wires their painters, hit tests and click events.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param effects Status effects of the own player.
            /// @param hooks Callbacks into remote control, instance, battle and time-left requests.
            TimeEffectIcons(
                GameServices& services,
                ui::Screen& screen,
                const StatusEffects& effects,
                Hooks hooks
            );

        private:
            /// @brief Adds or removes the icons that apply and lays them out from the last place to the first.
            /// @details Original: RenderWithChildren.
            void layout();

            /// @brief Gives the hint of an effect icon.
            /// @details Original: the hint AddTimeEffectIcon gives.
            /// @param tag Tag of the icon.
            /// @return Hint text.
            std::string hintOf(int tag) const;

            /// @brief Puts an icon in the lowest free place unless it is already there.
            /// @details Original: AddTimeEffectIcon, by the tag of the icon (1 the Remote Control, 10 enter, 11 quit); FEffectIcons[1..17] by place.
            /// @param tag Tag of the icon.
            void add(int tag);

            /// @brief Frees the place of an icon.
            /// @details Original: RemoveTimeEffectIcon.
            /// @param tag Tag of the icon.
            void remove(int tag);

            /// @brief An icon of an effect of the own player: the exp, Lucky Grass, Spa Buns, Training Island item, Riceball and stat buff icons.
            struct StatusIcon {
                int tag;
                ui::DrawingArea& area;
            };

            GameServices& m_services;
            Hooks m_hooks;
            const StatusEffects& m_effects;
            ui::ImageButton& m_enter;
            ui::ImageButton& m_quit;
            ui::ImageButton& m_war;
            ui::DrawingArea& m_remote;
            ui::DrawingArea& m_puppet;
            std::array<StatusIcon, 8> m_statusIcons; // the eight effect icons
            std::array<int, 18> m_places{}; // places 1..17 (index 0 unused)
    };

}

#endif
