#ifndef NORTH_ISLAND_FORMATS_MOVIE_STEP_EFFECT_H
#define NORTH_ISLAND_FORMATS_MOVIE_STEP_EFFECT_H

namespace formats {

    /// @brief What a movie step does to the screen.
    /// @details Original: Tft_Efficacy, 11 bytes per step in the .sty file; the fade kind it begins with (0 cut to black, 1 fade in, 2 fade out,
    /// 3 to 5 and 10 the scene effects of that number, 7 to 9 a tint, 9 the fill below), two screen filter kinds, the music (an id of
    /// soundtabel.txt, 0 none) and the fill colour of kind 9.
    struct MovieStepEffect {
        int fadeKind = 0;
        int filter1 = 0;
        int filter2 = 0;
        int musicId = 0;
        int fillAlpha = 0;
        int fillRed = 0;
        int fillGreen = 0;
        int fillBlue = 0;
    };

}

#endif
