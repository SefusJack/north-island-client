#include "client/viewmodels/crafting/MakeQuestions.h"

#include "protocol/ManufactureRequests.h"

namespace client {

    namespace {

        constexpr const char* kAbortPicture = "btn_Abort_1";
        constexpr std::size_t kBoxWrap = 32; // question box line length, characters

        /// @brief Builds the timing of the question boxes: lines of 32 characters, the game windows stay (Se_TalkMsgFormPlus_4, Re_TalkMsgFormPlus_1).
        /// @return The timing options.
        TalkDialog::Timing boxTiming() {
            TalkDialog::Timing timing;
            timing.wrapChars = kBoxWrap;
            timing.exclusive = false;
            return timing;
        }

        /// @brief Builds the box timing whose Close button wears the Abort picture: it gives the job up, and says so.
        /// @return The timing options.
        TalkDialog::Timing abortTiming() {
            TalkDialog::Timing timing = boxTiming();
            timing.closePicture = kAbortPicture;
            return timing;
        }

    }

    MakeQuestions::MakeQuestions(
        MakeJobs& jobs,
        TalkDialog& talk,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_jobs(jobs),
          m_talk(talk),
          m_messages(messages),
          m_hooks(std::move(hooks)) {}

    void MakeQuestions::askContinue(
        int slot,
        const std::string& productName,
        int piece,
        int blueprintSource,
        int blueprintSlot
    ) {
        if (m_aborting != 0) {
            return;
        }
        m_slot = slot;
        m_talk.askOrClose(
            "Continue manufacturing " + productName + "? ",
            std::nullopt,
            [this, piece, blueprintSource, blueprintSlot] { continueJob(piece, blueprintSource, blueprintSlot); },
            [] {},
            [this] { askAbort(); },
            abortTiming()
        );
    }

    void MakeQuestions::continueJob(int piece, int blueprintSource, int blueprintSlot) {
        if (m_hooks.hasMaid()) {
            m_hooks.choosePartner([this, piece, blueprintSource, blueprintSlot](int petSlot, int petId) {
                return continueWith(piece, blueprintSource, blueprintSlot, petSlot, petId);
            });
            return;
        }
        continueWith(piece, blueprintSource, blueprintSlot, 0, 0);
    }

    bool MakeQuestions::continueWith(
        int piece,
        int blueprintSource,
        int blueprintSlot,
        int petSlot,
        int petId
    ) {
        if (m_jobs.playerSlot() != 0) {
            m_messages.show("Manufacture one item once", 2000); // message duration in ms
        } else if (const std::string why = m_hooks.refusal(); petSlot == 0 && !why.empty()) {
            m_messages.show(why, 2000); // message duration in ms
        } else {
            m_jobs.setPet(petSlot, petId);
            m_hooks.send(protocol::continueMakingRequest(piece, m_slot, blueprintSource, blueprintSlot, petSlot, petId));
            return true;
        }
        return false;
    }

    bool MakeQuestions::askStop() {
        if (m_aborting != 0) {
            return false;
        }
        m_slot = m_jobs.activeSlot();
        m_talk.askOrClose(
            "Moving will stop manufacturing, are you sure? ",
            std::nullopt,
            [this] {
                m_hooks.send(protocol::stopMakingRequest(m_slot));
                m_hooks.stopped();
            },
            [] {},
            [this] { askAbort(); },
            abortTiming()
        );
        return true;
    }

    void MakeQuestions::askAbort() {
        ++m_aborting;
        m_talk.ask(
            "The material will not return after giving up manufacturing, are you sure?",
            std::nullopt,
            [this] {
                m_aborting = 0;
                m_hooks.send(protocol::giveUpMakingRequest(m_slot));
                m_hooks.closeWindows();
            },
            [this] { m_aborting = 0; },
            boxTiming()
        );
        m_hooks.closeWindows();
    }

}
