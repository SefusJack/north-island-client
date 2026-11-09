#ifndef NORTH_ISLAND_UI_CURSOR_KIND_H
#define NORTH_ISLAND_UI_CURSOR_KIND_H

namespace ui {

    /// @brief The mouse cursor an element asks for while the mouse is over it.
    /// @details Original game cursor types: Normal = cursor1.ani, Hand = cursor4.ani; in battles Attack = cursor3.ani, Magic = cursor6.ani, Capture = cursor8.ani; in the world Attack also asks for a PK, Watch = cursor5.ani and Join = cursor11.ani for a running battle; the mini-game mallet is cursor12.ani, hitting cursor13.ani and coming back up cursor14.ani (types 13 to 15); the thrown item's is cursor18.ani (type 18); the dig hole's pointing hand is cursor99.ani (type 99).
    enum class CursorKind { Normal, Hand, Attack, Magic, Capture, Watch, Join, Mallet, MalletDown, MalletUp, Throw, Finger };

}

#endif
