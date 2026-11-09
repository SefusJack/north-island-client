#include "client/scenes/world/WorldScene.h"

#include "protocol/GmQuestion.h"

namespace client {

    namespace {

        constexpr std::size_t kRequestBoxWrap = 32; // characters per line in the GM question box
        constexpr int kUnanswered = 255; // answer code 255: the question was closed without a choice

    }

    void WorldScene::onGmQuestion(int kind, const std::string& question, const std::vector<std::string>& choices) {
        m_gmQuestion = kind;
        TalkDialog::Timing timing;
        timing.wrapChars = kRequestBoxWrap;
        timing.exclusive = false;
        m_talk.choose(
            question,
            std::nullopt,
            choices,
            [this](int choice) { answerGmQuestion(choice + 1); }, // answers are 1-based
            [this] { answerGmQuestion(kUnanswered); },
            std::move(timing)
        );
    }

    void WorldScene::onGmStatus(int flags) {
        m_isGm = (flags & 1) != 0; // bit 0 of the 16/200 flags: the player is a GM
        m_adminItems.setAdminMode(m_isGm && (flags & 2) != 0); // bit 1 of the 16/200 flags: admin mode is on
    }

    void WorldScene::answerGmQuestion(int answer) {
        if (answer != kUnanswered) {
            m_services.playSound("sound/wav0151.wav");
        }
        if (const core::Bytes request = protocol::gmAnswerRequest(m_gmQuestion, answer); !request.empty()) {
            m_services.connection.send(request);
        }
        if (answer == kUnanswered && m_gmQuestion != 199) { // question kind 199 is not told "Color" when unanswered
            m_messages.show("Color", 1200); // message display time in ms
        }
        m_gmQuestion = 0;
    }

}
