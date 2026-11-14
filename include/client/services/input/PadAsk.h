#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_ASK_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_ASK_H

namespace client {

    /// @brief What a game pad asks of a scene directly, without the pointer.
    /// @details Accept and Cancel answer what is asked on the screen (a talk box); Interact uses what stands before the
    /// player; Up, Down, Left and Right step through what is offered (a list of choices, the battle's command ring); Erase
    /// and Space are the two upper buttons while the keyboard on the screen is up; Menu is the start button where the
    /// screen has no button bars (the settings before the game).
    enum class PadAsk { Accept, Interact, Cancel, Up, Down, Left, Right, Erase, Space, Menu };

}

#endif
