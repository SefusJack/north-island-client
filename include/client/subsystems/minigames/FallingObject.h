#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FALLING_OBJECT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FALLING_OBJECT_H

namespace client {

    /// @brief A falling coin (kind 1) or bomb (kind 2) of the catching game (original MK_Explorer.pas TTobj).
    struct FallingObject {
        int x = 0;
        int y = 0;
        int previousY = 0;
        int kind = 0;
        int frameWidth = 0;
        int frameHeight = 0;
        int halfWidth = 0;
        int anchorY = 0;
        int curFrame = 0;
        int lastFrame = 0;
        int fallSpeed = 0;
        bool inUse = false;
    };

}

#endif
