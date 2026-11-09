#ifndef NORTH_ISLAND_CLIENT_MODELS_CRAFTING_MAKE_JOBS_H
#define NORTH_ISLAND_CLIENT_MODELS_CRAFTING_MAKE_JOBS_H

#include "client/models/crafting/MakeSlot.h"
#include "core/Bytes.h"
#include "core/Point.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief The player's manufacturing: the job list the server sends (64/1) and what the later messages do to it.
    /// @details Original: DZ_Thing.pas TUtensilManage.FMakeSlots with HandleMakePacket's state changes, DZ_Human.pas Player.FMakeThingSlot and its start time.
    class MakeJobs {
        public:
            static constexpr int kSlots = 5; // job slots 1..5
            static constexpr int kBarLength = 47; // length of the progress bar when a job is done, px

            /// @brief A job slot; slots 1..5.
            /// @param index Slot number.
            /// @return The slot, an empty one for any other number.
            const MakeSlot& slot(int index) const;

            /// @brief The first free slot.
            /// @details Original: FindFreeMakeSlot.
            /// @return Slot number, 0 when all are taken.
            int freeSlot() const;

            /// @brief The first slot being worked on.
            /// @details Original: FindActiveMakeSlot.
            /// @return Slot number, 0 for none.
            int activeSlot() const;

            /// @brief Clears all slots and the player's job.
            /// @details Original: ClearMakeSlots.
            void clear();

            /// @brief Applies the job list of the server.
            /// @details Server 64/1: the slots (slot, product, elapsed milliseconds, batches) of the message.
            /// @param payload The whole payload.
            void applyList(const core::Bytes& payload);

            /// @brief Starts working on a slot; the player makes it.
            /// @details Server 64/2.
            /// @param slot Slot number.
            /// @param nowMs Current time in ms.
            void start(int slot, std::uint64_t nowMs);

            /// @brief Stops working on a slot.
            /// @details Server 64/3.
            /// @param slot Slot number.
            void stop(int slot);

            /// @brief Frees a slot: the job is done or given up.
            /// @details Server 64/4 and 64/5.
            /// @param slot Slot number.
            void release(int slot);

            /// @brief Marks a job as unable to finish.
            /// @details Server 64/6..8.
            /// @param slot Slot number.
            /// @param fullMs The full elapsed time in ms.
            void fail(int slot, std::uint32_t fullMs);

            /// @brief The slot the player works on.
            /// @details Original: Player.FMakeThingSlot.
            /// @return Slot number, 0 for none.
            int playerSlot() const { return m_playerSlot; }

            /// @brief The slot made by the partner pet.
            /// @details Original: Player.FMakePetSlot; the partner (a pet of a human kind) makes it in the player's place.
            /// @return Slot number, 0 when the player himself makes it.
            int petSlot() const { return m_petSlot; }

            /// @brief The partner pet's id.
            /// @details Original: Player.FMakePetId.
            /// @return Pet id, 0 for none.
            int petId() const { return m_petId; }

            /// @brief Sets the partner pet.
            /// @param slot Slot number.
            /// @param id Pet id.
            void setPet(int slot, int id) {
                m_petSlot = slot;
                m_petId = id;
            }

            /// @brief Removes the partner pet.
            void clearPet() { setPet(0, 0); }

            /// @brief The room of the house where the partner works.
            /// @details Original: Player.FMakeRoom; 250 for none.
            /// @return Room number.
            int workRoom() const { return m_workRoom; }

            /// @brief The piece's place where the partner works.
            /// @details Original: Player.FMakeX and FMakeY.
            /// @return The place.
            core::Point workAt() const { return m_workAt; }

            /// @brief Sets where the partner works; set by the first slot.
            /// @param room Room of the house.
            /// @param at The piece's place.
            void setWork(int room, core::Point at) {
                m_workRoom = room;
                m_workAt = at;
            }

            /// @brief The whole time of a slot in seconds with the furniture's savings.
            /// @details Furniture flags as FurnitureBonus. Under a Magic Sandglass it is a tenth and the furniture saves nothing.
            /// @param slot Slot number.
            /// @param recipeMinutes The recipe's minutes.
            /// @param furnitureFlags Furniture bonus flags.
            /// @param underSandglass Whether a Magic Sandglass is active.
            /// @return Seconds.
            int totalSeconds(int slot, int recipeMinutes, int furnitureFlags, bool underSandglass = false) const;

            /// @brief The seconds worked on the slot the player makes.
            /// @param nowMs Current time in ms.
            /// @return Seconds.
            std::int64_t workedSeconds(std::uint64_t nowMs) const;

            /// @brief The width of the progress bar over the player (0..47).
            /// @param totalSeconds The whole time in seconds.
            /// @param nowMs Current time in ms.
            /// @return Bar length in pixels.
            int barLength(int totalSeconds, std::uint64_t nowMs) const;

            /// @brief The seconds still to go.
            /// @param totalSeconds The whole time in seconds.
            /// @param nowMs Current time in ms.
            /// @return Seconds left.
            int secondsLeft(int totalSeconds, std::uint64_t nowMs) const;

        private:
            /// @brief A job slot for writing.
            /// @param index Slot number.
            /// @return The slot.
            MakeSlot& at(int index);

            std::array<MakeSlot, kSlots + 1> m_slots{};
            int m_playerSlot = 0;
            int m_petSlot = 0;
            int m_petId = 0;
            int m_workRoom = 0;
            core::Point m_workAt;
            std::uint64_t m_startMs = 0;
    };

}

#endif
