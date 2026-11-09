#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GAME_HOOKS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GAME_HOOKS_H

#include "core/Bytes.h"

#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief What the mini games of the game hall ask of the world (original TSportManage and the shared forms): server, points, bag, messages, sounds, game end.
    struct GameHooks {
        std::function<void(const core::Bytes&)> send;
        std::function<int()> imPoints;
        std::function<int(int itemId)> countItem;
        std::function<int()> freeBagPlaces;
        std::function<bool()> freePlay;
        std::function<std::string(int itemId)> itemName;
        std::function<int(int itemId)> freeItemPlace;
        std::function<int(int place)> itemInPlace;
        std::function<void(const std::string&, int)> message;
        std::function<void(const std::string&, int)> requestMessage;
        std::function<void(const std::string&, bool mainBox, std::function<void()> ok, std::function<void()> cancel)> ask;
        std::function<void(const std::string&, const std::vector<std::string>&, std::function<void(int)>)> choose;
        std::function<void(const std::string&)> playSound;
        std::function<void()> stopSounds;
        std::function<void(const std::string&)> playMusic;
        std::function<void()> stopMusic;
        std::function<void()> endGame;
        std::function<void(int result)> finishGame;
    };

}

#endif
