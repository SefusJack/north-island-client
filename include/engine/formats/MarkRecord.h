#ifndef NORTH_ISLAND_FORMATS_MARK_RECORD_H
#define NORTH_ISLAND_FORMATS_MARK_RECORD_H

#include <string>

namespace formats {

    /// @brief One quest mark of data\Mark.Dat.
    /// @details A quest is a pair: a counter mark whose text holds the steps ("#01 ...#02 ...", #99 the ending) and the mark
    /// after it, with the same name, whose flag bit is set when the quest is done. FHiddenInTaskList keeps a mark out of the quest window.
    struct MarkRecord {
        int markId = 0;
        int flagBit = 0;
        std::string name;
        std::string text;
        bool hiddenInList = false;
    };

}

#endif
