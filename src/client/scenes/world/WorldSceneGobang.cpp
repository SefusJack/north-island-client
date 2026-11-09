#include "client/rules/FurnitureBonus.h"
#include "client/scenes/world/WorldScene.h"
#include "core/ByteWriter.h"

#include <string>

namespace client {

    namespace {

        constexpr int kTvItem = 39051; // item 39051: TV
        constexpr int kLcdTvItem = 39113; // item 39113: LCD TV
        constexpr int kTvRadius = 2; // furniture search radius around the console, in furniture cells
        constexpr int kFiveInARow = 18; // sport/game id 18: Five in a Row (Gobang)
        constexpr int kLeastLevel = 10; // minimum player level to use the TV games
        constexpr std::size_t kQuestionWrap = 32; // characters per line in the question box

    }

    GobangWindow::Hooks WorldScene::gobangHooks() {
        GobangWindow::Hooks hooks;
        GobangGame::Hooks& game = hooks.game;
        game.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        game.notice = [this](const std::string& text, int ms) { m_centreMessages.show(text, ms); };
        game.message = [this](const std::string& text, int ms) { m_messages.show(text, ms); };
        game.ask = [this](const std::string& text, std::function<void()> onOk, std::function<void()> onCancel) {
            TalkDialog::Timing timing;
            timing.wrapChars = kQuestionWrap;
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), std::move(onCancel), std::move(timing));
        };
        game.sound = [this](const std::string& file) { m_services.playSound(file); };
        game.ownId = [this] { return m_player.id; };
        game.gold = [this] { return m_status.gold; };
        game.ownSeat = [this] { return GobangSeat{m_player.id, m_player.name, m_player.look}; };
        game.playerOf = [this](std::int32_t id) -> std::optional<GobangSeat> {
            const auto other = m_others.find(id);
            if (other == m_others.end()) {
                return std::nullopt;
            }
            return GobangSeat{other->second.id, other->second.name, other->second.look};
        };
        game.anteError = [this](int code) {
            // ante error code 1: not enough money; any other code: bet too high
            m_messages.show(code == 1 ? "You don't have enough money!" : "too much money, can't start five in a row", 1200); // message display time in ms
        };
        hooks.endSport = [this] { endSportGame(0); };
        hooks.closeInstructions = [this] { m_gameStart.close(); };
        hooks.stopMusic = [this] { m_services.music.stop(); };
        hooks.noticeTwo = [this](const std::string& text, int ms) { m_centreMessages.show(text, ms); };
        hooks.whisper = [this](std::int32_t playerId) { m_inputBar.setWhisperTarget(playerId); };
        return hooks;
    }

    void WorldScene::openTvConsole(int piece) {
        int tv = FurnitureBonus::findNear(m_house.furniture(), piece, kTvItem, kTvRadius);
        if (tv == 0) {
            tv = FurnitureBonus::findNear(m_house.furniture(), piece, kLcdTvItem, kTvRadius);
        }
        if (tv == 0) {
            m_messages.show("No TV around", 1000); // message display time in ms
            return;
        }
        TalkDialog::Timing timing;
        timing.wrapChars = kQuestionWrap;
        m_talk.choose(
            "Choose which game?",
            TalkDialog::Speaker{m_player.look, m_player.name, true},
            {"Five in a Row"},
            [this, piece, tv](int choice) {
                if (choice == 0) {
                    selectTvGame(piece, tv);
                }
            },
            [] {},
            std::move(timing)
        );
    }

    void WorldScene::selectTvGame(int consolePiece, int tvPiece) {
        if (m_status.level < kLeastLevel) {
            m_messages.show("You can't use the function, because your level is less than 10", 2000); // message display time in ms
            return;
        }
        if (!m_enoughOnlineTime) {
            m_messages.show("Can't use the function, because your online time is not enough.", 2000); // message display time in ms
            return;
        }
        core::ByteWriter request;
        // message id 62/54: the request for the TV game hall (console piece, TV item id, TV piece, game id)
        request.u8(62).u8(54).u8(static_cast<std::uint8_t>(consolePiece)).u16(kTvItem).u8(static_cast<std::uint8_t>(tvPiece)).u8(kFiveInARow);
        m_services.connection.send(request.take());
    }

    void WorldScene::onGobangMessage(const core::Bytes& payload) {
        if (m_activeSport == kFiveInARow) {
            m_gobang.game().receive(payload, m_nowMs);
        }
    }

}
