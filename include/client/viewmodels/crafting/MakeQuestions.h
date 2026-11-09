#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_MAKE_QUESTIONS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_MAKE_QUESTIONS_H

#include "client/models/crafting/MakeJobs.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "core/Bytes.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The questions manufacturing asks: go on with a stopped job, stop the job when the player moves, and give a job up (its materials are lost).
    /// @details Original: DZ_Thing.pas ConfirmContinueMake, ConfirmStopMake, AskAbortMake, ConfirmAbortMake. Each has Confirm, Cancel and, for the last, the Close button wearing the Abort picture.
    class MakeQuestions {
        public:
            /// @brief Callbacks into the scene: packet send, closing the manufacture windows, stop notice, refusal text and the maid partner choice.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void()> closeWindows;
                std::function<void()> stopped;
                std::function<std::string()> refusal;
                std::function<bool()> hasMaid;
                std::function<void(std::function<bool(int, int)>)> choosePartner;
            };

            /// @brief Creates the question set over the manufacture jobs.
            /// @param jobs Manufacture jobs.
            /// @param talk Talk dialog the questions are asked in.
            /// @param messages Player message line.
            /// @param hooks Scene callbacks.
            MakeQuestions(
                MakeJobs& jobs,
                TalkDialog& talk,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Asks "Continue manufacturing ...?" for a recipe whose product is a job waiting (PickMakeThingRecipe).
            /// @param slot Job slot waiting.
            /// @param productName Name of the product.
            /// @param piece Furniture piece the job is made at.
            /// @param blueprintSource Source of the blueprint.
            /// @param blueprintSlot Place of the blueprint.
            void askContinue(
                int slot,
                const std::string& productName,
                int piece,
                int blueprintSource,
                int blueprintSlot
            );

            /// @brief Asks "Moving will stop manufacturing, are you sure?" when the player moved while a job runs (ConfirmStopMake).
            /// @return False when a question is already up.
            bool askStop();

            /// @brief Forgets the abort question: it is gone on a new place (ClearMakeSlots).
            void reset() { m_aborting = 0; }

        private:
            /// @brief Asks whether to give the job up (AskAbortMake).
            void askAbort();

            /// @brief Goes on with the stopped job; with a Maid in the house it asks which partner goes on (ConfirmContinueMake).
            /// @param piece Furniture piece.
            /// @param blueprintSource Source of the blueprint.
            /// @param blueprintSlot Place of the blueprint.
            void continueJob(int piece, int blueprintSource, int blueprintSlot);

            /// @brief Does the checks then sends 64/2 with the partner (the rest of ConfirmContinueMake and NpcMakeItemPicked's old order).
            /// @param piece Furniture piece.
            /// @param blueprintSource Source of the blueprint.
            /// @param blueprintSlot Place of the blueprint.
            /// @param petSlot Partner's place (0 for the player).
            /// @param petId Partner's id (0 for the player).
            /// @return True when done.
            bool continueWith(
                int piece,
                int blueprintSource,
                int blueprintSlot,
                int petSlot,
                int petId
            );

            MakeJobs& m_jobs;
            TalkDialog& m_talk;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            int m_aborting = 0;
            int m_slot = 0;
    };

}

#endif
