#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_CAREER_HOOKS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_CAREER_HOOKS_H

#include "client/models/character/CharacterLook.h"
#include "core/Bytes.h"

#include <functional>
#include <string>

namespace client {

    /// @brief Callbacks the career forms use to reach the scene.
    /// @details Original: MZ_Menu.pas TCareerForm and TBeforeCareerForm.
    struct CareerHooks {
        std::function<CharacterLook()> look;
        std::function<int()> job;
        std::function<int()> statPoints;
        std::function<int()> level;
        std::function<bool()> fighting;
        std::function<int()> cloakSlot;
        std::function<int()> mirrorSlot;
        std::function<void(const std::string&, int)> message;
        std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
        std::function<void(const core::Bytes&)> send;
        std::function<void()> closeOthers;
        std::function<void()> eventGoesOn;
        std::function<void(int line, int changeMode)> showCareer;
    };

}

#endif
