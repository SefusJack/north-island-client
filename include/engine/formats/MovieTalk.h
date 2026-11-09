#ifndef NORTH_ISLAND_FORMATS_MOVIE_TALK_H
#define NORTH_ISLAND_FORMATS_MOVIE_TALK_H

namespace formats {

    /// @brief A talk box of a movie.
    /// @details Original: Tft_Talk, 16 bytes in the .sty file; who speaks (an Npc.dat id, 65535 the player, 99901 to 99917 the team and pets),
    /// the Talk.dat id, the step it is said in and the time in ms its box waits before it may be closed.
    struct MovieTalk {
        int speaker = 0;
        int textId = 0;
        int step = 0;
        int showTime = 0;
    };

}

#endif
