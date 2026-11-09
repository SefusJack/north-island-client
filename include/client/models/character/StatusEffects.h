#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_STATUS_EFFECTS_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_STATUS_EFFECTS_H

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The timed effects that run on the own player, as the server reports them.
    /// @details Original: TPlayer.FDoubleExpActive, FExpPotionRate, FLuckyGrassOn, FUsingSpaBuns, FDoubleExpPaused, FTrainingIslandItem, FStrengthenBuffMask, FBodyShiftState; HandleDoubleExpPacket, HandleExpPotionPacket, HandleLuckyGrassPacket, HandleSpaBunPacket, HandleTrainingIslandItemPacket and HandleStrengthenBuffPacket in DZ_Human.pas, TChangeBody in CJ_Menu.pas. Each call keeps the state the packet sets and returns the line the game says at the top of the screen ("" for none).
    class StatusEffects {
        public:
            /// @brief Handles the double-exp packet.
            /// @details Server 5/14: kind 1 on (value: minutes left, 0 at the upper limit), 2 refused (value 1), 0 off.
            /// @param kind Packet kind.
            /// @param value Minutes left or refusal value, if sent.
            /// @return The line to say, "" for none.
            std::string doubleExp(int kind, std::optional<int> value);

            /// @brief Sets the exp potion icon.
            /// @details Server 5/17 1: the icon (active, rate x 10).
            /// @param active New double-exp state.
            /// @param rate Potion rate times 10.
            /// @return The line to say, "" for none.
            std::string expPotionIcon(int active, int rate);

            /// @brief The refusal line of an exp potion.
            /// @details Server 5/17 2: reason 1 once only, 2 against a Statue Voucher.
            /// @param reason Refusal reason.
            /// @return The line to say, "" for none.
            std::string expPotionRefused(int reason);

            /// @brief The time left of an exp potion.
            /// @details Server 5/17 3.
            /// @param active New double-exp state.
            /// @param minutes Minutes left.
            /// @param rate Potion rate times 10.
            /// @return The line to say.
            std::string expPotionRemaining(int active, int minutes, int rate);

            /// @brief An exp potion expired.
            /// @details Server 5/17 4.
            /// @param active New double-exp state.
            /// @return The line to say.
            std::string expPotionExpired(int active);

            /// @brief Handles the lucky grass packet.
            /// @details Server 5/15: kind 0 off, 1 on (minutes left), 2 not in effect, 3 ending, 4 the time left.
            /// @param kind Packet kind.
            /// @param minutes Minutes left.
            /// @return The line to say, "" for none.
            std::string luckyGrass(int kind, int minutes);

            /// @brief Handles the spa buns packet.
            /// @details Server 5/16: kind 1 on (minutes left), 2 refused (value 0: the sauna bag is at its upper limit).
            /// @param kind Packet kind.
            /// @param value Minutes left or refusal value, if sent.
            /// @return The line to say, "" for none.
            std::string spaBuns(int kind, std::optional<int> value);

            /// @brief Handles the Stopwatch packet.
            /// @details Server 23/204: sub 1 gives the state, sub 2 says it was used (paused) or cancelled.
            /// @param sub Sub id.
            /// @param paused Whether the double exp is paused.
            /// @param itemName Name of the Stopwatch item.
            /// @return The line to say, "" for none.
            std::string expPause(int sub, bool paused, const std::string& itemName);

            /// @brief What 23/106 asks of the game: the line to say, whether to ask about the teleport (with 10 seconds to answer) instead, and the sound.
            struct TrainingNotice {
                std::string text;
                bool askTeleport = false;
                bool sound = false;
            };

            /// @brief Handles the Training Island item packet.
            /// @details Server 23/106: kind 1 the effect (kind of effect, minutes left, sound) runs, 2 it is over, 3 one is already running, 4 the time is at its limit. The item is Training Island item 34181; island map is 13050.
            /// @param kind Packet kind.
            /// @param effect Kind of effect.
            /// @param minutes Minutes left.
            /// @param sound Whether to play the sound.
            /// @param fighting The player is in a battle (or already on the island's map 13050).
            /// @param itemName Name of the Training Island item.
            /// @return The notice to show.
            TrainingNotice trainingItem(
                int kind,
                int effect,
                int minutes,
                bool sound,
                bool fighting,
                const std::string& itemName
            );

            /// @brief The minutes left of the five buffs as one line for the top of the screen.
            /// @details Server 23/119 1: the five buffs are ATK, DEF, MATK, MDEF, SPD.
            /// @param minutes Minutes left per buff.
            /// @return The line to say.
            std::string strengthBuffTimes(const std::array<int, 5>& minutes) const;

            /// @brief Sets the new buff mask (a nibble per stat from the third).
            /// @details Server 23/119 2.
            /// @param mask The new mask.
            /// @return The lines of the buffs that ran out.
            std::vector<std::string> strengthBuffMask(std::uint32_t mask);

            /// @brief The level of a buff.
            /// @details Original: GetBitField(FStrengthenBuffMask, 4, stat + 2).
            /// @param stat Stat 1..5.
            /// @return The level nibble.
            int strengthLevel(int stat) const;

            /// @brief The amount a buff gives.
            /// @details Original: StrengthenBuffValues (DZ_String.pas).
            /// @param stat Stat 1..5.
            /// @param level Buff level.
            /// @return The amount, 0 for none.
            static int strengthValue(int stat, int level);

            /// @brief Handles the body shift packet.
            /// @details Server 23/207: u8 state (0 time up, 1 changed, 2 off, 3 off silently).
            /// @param state The state.
            /// @return The line to say, "" for none.
            std::string bodyShift(int state);

            /// @brief The refusal line of a body shift.
            /// @details Server 23/207 3: refusal 1..4.
            /// @param reason Refusal reason.
            /// @return The line to say.
            static std::string bodyShiftRefusal(int reason);

            /// @brief The question about the time left of a body shift.
            /// @details Server 23/207 2.
            /// @param minutes Minutes left.
            /// @return The question, "" when none is left.
            std::string bodyShiftAsk(int minutes) const;

            /// @brief Whether the exp icon (tag 2) shows.
            /// @details Original: FDoubleExpActive = 1.
            /// @return True when double exp is on.
            bool doubleExpOn() const { return m_doubleExpActive == 1; }

            /// @brief Whether the double exp ran out (what the EXP Bag's auto use waits for).
            /// @details Original: FDoubleExpActive = 2.
            /// @return True when it ended.
            bool doubleExpEnded() const { return m_doubleExpActive == 2; }

            /// @brief The exp potion rate times 10.
            /// @return The rate.
            int expPotionRate() const { return m_expPotionRate; }

            /// @brief Whether lucky grass is on.
            /// @return True when on.
            bool luckyGrassOn() const { return m_luckyGrassOn; }

            /// @brief Whether spa buns are on.
            /// @return True when on.
            bool spaBunsOn() const { return m_spaBuns == 1; }

            /// @brief Whether the Dreaming Pill icon (tag 9) shows.
            /// @details Original: FDreamingPillOn.
            /// @return True when on.
            bool dreamingPillOn() const { return m_dreamingPill; }

            /// @brief Sets the Dreaming Pill state.
            /// @param on New state.
            void setDreamingPill(bool on) { m_dreamingPill = on; }

            /// @brief Whether the double exp is paused.
            /// @return True when paused.
            bool doubleExpPaused() const { return m_doubleExpPaused; }

            /// @brief The Training Island item state.
            /// @details Original: FTrainingIslandItem (0 none, 1 the Training Island item 34181).
            /// @return The state.
            int trainingItem() const { return m_trainingItem; }

            /// @brief Whether any strengthen buff runs.
            /// @return True when the mask is not 0.
            bool strengthOn() const { return m_strengthMask != 0; }

            /// @brief The strengthen buff mask.
            /// @return The mask.
            std::uint32_t strengthMask() const { return m_strengthMask; }

            /// @brief The body shift state.
            /// @details Original: FBodyShiftState: 0 none, 1 shifted, 2 the shift is off.
            /// @return The state.
            int bodyShiftState() const { return m_bodyShift; }

            /// @brief The hint of the exp icon: "<rate / 10>X Exp" for a potion, else "2X Exp".
            /// @details Original: AddTimeEffectIcon 2.
            /// @return The hint text.
            std::string expHint() const;

            /// @brief The item whose picture the exp icon wears.
            /// @details Original: SetExpMultiplierIcon.
            /// @return Item id.
            int expIconItem() const;

        private:
            int m_doubleExpActive = 0;
            int m_expPotionRate = 0;
            bool m_luckyGrassOn = false;
            int m_spaBuns = 0;
            bool m_dreamingPill = false;
            bool m_doubleExpPaused = false;
            int m_trainingItem = 0;
            std::uint32_t m_strengthMask = 0;
            int m_bodyShift = 0;
    };

}

#endif
