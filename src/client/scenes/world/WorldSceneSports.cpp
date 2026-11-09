#include "client/scenes/world/WorldScene.h"

#include "client/rules/CharacterActions.h"
#include "client/rules/Walking.h"
#include "core/ByteReader.h"
#include "protocol/MallMessages.h"
#include "protocol/MessageId.h"

namespace client {

    namespace {

        // sport (mini-game) kind numbers, as sent by the server
        constexpr int kFortuneEgg = 6; // sport kind 6: fortune egg game
        constexpr int kThrow = 1; // sport kind 1: throwing game
        constexpr int kHitMouse = 3; // sport kind 3: whack-a-mouse game
        constexpr int kHunter = 4; // sport kind 4: hunter game
        constexpr int kSheep = 5; // sport kind 5: sheep game
        constexpr int kClaw = 7; // sport kind 7: claw machine (dolls)
        constexpr int kBoxing = 9; // sport kind 9: boxing game
        constexpr int kLucky = 13; // sport kind 13: lucky game
        constexpr int kMemoryGame = 15; // sport kind 15: memory card game
        constexpr int kLuckyCards = 16; // sport kind 16: lucky cards
        constexpr int kGobang = 18; // sport kind 18: gobang (five in a row)
        constexpr int kLuckyWheel = 21; // sport kind 21: lucky wheel

        /// @brief Tells whether a sport kind is one of the slot machines of the game hall.
        /// @param kind The sport kind.
        /// @return True for kinds 8, 10 and 19.
        bool isSlotMachine(int kind) { return kind == 8 || kind == 10 || kind == 19; } // sport kinds of the slot machines

        /// @brief Tells whether a sport kind is one of the jigsaw puzzle games.
        /// @param kind The sport kind.
        /// @return True for kinds 11, 12 and 14.
        bool isJigsaw(int kind) { return kind == 11 || kind == 12 || kind == 14; } // sport kinds of the jigsaw games

        constexpr std::size_t kRequestBoxWrap = 32; // characters per line in the request-style talk box
        constexpr std::size_t kMainBoxWrap = 40; // characters per line in the main talk box
        constexpr std::size_t kCompassWrap = 64; // characters per line in the compass quiz box

    }

    LuckyWheel::Hooks WorldScene::luckyWheelHooks() {
        LuckyWheel::Hooks hooks;
        static_cast<GameHooks&>(hooks) = sportHooks();
        return hooks;
    }

    GameHooks WorldScene::sportHooks() {
        GameHooks hooks;
        hooks.send = [this](const core::Bytes& bytes) { m_services.connection.send(bytes); };
        hooks.imPoints = [this] { return static_cast<int>(m_mallShop.points); };
        hooks.countItem = [this](int itemId) {
            int total = 0;
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                if (m_inventory.bagItem(slot).itemId == itemId) {
                    total += m_inventory.bagItem(slot).count;
                }
            }
            return total;
        };
        hooks.freeBagPlaces = [this] {
            int free = 0;
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                if (m_inventory.bagItem(slot).empty()) {
                    ++free;
                }
            }
            return free;
        };
        hooks.freePlay = [this] { return m_newYear.freePlay(); };
        hooks.itemName = [this](int itemId) {
            const auto record = m_services.items.find(itemId);
            return record ? record->name : std::string();
        };
        hooks.freeItemPlace = [this](int itemId) {
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                if (m_inventory.bagItem(slot).itemId == itemId && m_bagMarks.isAvailable(slot) && m_bagMarks.useMark(slot) == 0) {
                    return slot;
                }
            }
            return 0;
        };
        hooks.itemInPlace = [this](int place) { return place >= 1 && place <= Inventory::kBagSlots ? m_inventory.bagItem(place).itemId : 0; };
        hooks.message = [this](const std::string& text, int durationMs) { m_messages.show(text, durationMs); };
        hooks.requestMessage = [this](const std::string& text, int durationMs) { m_centreMessages.show(text, durationMs); };
        hooks.ask = [this](const std::string& text, bool mainBox, std::function<void()> onOk, std::function<void()> onCancel) {
            TalkDialog::Timing timing;
            timing.wrapChars = mainBox ? kMainBoxWrap : kRequestBoxWrap;
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), std::move(onCancel), std::move(timing));
        };
        hooks.choose = [this](const std::string& text, const std::vector<std::string>& choices, std::function<void(int)> chosen) {
            TalkDialog::Timing timing;
            timing.wrapChars = kRequestBoxWrap;
            timing.exclusive = false;
            m_talk.choose(text, TalkDialog::Speaker{m_player.look, m_player.name}, choices, std::move(chosen), [] {}, std::move(timing));
        };
        hooks.playSound = [this](const std::string& file) { m_services.playSound(file); };
        hooks.stopSounds = [this] { m_services.sounds.stopAll(); };
        hooks.playMusic = [this](const std::string& name) {
            const std::filesystem::path file = m_services.game.data("Sound/" + name + ".wav");
            if (std::filesystem::exists(file)) {
                m_services.music.playLooped(file);
            }
        };
        hooks.stopMusic = [this] { m_services.music.stop(); };
        hooks.finishGame = [this](int result) {
            endSportGame(result);
            if (const auto scene = sportScene(m_activeSport)) {
                scene->get().close();
                m_gameStart.close();
            }
            if (m_activeSport == kMemoryGame) {
                m_memory.close();
                m_gameStart.close();
            } else if (m_activeSport == kHitMouse) {
                m_hitMouse.close();
                m_gameStart.close();
            } else if (m_activeSport == kLucky) {
                m_lucky.close();
                m_gameStart.close();
            } else if (m_activeSport == kHunter) {
                m_hunter.close();
                m_gameStart.close();
            } else if (m_activeSport == kSheep) {
                m_sheep.close();
                m_gameStart.close();
            } else if (m_activeSport == kThrow) {
                m_throw.close();
                m_gameStart.close();
            }
        };
        hooks.endGame = [this] {
            endSportGame(0);
            if (const auto scene = sportScene(m_activeSport)) {
                scene->get().close();
                m_gameStart.close();
            }
            if (m_activeSport == kThrow) {
                m_throw.close();
                m_gameStart.close();
            } else if (m_activeSport == kSheep) {
                m_sheep.close();
                m_gameStart.close();
            } else if (m_activeSport == kHunter) {
                m_hunter.close();
                m_gameStart.close();
            } else if (m_activeSport == kLucky) {
                m_lucky.close();
                m_gameStart.close();
            } else if (m_activeSport == kClaw) {
                m_dolls.close();
                m_gameStart.close();
            } else if (isJigsaw(m_activeSport)) {
                m_jigsaw.close();
                m_gameStart.close();
            } else if (m_activeSport == kBoxing) {
                m_boxing.close();
                m_gameStart.close();
            } else if (m_activeSport == kMemoryGame) {
                m_memory.close();
                m_gameStart.close();
            } else if (m_activeSport == kLuckyCards) {
                m_luckyCards.close();
                m_gameStart.close();
            } else if (m_activeSport == kFortuneEgg) {
                m_fortuneEgg.close();
                m_gameStart.close();
            } else if (isSlotMachine(m_activeSport)) {
                m_slotMachine.close();
                m_gameStart.close();
            }
        };
        return hooks;
    }

    QuizGame::Hooks WorldScene::quizHooks() {
        QuizGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = sportHooks();
        hooks.chat = [this](const std::string& text, bool byPlayer) { m_chat.add(0, byPlayer ? text : "(System Prompt):" + text, byPlayer ? m_player.id : 0); };
        hooks.playerName = [this](std::int32_t id) { return playerName(id); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.enterScreen = [this] {
            closeWindows();
            m_hotKeys.hide();
            m_mainButtons.hide();
            m_functionButtons.hide();
            m_mainStatus.hide();
            m_miniMap.setHidden(true);
            m_inputBar.show();
            m_chat.show();
            m_talk.setCovered(true);
            m_quiz.open();
            for (const char* name : {"QuizForm", "ChatWindow", "InputBar"}) {
                m_screen.context().bringToFront(m_screen.get<ui::Element>(name).id());
            }
        };
        hooks.leaveScreen = [this] {
            m_quiz.close();
            m_talk.setCovered(false);
            setHudShown(true);
        };
        return hooks;
    }

    CompassGame::Hooks WorldScene::compassHooks() {
        CompassGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = sportHooks();
        hooks.chat = [this](const std::string& text, bool byPlayer) { m_chat.add(0, byPlayer ? text : "(System Prompt):" + text, byPlayer ? m_player.id : 0); };
        hooks.playerName = [this](std::int32_t id) { return playerName(id); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.enterScreen = [this] {
            closeWindows();
            m_hotKeys.hide();
            m_functionButtons.hide();
            m_mainStatus.hide();
            m_inputBar.show();
            m_chat.show();
            m_compass.open();
        };
        hooks.leaveScreen = [this] {
            m_compass.close();
            m_talk.close();
            m_mainStatus.show();
            m_hotKeys.show();
            m_functionButtons.show();
        };
        hooks.showQuestion = [this](const std::string& question, const std::vector<std::string>& choices, std::function<int()> shownNumber) {
            TalkDialog::Timing timing;
            timing.wrapChars = kCompassWrap;
            timing.exclusive = false;
            timing.compassBox = true;
            timing.shownNumber = std::move(shownNumber);
            m_talk.choose(question, std::nullopt, choices, [](int) {}, [] {}, std::move(timing));
        };
        hooks.clearQuestion = [] {};
        hooks.showAnswer = [this](const std::string& text, int durationMs) {
            TalkDialog::Timing timing;
            timing.wrapChars = kCompassWrap;
            timing.exclusive = false;
            timing.compassBox = true;
            timing.timeoutMs = static_cast<std::uint64_t>(durationMs);
            m_talk.say(text, std::nullopt, {}, std::move(timing));
        };
        hooks.playerActs = [this](int slot) {
            startWalking(m_player, {}, m_nowMs);
            startActionSlot(m_player, slot);
        };
        return hooks;
    }

    GameStartWindow::Hooks WorldScene::gameStartHooks() {
        GameStartWindow::Hooks hooks;
        hooks.begin = [this](int kind) {
            if (const auto scene = sportScene(kind)) {
                scene->get().begin(m_nowMs);
                m_gameStart.close();
            } else if (kind == kLuckyWheel) {
                m_luckyWheel.wheel().begin(m_nowMs);
                m_gameStart.close();
            } else if (kind == kLuckyCards) {
                m_luckyCards.begin();
                m_gameStart.close();
            } else if (kind == kFortuneEgg) {
                m_fortuneEgg.begin();
                m_gameStart.close();
            } else if (isSlotMachine(kind)) {
                m_slotMachine.begin();
                m_gameStart.close();
            } else if (kind == kMemoryGame) {
                m_memory.game().begin();
                m_gameStart.close();
            } else if (kind == kHitMouse) {
                m_hitMouse.begin();
                m_gameStart.close();
            } else if (kind == kBoxing) {
                m_gameStart.close();
            } else if (isJigsaw(kind)) {
                m_jigsaw.game().begin();
                m_gameStart.close();
            } else if (kind == kClaw) {
                m_dolls.game().begin();
                m_gameStart.close();
            } else if (kind == kLucky) {
                m_gameStart.close();
            } else if (kind == kHunter) {
                m_hunter.game().begin(m_nowMs);
                m_gameStart.close();
            } else if (kind == kSheep) {
                m_sheep.begin(m_nowMs);
                m_gameStart.close();
            } else if (kind == kThrow) {
                m_throw.begin();
                m_gameStart.close();
            } else if (kind == kGobang) {
                m_gobang.begin();
                m_gameStart.close();
            }
        };
        hooks.leave = [this](int kind) {
            if (const auto scene = sportScene(kind)) {
                scene->get().leave();
                m_gameStart.close();
            } else if (kind == kLuckyWheel) {
                m_luckyWheel.wheel().exitClicked();
                m_gameStart.close();
            } else if (kind == kLuckyCards) {
                m_luckyCards.cards().leave();
                m_gameStart.close();
            } else if (kind == kFortuneEgg) {
                m_fortuneEgg.egg().leave();
                m_gameStart.close();
            } else if (isSlotMachine(kind)) {
                m_slotMachine.machine().leave();
                m_gameStart.close();
            } else if (kind == kMemoryGame) {
                m_memory.game().leave();
                m_gameStart.close();
            } else if (kind == kHitMouse) {
                m_hitMouse.game().leave();
                m_gameStart.close();
            } else if (kind == kBoxing) {
                m_boxing.game().quitClicked();
                m_gameStart.close();
            } else if (isJigsaw(kind)) {
                m_jigsaw.game().leave();
                m_gameStart.close();
            } else if (kind == kClaw) {
                m_dolls.game().leave();
                m_gameStart.close();
            } else if (kind == kLucky) {
                m_lucky.game().quitClicked();
                m_gameStart.close();
            } else if (kind == kHunter) {
                m_hunter.game().leave();
                m_gameStart.close();
            } else if (kind == kSheep) {
                m_sheep.game().leave();
                m_gameStart.close();
            } else if (kind == kThrow) {
                m_throw.game().leave();
                m_gameStart.close();
            } else if (kind == kGobang) {
                m_gobang.quit();
                m_gameStart.close();
            }
        };
        return hooks;
    }

    std::optional<std::reference_wrapper<SportScene>> WorldScene::sportScene(int kind) {
        if (kind == m_digHole.kind()) {
            return std::ref<SportScene>(m_digHole);
        }
        if (kind == m_pumpkin.kind()) {
            return std::ref<SportScene>(m_pumpkin);
        }
        if (kind == m_scotd.kind()) {
            return std::ref<SportScene>(m_scotd);
        }
        return std::nullopt;
    }

    void WorldScene::onSportStart(int kind, int npcId, int level, int scene) {
        (void)scene;
        if (const auto sport = sportScene(kind)) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kind);
            sport->get().startedWith(npcId, level);
            sport->get().open(m_nowMs);
            m_services.music.stop();
            m_activeSport = kind;
            return;
        }
        if (kind == kThrow) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kThrow);
            m_throw.open(npcId, m_nowMs);
            m_services.music.stop();
            m_activeSport = kThrow;
            return;
        }
        if (kind == kSheep) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kSheep, level == 1 ? 1 : 0); // level 1 gives start variant 1, other levels variant 0
            m_sheep.open(level, m_nowMs);
            m_services.music.stop();
            m_activeSport = kSheep;
            return;
        }
        if (kind == kHunter) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kHunter);
            m_hunter.open(m_nowMs);
            m_services.music.stop();
            m_activeSport = kHunter;
            return;
        }
        if (kind == kLucky) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kLucky);
            m_lucky.open(npcId, m_nowMs);
            m_services.music.stop();
            m_activeSport = kLucky;
            return;
        }
        if (kind == kClaw) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kClaw);
            m_dolls.open(m_nowMs);
            m_services.music.stop();
            m_activeSport = kClaw;
            return;
        }
        if (isJigsaw(kind)) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kind);
            m_jigsaw.open(kind, m_nowMs);
            m_services.music.stop();
            m_activeSport = kind;
            return;
        }
        if (kind == kBoxing) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kBoxing);
            m_boxing.open(m_nowMs);
            m_services.music.stop();
            m_activeSport = kBoxing;
            return;
        }
        if (kind == kHitMouse) {
            closeWindows();
            setHudShown(false);
            m_hitMouse.open(npcId, m_nowMs);
            m_gameStart.open(kHitMouse, static_cast<int>(m_hitMouse.game().look()));
            m_services.music.stop();
            m_activeSport = kHitMouse;
            return;
        }
        if (kind == kMemoryGame) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kMemoryGame);
            m_memory.open(m_nowMs);
            m_services.music.stop();
            m_activeSport = kMemoryGame;
            return;
        }
        if (kind == kLuckyCards) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kLuckyCards);
            m_luckyCards.open(m_nowMs);
            m_services.music.stop();
            m_activeSport = kLuckyCards;
            return;
        }
        if (kind == kFortuneEgg || isSlotMachine(kind)) {
            closeWindows();
            setHudShown(false);
            m_gameStart.open(kind);
            if (kind == kFortuneEgg) {
                m_fortuneEgg.open(m_nowMs);
            } else {
                m_slotMachine.open(kind, m_nowMs);
            }
            m_services.music.stop();
            m_activeSport = kind;
            return;
        }
        if (kind == kGobang) {
            closeWindows();
            setHudShown(false);
            m_inputBar.show();
            m_chat.show();
            m_gameStart.open(kGobang);
            m_gobang.open(m_nowMs);
            m_services.music.stop();
            const std::filesystem::path song = m_services.game.data("Sound/BGM0011.wav");
            if (std::filesystem::exists(song)) {
                m_services.music.playLooped(song);
            }
            m_activeSport = kGobang;
            return;
        }
        if (kind != kLuckyWheel) {
            return;
        }
        closeWindows();
        setHudShown(false);
        m_gameStart.open(kLuckyWheel);
        m_luckyWheel.open(m_nowMs);
        m_services.music.stop();
        m_activeSport = kLuckyWheel;
    }

    void WorldScene::endSportGame(int result) {
        m_services.connection.send({57, 1, static_cast<std::uint8_t>(result)}); // client 57/1: end of the sport game with its result
        m_eventRunner.moveOn();
    }

    void WorldScene::onSportEnd() {
        if (m_activeSport == 0) {
            return;
        }
        m_luckyWheel.close();
        m_luckyCards.close();
        m_gobang.close();
        m_fortuneEgg.close();
        m_slotMachine.close();
        m_memory.close();
        m_hitMouse.close();
        m_boxing.close();
        m_jigsaw.close();
        m_dolls.close();
        m_lucky.close();
        m_hunter.close();
        m_sheep.close();
        m_throw.close();
        m_digHole.close();
        m_pumpkin.close();
        m_scotd.close();
        m_gameStart.close();
        setHudShown(true);
        if (m_mallShop.gameAsked) {
            m_services.connection.send({57, 1, 0}); // client 57/1: end of the sport game, result 0
            m_mall.show();
            m_services.connection.send(protocol::imPointsRequest(0));
        }
        playMapMusic();
        m_activeSport = 0;
    }

    void WorldScene::onWheelMessage(const core::Bytes& payload) {
        if (m_activeSport == kLuckyWheel) {
            m_luckyWheel.wheel().receiveList(payload, m_nowMs);
        }
    }

    void WorldScene::onQuizMessage(const core::Bytes& payload) { m_quiz.game().receive(payload, m_nowMs); }

    void WorldScene::onCompassMessage(const core::Bytes& payload) { m_compass.game().receive(payload, m_nowMs); }

    void WorldScene::onPokeMessage(const core::Bytes& payload) {
        if (m_activeSport == kLuckyCards) {
            m_luckyCards.cards().receive(payload, m_nowMs);
        }
    }

    void WorldScene::onMachineMessage(const core::Bytes& payload) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (const auto scene = sportScene(id.sub); scene && id.sub == m_activeSport) {
            scene->get().receive(payload);
            return;
        }
        if (id.sub == kClaw && m_activeSport == kClaw) {
            m_dolls.game().receive(payload);
            return;
        }
        if (isJigsaw(id.sub) && id.sub == m_activeSport) {
            m_jigsaw.game().receive(payload);
            return;
        }
        if (id.sub == kBoxing && m_activeSport == kBoxing) {
            m_boxing.game().receive(payload);
            return;
        }
        if (id.sub == kLuckyCards && m_activeSport == kLuckyCards) {
            m_luckyCards.cards().receiveMachine(payload);
            return;
        }
        if (id.sub == kFortuneEgg && m_activeSport == kFortuneEgg) {
            m_fortuneEgg.egg().receive(payload);
            return;
        }
        if (isSlotMachine(id.sub) && id.sub == m_activeSport) {
            m_slotMachine.machine().receive(payload);
            return;
        }
        if (id.sub != kLuckyWheel || m_activeSport != kLuckyWheel) {
            return;
        }
        core::ByteReader reader(payload, 2); // the body starts after the 2 id bytes
        const int code = reader.u8();
        if (code == 2) { // 71/21 code 2: the spin cannot be paid
            m_luckyWheel.wheel().receiveError(payload);
        } else if (code == 3) { // 71/21 code 3: where the wheel stops
            m_luckyWheel.wheel().receiveResult(payload, m_nowMs);
        }
    }

    void WorldScene::setHudShown(bool shown) {
        if (shown) {
            m_inputBar.show();
            m_mainButtons.show();
            m_functionButtons.show();
            m_mainStatus.show();
            m_hotKeys.show();
            m_chat.show();
            m_miniMap.setHidden(false);
        } else {
            m_inputBar.hide();
            m_mainButtons.hide();
            m_functionButtons.hide();
            m_mainStatus.hide();
            m_hotKeys.hide();
            m_chat.hide();
            m_miniMap.setHidden(true);
        }
    }

}
