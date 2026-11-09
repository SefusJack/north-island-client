#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_HOST_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_HOST_H

#include "formats/MovieTalk.h"

#include <string>

namespace client {

    /// @brief What a running movie asks of the game around it: music, sounds, talk boxes, zoom.
    /// @details Original: FT_Movie_Object.pas calls these directly; ShowMovieTalk puts the speaker and the Talk.dat text in MainTalkMsgForm.
    class MovieHost {
        public:
            virtual ~MovieHost() = default;

            /// @brief Plays the music of a step, from the sound table.
            /// @details Original: PlayMusic(sound\file).
            /// @param soundFile Sound file name.
            virtual void playMusic(const std::string& soundFile) = 0;

            /// @brief Plays the scene's own music: step 1 without a named music, and the end of a movie.
            /// @details Original: PlayMusic of the scene's BGM.
            virtual void playSceneMusic() = 0;

            /// @brief Plays a sound effect.
            /// @details Original: PlaySoundEffect(sound\file).
            /// @param soundFile Sound file name.
            virtual void playSound(const std::string& soundFile) = 0;

            /// @brief Stops the loop the old handle names and loops a file.
            /// @details Original: PlayLoopedSoundEffect(sound\file).
            /// @param soundFile Sound file name.
            /// @param oldHandle Handle of the loop to stop.
            /// @return Handle of the new loop, 0 when nothing plays.
            virtual int playLoopedSound(const std::string& soundFile, int oldHandle) = 0;

            /// @brief Stops a looped sound (Stop and Looped := False on its stream).
            /// @param handle Handle of the loop.
            virtual void stopLoopedSound(int handle) = 0;

            /// @brief Draws a random number for the movie's team-look seat and face draws.
            /// @details Original: Random(below) in LoadMovieScript.
            /// @param below Exclusive upper bound.
            /// @return Value from 0 to below - 1.
            virtual int random(int below) = 0;

            /// @brief Tells whether the human a team look names is the player himself.
            /// @details Seat surface 99901 is the leader (the player when alone), 99902 to 99904 the others as they joined; an empty seat is not the player.
            /// @param seatSurface Seat surface id (99901 to 99904).
            /// @return True for the player.
            virtual bool ownSeat(int seatSurface) = 0;

            /// @brief Opens a talk box.
            /// @details Original: ShowMovieTalk.
            /// @param talk The speaker and text.
            virtual void say(const formats::MovieTalk& talk) = 0;

            /// @brief Gives how many frames a role's sprite has in an action slot.
            /// @details Original: GetActionFrameCount.
            /// @param surface Role surface id.
            /// @param slot Action slot (direction and action).
            /// @return Frame count; 0 when the role is nobody of the game's or has no such action.
            virtual int roleFrames(int surface, int slot) = 0;

            /// @brief Zooms the view in (screen filters 4 to 6).
            /// @details Original: ZoomViewIn.
            virtual void zoomIn() = 0;

            /// @brief Zooms the view out.
            /// @details Original: ZoomViewOut.
            virtual void zoomOut() = 0;

            /// @brief Zooms the view to the most (filter 6).
            virtual void zoomMost() = 0;

            /// @brief Tells whether the talk box is still up (the movie waits for the player to close it).
            /// @details Original: MainTalkMsgForm.FIsVisible.
            /// @return True while open.
            virtual bool talkOpen() const = 0;
    };

}

#endif
