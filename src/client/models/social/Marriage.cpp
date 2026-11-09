#include "client/models/social/Marriage.h"

#include "core/ByteReader.h"
#include "protocol/ItemRequests.h"
#include "protocol/MarriageRequests.h"
#include "protocol/MessageId.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kLongMs = 3000; // long message / speech time, ms
        constexpr int kMarryFee = 60000; // marriage fee in gold
        constexpr int kDivorceFee = 200000; // divorce fee in gold
        const char* const kThreeDays =
            "If you haven't finish the marriage in 3 days after you applied to get married, your marriage application will be cancelled";
        const char* const kFromNow = "From now on, we will help each other, no matter what difficulties we will encounter. We will love each other forever.";
        constexpr int kGroomRing = 25610; // Diamond Band
        constexpr int kBrideRing = 25609; // Wedding Band
        constexpr int kDivorceItem = 34196; // Divorce Seal

    }

    Marriage::Marriage(
        const formats::ItemTable& items,
        Hooks hooks
    )
        : m_items(items),
          m_hooks(std::move(hooks)) {}

    std::string Marriage::playerNameOf(std::int32_t id) const {
        if (id == m_hooks.ownId()) {
            return {};
        }
        const auto mate = m_hooks.player(id);
        return mate ? mate->name : std::string();
    }

    void Marriage::acceptMarriage() {
        m_hooks.send(protocol::marryAnswerRequest(m_hooks.ownId(), m_partnerId, true));
        m_accepted = true;
    }

    void Marriage::cancelMarriage() {
        m_hooks.send(protocol::marryAnswerRequest(m_hooks.ownId(), m_partnerId, false));
        m_hooks.eventMoveOn();
        m_hooks.closeApplication();
    }

    void Marriage::acceptDivorce() {
        m_hooks.send(protocol::divorceAnswerRequest(m_hooks.ownId(), m_partnerId, true));
        m_accepted = true;
    }

    void Marriage::cancelDivorce() {
        m_hooks.send(protocol::divorceAnswerRequest(m_hooks.ownId(), m_partnerId, false));
        m_hooks.eventMoveOn();
        m_hooks.closeDivorce();
    }

    void Marriage::agreeToDivorce() { m_hooks.send(protocol::divorceConsentRequest(true)); }

    void Marriage::refuseDivorce() {
        m_hooks.send(protocol::divorceConsentRequest(false));
        m_hooks.eventMoveOn();
    }

    void Marriage::askForcedDivorce(int bagSlot) {
        m_divorceItemSlot = bagSlot;
        const auto item = m_items.find(kDivorceItem);
        m_hooks.ask(
            "Are you sure that you want to use#R" + (item ? item->name : std::string()) + "/#R The violence divorce?",
            [this] {
                if (m_divorceItemSlot > 0 && m_divorceItemSlot <= 50) { // bag places 1..50
                    m_hooks.send(protocol::forcedDivorceRequest(m_divorceItemSlot));
                }
                m_divorceItemSlot = 0;
            },
            [this] { m_divorceItemSlot = 0; }
        );
    }

    void Marriage::requestCard() { m_hooks.send(protocol::spouseCardRequest()); }

    void Marriage::receive(const core::Bytes& payload) {
        switch (protocol::MessageId::of(payload).sub) {
            case 1: receiveApplication(payload, false); break; // 45/1: marriage application form (RecvMarryApplication)
            case 2: receiveProposalAnswer(payload, " would marry you"); break; // 45/2: a player would marry you (RecvMarryAccepted)
            case 3: // 45/3: spouse card (RecvSpouseInfo)
            case 25: receiveCard(payload); break; // 45/25: spouse card (RecvSpouseInfo)
            case 4: receiveEquipment(payload); break; // 45/4: spouse equipment change (RecvSpouseEquip)
            case 5: { // 45/5: spouse nickname (RecvSpouseName)
                core::ByteReader reader(payload, 6); // skips the 2 message id bytes and the 4-byte player id
                const std::string nickname = reader.shortText();
                if (!reader.failed()) {
                    m_card.nickname = nickname;
                }
                break;
            }
            case 6: { // 45/6: spouse online state (RecvSpouseOnlineState)
                const int state = core::ByteReader(payload, 2).u8(); // skips the 2 message id bytes
                if (state <= 3) { // online states 0..3
                    m_card.onlineState = state;
                }
                break;
            }
            case 7: { // 45/7: spouse guild name (RecvSpouseGuildName)
                core::ByteReader reader(payload, 2); // skips the 2 message id bytes
                const std::string guild = reader.shortText();
                if (!reader.failed()) {
                    m_card.guild = guild;
                }
                break;
            }
            case 8: { // 45/8: spouse STR, CON, INT, WIS, AGI (RecvSpouseAttrs)
                core::ByteReader reader(payload, 2); // skips the 2 message id bytes
                const int strength = reader.u16();
                const int constitution = reader.u16();
                const int intelligence = reader.u16();
                const int wisdom = reader.u16();
                const int agility = reader.u16();
                if (!reader.failed()) {
                    m_card.strength = strength;
                    m_card.constitution = constitution;
                    m_card.intelligence = intelligence;
                    m_card.wisdom = wisdom;
                    m_card.agility = agility;
                }
                break;
            }
            case 9: { // 45/9: spouse id (RecvSpouseId)
                core::ByteReader reader(payload, 2); // skips the 2 message id bytes
                const std::int32_t id = reader.i32();
                if (!reader.failed()) {
                    m_spouseId = id;
                }
                break;
            }
            case 10: receiveLocation(payload); break; // 45/10: spouse location (RecvSpouseLocation)
            case 11: // 45/11: a player refuses to marry you (RecvMarryRefused)
            case 17: receiveProposalAnswer(payload, " Refuse to marry you"); break; // 45/17: a player refuses to marry you (ShowMarryRefusal)
            case 12: { // 45/12: play an action of the own player
                const int action = core::ByteReader(payload, 2).u8(); // skips the 2 message id bytes
                m_hooks.playAction(action);
                break;
            }
            case 13: { // 45/13: put a player at a place (RecvSetHumanPos)
                core::ByteReader reader(payload, 2); // skips the 2 message id bytes
                const std::int32_t id = reader.i32();
                reader.u16();
                const int x = reader.u16();
                const int y = reader.u16();
                if (!reader.failed()) {
                    m_hooks.setPosition(id, {x, y});
                }
                break;
            }
            case 14: // 45/14: player flag on, his pet is hidden (RecvPlayerFlagOn)
            case 15: { // 45/15: player flag off, his pet is shown (RecvPlayerFlagOff)
                core::ByteReader reader(payload, 2); // skips the 2 message id bytes
                const std::int32_t id = reader.i32();
                if (!reader.failed()) {
                    m_hooks.hidePet(id, protocol::MessageId::of(payload).sub == 14); // sub 14: flag on
                }
                break;
            }
            case 16: receiveDivorceRequest(payload); break; // 45/16: the spouse requests a divorce (RecvDivorceRequest)
            case 18: say("Both agree to get divorced"); break; // 45/18: both agreed to the divorce
            case 19: receiveCooldown(payload); break; // 45/19: marriage cooldown notice (RecvMarryCooldown)
            case 20: // 45/20: marriage result, then the application form closes (RecvMarryResult)
                receiveResult(payload);
                m_hooks.closeApplication();
                break;
            case 21: receiveWeddingStep(payload); break; // 45/21: wedding step (RecvWeddingStep)
            case 22: { // 45/22: lock or unlock the input (u8 locked)
                const int locked = core::ByteReader(payload, 2).u8(); // skips the 2 message id bytes
                m_hooks.lockInput(locked != 0);
                break;
            }
            case 23: receiveApplication(payload, true); break; // 45/23: divorce application form (RecvDivorceApplication)
            case 24: // 45/24: divorce result, then the divorce form closes (RecvDivorceResult)
                receiveDivorceResult(payload);
                m_hooks.closeDivorce();
                break;
            case 26: say("You can't team up in the scene"); break; // 45/26: notice, no teaming up in the scene
            case 27: say("Only host and host's spouse can enter"); break; // 45/27: notice, only host and spouse can enter
            case 28: say("The scene can't be discarded"); break; // 45/28: notice, the scene can't be discarded
            case 29: // 45/29: notice, too many furniture
                say("Too many funiture, please reserve enough space, you will get later.");
                m_hooks.chat("Too many funiture, please reserve enough space, you will get later.");
                break;
            default: break; // other sub ids are ignored
        }
    }

    void Marriage::receiveApplication(const core::Bytes& payload, bool divorce) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const std::int32_t id = reader.i32();
        const int flag = divorce ? reader.u8() : 0;
        if (reader.failed()) {
            return;
        }
        const auto partner = m_hooks.player(id);
        if (!partner) {
            return;
        }
        m_partnerId = id;
        m_partner = partner;
        m_applicant = m_hooks.player(m_hooks.ownId());
        m_accepted = false;
        if (divorce) {
            if (flag == 1) { // flag 1: the spouse applied
                m_hooks.message("Spouse applied to get divorced", kLongMs);
            }
            m_hooks.openDivorce();
        } else {
            m_hooks.openApplication();
        }
    }

    void Marriage::receiveResult(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const int code = reader.u8();
        const std::size_t length = payload.size() - 1;
        const std::int32_t own = m_hooks.ownId();
        std::string name1;
        std::string name2;
        int bodyType = 0;
        std::int32_t first = 0;
        std::int32_t second = 0;
        // length 6: one player id follows; 10 and more: two ids follow (payload bytes)
        if (length == 6 || length >= 10) {
            first = reader.i32();
            const auto self = m_hooks.player(own);
            if (length == 6) {
                if (first != own) {
                    name1 = "Spouse ";
                    if (const auto mate = m_hooks.player(first)) {
                        bodyType = mate->look.bodyType;
                    }
                } else if (self) {
                    bodyType = self->look.bodyType;
                }
            } else {
                second = reader.i32();
                name1 = first != own ? playerNameOf(first) : (self ? self->name : std::string());
                name2 = second != own ? playerNameOf(first) : (self ? self->name : std::string());
            }
        }
        switch (code) {
            case 1: say(name1 + " Marriage Application require your  level to be 30+"); break; // result 1: level must be 30+
            case 2: say("Applicants should be those who have never got married"); break; // result 2: applicants must be unmarried
            case 3: say("Has already apply to get married before"); break; // result 3: already applied
            case 4: say("Please wait,Application capacity is full "); break; // result 4: application capacity full
            case 5: say(name1 + " Cancel marriage application"); break; // result 5: application cancelled
            case 6: say(name1 + " don't have enough money"); break; // result 6: not enough money
            case 7: { // result 7: marriage fee deducted (i32 gold left)
                m_hooks.setGold(static_cast<std::int32_t>(core::ByteReader(payload, 2 + 1).i32())); // skips the 2 message id bytes and the result code
                const std::string fee = "Marriage fee " + std::to_string(kMarryFee) + " will be deducted.";
                say(fee);
                m_hooks.chat(fee);
                break;
            }
            case 8: { // result 8: applied, text with the deadline
                core::ByteReader deadline(payload, 3); // skips the 2 message id bytes and the result code
                const std::string text = deadline.shortText();
                say("Applied successfully");
                m_hooks.chat("The appication will be cancelled if the wedding hasn't been held before " + text);
                break;
            }
            case 9: say("Couple who want to get married can't be the same gender"); break; // result 9: same gender
            case 10: say("Too many people applied"); break; // result 10: too many applicants
            case 11: // result 11: the wedding is over
                if (m_wedding[0] != 0 && m_wedding[1] != 0) {
                    m_wedding = {};
                    say("The wedding is over");
                    m_hooks.chat("The wedding is over");
                }
                break;
            case 12: say("Has applied to get married"); break; // result 12: has applied
            case 13: say(name1 + " Need to take necessary equipment of groom"); break; // result 13: groom's equipment needed
            case 14: // result 14: spouse offline for 30 days, divorce needs permission
                say("Your spouse hasn't been online for 30 days, you can get divorced after obtaining spoues' permission.");
                break;
            case 15: say("Can't get married, because no one apply to get married."); break; // result 15: no one applied
            case 16: say("Member can't apply to get married"); break; // result 16: a team member can't apply
            case 17: say("Can't change equipment during the wedding"); break; // result 17: no equipment change during the wedding
            case 18: say("No reponse, cancel marriage application"); break; // result 18: no response, application cancelled
            case 19: // result 19: the ceremony started (two ids, then the place text)
                if (!ceremonyOf(first, second)) {
                    const std::size_t at = 10; // offset of the place text (original Copy(AData, 10, ...))
                    const std::string place =
                        payload.size() > at ? std::string(payload.begin() + static_cast<std::ptrdiff_t>(at), payload.end()) : std::string();
                    m_wedding = {first, second};
                    m_hooks.chat(name1 + " And " + name2 + " is " + place + " The ceremony started ");
                }
                break;
            case 20: say(name1 + " Should to take necessary equipment of Bride"); break; // result 20: bride's equipment needed
            case 21: { // result 21: play the wedding music (u8 song)
                const bool song = core::ByteReader(payload, 2 + 1).u8() != 0; // skips the 2 message id bytes and the result code
                m_hooks.playMusic(song ? "BGM0041" : "BGM0037");
                break;
            }
            case 22: // result 22: wedding flag (RecvWeddingFlag)
                m_hooks.weddingFlag(core::ByteReader(payload, 2 + 1).u8() != 0); // skips the 2 message id bytes and the result code
                break;
            case 23: { // result 23: the groom or bride is handed an item to wear (SetEquipSlotFromRec)
                core::ByteReader record(payload, 7); // skips the message id, the result code and the 4-byte id
                protocol::ItemInstance item;
                item.itemId = record.u16();
                item.wear = record.u8();
                record.skip(5); // five bytes of the record that are not kept
                item.byteA = record.u8();
                item.byteB = record.u8();
                item.insertItemId = record.u16();
                item.expiry = record.f64();
                item.fusionLevel = record.u8();
                item.attributeAdjust = record.u8();
                item.lookItemId = record.u16();
                if (!record.failed() && item.itemId != 0) {
                    item.count = 1;
                    m_hooks.equip(item);
                }
                break;
            }
            case 24: say(name1 + " you can't get married until seven days after the divorce."); break; // result 24: seven days after a divorce
            case 25: say("The wedding is going on..."); break; // result 25: a wedding is going on
            case 26: say("Must have memebers who have applied to get married"); break; // result 26: members must have applied
            case 27: { // result 27: the ring is missing
                const auto ring = m_items.find(bodyType % 2 == 1 ? kGroomRing : kBrideRing); // odd body type: groom
                say(name1 + " Has no " + "#R" + (ring ? ring->name : std::string()) + "/#R");
                break;
            }
            case 28: { // result 28: divorce fee deducted (i32 gold left)
                m_hooks.setGold(static_cast<std::int32_t>(core::ByteReader(payload, 2 + 1).i32())); // skips the 2 message id bytes and the result code
                const std::string fee = "Divorce fee " + std::to_string(kDivorceFee) + " will be deducted";
                say(fee);
                m_hooks.chat(fee);
                break;
            }
            case 29: // result 29: the ceremony started (groom's and bride's names, then the place)
                if (!ceremonyOf(first, second)) {
                    core::ByteReader names(payload, 11); // skips the message id, the result code and the two 4-byte ids
                    const std::string groom = names.shortText();
                    const std::string bride = names.shortText();
                    const std::string place = names.rest();
                    m_wedding = {first, second};
                    m_hooks.chat(groom + " And " + bride + " is " + place + " The ceremony started ");
                }
                break;
            case 30: // result 30: the application is cancelled after 3 days
                say(kThreeDays);
                m_hooks.chat(kThreeDays);
                break;
            case 31: say("You should be in the same stream with your spouse, you can get divorced."); break; // result 31: same stream needed
            case 32: say(name1 + " Body-shifting can't be applied"); break; // result 32: body-shifting can't be applied
            default: break; // other result codes are ignored
        }
    }

    bool Marriage::ceremonyOf(std::int32_t first, std::int32_t second) const {
        return m_wedding[0] != 0 && m_wedding[1] != 0 && (m_wedding[0] == first || m_wedding[0] == second || m_wedding[1] == first || m_wedding[1] == second);
    }

    void Marriage::receiveProposalAnswer(const core::Bytes& payload, const char* suffix) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const std::int32_t id = reader.i32();
        const auto mate = reader.failed() ? std::nullopt : m_hooks.player(id);
        if (mate) {
            say(mate->name + suffix);
        }
    }

    void Marriage::receiveDivorceRequest(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const std::int32_t id = reader.i32();
        const auto mate = reader.failed() ? std::nullopt : m_hooks.player(id);
        if (mate) {
            m_hooks.ask("Would you marry " + mate->name + " Want to get divorced", [this] { agreeToDivorce(); }, [this] { refuseDivorce(); });
        }
    }

    void Marriage::receiveCooldown(const core::Bytes& payload) {
        switch (core::ByteReader(payload, 2).u8()) { // skips the 2 message id bytes
            case 1: say("Can't get married, more than 7 days after divorce are needed."); break; // cooldown 1: 7 days after a divorce are needed
            case 2: say("Can't get married, more than 7 days after divorce are needed"); break; // cooldown 2: 7 days after a divorce are needed
            default: break; // other codes are ignored
        }
    }

    void Marriage::receiveWeddingStep(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const int step = reader.u8();
        const std::int32_t own = m_hooks.ownId();
        std::array<std::int32_t, 2> turned{};
        if (payload.size() - 1 > 2) { // more than the id and step bytes: the groom's and bride's ids follow
            const std::int32_t ids[2] = {reader.i32(), reader.i32()};
            const std::array<std::reference_wrapper<std::optional<TeamMate>>, 2> people{m_groom, m_bride};
            for (int i = 0; i < 2; ++i) {
                if (const auto mate = m_hooks.player(ids[i])) {
                    people[static_cast<std::size_t>(i)].get() = mate;
                    if (ids[i] != own) {
                        turned[static_cast<std::size_t>(i)] = ids[i];
                    }
                }
            }
        }
        constexpr int kFacingPriest = 7;
        constexpr int kGroomFaces = 1;
        constexpr int kBrideFaces = 5;
        if (step >= 1 && step <= 13) { // steps 1..13: before the vows
            for (const std::int32_t id : turned) {
                if (id != 0) {
                    m_hooks.turn(id, kFacingPriest);
                }
            }
        } else if (step >= 13 && step <= 21) { // steps 13..21: the vows and after
            if (turned[0] != 0) {
                m_hooks.turn(turned[0], kGroomFaces);
            }
            if (turned[1] != 0) {
                m_hooks.turn(turned[1], kBrideFaces);
            }
        }
        const std::string groom = m_groom ? m_groom->name : std::string();
        const std::string bride = m_bride ? m_bride->name : std::string();
        constexpr int kShortMs = 2000; // time of a short ceremony line, ms
        constexpr int kSlowMs = 3500; // time of a long ceremony line, ms
        const auto priest = [&](const std::string& text, int ms, bool indent = true) { m_hooks.speak(text, std::nullopt, ms, indent); };
        const auto groomSays = [&](const std::string& text, int ms) { m_hooks.speak(text, m_groom, ms, true); };
        const auto brideSays = [&](const std::string& text, int ms) { m_hooks.speak(text, m_bride, ms, true); };
        switch (step) {
            case 1:
                priest(
                    groom +
                        "\xA1"
                        "B" +
                        bride + " You will get married in front of your relatives and friends",
                    kSlowMs
                );
                break; // step 1: priest opens
            case 2: priest("You should be responsible for your marriage.", kShortMs); break; // step 2: priest
            case 3: priest("You should love each other forever", kShortMs); break; // step 3: priest
            case 4: priest("On behalf of the God, please present your heart.", kSlowMs); break; // step 4: priest
            case 5: priest(groom + " Mr, Are you willing to get married?", kShortMs); break; // step 5: priest asks the groom
            case 6: groomSays("Yes", kShortMs); break; // step 6: groom answers
            case 7: priest(bride + " Miss, are you willing to get married?", kShortMs, false); break; // step 7: priest asks the bride
            case 8: brideSays("Yes", kShortMs); break; // step 8: bride answers
            case 9: priest("Will both of you love each other forever?", kSlowMs); break; // step 9: priest asks both
            case 10: groomSays("I will", kShortMs); break; // step 10: groom answers
            case 11: brideSays("I will", kShortMs); break; // step 11: bride answers
            case 12: priest("You should take an oath in front of the God", kSlowMs); break; // step 12: priest, the oath
            case 14: groomSays("I feel so happy that I can give you this ring, and choose you as my wife", kLongMs); break; // step 14: groom's vow
            case 15: groomSays(kFromNow, kLongMs); break; // step 15: groom's vow
            case 16: brideSays("I feel so happy that I could give you this ring , and choose you as my husband", kLongMs); break; // step 16: bride's vow
            case 17: brideSays(kFromNow, kLongMs); break; // step 17: bride's vow
            case 18: priest("The God wish you a long happiness", kShortMs); break; // step 18: priest
            case 19: priest(groom + " You can kiss your bride now", kShortMs); break; // step 19: priest, the kiss
            case 21: priest("The wedding is over! Now " + groom + " And " + bride + " get married!", kShortMs); break; // step 21: the wedding is over
            case 23: m_hooks.chat("Congratulations " + groom + " And " + bride + " get married!"); break; // step 23: congratulations in the chat
            case 24: priest("Clear the wedding place for 2 minutes", kShortMs); break; // step 24: priest, clear the place
            case 25: priest("You can apply to obtain the spot", kShortMs); break; // step 25: priest
            case 99: priest("Wedding interrupted", kShortMs); break; // step 99: the wedding is interrupted
            default: break; // other steps say nothing
        }
    }

    void Marriage::receiveDivorceResult(const core::Bytes& payload) {
        switch (core::ByteReader(payload, 2).u8()) { // skips the 2 message id bytes
            case 1: say("Too many people get divorced"); break; // divorce result 1: too many divorces
            case 2: // divorce result 2: divorced
                say("Got divorced successfully");
                m_hooks.chat("Got divorced successfully");
                break;
            case 3: say("One wants to get divorced"); break; // divorce result 3: one wants to get divorced
            case 4: // divorce result 4: spouse offline for 30 days, permission needed
                say("Your spouse hasn't been online for 30 days, you can get divorced after obtaining spoues' permission.");
                break;
            case 5: say("Cancel divorce application"); break; // divorce result 5: application cancelled
            case 6: say("No response, cancel divorce application"); break; // divorce result 6: no response
            case 7: say("Has no spouse, so can't get divorced"); break; // divorce result 7: no spouse
            case 8: // divorce result 8: the spouse got divorced
                say("Spouse has get divorced");
                m_hooks.chat("Spouse has get divorced");
                break;
            case 9: m_spouseId = 0; break; // divorce result 9: forget the spouse
            default: break; // other codes are ignored
        }
    }

    void Marriage::dress() { m_card.look = CharacterLook::dressed(m_card.look.bodyType, m_card.look.head, m_card.look.basePattern, m_card.equipment, m_items); }

    void Marriage::receiveCard(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        SpouseCard card;
        card.id = reader.i32();
        const int bodyType = reader.u8();
        reader.u8();
        const int head = reader.u8();
        reader.u8();
        card.element = reader.u8();
        for (std::uint16_t& item : card.equipment) {
            item = reader.u16();
        }
        const std::int32_t code1 = reader.i32();
        const std::int32_t code5 = reader.i32();
        reader.i32();
        reader.u16();
        card.strength = reader.u16();
        card.constitution = reader.u16();
        card.intelligence = reader.u16();
        card.wisdom = reader.u16();
        card.agility = reader.u16();
        card.level = reader.u8();
        reader.i32();
        reader.u8();
        card.job = reader.u8();
        const int mapId = reader.u16();
        const int nameLength = reader.i32();
        card.name = reader.text(static_cast<std::size_t>(std::max(nameLength, 0)));
        card.guild = reader.text(static_cast<std::size_t>(std::max(static_cast<int>(reader.i32()), 0)));
        card.nickname = reader.text(static_cast<std::size_t>(std::max(static_cast<int>(reader.i32()), 0)));
        const std::string place = reader.shortText();
        if (reader.failed()) {
            return;
        }
        card.look = CharacterLook::fromColorCodes(bodyType, head, code1, code5, m_items);
        card.location = place + " " + m_hooks.sceneName(mapId);
        card.onlineState = m_card.onlineState;
        m_card = card;
        m_spouseId = card.id;
        dress();
    }

    void Marriage::receiveEquipment(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const int sub = reader.u8();
        reader.skip(4); // 4 bytes not read
        const int itemId = reader.u16();
        if (reader.failed()) {
            return;
        }
        if (const auto item = m_items.find(itemId); item && item->equipSlot >= 1 && item->equipSlot <= 6) { // the six equipment slots
            m_card.equipment[static_cast<std::size_t>(item->equipSlot - 1)] = sub == 1 ? 0 : static_cast<std::uint16_t>(itemId);
            dress();
        }
    }

    void Marriage::receiveLocation(const core::Bytes& payload) {
        core::ByteReader reader(payload, 6); // skips the 2 message id bytes and the 4-byte player id
        const int length = reader.i32();
        const std::string place = reader.text(static_cast<std::size_t>(std::max(length, 0)));
        const int mapId = reader.u16();
        if (!reader.failed()) {
            m_card.location = place + " " + m_hooks.sceneName(mapId);
        }
    }

}
