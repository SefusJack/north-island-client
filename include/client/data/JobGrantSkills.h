#ifndef NORTH_ISLAND_CLIENT_DATA_JOB_GRANT_SKILLS_H
#define NORTH_ISLAND_CLIENT_DATA_JOB_GRANT_SKILLS_H

namespace client {

    /// @brief Gives a skill from the JobGrantSkills table: the skills a skill of an element's tree brings with it.
    /// @details Original: JobGrantSkills[element 0..4, group 0..3, row 0..10, column 0..5] (Re_PubConst.pas); column 1 is the skill itself, the next ones what it grants; 0 for none.
    /// @param element Element 0..4.
    /// @param group Group 0..3.
    /// @param row Row 0..10.
    /// @param column Column 0..5.
    /// @return The skill id, or 0 for none or an index out of range.
    int jobGrantSkill(int element, int group, int row, int column);

}

#endif
