#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_SANDGLASS_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_SANDGLASS_H

#include "formats/ItemTable.h"

#include <string>

namespace client {

    /// @brief The Magic Sandglass: the owner uses a piece of the house to make the next job take a tenth of its time.
    /// @details Original: DZ_Thing.pas TUtensilManage.ConfirmUseSandglass, HandleSandglassEffectPacket, DrawUtensilName; MZ_Menu.pas
    /// TTimeEffectStatusForm.IsSandglassSlotActive, ShowSandglassEffects.
    class Sandglass {
        public:
            static constexpr int kUnusedFlag = 32; // flag bit 32: a use not yet taken by a job

            /// @brief Tells whether an item is a Magic Sandglass piece (39074 or 39097).
            /// @details Original: IsSandglassItem.
            /// @param items Item table.
            /// @param itemId The item.
            /// @return True for a sandglass.
            static bool isSandglassItem(const formats::ItemTable& items, int itemId);

            /// @brief The "(Remaining times:N Times)" label shown after the piece's name.
            /// @details Original: DrawUtensilName; the used amount counts up to 250.
            /// @param maxUses Uses the piece has (from Item.dat).
            /// @param usedAmount Used amount (of 250).
            /// @return The label, empty for a piece without uses.
            static std::string remainingLabel(int maxUses, int usedAmount);

            /// @brief The seconds a job takes under a sandglass.
            /// @details Original: DrawMakeThingBar.
            /// @param wholeSeconds The job's normal seconds.
            /// @return A tenth of them.
            static int jobSeconds(int wholeSeconds) { return wholeSeconds * 10 / 100; } // the job takes 10 percent of its time

            /// @brief Sets the effect flags (bit n - 1 for the job of slot n).
            /// @details Original: HandleSandglassEffectPacket.
            /// @param flags The flag bits.
            void setFlags(int flags) { m_flags = flags; }

            /// @brief Tells whether the effect is on (any flag set).
            /// @return True when active.
            bool active() const { return m_flags != 0; }

            /// @brief Tells whether the effect is on for a job slot.
            /// @details Original: IsSandglassSlotActive.
            /// @param slot Job slot 1..8.
            /// @return True when its flag is set.
            bool slotActive(int slot) const;

            /// @brief Tells whether some use has not been taken by a job (flag 32).
            /// @return True when unused.
            bool unused() const { return (m_flags & kUnusedFlag) != 0; }

            /// @brief Remembers the piece the owner is being asked about.
            /// @details Original: FSandglassFurnIndex.
            /// @param piece The piece number.
            void ask(int piece) { m_piece = piece; }

            /// @brief The piece the owner is being asked about.
            /// @return The piece number.
            int piece() const { return m_piece; }

        private:
            int m_flags = 0;
            int m_piece = 0;
    };

}

#endif
