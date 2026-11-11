#ifndef NORTH_ISLAND_CLIENT_SERVICES_SERVER_EVENTS_H
#define NORTH_ISLAND_CLIENT_SERVICES_SERVER_EVENTS_H

#include "client/models/events/EventResult.h"
#include "client/services/LoginRejection.h"
#include "client/services/NameCheckResult.h"
#include "client/models/chat/SystemAnnounce.h"
#include "core/Point.h"
#include "protocol/ActivityLists.h"
#include "protocol/BagItemRecord.h"
#include "protocol/BattleAction.h"
#include "protocol/BattleStart.h"
#include "protocol/FighterCast.h"
#include "protocol/FighterWeaponLook.h"
#include "protocol/GroundItemRecord.h"
#include "protocol/GuildRecord.h"
#include "protocol/LotteryLists.h"
#include "protocol/CharacterList.h"
#include "protocol/BlacklistEntry.h"
#include "protocol/FriendRecord.h"
#include "protocol/HotKeyRecord.h"
#include "protocol/MallRow.h"
#include "protocol/FriendState.h"
#include "protocol/ItemInstance.h"
#include "protocol/NoticeContent.h"
#include "protocol/NoticePageInfo.h"
#include "protocol/NoticeRow.h"
#include "protocol/MessageId.h"
#include "protocol/OtherPlayerRecord.h"
#include "protocol/OwnAttributes.h"
#include "protocol/OwnPlayerRecord.h"
#include "protocol/OwnStall.h"
#include "protocol/ParkedVehicle.h"
#include "protocol/PetStallGoods.h"
#include "protocol/PetTradeOffer.h"
#include "protocol/SaleNetList.h"
#include "protocol/PuppetPacket.h"
#include "protocol/PetJoined.h"
#include "protocol/PetRecord.h"
#include "protocol/PetRide.h"
#include "protocol/PetsMounted.h"
#include "protocol/ScenePet.h"
#include "protocol/SceneRider.h"
#include "protocol/SceneTeam.h"
#include "protocol/SceneVehicle.h"
#include "protocol/StallGoods.h"
#include "protocol/StallSale.h"
#include "protocol/UtensilStallGoods.h"
#include "protocol/StallSignRecord.h"
#include "protocol/TradeOffer.h"
#include "protocol/ServerSelected.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief What the server's messages mean to the screens: the action handlers (Action00Kick.cpp, Action01System.cpp ...) decode a message and call one of these; a scene overrides the ones it cares about.
    /// @details Each hook names the original packet as action/sub id (server action 0 = kick, 1 = system, ...) where known.
    class ServerEvents {
        public:
            virtual ~ServerEvents() = default;

            /// @brief The server ends the session (packet 0/reason).
            /// @param reason Kick reason code.
            virtual void onKicked(int reason) { (void)reason; }

            /// @brief The chosen slot is empty; open character creation (packet 1/3).
            /// @param askCipher True when the account has no cipher yet, so the wizard asks for one.
            virtual void onCreateCharacter(bool askCipher) { (void)askCipher; }

            /// @brief The server accepted the connection; show the login form (packet 1/9).
            /// @param message The server's answer.
            virtual void onServerSelected(const protocol::ServerSelected& message) { (void)message; }

            /// @brief The login was rejected (packets 1/5, 1/6, 1/7).
            /// @param reason Why it was rejected.
            virtual void onLoginRejected(LoginRejection reason) { (void)reason; }

            /// @brief A character (packet 1/8) or the whole account (packet 1/10) is locked for some seconds.
            /// @param wholeAccount True for the whole account, false for one character.
            /// @param seconds Lock duration in seconds.
            virtual void onLocked(bool wholeAccount, std::uint32_t seconds) {
                (void)wholeAccount;
                (void)seconds;
            }

            /// @brief The server's answer to the name a new character should get (packet 9/3).
            /// @param result Outcome of the name check.
            virtual void onNameChecked(NameCheckResult result) { (void)result; }

            /// @brief The characters of the account (packet 63/1).
            /// @param list The character list.
            virtual void onCharacterList(const protocol::CharacterList& list) { (void)list; }

            /// @brief The account's login id (packet 63/2).
            /// @param loginId The id.
            virtual void onLoginId(std::uint32_t loginId) { (void)loginId; }

            /// @brief The answer to deleting a character (packet 35/2).
            /// @param result 1 when deleted.
            /// @param slot Character slot.
            virtual void onCharacterDeleted(int result, int slot) {
                (void)result;
                (void)slot;
            }

            /// @brief Enter the world (packet 3 with the own id).
            /// @param record The own player's record.
            virtual void onOwnPlayerEntered(const protocol::OwnPlayerRecord& record) { (void)record; }

            /// @brief The server has sent what the map needs; show it (packet 1/11).
            virtual void onEnterWorldFinished() {}

            /// @brief The own player is in the world: players are drawn and may walk (packet 5/4).
            virtual void onWorldReady() {}

            /// @brief A player walks to a map point (packets 6/1, with the direction to face on arrival, and 6/3).
            /// @param playerId The walking player.
            /// @param to Target map point.
            /// @param direction Direction to face on arrival, if the packet gave one.
            virtual void onPlayerWalks(std::int32_t playerId, core::Point to, std::optional<int> direction) {
                (void)playerId;
                (void)to;
                (void)direction;
            }

            /// @brief A map NPC (eve.Emg index) walks to / stands at a map point (packets 22/2 for walk, 22/5).
            /// @param npcIndex The NPC's place number on the map.
            /// @param to Target map point.
            /// @param walk True to walk there, false to stand there.
            virtual void onNpcMoves(int npcIndex, core::Point to, bool walk) {
                (void)npcIndex;
                (void)to;
                (void)walk;
            }

            /// @brief A map scene object shows one frame still (packet 22/1).
            /// @param npcIndex The NPC's place number on the map.
            /// @param frame Frame to hold; 255 animates again.
            virtual void onNpcFrame(int npcIndex, int frame) {
                (void)npcIndex;
                (void)frame;
            }

            /// @brief The state of a map NPC (packet 22/4, RevMapNpcStateList, one 14-byte record).
            /// @param npcIndex The NPC's place number on the map.
            /// @param holdFrame Frame a scene object holds.
            /// @param at Where it stands.
            /// @param showState 1 shown, 2 hidden.
            /// @param hideMs Milliseconds to hide it for (0: not).
            /// @param action The action it plays.
            virtual void onNpcState(
                int npcIndex,
                int holdFrame,
                core::Point at,
                int showState,
                std::uint32_t hideMs,
                int action
            ) {
                (void)npcIndex, (void)holdFrame, (void)at, (void)showState, (void)hideMs, (void)action;
            }

            /// @brief The sign of a map NPC, door or mine on the small map (packet 22/12).
            /// @param kind 1 map NPC, 2 door, 3 mine.
            /// @param index Index of the thing.
            /// @param sign Sign to show.
            virtual void onMapSign(int kind, int index, int sign) {
                (void)kind;
                (void)index;
                (void)sign;
            }

            /// @brief The show state of a map NPC (packet 22/6).
            /// @param npcIndex The NPC's place number on the map.
            /// @param state 1 shown, 2 hidden, 3 shown for a while, 4 hidden for a while.
            virtual void onNpcShowState(int npcIndex, int state) {
                (void)npcIndex;
                (void)state;
            }

            /// @brief A map NPC is hidden for some seconds (packets 22/3, 22/10).
            /// @param npcIndex The NPC's place number on the map.
            /// @param seconds Seconds hidden; 65535 for good.
            virtual void onNpcHidden(int npcIndex, int seconds) {
                (void)npcIndex;
                (void)seconds;
            }

            /// @brief A map NPC is shown for some seconds (packet 22/11).
            /// @param npcIndex The NPC's place number on the map.
            /// @param seconds Seconds shown.
            virtual void onNpcRevealed(int npcIndex, int seconds) {
                (void)npcIndex;
                (void)seconds;
            }

            /// @brief The room of a house a player comes to, before the map change itself (packet 12).
            /// @param playerId The player.
            /// @param room 0 floor 1, 1 floor 2, 2 and up its own rooms.
            /// @param sizesGiven True when the packet named the floor room sizes.
            /// @param sizes Sizes of the floor rooms the packet names (index 0..4).
            virtual void onHouseRoom(std::int32_t playerId, int room, bool sizesGiven, const std::array<int, 5>& sizes) {
                (void)playerId;
                (void)room;
                (void)sizesGiven;
                (void)sizes;
            }

            /// @brief A player goes to another map; for the own player, load it (packet 12).
            /// @param playerId The player.
            /// @param mapId Target map.
            /// @param position Position on the target map.
            virtual void onMapChanged(std::int32_t playerId, int mapId, core::Point position) {
                (void)playerId;
                (void)mapId;
                (void)position;
            }

            /// @brief A result of the running map event, by its kind (packets 20/1..6); 20/8 is the event being over.
            /// @param result The event result.
            virtual void onEventResult(const EventResult& result) { (void)result; }

            /// @brief An NPC shows an emotion (packet 22/7).
            /// @param npcIndex The NPC's place number on the map.
            /// @param emotionId The emotion.
            virtual void onNpcEmotion(int npcIndex, int emotionId) {
                (void)npcIndex;
                (void)emotionId;
            }

            /// @brief An NPC plays an action (packets 22/8, 22/9).
            /// @param npcIndex The NPC's place number on the map.
            /// @param action The action.
            /// @param holdFrame Frame the NPC holds in it; 255 means it plays.
            /// @param needsFrames True for 22/8, which plays only an action the sprite has frames of.
            virtual void onNpcAction(int npcIndex, int action, int holdFrame, bool needsFrames) {
                (void)npcIndex;
                (void)action;
                (void)holdFrame;
                (void)needsFrames;
            }

            /// @brief The server lets the event go on, waits or tells something (packets 20/7, 20/9..20/19).
            /// @param sub The packet's sub id.
            /// @param value The packet's value.
            virtual void onEventSignal(int sub, std::int32_t value) {
                (void)sub;
                (void)value;
            }

            /// @brief The running map event is over.
            virtual void onEventFinished() {}

            /// @brief How a purchase from an NPC store went (packet 27/1).
            /// @param result 0 fine.
            virtual void onStoreBought(int result) { (void)result; }

            /// @brief How a sale to an NPC went (packet 27/2).
            /// @param result 0 fine.
            virtual void onStoreSold(int result) { (void)result; }

            /// @brief The NPC buys from the player (packets 27/3 for goods, 27/4).
            /// @param goods True for 27/3 (goods).
            virtual void onSaleOpened(bool goods) { (void)goods; }

            /// @brief The gold in the bank account (packet 29/4).
            /// @param gold Gold in the account.
            virtual void onBankGold(std::int32_t gold) { (void)gold; }

            /// @brief Gold put in (+) or taken out (-) of the bank account (packets 29/1, 29/2).
            /// @param amount Signed gold amount.
            virtual void onBankGoldMoved(std::int32_t amount) { (void)amount; }

            /// @brief The bank account is full (packet 29/3).
            virtual void onBankFull() {}

            /// @brief The bank's money account form (packet 29/5) or item form (29/6) opens.
            /// @param items True for the item form.
            virtual void onBankOpened(bool items) { (void)items; }

            /// @brief The money form's withdrawal (packet 29/7) or deposit (29/8) is done.
            /// @param deposit True for a deposit.
            virtual void onBankGoldDone(bool deposit) { (void)deposit; }

            /// @brief An item in a place of the bank (packets 30/1, 30/4, 30/10).
            /// @param record The item and its place.
            virtual void onBankItemPut(const protocol::BagItemRecord& record) { (void)record; }

            /// @brief An item the client places in the bank (packet 30/2).
            /// @param item The item.
            virtual void onBankItemAdded(const protocol::ItemInstance& item) { (void)item; }

            /// @brief So many of an item taken out of the bank (packet 30/3).
            /// @param itemId The item.
            /// @param count How many.
            virtual void onBankItemRemoved(int itemId, int count) {
                (void)itemId;
                (void)count;
            }

            /// @brief Fewer items in a bank place (packet 30/5).
            /// @param slot Bank place.
            /// @param count How many fewer.
            virtual void onBankSlotDecreased(int slot, int count) {
                (void)slot;
                (void)count;
            }

            /// @brief A withdrawal (packet 30/6) or deposit (30/7) of items is done.
            /// @param deposit True for a deposit.
            virtual void onBankItemsDone(bool deposit) { (void)deposit; }

            /// @brief The client's copy of the bank is emptied (packet 30/8).
            virtual void onBankCleared() {}

            /// @brief What lies on the map (packet 23/4).
            /// @param records The items on the ground.
            virtual void onGroundItems(const std::vector<protocol::GroundItemRecord>& records) { (void)records; }

            /// @brief An item appears on the ground (packets 23/3, 23/18).
            /// @param itemId The item.
            /// @param position Map point.
            /// @param extra Extra value of the packet.
            /// @param appearMode 0 at once, 1 flying from the player, 2 from the event's NPC, 3 a bribe.
            virtual void onGroundItemAdded(int itemId, core::Point position, std::int32_t extra, int appearMode) {
                (void)itemId;
                (void)position;
                (void)extra;
                (void)appearMode;
            }

            /// @brief Gold appears on the ground (packets 23/3, 23/18).
            /// @param gold Gold amount.
            /// @param position Map point.
            /// @param dropped True when the pile flies from the player.
            virtual void onGroundGoldAdded(std::int32_t gold, core::Point position, bool dropped) {
                (void)gold;
                (void)position;
                (void)dropped;
            }

            /// @brief A thing on the ground is gone (packet 23/2).
            /// @param index Index of the thing.
            /// @param pickedUp True when the thing flies to the player.
            virtual void onGroundItemRemoved(int index, bool pickedUp) {
                (void)index;
                (void)pickedUp;
            }

            /// @brief An item obtained, for the bag (packet 23/6).
            /// @param item The item.
            virtual void onItemObtained(const protocol::ItemInstance& item) { (void)item; }

            /// @brief An item into the city warehouse (packets 23/43, 23/44).
            /// @param record The item and its place (slot 0: the client places it).
            virtual void onWarehouseItem(const protocol::BagItemRecord& record) { (void)record; }

            /// @brief A place of the warehouse emptied (packet 23/45).
            /// @param slot The place.
            virtual void onWarehouseSlotCleared(int slot) { (void)slot; }

            /// @brief The warehouse form opens (packet 23/46).
            virtual void onWarehouseOpened() {}

            /// @brief An item into the sale store (packets 23/60, 23/61).
            /// @param record The item and its place (slot 0: the client places it).
            virtual void onSaleStoreItem(const protocol::BagItemRecord& record) { (void)record; }

            /// @brief A place of the sale store emptied (packet 23/62).
            /// @param slot The place.
            virtual void onSaleStoreCleared(int slot) { (void)slot; }

            /// @brief The sale net's list, its form (packet 23/63).
            /// @param list The list.
            virtual void onSaleNetList(const protocol::SaleNetList& list) { (void)list; }

            /// @brief The sale menu opens (packet 23/64).
            virtual void onSaleMenuOpened() {}

            /// @brief The request the sale net waited for is answered (packet 23/65).
            virtual void onSaleRequestDone() {}

            /// @brief The website could not be reached (packet 23/67 u8, value 1).
            /// @param code The error code.
            virtual void onSaleWebError(int code) { (void)code; }

            /// @brief How much of a kind of weapon material the guild's weapon bank holds (packet 39/27).
            /// @param kind Material kind, 1..7.
            /// @param count Amount held.
            virtual void onGuildWeaponCount(int kind, std::int32_t count) {
                (void)kind;
                (void)count;
            }

            /// @brief An item was used up, shown as "<item> Decrease " (packet 23/26).
            /// @param itemId The item.
            /// @param kind Message kind (1).
            virtual void onItemDecreased(int itemId, int kind) {
                (void)itemId;
                (void)kind;
            }

            /// @brief The sound of getting an item (packet 23/36).
            virtual void onItemSound() {}

            /// @brief Items given into places of the bag (packet 23/100, RecvBagItemsGained).
            /// @param records The items and their places.
            virtual void onBagItemsGained(const std::vector<protocol::BagItemRecord>& records) { (void)records; }

            /// @brief Presents sent because the bag was full (packet 23/101, RecvBagFullGiftNotice).
            /// @param items The presents.
            virtual void onBagFullGifts(const std::vector<protocol::ItemInstance>& items) { (void)items; }

            /// @brief The exp of the own player or one of his pets grew, "<name> Increases EXP <n>" (packet 23/135).
            /// @param playerId The player.
            /// @param petPlace Pet place; 0 is the own player.
            /// @param exp Exp gained.
            virtual void onExpIncreased(std::int32_t playerId, int petPlace, std::int32_t exp) {
                (void)playerId;
                (void)petPlace;
                (void)exp;
            }

            /// @brief "<item>Give off a mysterious light" (packet 23/139/3).
            /// @param itemId The item.
            virtual void onItemGlows(int itemId) { (void)itemId; }

            /// @brief A player teleported to the carnie cannot team up; the message is the own player's (packets 5/26, 5/27).
            /// @param playerId The player.
            virtual void onCarnieTeamRefused(std::int32_t playerId) { (void)playerId; }

            /// @brief "Use auto-healing potion<n>times" (packet 11/9, flag 1).
            /// @param times How many times.
            virtual void onAutoHealPotion(int times) { (void)times; }

            /// @brief How long a soldier is available (packet 15/24).
            /// @param minutes Minutes.
            virtual void onSoldierTime(int minutes) { (void)minutes; }

            /// @brief The pet of a player on the map turns into another kind (packet 15/25, RecvScenePetNpcChange).
            /// @param playerId The owner.
            /// @param petKind The pet's current kind.
            /// @param newKind The kind it turns into.
            virtual void onScenePetChanged(std::int32_t playerId, int petKind, int newKind) { (void)playerId, (void)petKind, (void)newKind; }

            /// @brief An auto pill was used that many more times (packet 11/7, AddAutoPillUse).
            /// @param kind 1 attack, 2 F1 potion.
            /// @param count How many more times.
            virtual void onAutoPillUsed(int kind, int count) { (void)kind, (void)count; }

            /// @brief A fighter gave up its turn (packet 11/11, RevFightAbstain).
            /// @param playerId The fighter.
            virtual void onFighterAbstains(std::int32_t playerId) { (void)playerId; }

            /// @brief A pet's gear went back into the warehouse at home (packet 15/21).
            /// @param npcId The pet's kind.
            virtual void onPetGearBackHome(int npcId) { (void)npcId; }

            /// @brief A helper offers to help in a fight, or nobody does (packet 53/6, RevFightHelpMsg).
            /// @param mode The offer mode.
            /// @param helperId The helper.
            /// @param kind 2 when the helper is a player, else an NPC.
            virtual void onFightHelp(int mode, std::int32_t helperId, int kind) {
                (void)mode;
                (void)helperId;
                (void)kind;
            }

            /// @brief A fighter put on another cell (packet 53/7, RevMoveFighterCell).
            /// @param fromRow Old row.
            /// @param fromCol Old column.
            /// @param toRow New row.
            /// @param toCol New column.
            virtual void onFighterMoved(int fromRow, int fromCol, int toRow, int toCol) {
                (void)fromRow;
                (void)fromCol;
                (void)toRow;
                (void)toCol;
            }

            /// @brief The result of recommending a friend (packet 35/7).
            /// @param result 1 done, 2 refused.
            virtual void onRecommendResult(int result) { (void)result; }

            /// @brief Points for being recommended (packet 35/9).
            /// @param points The points.
            virtual void onRecommendPoints(std::int32_t points) { (void)points; }

            /// @brief Gold into / out of the sale account (packets 26/5, 26/6).
            /// @param amount Gold amount.
            /// @param added True when gold went in.
            virtual void onSaleGoldChanged(std::int32_t amount, bool added) {
                (void)amount;
                (void)added;
            }

            /// @brief The inn asks to rest for that much (packet 31/2).
            /// @param price Price; 0 free, -1 no need.
            virtual void onInnPrice(std::int32_t price) { (void)price; }

            /// @brief How the rest went (packet 31/1).
            /// @param answer 0 rested, 1 too little gold, 2 no need.
            virtual void onInnAnswer(int answer) { (void)answer; }

            /// @brief The player sleeps (packet 31/10).
            virtual void onInnSleep() {}

            /// @brief The pets stored at the inn (packet 31/6).
            /// @param pets The stored pets.
            virtual void onStoredPets(const std::vector<protocol::PetRecord>& pets) { (void)pets; }

            /// @brief The pet storage form opens (packet 31/7).
            virtual void onPetStorageOpened() {}

            /// @brief The pet in a slot is stored at a place (packet 31/3).
            /// @param place Storage place.
            /// @param petSlot The pet's slot.
            virtual void onPetStored(int place, int petSlot) {
                (void)place;
                (void)petSlot;
            }

            /// @brief The carried pet and the stored one change places (packet 31/15, RecvInnPetSwapped).
            /// @param petSlot The carried pet's slot.
            /// @param place Storage place.
            virtual void onInnPetSwapped(int petSlot, int place) { (void)petSlot, (void)place; }

            /// @brief The stored pet goes into a slot (packets 31/5, 31/11).
            /// @param place Storage place.
            /// @param petSlot The pet's slot.
            virtual void onStoredPetTaken(int place, int petSlot) {
                (void)place;
                (void)petSlot;
            }

            /// @brief A storage place is emptied (packet 31/4).
            /// @param place Storage place.
            virtual void onStoredPetCleared(int place) { (void)place; }

            /// @brief The pet storage answer is done (packet 31/12).
            virtual void onPetStorageDone() {}

            /// @brief The pet storage request was refused (packets 31/13, 31/14).
            /// @param storerHasIt Which side refused (the storer has the pet).
            virtual void onPetStorageRefused(bool storerHasIt) { (void)storerHasIt; }

            /// @brief The account's IM points (packet 35/4).
            /// @param points The points.
            virtual void onImPoints(std::int32_t points) { (void)points; }

            /// @brief The item mall's catalog (packet 75/x).
            /// @param rows The catalog rows.
            virtual void onMallCatalog(const std::vector<protocol::MallRow>& rows) { (void)rows; }

            /// @brief The catalog changed while the player plays (packet 75/2).
            /// @param rows The changed rows.
            virtual void onMallCatalogUpdate(const std::vector<protocol::MallRow>& rows) { (void)rows; }

            /// @brief A mall line was paid with points (packet 75/x).
            /// @param points Points left.
            /// @param spent Points spent.
            /// @param itemId The item bought.
            /// @param count How many.
            virtual void onMallPointsSpent(std::int32_t points, std::int32_t spent, int itemId, int count) {
                (void)points, (void)spent, (void)itemId, (void)count;
            }

            /// @brief The bonus points the account has (packet 75/8, RecvBonusPoints).
            /// @param points The bonus points.
            virtual void onMallBonusPoints(std::int32_t points) { (void)points; }

            /// @brief A mall line was bought with bonus points (packet 75/9, RecvBonusPointsDeducted).
            /// @param points Points left.
            /// @param spent Points spent.
            /// @param itemId The item bought.
            /// @param count How many.
            virtual void onMallBonusSpent(std::int32_t points, std::int32_t spent, int itemId, int count) {
                (void)points, (void)spent, (void)itemId, (void)count;
            }

            /// @brief A mall line was bought or not (packet 75/x).
            /// @param itemId The item.
            /// @param category Mall category.
            /// @param count How many.
            /// @param key Line key.
            /// @param ok True when bought.
            virtual void onMallBought(
                int itemId,
                int category,
                int count,
                int key,
                bool ok
            ) { (void)itemId, (void)category, (void)count, (void)key, (void)ok; }

            /// @brief A mall purchase was refused (packet 75/x).
            /// @param reason Refusal reason.
            virtual void onMallRefused(int reason) { (void)reason; }

            /// @brief The slot machines' tab of the mall (packet 75/x).
            /// @param open True to open it.
            virtual void onMallGamesTab(bool open) { (void)open; }

            /// @brief How the mall's forge went (packet 75/6).
            /// @param outcome Outcome code; the bag place and fusion level come with outcome 1.
            /// @param bagSlot Bag place.
            /// @param fusionLevel Fusion level.
            virtual void onMallForgeResult(int outcome, int bagSlot, int fusionLevel) {
                (void)outcome;
                (void)bagSlot;
                (void)fusionLevel;
            }

            /// @brief The own guild (packet 39/2).
            /// @param info The guild.
            virtual void onGuildInfo(const protocol::GuildInfo& info) { (void)info; }

            /// @brief A player's guild, the title over him (packet 39/9).
            /// @param playerId The player.
            /// @param guildId The guild.
            /// @param guild Guild name.
            virtual void onGuildTitle(std::int32_t playerId, std::uint32_t guildId, const std::string& guild) { (void)playerId, (void)guildId, (void)guild; }

            /// @brief The text over a player's name on the PvP maps (packet 37/1, RecvPvpVictories).
            /// @param playerId The player.
            /// @param wins Number of wins.
            virtual void onPvpVictories(std::int32_t playerId, int wins) { (void)playerId, (void)wins; }

            /// @brief The PvP rank over a player's name (packet 37/2, RecvPvpRank).
            /// @param playerId The player.
            /// @param rank The rank.
            virtual void onPvpRank(std::int32_t playerId, int rank) { (void)playerId, (void)rank; }

            /// @brief The arena victories over a player's name (packet 37/6, RecvArenaVictory).
            /// @param playerId The player.
            /// @param victories Number of victories.
            virtual void onArenaVictory(std::int32_t playerId, int victories) { (void)playerId, (void)victories; }

            /// @brief The code of a notice of the flee-killing event; 1 begins it and 3 ends it (packet 77/1, RecvFleeKillEvent).
            /// @param code The notice code.
            virtual void onFleeKillEvent(int code) { (void)code; }

            /// @brief The own survival wins (packet 77/3, RecvSurvivalVictory).
            /// @param victories Number of wins.
            virtual void onSurvivalVictory(int victories) { (void)victories; }

            /// @brief A guild badge: guild id, version and pixels.
            /// @details Original: packets 39/31 (RecvEnsignList: the guilds that have a badge with its version), 39/32 (RecvEnsign: badges, 288 bytes each), 39/30 (RecvOwnEnsignUpdate: the own guild's), 39/34 (RecvEnsignReset).
            struct BadgeRecord {
                std::uint32_t guildId = 0;
                int version = 0;
                core::Bytes pixels;
            };

            /// @brief The guilds that have a badge, with its version (packet 39/31, RecvEnsignList).
            /// @param listed The listed badges.
            virtual void onBadgeList(const std::vector<BadgeRecord>& listed) { (void)listed; }

            /// @brief Badges with their pixels, 288 bytes each (packet 39/32, RecvEnsign).
            /// @param badges The badges.
            virtual void onBadges(const std::vector<BadgeRecord>& badges) { (void)badges; }

            /// @brief The own guild's badge (packet 39/30, RecvOwnEnsignUpdate).
            /// @param version Badge version.
            /// @param pixels Badge pixels.
            virtual void onOwnBadge(int version, const core::Bytes& pixels) { (void)version, (void)pixels; }

            /// @brief The badge of that guild is to be replaced by the sample file (packet 39/34, RecvEnsignReset).
            /// @param guildId The guild.
            /// @param sampleNumber Number of the sample file.
            virtual void onBadgeReset(std::uint32_t guildId, int sampleNumber) { (void)guildId, (void)sampleNumber; }

            /// @brief The create guild form opens (packet 39/40).
            virtual void onGuildCreateForm() {}

            /// @brief How founding a guild went (packet 39/1).
            /// @param result 0 founded.
            virtual void onGuildCreateResult(int result) { (void)result; }

            /// @brief A player invites the own player to a guild (packet 39/3).
            /// @param inviterId The inviting player.
            virtual void onGuildInvite(std::int32_t inviterId) { (void)inviterId; }

            /// @brief How an invitation of the own player went (packet 39/4).
            /// @param targetId The invited player.
            /// @param answer The answer code.
            virtual void onGuildInviteAnswer(std::int32_t targetId, int answer) { (void)targetId, (void)answer; }

            /// @brief A member is off the list (packet 39/6).
            /// @param id The member.
            virtual void onGuildMemberGone(std::int32_t id) { (void)id; }

            /// @brief Leaving or sending away a member (packet 39/7).
            /// @param result The result code.
            /// @param id The member.
            virtual void onGuildLeaveResult(int result, std::int32_t id) { (void)result, (void)id; }

            /// @brief A player joined the guild (packet 39/8).
            /// @param id The player.
            virtual void onGuildMemberJoined(std::int32_t id) { (void)id; }

            /// @brief The guild is disbanded (packet 39/10).
            /// @param result The result code.
            virtual void onGuildDisbanded(int result) { (void)result; }

            /// @brief The guild's rules (packet 39/11).
            /// @param rules The rules text.
            virtual void onGuildRules(const std::string& rules) { (void)rules; }

            /// @brief The date of the automatic disbanding (packet 39/16).
            /// @param date The date.
            virtual void onGuildDisbandDate(double date) { (void)date; }

            /// @brief A vice lord appointed or deposed (packets 39/12, 39/13).
            /// @param id The member.
            /// @param appointed True when appointed.
            virtual void onGuildVice(std::int32_t id, bool appointed) { (void)id, (void)appointed; }

            /// @brief A new lord, with how the old one goes on (packet 39/14).
            /// @param mode How the old lord goes on.
            /// @param id The new lord.
            virtual void onGuildNewLord(int mode, std::int32_t id) { (void)mode, (void)id; }

            /// @brief Why a post could not be given (packet 39/15).
            /// @param code The reason code.
            virtual void onGuildPositionError(int code) { (void)code; }

            /// @brief The lord offers a vice lord's post (packet 39/18).
            /// @param lordId The lord.
            virtual void onGuildViceOffer(std::int32_t lordId) { (void)lordId; }

            /// @brief The lord offers his own post (packet 39/19).
            /// @param lordId The lord.
            virtual void onGuildLordOffer(std::int32_t lordId) { (void)lordId; }

            /// @brief The offered one agreed (packet 39/25).
            /// @param post 1 vice lord, 2 lord.
            /// @param id The member.
            virtual void onGuildPostAccepted(int post, std::int32_t id) { (void)post, (void)id; }

            /// @brief A member's contribution (packet 39/23).
            /// @param id The member.
            /// @param contribution Contribution value.
            virtual void onGuildContribution(std::int32_t id, std::int32_t contribution) { (void)id, (void)contribution; }

            /// @brief A member's right changed (packet 39/24).
            /// @param id The member.
            /// @param right Which right.
            /// @param on Whether it is now granted.
            virtual void onGuildRight(std::int32_t id, int right, bool on) { (void)id, (void)right, (void)on; }

            /// @brief The guild welcome (packet 39/62).
            virtual void onGuildWelcome() {}

            /// @brief The members' online states (packet 39/61, ParseOrgMemberStates).
            /// @param states The states.
            virtual void onGuildStates(const std::vector<protocol::FriendState>& states) { (void)states; }

            /// @brief The Top 100 window opens (packet 84/1).
            virtual void onTop100Opened() {}

            /// @brief One of the Top 100 lists (packet 84/2); how many rows it holds depends on the sizes the window knows.
            /// @param payload The whole message.
            virtual void onTop100List(const core::Bytes& payload) { (void)payload; }

            /// @brief A mini game of this kind starts (packet 57/1, RecvSportStart).
            /// @param kind Game kind.
            /// @param npcId The NPC that started it.
            /// @param level The packet's fifth byte.
            /// @param scene For the throw game, its scene.
            virtual void onSportStart(int kind, int npcId, int level, int scene) { (void)kind, (void)npcId, (void)level, (void)scene; }

            /// @brief The mini game is over (packet 57/2).
            virtual void onSportEnd() {}

            /// @brief Plays a story movie (packet 23/151, PlayStoryMovie).
            /// @param index Movie 1 to 8.
            virtual void onStoryMovie(int index) { (void)index; }

            /// @brief The movie finishes (packet 20/48, FinishMovie).
            virtual void onMovieFinish() {}

            /// @brief The Lucky Wheel's list and answers (packet 57/12).
            /// @param payload The whole message.
            virtual void onWheelMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A prize machine's answer (packet 71/x).
            /// @param payload The whole message.
            virtual void onPokeMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A prize machine's answer (packet 71/x).
            /// @param payload The whole message.
            virtual void onQuizMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A prize machine's answer (packet 71/x).
            /// @param payload The whole message.
            virtual void onCompassMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief Five in a Row (packet 57/11).
            /// @param payload The whole message.
            virtual void onGobangMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A prize machine's answer (packet 71/x).
            /// @param payload The whole message.
            virtual void onMachineMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief Tents (packet 65/x).
            /// @param payload The whole message.
            virtual void onTentMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A player's colours changed (packets 23/53 and 23/103: HandleColorPatt1Packet and HandleColorPatt5Packet).
            /// @param playerId The player.
            /// @param part 1 the hair and skin, 5 the clothes and eyes.
            /// @param code The new code of that part.
            virtual void onPlayerColors(std::int32_t playerId, int part, std::int32_t code) { (void)playerId, (void)part, (void)code; }

            /// @brief The express pet is off or back (packet 25/20, RecvPetDeliveryState).
            /// @param petKind The pet's kind.
            /// @param state 1 off, 2 back.
            virtual void onExpressState(std::int32_t petKind, int state) { (void)petKind, (void)state; }

            /// @brief The express pet walks off the own player's screen or back (packet 25/21, HandlePetWalkAnimPacket).
            /// @param petKind The pet's kind.
            /// @param mode 1 off, 3 back; 2 and 4 back and off again.
            virtual void onPetWalk(std::int32_t petKind, int mode) { (void)petKind, (void)mode; }

            /// @brief An NPC of that kind walks by (packet 25/33, HandleNpcWalkByPacket).
            /// @param npcId The NPC kind.
            /// @param mode 1 or 2.
            virtual void onNpcWalkBy(std::int32_t npcId, int mode) { (void)npcId, (void)mode; }

            /// @brief The express failed (packet 25/22, RecvExpressError).
            /// @param code The error code.
            /// @param itemId An item, for code 1.
            virtual void onExpressFailed(int code, int itemId) { (void)code, (void)itemId; }

            /// @brief An item arrived from another player or was delivered to him (packet 25/23, RecvExpressResult).
            /// @param kind 1 arrived, 2 delivered.
            /// @param otherId The other player.
            /// @param itemId The item.
            /// @param count How many.
            virtual void onExpressResult(int kind, std::int32_t otherId, int itemId, int count) { (void)kind, (void)otherId, (void)itemId, (void)count; }

            /// @brief The pet found an item for the Dog Bone (packet 23/146, RecvForageFound).
            /// @param itemId The item.
            /// @param count How many.
            virtual void onForageFound(int itemId, int count) { (void)itemId, (void)count; }

            /// @brief The search ended (packet 23/147, RecvForageEnd).
            /// @param boneSlot The bone's place.
            /// @param petPlace The pet's place.
            /// @param code 1 it failed, else it was cancelled.
            virtual void onForageEnded(int boneSlot, int petPlace, int code) { (void)boneSlot, (void)petPlace, (void)code; }

            /// @brief The Picnic Box's cells (packet 23/217, RecvPicnicPacket).
            /// @param payload The whole message.
            virtual void onPicnicMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The Wishing God's list of pets and its talks (packets 66/11 to 66/16).
            /// @param payload The whole message.
            virtual void onRebornWishMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The EXP Bag's cells (packet 23/218, RecvExpBoxPacket).
            /// @param payload The whole message.
            virtual void onExpBoxMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The house blackboard's messages and its refusal (packets 72/1, 72/2).
            /// @param payload The whole message.
            virtual void onBlackBoardMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The second ID and password form opens in that mode (packets 21/1, 21/2).
            /// @param mode The form mode.
            virtual void onIdAndPwForm(int mode) { (void)mode; }

            /// @brief The internet cafe number form opens (packets 55/1, 55/2 2).
            virtual void onCafeIdForm() {}

            /// @brief The gold a point is worth (packet 23/200, RecvExchangeRate).
            /// @param rate Gold per point.
            virtual void onExchangeRate(std::int32_t rate) { (void)rate; }

            /// @brief The gold now and what was gained (packet 23/201, RecvExchangeGold).
            /// @param gold Gold now.
            /// @param gained Gold gained.
            virtual void onExchangeGold(std::int32_t gold, std::int32_t gained) { (void)gold, (void)gained; }

            /// @brief The exchange fee message (packet 23/202, RecvExchangeFeeMsg).
            /// @param kind Message kind.
            /// @param playerId The player.
            /// @param value The value.
            virtual void onExchangeFee(int kind, std::int32_t playerId, std::int32_t value) { (void)kind, (void)playerId, (void)value; }

            /// @brief The point store (packets 80/1 to 80/8).
            /// @param payload The whole message.
            virtual void onPointStoreMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A recorded place of the GPS items (packet 74/1, RecvGpsMarkSlot).
            /// @param point Slot 1..3.
            /// @param map Map id.
            /// @param x Map x.
            /// @param y Map y.
            virtual void onGpsMark(int point, int map, int x, int y) { (void)point, (void)map, (void)x, (void)y; }

            /// @brief The Gift Exchange Cards (packet 23/108).
            /// @param payload The whole message.
            virtual void onGiftCardMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The piece of a bag place wears this look now, or the pose failed (packet 23/105 1 and 2, RecvSewingKitResult).
            /// @param code 1 worn, 2 failed.
            /// @param bagSlot Bag place.
            /// @param look The look.
            virtual void onSewingKitResult(int code, int bagSlot, int look) { (void)code, (void)bagSlot, (void)look; }

            /// @brief The Memory Card's and puppet training's messages (packet 23/104, Recv routines of CJ_Menu.pas).
            /// @param packet The puppet packet.
            virtual void onPuppetPacket(const protocol::PuppetPacket& packet) { (void)packet; }

            /// @brief The Magnifying Glass's answer (packet 23/166, RecvReferResult).
            /// @param payload The whole message.
            virtual void onReferResult(const core::Bytes& payload) { (void)payload; }

            /// @brief The Present Case packed the piece (packets 23/107 and 23/205, RecvPackingResult).
            /// @param code 1 packed.
            virtual void onPackingResult(int code) { (void)code; }

            /// @brief The Rebirth Box gave a piece its own look back or failed (packet 23/105 3 and 4, RecvMagicBoxResult).
            /// @param code 3 gave the look back, 4 failed.
            virtual void onMagicBoxResult(int code) { (void)code; }

            /// @brief The Bliss Lamp's result (packet 23/215, RecvBlessBalloonResult).
            /// @param outcome 0 to 3 not used (0 released, 1 illegal words, 2 on his block list, 3 the target fights), 4 a player told another.
            /// @param sender The sending player.
            /// @param target The target player.
            /// @param text The message text.
            virtual void onBlessBalloonResult(int outcome, std::int32_t sender, std::int32_t target, const std::string& text) {
                (void)outcome, (void)sender, (void)target, (void)text;
            }

            /// @brief The Passion Bouquet's result (packet 23/209, RecvLoveFlowerResult).
            /// @param outcome 0 not used (sender holds why: 1 block list, 2 the target fights, 3 off line, 4 illegal words), 1 sent, 2 the hearts over the target.
            /// @param sender The sending player.
            /// @param target The target player.
            /// @param text The message text.
            virtual void onLoveFlowerResult(int outcome, std::int32_t sender, std::int32_t target, const std::string& text) {
                (void)outcome, (void)sender, (void)target, (void)text;
            }

            /// @brief The fusion result (packet 23/210, RecvReinforceResult).
            /// @param code 1 the fusion level of a bag place is now this, 2 the fusion failed.
            /// @param bagSlot Bag place.
            /// @param level Fusion level.
            virtual void onReinforceResult(int code, int bagSlot, int level) { (void)code, (void)bagSlot, (void)level; }

            /// @brief The stone mounted in the piece of a bag place is now this item (packet 23/214, RecvConfuseStoneResult).
            /// @param step The Chaos Crystal's new step, 0 when it fell off.
            /// @param bagSlot Bag place.
            /// @param stoneItemId The stone item.
            virtual void onConfuseStoneResult(int step, int bagSlot, int stoneItemId) { (void)step, (void)bagSlot, (void)stoneItemId; }

            /// @brief The stone of a piece of the bag was taken out; the wear record of the piece follows (packet 23/143, RecvDismantleResult).
            /// @param container Container of the piece.
            /// @param slot Place of the piece.
            /// @param wear The wear value.
            /// @param petPlace Pet place.
            virtual void onDismantled(int container, int slot, std::int32_t wear, int petPlace) { (void)container, (void)slot, (void)wear, (void)petPlace; }

            /// @brief The Dog Bone of a bag place is out finding; the cell cannot be moved (packet 23/152, MarkForageBone).
            /// @param boneSlot Bag place of the bone.
            virtual void onForageBone(int boneSlot) { (void)boneSlot; }

            /// @brief The fish tank (packet 23/160).
            /// @param payload The whole message.
            virtual void onFishTankMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The Wit Maid's menu (packet 76/x, RecvMaidAction).
            /// @param payload The whole message.
            virtual void onMaidAction(const core::Bytes& payload) { (void)payload; }

            /// @brief What a Lucky Bag of the mall can hold (packet 91/2, RecvLuckyBagInfo).
            /// @param payload The whole message.
            virtual void onLuckyBagMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The furniture of a house (packet 62/x).
            /// @param payload The whole message.
            virtual void onFurnitureMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The manufacturing of the house (packet 64/x).
            /// @param payload The whole message.
            virtual void onManufactureMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The Tent Palette or Shape Clay was refused (packet 23/158).
            /// @param code 1 only for an ordinary tent, 2 no tent.
            virtual void onTentStyleRefused(int code) { (void)code; }

            /// @brief The style chooser opens for the item in a bag place and its set (packet 23/159).
            /// @param bagSlot Bag place.
            /// @param set Style set.
            virtual void onTentStyleChooser(int bagSlot, int set) {
                (void)bagSlot;
                (void)set;
            }

            /// @brief The career form opens (packet 79/1).
            virtual void onCareerForm() {}

            /// @brief The answer to the career choice (packet 79/2).
            /// @param code The answer; 1 means no room for the class's cloak.
            virtual void onCareerResult(int code) { (void)code; }

            /// @brief Marriage (packet 45/x).
            /// @param payload The whole message.
            virtual void onMarriageMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The instances (packet 85/x).
            /// @param payload The whole message.
            virtual void onInstanceMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The rows of a notice board's page (packets 82/1, 82/11).
            /// @param guild True for the guild's board.
            /// @param rows The rows.
            virtual void onNoticeRows(bool guild, const std::vector<protocol::NoticeRow>& rows) { (void)guild, (void)rows; }

            /// @brief How many notices and the page they are of (packets 82/4, 82/14; 82/3, 82/13 with restoreAsked).
            /// @param guild True for the guild's board.
            /// @param info Page information.
            /// @param restoreAsked True when the page asked before is the one shown.
            virtual void onNoticePages(bool guild, const protocol::NoticePageInfo& info, bool restoreAsked) { (void)guild, (void)info, (void)restoreAsked; }

            /// @brief The notice opened (packets 82/2, 82/12).
            /// @param guild True for the guild's board.
            /// @param content The notice.
            virtual void onNoticeContent(bool guild, const protocol::NoticeContent& content) { (void)guild, (void)content; }

            /// @brief The form to write a notice (packet 82/8).
            /// @param guild True for the guild's board.
            virtual void onNoticeWriteForm(bool guild) { (void)guild; }

            /// @brief The public board opens (packet 82/9).
            virtual void onNoticeBoardOpened() {}

            /// @brief A notice board line for the player (packets 82/5, 82/6, 82/7, 82/10, 82/15, 82/16).
            /// @param text The line.
            /// @param durationMs How long to show it, in milliseconds.
            virtual void onNoticeMessage(const std::string& text, int durationMs) { (void)text, (void)durationMs; }

            /// @brief The Christmas event's chat line (packet 54/x and 23/57 notice 93).
            /// @param text The chat line.
            virtual void onChristmasAnnounced(const std::string& text) { (void)text; }

            /// @brief After the hide, the NPC went for the seconds the message said (packet 22/3).
            /// @param npcIndex The NPC's place number on the map.
            virtual void onNpcVanished(int npcIndex) { (void)npcIndex; }

            /// @brief The Christmas event starts or ends: snow and music on its maps (23/57 notices 93 / 94).
            /// @param on True at the start.
            virtual void onChristmasEvent(bool on) { (void)on; }

            /// @brief The game logs in again by itself (packet 87/1: sub 1 go to the PvP server, 2 come back; ScheduleServerRelogin and LogoutToLogin).
            /// @param toPvp True to go to the PvP server, false to come back.
            virtual void onServerRelogin(bool toPvp) { (void)toPvp; }

            /// @brief The New Year event goes on or off; the Lucky 7 count is cleared (event notices 95 / 96, FNewYearEventOn).
            /// @param on True when the event is on.
            virtual void onNewYearEvent(bool on) { (void)on; }

            /// @brief The Lucky 7 count (packet 27/5, FLucky7Count).
            /// @param count The count.
            virtual void onLucky7Count(int count) { (void)count; }

            /// @brief A Trojan war or castle line (packet 70/1).
            /// @param text The line.
            /// @param inChat True for the chat, false for the message box.
            virtual void onCastleNotice(const std::string& text, bool inChat) { (void)text, (void)inChat; }

            /// @brief Who owns the castle (packet 70/1).
            /// @param guild The owning guild's name, "" for no guild.
            virtual void onCastleOwner(const std::string& guild) { (void)guild; }

            /// @brief The light round the own player, another minute of it (packet 23/38, StartEffectS10233).
            virtual void onTimedLight() {}

            /// @brief The kinds of the lights round a player (packet 23/55, RecvPlayerEffectLights).
            /// @param playerId The player.
            /// @param kinds Light kinds.
            virtual void onEffectLights(std::int32_t playerId, const std::vector<int>& kinds) { (void)playerId, (void)kinds; }

            /// @brief The body of a player turns into another role's (packet 5/12, RecvRoleChange).
            /// @param playerId The player.
            /// @param role New role, 1..13.
            virtual void onRoleChange(std::int32_t playerId, int role) { (void)playerId, (void)role; }

            /// @brief The prepaid card form opens or closes (packets 35/6 ShowReFillForm and 35/11 CloseReFillForm).
            /// @param open True to open it.
            virtual void onReFillForm(bool open) { (void)open; }

            /// @brief The thing of a special house room (packets 81/1, 81/2, 81/4).
            /// @param payload The whole message.
            virtual void onSpecFloorMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The lost property office (packets 86/1, 86/2, 86/3).
            /// @param payload The whole message.
            virtual void onPropertyMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The announcement form opens (packet 36/1) with its text (36/2).
            /// @param open True to open the form.
            /// @param text The announcement text.
            virtual void onAnnounceForm(bool open, const std::string& text) { (void)open, (void)text; }

            /// @brief The guestbook (packets 36/3 to 36/7).
            /// @param payload The whole message.
            virtual void onLeaveWordMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief A group of the galaxy ranking (packet 36/9) or the form opening (36/10).
            /// @param payload The whole message.
            virtual void onGalaxyRankMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The pet rebirth form opens (packet 44/2).
            virtual void onNpcUpgradeForm() {}

            /// @brief The help text for a lord takes the place of the rules in the guild window (packet 39/17, ShowGuildRuleHelp).
            virtual void onGuildRuleHelp() {}

            /// @brief The record of a member of the own guild; his contribution and rights stay (packet 39/60, RecvOrgMemberInfo).
            /// @param record The member record.
            virtual void onGuildMemberInfo(const protocol::GuildMemberRecord& record) { (void)record; }

            /// @brief What the own player does for the guild (labour, crafts, feathers) and its end (packets 39/50, 39/51, 39/53 to 39/59).
            /// @param payload The whole message.
            virtual void onGuildWork(const core::Bytes& payload) { (void)payload; }

            /// @brief The facility a product is made at from now on (packet 41/1, RecvCompoundFacilityUpdate).
            /// @param changes (product, facility) pairs; the facility is the piece of furniture, 0 nowhere.
            virtual void onCompoundFacility(const std::vector<std::pair<int, int>>& changes) { (void)changes; }

            /// @brief The alliance of the guild (packets 43/1 to 43/5).
            /// @param payload The whole message.
            virtual void onAllianceMessage(const core::Bytes& payload) { (void)payload; }

            /// @brief The member and the lord change places in the list (packet 39/63, RecvLordChanged).
            /// @param memberId The member.
            virtual void onGuildLordChanged(std::int32_t memberId) { (void)memberId; }

            /// @brief The guild's new name (packet 39/33, RecvGuildNameChanged).
            /// @param name The new name.
            virtual void onGuildRenamed(const std::string& name) { (void)name; }

            /// @brief The map NPC of that index jumps and lights the ground where it lands (packet 20/46, StartTreasureNpcJump).
            /// @param npcIndex The NPC's place number on the map.
            virtual void onTreasureNpcJump(int npcIndex) { (void)npcIndex; }

            /// @brief The waiting icon over a player (packet 20/44, RecvWaitEventIcon).
            /// @param playerId The player.
            /// @param mode 1 shows, 2 hides.
            virtual void onWaitEventIcon(std::int32_t playerId, int mode) { (void)playerId, (void)mode; }

            /// @brief Asks to buy a level (packet 20/58 codes 3 and 5: AskBuyPlayerLevel, AskBuyPetLevel).
            /// @param pet True for a pet's level, false for the player's.
            virtual void onBuyLevelAsked(bool pet) { (void)pet; }

            /// @brief The result of buying a level (packet 20/58 code 6, RecvBuyLevelResult).
            /// @param code The result; 1 means the pet may not pass the player.
            virtual void onBuyLevelResult(int code) { (void)code; }

            /// @brief The castle's Gold Eagle prefix, replaced or put (packets 70/10 and 70/11: RecvGoldEaglePrefixList / Update).
            /// @param castle The castle.
            /// @param replace True to replace the whole list (70/10), false to put entries (70/11).
            /// @param entries (slot, prefix) pairs.
            virtual void onGoldEaglePrefix(int castle, bool replace, const std::vector<std::pair<int, std::string>>& entries) {
                (void)castle, (void)replace, (void)entries;
            }

            /// @brief The castle's Gold Eagle prefix removed (packet 70/12).
            /// @param castle The castle.
            /// @param slot The slot.
            virtual void onGoldEaglePrefixRemoved(int castle, int slot) { (void)castle, (void)slot; }

            /// @brief The castle's four defense NPC prefixes, replaced or put (packets 70/3 and 70/4: RecvDefenseNpcPrefixList / Update).
            /// @param castle The castle.
            /// @param replace True to replace the whole list (70/3), false to put entries (70/4).
            /// @param entries (slot, prefix) pairs.
            virtual void onDefenseNpcPrefix(int castle, bool replace, const std::vector<std::pair<int, std::string>>& entries) {
                (void)castle, (void)replace, (void)entries;
            }

            /// @brief A defense NPC prefix removed (packet 70/5).
            /// @param castle The castle.
            /// @param slot The slot.
            virtual void onDefenseNpcPrefixRemoved(int castle, int slot) { (void)castle, (void)slot; }

            /// @brief The castle's four Stone Eagle prefixes, replaced or put (packets 70/7 and 70/8: RecvStoneEaglePrefixList / Update).
            /// @param castle The castle.
            /// @param replace True to replace the whole list (70/7), false to put entries (70/8).
            /// @param entries (slot, prefix) pairs.
            virtual void onStoneEaglePrefix(int castle, bool replace, const std::vector<std::pair<int, std::string>>& entries) {
                (void)castle, (void)replace, (void)entries;
            }

            /// @brief A Stone Eagle prefix removed (packet 70/9).
            /// @param castle The castle.
            /// @param slot The slot.
            virtual void onStoneEaglePrefixRemoved(int castle, int slot) { (void)castle, (void)slot; }

            /// @brief The F1-F8 bar's places (packet 40/1).
            /// @param records The hot key records.
            virtual void onHotKeys(const std::vector<protocol::HotKeyRecord>& records) { (void)records; }

            /// @brief A quest mark counter changed (packets 24/1 +, 24/2 -).
            /// @param markId The mark.
            /// @param change The change in count.
            virtual void onMarkCount(int markId, int change) { (void)markId, (void)change; }

            /// @brief A quest mark was deleted (packet 24/x).
            /// @param markId The mark.
            virtual void onMarkDeleted(int markId) { (void)markId; }

            /// @brief A quest flag bit changed (packet 24/x).
            /// @param flagBit The bit.
            /// @param value Its value.
            virtual void onMarkFlag(int flagBit, int value) { (void)flagBit, (void)value; }

            /// @brief A place of the quest mark list (packet 24/6).
            /// @param slot The place.
            /// @param markId The mark.
            /// @param count Its count.
            virtual void onMarkSlot(int slot, int markId, int count) { (void)slot, (void)markId, (void)count; }

            /// @brief A byte of the quest flags (packet 24/7).
            /// @param byteIndex Byte index.
            /// @param bits The byte's bits.
            virtual void onMarkFlagByte(int byteIndex, int bits) { (void)byteIndex, (void)bits; }

            /// @brief The quest volume is full (packet 24/3).
            virtual void onQuestsFull() {}

            /// @brief Another player's fight state (packet 11/4).
            /// @param playerId The player.
            /// @param state The fight state.
            virtual void onPlayerFightState(std::int32_t playerId, int state) { (void)playerId, (void)state; }

            /// @brief Fighters enter the running battle (packet 11/5).
            /// @param fighters The joining fighters.
            virtual void onFightersJoin(const std::vector<protocol::FighterRecord>& fighters) { (void)fighters; }

            /// @brief The weapons the pets of the battle hold (packet 11/251, RecvFighterEquipLook).
            /// @param looks The weapon looks.
            virtual void onFighterWeaponLooks(const std::vector<protocol::FighterWeaponLook>& looks) { (void)looks; }

            /// @brief A player left the battle, his code 0 (packet 11/0).
            /// @param playerId The player.
            virtual void onPlayerLeftBattle(std::int32_t playerId) { (void)playerId; }

            /// @brief The player's last three battle skills (packet 5/11, LoadRecentBattleSkills).
            /// @param skills The three skill ids.
            virtual void onRecentBattleSkills(const std::array<int, 3>& skills) { (void)skills; }

            /// @brief A battle starts (packet 11/250).
            /// @param start The battle set-up.
            virtual void onBattleStart(const protocol::BattleStart& start) { (void)start; }

            /// @brief The battle's type, i.e. the command time (packet 11/10).
            /// @param type The battle type.
            virtual void onBattleType(int type) { (void)type; }

            /// @brief A round of commands (packet 52/1).
            virtual void onBattleRound() {}

            /// @brief Actions played (packet 50/1); a fighter's attribute set is 51/1.
            /// @param actions The actions.
            virtual void onBattleActions(const std::vector<protocol::BattleAction>& actions) { (void)actions; }

            /// @brief The fighters go back to their cells for the round (packet 50/2, RevAttackItem).
            virtual void onBattleItemRound() {}

            /// @brief A cast that changes its target's attribute once the caster is back (packet 50/3, RecvFightSkillCast).
            /// @param cast The cast.
            virtual void onFighterCast(const protocol::FighterCast& cast) { (void)cast; }

            /// @brief The sound of a mark for each of the count of fighters listed (packet 50/5, RecvFighterMarks).
            /// @param count Number of fighters listed.
            virtual void onFighterMarks(int count) { (void)count; }

            /// @brief The fighter at the cell has its next action rewritten and takes the damage (packet 50/4, RecvFighterDamage).
            /// @param kind 1 thrown back, else knocked back.
            /// @param row Cell row.
            /// @param col Cell column.
            /// @param damage The damage.
            virtual void onFighterDamage(int kind, int row, int col, std::int32_t damage) { (void)kind, (void)row, (void)col, (void)damage; }

            /// @brief Which cells around its target the fighter's action reaches (packet 50/6, RecvFighterAttackArea).
            /// @param row Cell row.
            /// @param col Cell column.
            /// @param area The area code.
            virtual void onFighterAttackArea(int row, int col, int area) { (void)row, (void)col, (void)area; }

            /// @brief "Battle time is reset." and the battle's time limit clock goes off (packet 51/2, RevBattleTimeReset).
            virtual void onBattleTimeReset() {}

            /// @brief The battle's 3-minute limit starts (packet 81/3).
            virtual void onBattleTimeLimit() {}

            /// @brief A fighter's attribute set (packet 51/1).
            /// @param row Cell row.
            /// @param col Cell column.
            /// @param attribute Attribute id.
            /// @param value New value.
            virtual void onFighterAttribute(int row, int col, int attribute, std::int32_t value) {
                (void)row;
                (void)col;
                (void)attribute;
                (void)value;
            }

            /// @brief The fighter on a cell runs off (packet 11/1), is knocked off the field (53/3) or has chosen (53/5).
            /// @param row Cell row.
            /// @param col Cell column.
            virtual void onFighterLeaves(int row, int col) {
                (void)row;
                (void)col;
            }

            /// @brief A status put on a fighter or taken off (packet 53/1).
            /// @param row Cell row.
            /// @param col Cell column.
            /// @param kind Status kind.
            /// @param skillId The skill; 0 takes the status off.
            /// @param place Place of the status.
            virtual void onFighterStatus(
                int row,
                int col,
                int kind,
                int skillId,
                int place
            ) {
                (void)row;
                (void)col;
                (void)kind;
                (void)skillId;
                (void)place;
            }

            /// @brief A fighter is knocked out.
            /// @param row Cell row.
            /// @param col Cell column.
            virtual void onFighterKnockedOut(int row, int col) {
                (void)row;
                (void)col;
            }

            /// @brief An item dropped by the fighter at a cell to the cells (packet 53/4, RevThrowItem).
            /// @param itemId The item.
            /// @param row Thrower's cell row.
            /// @param col Thrower's cell column.
            /// @param targets Target cells.
            virtual void onItemThrown(int itemId, int row, int col, const std::vector<std::pair<int, int>>& targets) {
                (void)itemId;
                (void)row;
                (void)col;
                (void)targets;
            }

            /// @brief A monster met, the monster book's entry (packet 53/9).
            /// @param npcId The monster.
            virtual void onMonsterMet(int npcId) { (void)npcId; }

            /// @brief The monster book's bits by group (packet 53/10).
            /// @param payload The whole message.
            virtual void onMonsterBits(const core::Bytes& payload) { (void)payload; }

            /// @brief A fighter has acted.
            /// @param row Cell row.
            /// @param col Cell column.
            virtual void onFighterActed(int row, int col) {
                (void)row;
                (void)col;
            }

            /// @brief "LevelUP" rises over that player, the own one too with the level-up sound (packet 5/10).
            /// @param playerId The player.
            virtual void onPlayerLevelUp(std::int32_t playerId) { (void)playerId; }

            /// @brief ShowFloatText over the own player, or over the pet out on the map (packet 23/162: a training step gives exp as "ExpUp"; the contest's exp as "ExpUP").
            /// @param pet True for the pet out on the map.
            /// @param text The floating text.
            virtual void onFloatText(bool pet, const std::string& text) { (void)pet, (void)text; }

            /// @brief The training of a player at a utensil is over; only the own one shows anything (packet 23/162 1).
            /// @param playerId The player.
            /// @param flag 2: the pet's, which floats "ExpUp" too.
            virtual void onTrainingOver(std::int32_t playerId, int flag) { (void)playerId, (void)flag; }

            /// @brief The percent of the own player's training or of a pet's, with the exp given as floating text apart (packet 23/162 2 and 3).
            /// @param pet False for the own player, true for a pet.
            /// @param place Place of the team.
            /// @param percent The percent.
            virtual void onTrainingPercent(bool pet, int place, int percent) { (void)pet, (void)place, (void)percent; }

            /// @brief The training percent of the own player or of a pet goes back to 0 (packet 23/162 4 and 5).
            /// @param pet True for a pet.
            /// @param place Place of the team.
            virtual void onTrainingReset(bool pet, int place) { (void)pet, (void)place; }

            /// @brief The pet is being trained (packet 23/162 8).
            virtual void onTrainingBusy() {}

            /// @brief The exp an event gave: "<name> obtain Exp:N" in the chat, "ExpUP" over the player (packet 89/1/1).
            /// @param value The exp.
            virtual void onEventExp(std::int32_t value) { (void)value; }

            /// @brief The TGC contest raised the level of the player or of the pet out on the map (packet 20/42 5 and 7).
            /// @param pet True for the pet.
            /// @param level The new level.
            virtual void onContestLevelUp(bool pet, std::int32_t level) { (void)pet, (void)level; }

            /// @brief A player sat down at a piece of furniture of the house (packet 23/75, UseUtensil).
            /// @param playerId The player.
            /// @param furnIndex The piece's number.
            /// @param seat The seat.
            virtual void onUtensilUsed(std::int32_t playerId, int furnIndex, int seat) { (void)playerId, (void)furnIndex, (void)seat; }

            /// @brief A player got up from the furniture (packet 23/76).
            /// @param playerId The player.
            virtual void onUtensilLeft(std::int32_t playerId) { (void)playerId; }

            /// @brief The ATM's balance grew, fell or is amount (packets 26/8, 26/9 and 26/10).
            /// @param kind Which of the three packets.
            /// @param amount The amount.
            /// @param peerId Who sent it (0 for the own deposit) or who got it.
            virtual void onAtmBalance(int kind, std::int32_t amount, std::int32_t peerId) { (void)kind, (void)amount, (void)peerId; }

            /// @brief The ATM form the server opens (packet 23/165).
            /// @param form 1 deposit and withdrawal, 2 transfer.
            virtual void onAtmForm(int form) { (void)form; }

            /// @brief A prize announcement for the banner across the top (packet 2/15, RecvSystemAnnounce).
            /// @param announce The announcement.
            virtual void onSystemAnnounce(const SystemAnnounce& announce) { (void)announce; }

            /// @brief The system forbids local chat for 8 seconds (packet 2/8, BlockLocalChatBySystem).
            virtual void onChatBlocked() {}

            /// @brief A line of a player (packet 2/10).
            /// @param speakerId The speaker.
            /// @param name The speaker's name.
            /// @param flags Flags; 1 is the "(All)" kind.
            /// @param text The text.
            virtual void onChatLine(std::int32_t speakerId, const std::string& name, int flags, const std::string& text) {
                (void)speakerId, (void)name, (void)flags, (void)text;
            }

            /// @brief A GM silences channels for some minutes (packet 16/7, RecvChatSilence).
            /// @param kind 1 spam, 2 a GM's order.
            /// @param mask Channel mask; bit 0 is channel 1.
            /// @param minutes Minutes silenced.
            virtual void onChatSilenced(int kind, int mask, std::uint32_t minutes) { (void)kind, (void)mask, (void)minutes; }

            /// @brief The silence is over (packets 16/8 with mailToo, and 16/13).
            /// @param mailToo True when mail is released too.
            virtual void onChatSilenceReleased(bool mailToo) { (void)mailToo; }

            /// @brief A player healed another or himself with a skill: the notice lines in the chat (packet 23/37, ShowHelpRecoverMsg).
            /// @param casterId The healer.
            /// @param targetId The healed player.
            /// @param hp HP recovered.
            /// @param sp SP recovered.
            virtual void onHelpRecover(std::int32_t casterId, std::int32_t targetId, std::int32_t hp, int sp) {
                (void)casterId, (void)targetId, (void)hp, (void)sp;
            }

            /// @brief The own player or a pet recovered (packet 23/40, ShowRecoverHpSpMsg).
            /// @param petPlace Pet place; 0 is the own player.
            /// @param hp HP recovered.
            /// @param sp SP recovered.
            virtual void onRecoverNotice(int petPlace, std::int32_t hp, int sp) { (void)petPlace, (void)hp, (void)sp; }

            /// @brief AddTalkMessage on the system channel as the own player (the event notices of action 20).
            /// @param text The line.
            virtual void onOwnSystemLine(const std::string& text) { (void)text; }

            /// @brief Plays a sound file of the game folder (PlaySoundEffect).
            /// @param relative Path relative to the game folder.
            virtual void onSoundEffect(const std::string& relative) { (void)relative; }

            /// @brief What the event of the NPC being talked to announces: the arena, Team PK ... (packet 20/20).
            virtual void onEventAnnounce() {}

            /// @brief The Capture the Evil Corton contest's place: the line in the chat and the short message for the own player (packet 20/26).
            /// @param place The place.
            /// @param playerId The player.
            /// @param name The player's name.
            virtual void onCortonPlace(int place, std::int32_t playerId, const std::string& name) { (void)place, (void)playerId, (void)name; }

            /// @brief The short message of a text of Talk.Dat (packet 20/53).
            /// @param talkId The Talk.Dat text.
            /// @param durationMs Display time in milliseconds.
            virtual void onTalkNotice(int talkId, int durationMs) { (void)talkId, (void)durationMs; }

            /// @brief A praise from the game, said at the top of the screen by the own player (packet 5/22, ShowPraiseMsg).
            /// @param grade Grade 1..3.
            virtual void onPraise(int grade) { (void)grade; }

            /// @brief The double exp state (packet 5/14, HandleDoubleExpPacket).
            /// @param kind The state kind.
            /// @param minutes Minutes left, when the packet has them.
            virtual void onDoubleExp(int kind, std::optional<int> minutes) { (void)kind, (void)minutes; }

            /// @brief The Lucky Grass state (packet 5/15, HandleLuckyGrassPacket).
            /// @param kind The state kind.
            /// @param minutes Minutes.
            virtual void onLuckyGrass(int kind, int minutes) { (void)kind, (void)minutes; }

            /// @brief The Spa Bun state (packet 5/16, HandleSpaBunPacket).
            /// @param kind The state kind.
            /// @param value Optional value.
            virtual void onSpaBuns(int kind, std::optional<int> value) { (void)kind, (void)value; }

            /// @brief The exp potion state (packet 5/17, HandleExpPotionPacket).
            /// @param kind 1 the icon (first: active), 2 a refusal (first: the reason), 3 the time left, 4 expired.
            /// @param first First value, see kind.
            /// @param minutes Minutes left.
            /// @param rate Exp rate.
            virtual void onExpPotion(int kind, int first, int minutes, int rate) { (void)kind, (void)first, (void)minutes, (void)rate; }

            /// @brief The result of sending a flower (packet 25/31, RecvFlowerSendResult).
            /// @param result 1 sent, 2 the target's bag is full, 3 not enough money.
            virtual void onFlowerSent(int result) { (void)result; }

            /// @brief A player sent a flower (packet 25/32, RecvFlowerReceived).
            /// @param senderId The sender.
            virtual void onFlowerReceived(std::int32_t senderId) { (void)senderId; }

            /// @brief The lists of players that stopped or began collecting automatically, a Vacuum Cleaner at a gathering spot (packets 23/137 and 23/138).
            /// @param playerIds The players.
            /// @param collecting True when they began collecting.
            virtual void onAutoCollecting(const std::vector<std::int32_t>& playerIds, bool collecting) { (void)playerIds, (void)collecting; }

            /// @brief The level of a player of the scene (packet 5/18).
            /// @param playerId The player.
            /// @param level The level.
            virtual void onPlayerLevel(std::int32_t playerId, int level) { (void)playerId, (void)level; }

            /// @brief A player sits in a tub (packet 23/73).
            /// @param playerId The player.
            /// @param furnIndex The piece.
            /// @param seat The seat.
            /// @param sitterId The player whose sitting brought him there, 0 for none.
            virtual void onBathing(std::int32_t playerId, int furnIndex, int seat, std::int32_t sitterId) {
                (void)playerId, (void)furnIndex, (void)seat, (void)sitterId;
            }

            /// @brief A player gets out of the tub (packet 23/74).
            /// @param playerId The player.
            virtual void onBathEnd(std::int32_t playerId) { (void)playerId; }

            /// @brief A player threw an item to a map point (packet 23/117, HandleThrowFxPacket).
            /// @param kind Item kind 1..3.
            /// @param playerId The player.
            /// @param x Map x.
            /// @param y Map y.
            virtual void onThrowEffect(int kind, std::int32_t playerId, int x, int y) { (void)kind, (void)playerId, (void)x, (void)y; }

            /// @brief A player's fireworks (packet 23/148, HandleEffectS51016Packet).
            /// @param playerId The player.
            /// @param kind Kind 1..4.
            /// @param variant The variant.
            virtual void onFirework(std::int32_t playerId, int kind, int variant) { (void)playerId, (void)kind, (void)variant; }

            /// @brief The server asks whether an item is to be destroyed (packet 23/212, RecvDiscardConfirm).
            /// @param reason 1 under level 10, 2 not enough time online, 3 a style piece, else plain.
            /// @param bagSlot Bag place.
            /// @param itemId The item.
            /// @param count How many.
            virtual void onDiscardAsked(int reason, int bagSlot, int itemId, int count) { (void)reason, (void)bagSlot, (void)itemId, (void)count; }

            /// @brief A worn piece of the player or of a pet is destroyed (packet 23/35, RecvEquipBroken).
            /// @param owner 0 the player, 1 to 4 a pet.
            /// @param itemId The item.
            virtual void onEquipBroken(int owner, int itemId) { (void)owner, (void)itemId; }

            /// @brief The stone is set in the piece of that bag place (packet 23/77, RecvCrystalMounted).
            /// @param bagSlot Bag place.
            /// @param stoneId The stone.
            virtual void onCrystalMounted(int bagSlot, int stoneId) { (void)bagSlot, (void)stoneId; }

            /// @brief The Training Island item's effect (packet 23/106, HandleTrainingIslandItemPacket).
            /// @param kind 1 runs, 2 over, 3 already running, 4 at its limit.
            /// @param effect The effect.
            /// @param minutes Minutes left.
            /// @param sound Whether a sound is played.
            virtual void onTrainingItem(int kind, int effect, int minutes, bool sound) { (void)kind, (void)effect, (void)minutes, (void)sound; }

            /// @brief The minutes left of the five stat buffs (packet 23/119 1, HandleStrengthenBuffPacket).
            /// @param minutes Minutes per buff.
            virtual void onStrengthBuffTimes(const std::array<int, 5>& minutes) { (void)minutes; }

            /// @brief The mask of the buffs that run now (packet 23/119 2).
            /// @param mask Buff mask.
            /// @param sound Whether a sound is played.
            virtual void onStrengthBuffMask(std::uint32_t mask, bool sound) { (void)mask, (void)sound; }

            /// @brief The Stopwatch's pause of the double exp (packet 23/204).
            /// @param sub 1 the state at login, 2 it was used or cancelled.
            /// @param paused Whether it is paused.
            virtual void onExpPause(int sub, bool paused) { (void)sub, (void)paused; }

            /// @brief A stat was enhanced (packet 23/206).
            /// @param stat 0 to 4 increased by 1, 5 the pet can't be enhanced any more.
            virtual void onStatEnhanced(int stat) { (void)stat; }

            /// @brief The body shift (packet 23/207, TChangeBody).
            /// @param sub 1 the state of the body shift, 2 the minutes left (the question), 3 a refusal.
            /// @param value The sub-specific value.
            virtual void onBodyShift(int sub, int value) { (void)sub, (void)value; }

            /// @brief Another player took an item off or put it on (packets 5/1 and 5/2, RecvPlayerEquipChange).
            /// @param playerId The player.
            /// @param itemId The item.
            /// @param worn True when put on.
            virtual void onPlayerEquipChanged(std::int32_t playerId, int itemId, bool worn) { (void)playerId, (void)itemId, (void)worn; }

            /// @brief Everything another player wears (packet 5/0, RecvPlayerEquipList).
            /// @param playerId The player.
            /// @param items The worn items.
            virtual void onPlayerEquipList(std::int32_t playerId, const std::vector<int>& items) { (void)playerId, (void)items; }

            /// @brief The status of a player of the scene in the friend and guild lists (packet 5/8, RecvPlayerOnlineStatus).
            /// @param playerId The player.
            /// @param status The status.
            virtual void onPlayerOnlineStatus(std::int32_t playerId, int status) { (void)playerId, (void)status; }

            /// @brief The player looks like that NPC of Npc.dat (packet 5/5, RecvHumanTransform).
            /// @param playerId The player.
            /// @param npcId The NPC; 0 is his own look again.
            virtual void onHumanTransform(std::int32_t playerId, int npcId) { (void)playerId, (void)npcId; }

            /// @brief The job of a player of the scene, the own player included (packet 5/23, RecvPlayerJob).
            /// @param playerId The player.
            /// @param job The job.
            virtual void onPlayerJob(std::int32_t playerId, int job) { (void)playerId, (void)job; }

            /// @brief A pet exchange voucher gave an item, the centre message (packet 83/6).
            /// @param itemId The item.
            virtual void onVoucherItem(int itemId) { (void)itemId; }

            /// @brief Skills the player forgot (packet 67/5).
            /// @param skills Their ids.
            virtual void onSkillsForgotten(const std::vector<int>& skills) { (void)skills; }

            /// @brief The own player stops walking (packet 6/2).
            /// @param locked True when he may not walk until the event ends.
            virtual void onWalkStopped(bool locked) { (void)locked; }

            /// @brief A player is put somewhere else on the map (packet 7).
            /// @param playerId The player.
            /// @param position New map point.
            virtual void onPlayerJumped(std::int32_t playerId, core::Point position) {
                (void)playerId;
                (void)position;
            }

            /// @brief A chat line of a channel (packet 2/x).
            /// @param channel 0 system, 1 world, 2 local, 3 whisper, 4 GM, 5 team, 6 guild, 7 ally.
            /// @param speakerId The speaker; 0 is the system.
            /// @param text The text.
            virtual void onChat(int channel, std::int32_t speakerId, const std::string& text) {
                (void)channel;
                (void)speakerId;
                (void)text;
            }

            /// @brief The GM status of the player (packet 16/200, the qol server only).
            /// @param flags Bit 0 the player is a GM, bit 1 admin mode is on (/gamemode admin).
            virtual void onGmStatus(int flags) { (void)flags; }

            /// @brief The GM's question (packet 16/1, TAC_Question).
            /// @param kind The kind byte (Player.FGmQuestion).
            /// @param question The question.
            /// @param choices The four choices.
            virtual void onGmQuestion(int kind, const std::string& question, const std::vector<std::string>& choices) {
                (void)kind;
                (void)question;
                (void)choices;
            }

            /// @brief All of the own player's attributes (packet 5/3).
            /// @param attributes The attributes.
            virtual void onOwnAttributes(const protocol::OwnAttributes& attributes) { (void)attributes; }

            /// @brief One attribute of the own player (packet 8/1): 25 HP, 26 SP, 36 exp, 205 / 206 max HP / SP ...
            /// @param attribute Attribute id.
            /// @param value New value.
            /// @param extra The skill for 110 skill level / 111 skill experience.
            virtual void onAttributeChanged(int attribute, std::int32_t value, std::int32_t extra) {
                (void)attribute;
                (void)value;
                (void)extra;
            }

            /// @brief The gold changed by an amount, or is that amount (packet 26/x).
            /// @param amount The amount.
            /// @param absolute True when amount is the new total.
            virtual void onGoldChanged(std::int32_t amount, bool absolute) {
                (void)amount;
                (void)absolute;
            }

            /// @brief The bag (packet 23/5).
            /// @param records The items and their places.
            virtual void onBagItems(const std::vector<protocol::BagItemRecord>& records) { (void)records; }

            /// @brief Items added to a place of the bag (packet 23/8).
            /// @param record The item and its place.
            virtual void onBagItemAdded(const protocol::BagItemRecord& record) { (void)record; }

            /// @brief Count of an item are gone from the bag (packet 23/7).
            /// @param itemId The item.
            /// @param count How many.
            virtual void onBagItemsRemoved(int itemId, int count) {
                (void)itemId;
                (void)count;
            }

            /// @brief Count are gone from a place of the bag (packet 23/9).
            /// @param slot Bag place.
            /// @param count How many.
            virtual void onBagSlotDecreased(int slot, int count) {
                (void)slot;
                (void)count;
            }

            /// @brief Items moved within the bag (packet 23/10).
            /// @param fromSlot Source place.
            /// @param count How many.
            /// @param toSlot Target place.
            virtual void onBagItemsMoved(int fromSlot, int count, int toSlot) {
                (void)fromSlot;
                (void)count;
                (void)toSlot;
            }

            /// @brief What the own player wears (packet 23/11).
            /// @param items The worn items.
            virtual void onEquipment(const std::vector<protocol::ItemInstance>& items) { (void)items; }

            /// @brief A worn item is gone (packets 23/12 destroyed and 23/19).
            /// @param slot Equipment place.
            /// @param destroyed True for 23/12 (destroyed).
            virtual void onEquipmentRemoved(int slot, bool destroyed) {
                (void)slot;
                (void)destroyed;
            }

            /// @brief A worn item went into the bag (packet 23/16).
            /// @param equipSlot Equipment place.
            /// @param bagSlot Bag place.
            virtual void onUnequipped(int equipSlot, int bagSlot) {
                (void)equipSlot;
                (void)bagSlot;
            }

            /// @brief A bag item was put on; what was worn went to freeSlot (packet 23/17).
            /// @param bagSlot Bag place of the item.
            /// @param freeSlot Bag place that received the worn item.
            virtual void onEquippedFromBag(int bagSlot, int freeSlot) {
                (void)bagSlot;
                (void)freeSlot;
            }

            /// @brief A pet's worn item is gone (packets 23/21 and 23/51).
            /// @param place The pet's place.
            /// @param slot Equipment place.
            /// @param told True for 23/21 (destroyed, told).
            virtual void onPetEquipmentRemoved(int place, int slot, bool told) {
                (void)place;
                (void)slot;
                (void)told;
            }

            /// @brief A pet's worn item went into the bag (packet 23/22).
            /// @param place The pet's place.
            /// @param equipSlot Equipment place.
            /// @param bagSlot Bag place.
            virtual void onPetUnequipped(int place, int equipSlot, int bagSlot) {
                (void)place;
                (void)equipSlot;
                (void)bagSlot;
            }

            /// @brief A bag item was put on a pet; what it wore there went to the place of the bag the item came from (packet 23/23).
            /// @param place The pet's place.
            /// @param bagSlot Bag place of the item.
            virtual void onPetEquippedFromBag(int place, int bagSlot) {
                (void)place;
                (void)bagSlot;
            }

            /// @brief A pet wears an item (packet 23/24).
            /// @param place The pet's place.
            /// @param item The item.
            virtual void onPetEquipmentSet(int place, const protocol::ItemInstance& item) {
                (void)place;
                (void)item;
            }

            /// @brief The wear of a worn item (packets 23/27 and 23/28).
            /// @param place 0 the player's, else a pet's place.
            /// @param slot Equipment place.
            /// @param wear The wear.
            virtual void onEquipmentWear(int place, int slot, int wear) {
                (void)place;
                (void)slot;
                (void)wear;
            }

            /// @brief The wear of an item in a container (packets 23/168 and 23/140/1, RecvThingWearUpdate).
            /// @param container 1 the bag, 2 the worn items, 3 a pet's worn items by its place.
            /// @param slot Place in the container.
            /// @param wear The wear.
            /// @param petPlace The pet's place, for container 3.
            virtual void onThingWear(int container, int slot, std::int32_t wear, int petPlace) {
                (void)container;
                (void)slot;
                (void)wear;
                (void)petPlace;
            }

            /// @brief The teleport item in a bag place is free again (packet 23/169, RecvMagicMapRelease).
            /// @param bagSlot Bag place.
            virtual void onMagicMapRelease(int bagSlot) { (void)bagSlot; }

            /// @brief The usage period of an item ran out (packet 23/140/2, RecvThingExpired).
            /// @param container 1 bag, 2 worn, 3 a pet's gear, 4 warehouse, 5 another player's gear, 6 a piece of furniture, 7 bank, 8 a pet at the inn, 9 the bonus space.
            /// @param slot Place in the container.
            /// @param petPlace The pet's place.
            /// @param newItemId The item that replaces it (0: none).
            /// @param itemId For containers 5 and 6, the item.
            /// @param owner For container 5, whose gear it was.
            virtual void onThingExpired(
                int container,
                int slot,
                int petPlace,
                int newItemId,
                int itemId,
                const std::string& owner
            ) {
                (void)container;
                (void)slot;
                (void)petPlace;
                (void)newItemId;
                (void)itemId;
                (void)owner;
            }

            /// @brief The Space Contract (packet 23/136).
            /// @param kind 1 and 2 the Item Bag may (not) be used, 3 and 4 the panel over the hotel's last rows hides (shows), 5 the extra space that is open, 6 it ended, 7 the hours left, 8 and 9 the Garage form's state, 10 the upper limit.
            /// @param flags For kind 5: 4 tent, 2 hotel, 1 stock keeper, else garage.
            /// @param hours Time left in hours.
            virtual void onSpaceContract(int kind, int flags, std::int32_t hours) {
                (void)kind;
                (void)flags;
                (void)hours;
            }

            /// @brief The order of the client for the Item Bag is done (packets 68/1-68/3).
            /// @param kind 1 cell a to bag place b, 2 cell a to cell b, 3 bag place a to cell b.
            /// @param first Source.
            /// @param second Target.
            /// @param count How many.
            virtual void onItemBagMoved(int kind, int first, int second, int count) {
                (void)kind;
                (void)first;
                (void)second;
                (void)count;
            }

            /// @brief The whole Item Bag grid (packet 68/4).
            /// @param records The items and their cells.
            virtual void onItemBagList(const std::vector<protocol::BagItemRecord>& records) { (void)records; }

            /// @brief The Item Bag form shows (packet 68/9).
            virtual void onItemBagOpened() {}

            /// @brief The server stops the Remote Control (packet 23/79, ResetAutoPlayState).
            virtual void onRemoteReset() {}

            /// @brief The Magic Magnet (packet 23/80).
            /// @param kind 1 the player is asked to be teleported (id: who asks), 2 the wearer's request is on its way (id: the asked one, arg: the magnet's bag place), 3 the outcome (arg: its code).
            /// @param id Player id, see kind.
            /// @param arg Argument, see kind.
            virtual void onMagnetNotice(int kind, std::int32_t id, int arg) {
                (void)kind;
                (void)id;
                (void)arg;
            }

            /// @brief The vehicle on show on the house's Show Platforms (packet 23/144).
            /// @param cell The owner's Garage cell of it.
            /// @param itemId The vehicle item.
            virtual void onExhibitState(int cell, int itemId) {
                (void)cell;
                (void)itemId;
            }

            /// @brief The potential of the player or of the pet in a place is now this (packet 23/213).
            /// @param petPlace Pet place; 0 is the player.
            /// @param potential The potential.
            virtual void onPotentialResult(int petPlace, int potential) {
                (void)petPlace;
                (void)potential;
            }

            /// @brief How much of the potion in a bag place is used up (packet 23/208 1 and 3).
            /// @param bagSlot Bag place.
            /// @param amount The amount.
            /// @param add True when added (with the sound), false when set.
            virtual void onPotionUsedAmount(int bagSlot, int amount, bool add) {
                (void)bagSlot;
                (void)amount;
                (void)add;
            }

            /// @brief What the HP or SP Bottle holds now (packet 23/208 2).
            /// @param kind 1 and 3 the HP Bottle, 2 and 4 the SP Bottle; 1 and 2 tell it with the sound.
            /// @param amount What it holds.
            virtual void onBottlePool(int kind, int amount) {
                (void)kind;
                (void)amount;
            }

            /// @brief An item was used (packet 23/15).
            virtual void onItemUsed() {}

            /// @brief The own pets (packet 15/8).
            /// @param pets The pets.
            virtual void onPetList(const std::vector<protocol::PetRecord>& pets) { (void)pets; }

            /// @brief A pet joined a player's team (packet 15/1).
            /// @param joined The joining pet.
            virtual void onPetJoined(const protocol::PetJoined& joined) { (void)joined; }

            /// @brief A pet left a player's team (packet 15/2).
            /// @param ownerId The owner.
            /// @param place The pet's place.
            virtual void onPetLeft(std::int32_t ownerId, int place) {
                (void)ownerId;
                (void)place;
            }

            /// @brief A pet was renamed (packet 15/9).
            /// @param ownerId The owner.
            /// @param place The pet's place.
            /// @param name The new name.
            virtual void onPetRenamed(std::int32_t ownerId, int place, const std::string& name) {
                (void)ownerId;
                (void)place;
                (void)name;
            }

            /// @brief The own pet that fights (packets 19/1 with npcId, 19/2 with 0, 19/4 silent at login).
            /// @param npcId The pet's kind.
            /// @param silent True for 19/4 (at login).
            virtual void onActivePet(int npcId, bool silent) {
                (void)npcId;
                (void)silent;
            }

            /// @brief The pet of a place died in a city battle, or is back (packet 19/6).
            /// @param place The pet's place, 1..4.
            /// @param dead True when it died.
            virtual void onCityBattleDeadPet(int place, bool dead) {
                (void)place;
                (void)dead;
            }

            /// @brief The Dreaming Pill state (packet 74/3, HandleDreamingPillPacket).
            /// @param kind 1 minutes left, 2 on, 3 expired, 4 in use.
            virtual void onDreamingPill(int kind) { (void)kind; }

            /// @brief Sets Player.FDarkMode (packet 23/41).
            /// @param dark Whether dark mode is on.
            virtual void onDarkMode(bool dark) { (void)dark; }

            /// @brief RevertItemVariantsTo20747 (packet 23/42).
            virtual void onRevertItemVariants() {}

            /// @brief ShowPetSublimateFailMsg (packet 23/56).
            /// @param code The failure code.
            virtual void onPetSublimateFailed(int code) { (void)code; }

            /// @brief ForceLeaveWater (packet 23/121).
            virtual void onForceLeaveWater() {}

            /// @brief Sets Player.FNpcsHidden (packet 20/33).
            /// @param hidden Whether the NPCs are hidden.
            virtual void onNpcsHidden(bool hidden) { (void)hidden; }

            /// @brief Sets Player.FPrevMapId (packet 20/34).
            /// @param mapId The previous map.
            virtual void onPreviousMap(int mapId) { (void)mapId; }

            /// @brief Other players' fighting pets (packet 15/4).
            /// @param pets The pets.
            virtual void onScenePets(const std::vector<protocol::ScenePet>& pets) { (void)pets; }

            /// @brief The pets of a player of the scene, one of them ridden the old way (packet 15/7, RecvPetMounted).
            /// @param pets The pets.
            virtual void onPetsMounted(const protocol::PetsMounted& pets) { (void)pets; }

            /// @brief A player starts fishing (packet 23/123).
            /// @param playerId The player.
            /// @param rodKind Rod kind, the item rank.
            virtual void onFishingStarted(std::int32_t playerId, int rodKind) {
                (void)playerId;
                (void)rodKind;
            }

            /// @brief A player stops fishing (packet 23/122).
            /// @param playerId The player.
            virtual void onFishingStopped(std::int32_t playerId) { (void)playerId; }

            /// @brief The event schedule (packet 90/0).
            /// @param list The activities.
            virtual void onActivityList(const protocol::ActivityList& list) { (void)list; }

            /// @brief The states of the scheduled events (packet 90/1).
            /// @param states The states.
            virtual void onActivityStates(const protocol::ActivityStates& states) { (void)states; }

            /// @brief An event schedule notice (packet 90/2).
            /// @param kind 0 busy, 1 updated.
            virtual void onActivityNotice(int kind) { (void)kind; }

            /// @brief The online draw's prizes and the draws used (packet 104/1/1).
            /// @param info The lottery lists.
            virtual void onLotteryInfo(const protocol::LotteryInfo& info) { (void)info; }

            /// @brief The prize won (packet 104/1/2).
            /// @param result The draw result.
            virtual void onLotteryResult(const protocol::LotteryResult& result) { (void)result; }

            /// @brief A player rides his pet in a place (packet 15/16, HandleRidePetMountPacket).
            /// @param playerId The player.
            /// @param place The pet's place.
            /// @param npcId The pet's kind.
            virtual void onPetMounted(std::int32_t playerId, int place, int npcId) {
                (void)playerId;
                (void)place;
                (void)npcId;
            }

            /// @brief A player gets off his pet (packet 15/17).
            /// @param playerId The player.
            virtual void onPetDismounted(std::int32_t playerId) { (void)playerId; }

            /// @brief The pet out beside another player wears an item (packet 23/69, HandlePetEquipChangePacket).
            /// @param playerId The owner.
            /// @param itemId The item.
            virtual void onScenePetEquipped(std::int32_t playerId, int itemId) { (void)playerId, (void)itemId; }

            /// @brief A player's pet out puts its weapon away (packet 23/70, HandlePetUnequipPacket).
            /// @param playerId The owner.
            virtual void onScenePetUnequipped(std::int32_t playerId) { (void)playerId; }

            /// @brief The players of the scene riding pets (packet 23/116, HandleRidePetListPacket).
            /// @param rides The rides.
            virtual void onPetRides(const std::vector<protocol::PetRide>& rides) { (void)rides; }

            /// @brief Whether a player stands as the riding figure, a record of RecvPlayerFlagList (packet 5/19, FRidingPet).
            /// @param playerId The player.
            /// @param riding Whether he rides.
            virtual void onPlayerRidingFlag(std::int32_t playerId, bool riding) { (void)playerId, (void)riding; }

            /// @brief Whether a player is disguised in the Trojan War, a record of RecvTrojanWarTargets (packet 5/20, FTrojanWarFlag).
            /// @param playerId The player.
            /// @param disguised Whether he is disguised.
            virtual void onTrojanWarFlag(std::int32_t playerId, bool disguised) { (void)playerId, (void)disguised; }

            /// @brief A vehicle is put down at a map point (packets 15/12 and 15/18, HandleVehiclePlacedPacket).
            /// @param bagSlot Bag place of the vehicle.
            /// @param ownerId The owner.
            /// @param itemId The vehicle item.
            /// @param position Map point.
            /// @param board True for 15/18: the owner gets on it too.
            virtual void onVehiclePlaced(
                int bagSlot,
                std::int32_t ownerId,
                int itemId,
                core::Point position,
                bool board
            ) {
                (void)bagSlot;
                (void)ownerId;
                (void)itemId;
                (void)position;
                (void)board;
            }

            /// @brief A player gets on his vehicle (packet 15/10, HandleVehicleBoardedPacket).
            /// @param bagSlot Bag place of the vehicle.
            /// @param playerId The player.
            /// @param itemId The vehicle item.
            virtual void onVehicleBoarded(int bagSlot, std::int32_t playerId, int itemId) {
                (void)bagSlot;
                (void)playerId;
                (void)itemId;
            }

            /// @brief A player gets off his vehicle (packet 15/11, HandleVehicleDismountPacket).
            /// @param bagSlot Bag place of the vehicle.
            /// @param playerId The player.
            virtual void onVehicleDismounted(int bagSlot, std::int32_t playerId) {
                (void)bagSlot;
                (void)playerId;
            }

            /// @brief The own vehicle of a bag place goes back into the bag (packet 15/13).
            /// @param bagSlot Bag place.
            virtual void onVehicleTakenBack(int bagSlot) { (void)bagSlot; }

            /// @brief A player's vehicle leaves the map (packet 15/22).
            /// @param ownerId The owner.
            /// @param itemId The vehicle item.
            virtual void onVehicleRemoved(std::int32_t ownerId, int itemId) {
                (void)ownerId;
                (void)itemId;
            }

            /// @brief A vehicle broke at its owner's feet (packet 15/15).
            /// @param ownerId The owner.
            /// @param itemId The vehicle item.
            virtual void onVehicleDestroyed(std::int32_t ownerId, int itemId) {
                (void)ownerId;
                (void)itemId;
            }

            /// @brief The wear and the fuel of the vehicle of a bag place (packet 15/14, HandleVehicleWearPacket).
            /// @param bagSlot Bag place.
            /// @param ownerId The owner.
            /// @param wear The wear.
            /// @param fuel The fuel.
            virtual void onVehicleWear(int bagSlot, std::int32_t ownerId, std::int32_t wear, int fuel) {
                (void)bagSlot;
                (void)ownerId;
                (void)wear;
                (void)fuel;
            }

            /// @brief The vehicle has no fuel (packet 15/23).
            virtual void onVehicleNoFuel() {}

            /// @brief The potential of the pet in a place was taken off, the Anti-Potential Pill (packet 23/216 1).
            /// @param petPlace The pet's place.
            virtual void onPotentialReturned(int petPlace) { (void)petPlace; }

            /// @brief The server sorts a grid (packet 23/250, the qol server only).
            /// @param op 0 the server sorts, 1 the grid is to be emptied (its list follows), 2 the grid's sort ended with result.
            /// @param grid Which grid.
            /// @param result For op 2: 0 sorted, 1 already in order, 2 not now, 3 not here.
            virtual void onServerSort(int op, int grid, int result) {
                (void)op;
                (void)grid;
                (void)result;
            }

            /// @brief The Store Tank's row now holds this fuel (packet 23/219).
            /// @param kind 1 put in, 2 taken out.
            /// @param row Tank row.
            /// @param itemId The fuel item.
            /// @param count How many.
            virtual void onStoreTank(int kind, int row, int itemId, int count) {
                (void)kind;
                (void)row;
                (void)itemId;
                (void)count;
            }

            /// @brief The four cells of the parking lot (packet 23/153).
            /// @param cells The cells.
            virtual void onParkList(const std::vector<protocol::ParkedVehicle>& cells) { (void)cells; }

            /// @brief One cell of the parking lot (packet 23/154).
            /// @param cell The cell.
            virtual void onParkSlot(const protocol::ParkedVehicle& cell) { (void)cell; }

            /// @brief The answer to parking or redeeming a vehicle (packet 23/155).
            /// @param code Result code 1..6.
            /// @param itemId The vehicle's item, for codes 1 and 3.
            virtual void onParkResult(int code, int itemId) {
                (void)code;
                (void)itemId;
            }

            /// @brief The parking lot's cells are emptied (packet 23/156).
            virtual void onParkClear() {}

            /// @brief The parking form (packet 23/157).
            /// @param code 1 shows the form, 2 says the space is not enough.
            virtual void onParkOpen(int code) { (void)code; }

            /// @brief The garage's repair form (packet 23/81).
            /// @param code 0 opens it, 1 the work started, 2 done with the new wear, 3 failed, 4 not enough points.
            /// @param wear The new wear, for code 2.
            virtual void onVehicleRepair(int code, std::int32_t wear) {
                (void)code;
                (void)wear;
            }

            /// @brief The vehicles standing on the map (packet 23/102).
            /// @param vehicles The vehicles.
            virtual void onSceneVehicles(const std::vector<protocol::SceneVehicle>& vehicles) { (void)vehicles; }

            /// @brief The players riding (packet 23/115).
            /// @param riders The riders.
            virtual void onSceneRiders(const std::vector<protocol::SceneRider>& riders) { (void)riders; }

            /// @brief The stars the player has won (packet 15/19).
            /// @param payload The whole message.
            virtual void onStarsOwned(const core::Bytes& payload) { (void)payload; }

            /// @brief A new star won (packet 15/20).
            /// @param star The star.
            /// @param npcIndex The map NPC it came from, 0 the player himself.
            virtual void onStarAcquired(int star, int npcIndex) {
                (void)star;
                (void)npcIndex;
            }

            /// @brief Another player's pet rests (packet 19/7).
            /// @param playerId The owner.
            virtual void onScenePetRests(std::int32_t playerId) { (void)playerId; }

            /// @brief A player asks to join the own team (packet 13/1).
            /// @param requesterId The asking player.
            virtual void onTeamJoinRequest(std::int32_t requesterId) { (void)requesterId; }

            /// @brief The asked player is busy (packet 13/2).
            virtual void onTeamBusy() {}

            /// @brief The answer to the own request to join (packet 13/3).
            /// @param answer 1 accept, 2 refuse, 3 no answer.
            /// @param leaderId The team leader.
            virtual void onTeamJoinAnswer(int answer, std::int32_t leaderId) {
                (void)answer;
                (void)leaderId;
            }

            /// @brief A player left a team; a leader's id means the team is gone (packet 13/4).
            /// @param playerId The player.
            virtual void onTeamLeft(std::int32_t playerId) { (void)playerId; }

            /// @brief A member joined a leader (packet 13/5).
            /// @param leaderId The leader.
            /// @param memberId The member.
            virtual void onTeamJoined(std::int32_t leaderId, std::int32_t memberId) {
                (void)leaderId;
                (void)memberId;
            }

            /// @brief The teams on the map (packet 13/6).
            /// @param teams The teams.
            virtual void onSceneTeams(const std::vector<protocol::SceneTeam>& teams) { (void)teams; }

            /// @brief An invitation into a team (packet 13/9).
            /// @param inviterId The inviting player.
            virtual void onTeamInvite(std::int32_t inviterId) { (void)inviterId; }

            /// @brief The answer to the own invitation (packet 13/10).
            /// @param answer The answer code.
            /// @param inviteeId The invited player.
            virtual void onTeamInviteAnswer(int answer, std::int32_t inviteeId) {
                (void)answer;
                (void)inviteeId;
            }

            /// @brief The counsellor's fan icon shows over these players, or goes (packets 13/11 and 13/12).
            /// @param ids The players.
            /// @param show True to show the icon.
            virtual void onTeamFanIcon(const std::vector<std::int32_t>& ids, bool show) {
                (void)ids;
                (void)show;
            }

            /// @brief Why a shapeshifted player cannot do something (packet 13/14).
            /// @param reason 1 ride, 2 and 3 bathe.
            virtual void onTeamShapeRefusal(int reason) { (void)reason; }

            /// @brief The leader offers the lead (packet 13/15 1).
            /// @param leaderId The leader.
            virtual void onTeamLeaderOffer(std::int32_t leaderId) { (void)leaderId; }

            /// @brief The answer to the own offer of the lead (packet 13/15 2).
            /// @param answer 2 refused by the id, 3 no answer, 4 nothing to show, 5 busy.
            /// @param memberId The member asked.
            virtual void onTeamLeaderAnswer(int answer, std::int32_t memberId) {
                (void)answer;
                (void)memberId;
            }

            /// @brief The lead went from one player to another (packet 13/15 3, RecvTeamLeaderChanged, CY_Team.pas:565-665); the NEW leader's id comes first.
            /// @param newLeaderId The new leader.
            /// @param oldLeaderId The old leader.
            virtual void onTeamLeaderChanged(std::int32_t newLeaderId, std::int32_t oldLeaderId) {
                (void)newLeaderId;
                (void)oldLeaderId;
            }

            /// @brief Why a team could not be formed (packet 13/13).
            /// @param code The reason code.
            virtual void onTeamJoinError(int code) { (void)code; }

            /// @brief Mail (packet 14/1).
            /// @param senderId The sender.
            /// @param sendTime The time as a TDateTime.
            /// @param text The mail text.
            virtual void onMail(std::int32_t senderId, double sendTime, const std::string& text) {
                (void)senderId;
                (void)sendTime;
                (void)text;
            }

            /// @brief Guild mail (packet 39/5).
            /// @param senderId The sender.
            /// @param sendTime The time as a TDateTime.
            /// @param text The mail text.
            virtual void onGuildMail(std::int32_t senderId, double sendTime, const std::string& text) {
                (void)senderId;
                (void)sendTime;
                (void)text;
            }

            /// @brief Mail by a name (packet 14/16 2).
            /// @param senderId The sender.
            /// @param senderName The sender's name as the server gives it, "" for none.
            /// @param sendTime The time as a TDateTime.
            /// @param text The mail text.
            virtual void onFurMail(std::int32_t senderId, const std::string& senderName, double sendTime, const std::string& text) {
                (void)senderId;
                (void)senderName;
                (void)sendTime;
                (void)text;
            }

            /// @brief The result of a mail by a name (packet 14/16 1).
            /// @param code 1 no such player, 2 and 4 no mail box, 3 has a mail box.
            virtual void onFurMailResult(int code) { (void)code; }

            /// @brief The player stands at a mail box (packet 14/16 3).
            virtual void onFurMailBox() {}

            /// @brief The receiver's mail box is full (packet 14/6).
            virtual void onMailboxFull() {}

            /// @brief Too much mail sent (packet 14/10).
            virtual void onSentMailFull() {}

            /// @brief A player asks to be friends (packet 14/2).
            /// @param playerId The asking player.
            virtual void onFriendRequest(std::int32_t playerId) { (void)playerId; }

            /// @brief The answer to the own friend request (packet 14/3).
            /// @param playerId The asked player.
            /// @param answer 1 yes, 2 no, 3 no answer, else busy.
            virtual void onFriendAnswer(std::int32_t playerId, int answer) {
                (void)playerId;
                (void)answer;
            }

            /// @brief A friend left the list (packet 14/4).
            /// @param friendId The friend.
            virtual void onFriendRemoved(std::int32_t friendId) { (void)friendId; }

            /// @brief The whole friend list (packet 14/5).
            /// @param friends The friends.
            virtual void onFriendList(const std::vector<protocol::FriendRecord>& friends) { (void)friends; }

            /// @brief A player's guild name (packet 14/7).
            /// @param playerId The player.
            /// @param guild The guild name.
            virtual void onGuildName(std::int32_t playerId, const std::string& guild) {
                (void)playerId;
                (void)guild;
            }

            /// @brief A friend's record (packet 14/8).
            /// @param record The record.
            virtual void onFriendInfo(const protocol::FriendRecord& record) { (void)record; }

            /// @brief Friends' online states (packet 14/9).
            /// @param states The states.
            virtual void onFriendStates(const std::vector<protocol::FriendState>& states) { (void)states; }

            /// @brief A friend's record, added when new (packet 14/11).
            /// @param record The record.
            virtual void onFriendEntry(const protocol::FriendRecord& record) { (void)record; }

            /// @brief The asked player has no room for friends (packet 14/12).
            virtual void onFriendListFull() {}

            /// @brief A player's nickname (packet 10/1).
            /// @param playerId The player.
            /// @param nickname The nickname.
            virtual void onNickname(std::int32_t playerId, const std::string& nickname) {
                (void)playerId;
                (void)nickname;
            }

            /// @brief A player's heart number (packets 10/2, 10/3).
            /// @param playerId The player.
            /// @param heartNumber The heart number.
            virtual void onHeartNumber(std::int32_t playerId, int heartNumber) {
                (void)playerId;
                (void)heartNumber;
            }

            /// @brief The own item stall opened (packet 23/30).
            /// @param stall The stall.
            virtual void onOwnItemStall(const protocol::OwnStall& stall) { (void)stall; }

            /// @brief Item stalls kept on the map (packet 23/31).
            /// @param stalls The stall signs.
            virtual void onItemStalls(const std::vector<protocol::StallSignRecord>& stalls) { (void)stalls; }

            /// @brief A player's item stall closed, the own one too (packet 23/32).
            /// @param playerId The stall owner.
            virtual void onItemStallClosed(std::int32_t playerId) { (void)playerId; }

            /// @brief What someone's item stall sells (packet 23/33).
            /// @param goods The goods.
            virtual void onStallGoods(const protocol::StallGoods& goods) { (void)goods; }

            /// @brief The black list changed (result) or came (with its entries) (packet 14/13).
            /// @param result The result code.
            /// @param entries The entries.
            virtual void onBlacklist(int result, const std::vector<protocol::BlacklistEntry>& entries) {
                (void)result;
                (void)entries;
            }

            /// @brief A player has the own one on their black list (packet 14/14).
            /// @param code The code.
            virtual void onBlockedBy(int code) { (void)code; }

            /// @brief Why watching or fighting a player was refused (packet 33/1).
            /// @param reason The reason code.
            virtual void onPlayerRefused(int reason) { (void)reason; }

            /// @brief The PK, fighting, trade and party switches and the chat channel bits (packet 33/2).
            /// @param switches The four switches; 1 on, 2 off.
            /// @param channels The chat channel bits.
            virtual void onSwitches(const std::array<int, 4>& switches, int channels) {
                (void)switches;
                (void)channels;
            }

            /// @brief How changing the password went (packet 35/1).
            /// @param result The result code.
            virtual void onPasswordChanged(int result) { (void)result; }

            /// @brief How changing the account password went (packet 35/12, north-island-server only).
            /// @param result The result code.
            virtual void onAccountPasswordChanged(int result) { (void)result; }

            /// @brief Whether a revival spot is recorded (packet 5/21).
            /// @param code 1 yes, other non-zero values no, 0 no news.
            virtual void onReturnSpotRecorded(int code) { (void)code; }

            /// @brief A place of the Bookcase or the Desk now holds this item (packets 5/13 and 5/24: HandleBookCaseSlotPacket, HandleBookDeskSlotPacket).
            /// @param desk True for the Desk, false for the Bookcase.
            /// @param place The place.
            /// @param itemId The item, 0 for none.
            virtual void onBookShelfSlot(bool desk, int place, int itemId) { (void)desk, (void)place, (void)itemId; }

            /// @brief The own pet stall opened (packet 23/110).
            /// @param stall The stall.
            virtual void onOwnPetStall(const protocol::OwnStall& stall) { (void)stall; }

            /// @brief Pet stalls kept on the map (packet 23/111).
            /// @param stalls The stall signs.
            virtual void onPetStalls(const std::vector<protocol::StallSignRecord>& stalls) { (void)stalls; }

            /// @brief A player's pet stall closed, the own one too (packet 23/112).
            /// @param playerId The stall owner.
            virtual void onPetStallClosed(std::int32_t playerId) { (void)playerId; }

            /// @brief What someone's pet stall sells (packet 23/113).
            /// @param goods The goods.
            virtual void onPetStallGoods(const protocol::PetStallGoods& goods) { (void)goods; }

            /// @brief A pet sold from the own stall (packet 23/114).
            /// @param sale The sale.
            virtual void onPetStallSale(const protocol::StallSale& sale) { (void)sale; }

            /// @brief Something sold from the own stall (packet 23/34).
            /// @param sale The sale.
            virtual void onStallSale(const protocol::StallSale& sale) { (void)sale; }

            /// @brief The own stall of large items opened (packet 23/130, HandleOpenUtensilStallPacket).
            /// @param stall The stall.
            virtual void onOwnUtensilStall(const protocol::OwnStall& stall) { (void)stall; }

            /// @brief Stalls of large items kept on the map (packet 23/131, HandleStallTitleListPacket).
            /// @param stalls The stall signs.
            virtual void onUtensilStalls(const std::vector<protocol::StallSignRecord>& stalls) { (void)stalls; }

            /// @brief A player's stall of large items closed, the own one too (packet 23/132, HandleStallClosedPacket).
            /// @param playerId The stall owner.
            virtual void onUtensilStallClosed(std::int32_t playerId) { (void)playerId; }

            /// @brief What someone's stall of large items sells (packet 23/133, RecvUtensilStallList).
            /// @param goods The goods.
            virtual void onUtensilStallGoods(const protocol::UtensilStallGoods& goods) { (void)goods; }

            /// @brief A piece sold from the own stall (packet 23/134, RecvUtensilStallSale).
            /// @param sale The sale.
            virtual void onUtensilStallSale(const protocol::StallSale& sale) { (void)sale; }

            /// @brief What a compound made, in a bag slot (packet 23/13).
            /// @param itemId The made item.
            /// @param bagSlot Bag slot.
            virtual void onCompoundResult(int itemId, int bagSlot) {
                (void)itemId;
                (void)bagSlot;
            }

            /// @brief An item trade with a player opens (packet 25/1).
            /// @param partnerId The trade partner.
            virtual void onItemTradeStart(std::int32_t partnerId) { (void)partnerId; }

            /// @brief How an item trade ended (packet 25/2).
            /// @param result 4 done, else why not.
            virtual void onItemTradeResult(int result) { (void)result; }

            /// @brief What the partner put up (packet 25/3).
            /// @param offer The offer.
            virtual void onItemTradeOffer(const protocol::TradeOffer& offer) { (void)offer; }

            /// @brief The trade of large items starts with a player (packet 25/40, RecvUtensilTrade).
            /// @param partnerId The trade partner.
            virtual void onUtensilTradeStart(std::int32_t partnerId) { (void)partnerId; }

            /// @brief The partner's offer in a trade of large items (packet 25/41).
            /// @param offer The offer.
            virtual void onUtensilTradeOffer(const protocol::TradeOffer& offer) { (void)offer; }

            /// @brief Why a trade of large items ended (packet 25/42).
            /// @param code The reason code.
            virtual void onUtensilTradeResult(int code) { (void)code; }

            /// @brief A pet trade with a player opens (packet 25/10).
            /// @param partnerId The trade partner.
            virtual void onPetTradeStart(std::int32_t partnerId) { (void)partnerId; }

            /// @brief How a pet trade ended (packet 25/11).
            /// @param result 4 done, else why not.
            virtual void onPetTradeResult(int result) { (void)result; }

            /// @brief The partner's pet and gold (packet 25/12).
            /// @param offer The offer.
            virtual void onPetTradeOffer(const protocol::PetTradeOffer& offer) { (void)offer; }

            /// @brief Why a trade was refused (packet 25/43).
            /// @param reason Reason 1..4.
            virtual void onTradeRefused(int reason) { (void)reason; }

            /// @brief Whether the own player has been online long enough to trade (packet 25/44).
            /// @param enough True when long enough.
            virtual void onTradeOnlineTime(bool enough) { (void)enough; }

            /// @brief An emotion over a player's head (packet 32/1).
            /// @param playerId The player.
            /// @param emotionId The emotion.
            virtual void onEmotion(std::int32_t playerId, int emotionId) {
                (void)playerId;
                (void)emotionId;
            }

            /// @brief A player plays the action in a sprite slot (packet 32/2).
            /// @param playerId The player.
            /// @param slot The sprite slot.
            virtual void onAction(std::int32_t playerId, int slot) {
                (void)playerId;
                (void)slot;
            }

            /// @brief An attribute of one of the own pets (packet 8/2).
            /// @param place The pet's place.
            /// @param attribute Attribute id.
            /// @param value New value.
            /// @param extra Extra value.
            virtual void onPetAttributeChanged(int place, int attribute, std::int32_t value, std::int32_t extra) {
                (void)place;
                (void)attribute;
                (void)value;
                (void)extra;
            }

            /// @brief An attribute of another player of the team (packet 8/3): 25 HP, 26 SP, 29 CON, 33 WIS, 207 / 208 / 48 / 49 / 55 what adds to the maxima, 35 level, 31 job, 39 reborn.
            /// @param playerId The player.
            /// @param attribute Attribute id.
            /// @param value New value.
            /// @param extra Extra value.
            virtual void onWorldPlayerAttribute(std::int32_t playerId, int attribute, std::int32_t value, std::int32_t extra) {
                (void)playerId;
                (void)attribute;
                (void)value;
                (void)extra;
            }

            /// @brief A player left the map (packet 1/1).
            /// @param playerId The player.
            virtual void onPlayerLeft(std::int32_t playerId) { (void)playerId; }

            /// @brief Another player on the map (packets 3 when entering, or 4).
            /// @param record The player's record.
            /// @param entering True for packet 3.
            virtual void onPlayerAppeared(const protocol::OtherPlayerRecord& record, bool entering) {
                (void)record;
                (void)entering;
            }

            /// @brief A message no handler knows yet.
            /// @param id The message id.
            virtual void onUnhandled(protocol::MessageId id) { (void)id; }
    };

}

#endif
