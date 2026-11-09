#ifndef NORTH_ISLAND_CLIENT_RULES_SKILLS_H
#define NORTH_ISLAND_CLIENT_RULES_SKILLS_H

namespace client {

    constexpr int kRoleSkillPlaceholder = 15003; // skill 15003: Newbie's Stunt
    constexpr int kNormalAttack = 10001; // skill 10001: Hand Attack
    constexpr int kDefense = 60021; // skill 60021: Defense

    /// @brief The role's own skill.
    /// @details Original: GetRoleSkillId.
    /// @param bodyType The body type.
    /// @param head The head.
    /// @return The skill id, 0 for none.
    int roleSkillId(int bodyType, int head);

    /// @brief Whether a skill is one of the roles' own skills.
    /// @details Original: IsRoleSkillId.
    /// @param skillId The skill id.
    /// @return True for a role skill.
    bool isRoleSkill(int skillId);

    /// @brief Whether the skill window's tab shows a skill.
    /// @details Original: RefreshSkillList. Tabs: 1 physical, 2 magical, 3 aid, 4 daily.
    /// @param tab The tab, 1..4.
    /// @param skillId The skill id.
    /// @param skillType The skill's type.
    /// @return True when shown on the tab.
    bool skillOnTab(int tab, int skillId, int skillType);

    /// @brief The same for a pet's skills (no extra aid skills among them; the tree tab has none).
    /// @param tab The tab, 1..4.
    /// @param skillType The skill's type.
    /// @return True when shown on the tab.
    bool petSkillOnTab(int tab, int skillType);

}

#endif
