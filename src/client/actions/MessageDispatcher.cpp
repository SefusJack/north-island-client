#include "client/actions/MessageDispatcher.h"

#include "client/actions/Action00Kick.h"
#include "client/actions/Action01System.h"
#include "client/actions/Action02Chat.h"
#include "client/actions/Action03PlayerEnters.h"
#include "client/actions/Action04PlayerPresent.h"
#include "client/actions/Action05Status.h"
#include "client/actions/Action06Walk.h"
#include "client/actions/Action07Jump.h"
#include "client/actions/Action08Attribute.h"
#include "client/actions/Action09CreateCharacter.h"
#include "client/actions/Action10Names.h"
#include "client/actions/Action11Battle.h"
#include "client/actions/Action12MapChange.h"
#include "client/actions/Action13Team.h"
#include "client/actions/Action14Friends.h"
#include "client/actions/Action15Pets.h"
#include "client/actions/Action16Gm.h"
#include "client/actions/Action199Gm.h"
#include "client/actions/Action19ActivePet.h"
#include "client/actions/Action20Event.h"
#include "client/actions/Action22MapNpc.h"
#include "client/actions/Action23Items.h"
#include "client/actions/Action24Marks.h"
#include "client/actions/Action25Trade.h"
#include "client/actions/Action26Gold.h"
#include "client/actions/Action27Shop.h"
#include "client/actions/Action29BankGold.h"
#include "client/actions/Action30BankItems.h"
#include "client/actions/Action31Inn.h"
#include "client/actions/Action32Emotions.h"
#include "client/actions/Action33Switches.h"
#include "client/actions/Action35Password.h"
#include "client/actions/Action40HotKeys.h"
#include "client/actions/Action45Marriage.h"
#include "client/actions/Action54Christmas.h"
#include "client/actions/Action57Sports.h"
#include "client/actions/Action62Furniture.h"
#include "client/actions/Action64Manufacture.h"
#include "client/actions/Action67Prizes.h"
#include "client/actions/Action65Tents.h"
#include "client/actions/Action71Machines.h"
#include "client/actions/Action74Gps.h"
#include "client/actions/Action75Mall.h"
#include "client/actions/Action79Career.h"
#include "client/actions/Action80PointStore.h"
#include "client/actions/Action82Notices.h"
#include "client/actions/Action83PetVoucher.h"
#include "client/actions/Action84Top100.h"
#include "client/actions/Action89Exp.h"
#include "client/actions/Action81SpecialFloor.h"
#include "client/actions/Action91LuckyBag.h"
#include "client/actions/Action85Instances.h"
#include "client/actions/Action86Property.h"
#include "client/actions/Action104Lottery.h"
#include "client/actions/Action90Activities.h"
#include "client/actions/Action50BattleActions.h"
#include "client/actions/Action51FighterAttributes.h"
#include "client/actions/Action52BattleRound.h"
#include "client/actions/Action53Fighter.h"
#include "client/actions/Action39Guild.h"
#include "client/actions/Action63Account.h"
#include "client/actions/Action68ItemBag.h"
#include "client/actions/Action70Castle.h"
#include "client/actions/Action76Maid.h"
#include "client/actions/Action77FleeKill.h"
#include "client/actions/Action72BlackBoard.h"
#include "client/actions/Action66Reborn.h"
#include "client/actions/Action87PvpServer.h"
#include "client/actions/Action78Kicked.h"
#include "client/actions/Action69PvpEvents.h"
#include "client/actions/Action61Lotto.h"
#include "client/actions/Action55Cafe.h"
#include "client/actions/Action41Facility.h"
#include "client/actions/Action44Rebirth.h"
#include "client/actions/Action43Alliance.h"
#include "client/actions/Action37Pvp.h"
#include "client/actions/Action36Boards.h"
#include "client/actions/Action34Subscription.h"
#include "client/actions/Action21IdAndPw.h"

namespace client {

    void dispatchMessage(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.action) {
            case 0: // server action 0: kick
                handleAction00Kick(payload, context);
                break;
            case 1: // server action 1: system
                handleAction01System(payload, context);
                break;
            case 2: // server action 2: chat
                handleAction02Chat(payload, context);
                break;
            case 3: // server action 3: a player enters
                handleAction03PlayerEnters(payload, context);
                break;
            case 4: // server action 4: a player is present
                handleAction04PlayerPresent(payload, context);
                break;
            case 5: // server action 5: status
                handleAction05Status(payload, context);
                break;
            case 6: // server action 6: walk
                handleAction06Walk(payload, context);
                break;
            case 7: // server action 7: jump
                handleAction07Jump(payload, context);
                break;
            case 8: // server action 8: attribute
                handleAction08Attribute(payload, context);
                break;
            case 9: // server action 9: create character
                handleAction09CreateCharacter(payload, context);
                break;
            case 10: // server action 10: names
                handleAction10Names(payload, context);
                break;
            case 11: // server action 11: battle
                handleAction11Battle(payload, context);
                break;
            case 12: // server action 12: map change
                handleAction12MapChange(payload, context);
                break;
            case 13: // server action 13: team
                handleAction13Team(payload, context);
                break;
            case 14: // server action 14: friends
                handleAction14Friends(payload, context);
                break;
            case 15: // server action 15: pets
                handleAction15Pets(payload, context);
                break;
            case 16: // server action 16: GM
                handleAction16Gm(payload, context);
                break;
            case 19: // server action 19: active pet
                handleAction19ActivePet(payload, context);
                break;
            case 20: // server action 20: events
                handleAction20Event(payload, context);
                break;
            case 22: // server action 22: map NPCs
                handleAction22MapNpc(payload, context);
                break;
            case 23: // server action 23: items
                handleAction23Items(payload, context);
                break;
            case 24: // server action 24: marks
                handleAction24Marks(payload, context);
                break;
            case 25: // server action 25: trade
                handleAction25Trade(payload, context);
                break;
            case 26: // server action 26: gold
                handleAction26Gold(payload, context);
                break;
            case 27: // server action 27: shop
                handleAction27Shop(payload, context);
                break;
            case 29: // server action 29: bank gold
                handleAction29BankGold(payload, context);
                break;
            case 30: // server action 30: bank items
                handleAction30BankItems(payload, context);
                break;
            case 31: // server action 31: inn
                handleAction31Inn(payload, context);
                break;
            case 32: // server action 32: emotions
                handleAction32Emotions(payload, context);
                break;
            case 33: // server action 33: switches
                handleAction33Switches(payload, context);
                break;
            case 39: // server action 39: guild
                handleAction39Guild(payload, context);
                break;
            case 40: // server action 40: hot keys
                handleAction40HotKeys(payload, context);
                break;
            case 35: // server action 35: password
                handleAction35Password(payload, context);
                break;
            case 45: // server action 45: marriage
                handleAction45Marriage(payload, context);
                break;
            case 50: // server action 50: battle actions
                handleAction50BattleActions(payload, context);
                break;
            case 51: // server action 51: fighter attributes
                handleAction51FighterAttributes(payload, context);
                break;
            case 52: // server action 52: battle round
                handleAction52BattleRound(payload, context);
                break;
            case 53: // server action 53: fighter
                handleAction53Fighter(payload, context);
                break;
            case 62: // server action 62: furniture of a house
                handleAction62Furniture(payload, context);
                break;
            case 63: // server action 63: account after login
                handleAction63Account(payload, context);
                break;
            case 64: // server action 64: manufacturing of the house
                handleAction64Manufacture(payload, context);
                break;
            case 68: // server action 68: Item Bag
                handleAction68ItemBag(payload, context);
                break;
            case 70: // server action 70: castle and Trojan war
                handleAction70Castle(payload, context);
                break;
            case 74: // server action 74: GPS items and Dreaming Pill
                handleAction74Gps(payload, context);
                break;
            case 80: // server action 80: point store
                handleAction80PointStore(payload, context);
                break;
            case 75: // server action 75: item mall
                handleAction75Mall(payload, context);
                break;
            case 54: // server action 54: Christmas
                handleAction54Christmas(payload, context);
                break;
            case 57: // server action 57: sports
                handleAction57Sports(payload, context);
                break;
            case 65: // server action 65: tents
                handleAction65Tents(payload, context);
                break;
            case 67: // server action 67: prize and pet exchange refusals
                handleAction67Prizes(payload, context);
                break;
            case 71: // server action 71: prize machines
                handleAction71Machines(payload, context);
                break;
            case 79: // server action 79: career
                handleAction79Career(payload, context);
                break;
            case 82: // server action 82: notice boards
                handleAction82Notices(payload, context);
                break;
            case 83: // server action 83: pet egg / exchange voucher
                handleAction83PetVoucher(payload, context);
                break;
            case 84: // server action 84: Top 100 window
                handleAction84Top100(payload, context);
                break;
            case 85: // server action 85: instances
                handleAction85Instances(payload, context);
                break;
            case 86: // server action 86: lost property office
                handleAction86Property(payload, context);
                break;
            case 89: // server action 89: event exp
                handleAction89Exp(payload, context);
                break;
            case 90: // server action 90: event schedule
                handleAction90Activities(payload, context);
                break;
            case 81: // server action 81: battle time limit / special house room
                handleAction81SpecialFloor(payload, context);
                break;
            case 91: // server action 91: Lucky Bags
                handleAction91LuckyBag(payload, context);
                break;
            case 104: // server action 104: lottery
                handleAction104Lottery(payload, context);
                break;
            case 21: // server action 21: id and password
                handleAction21IdAndPw(payload, context);
                break;
            case 34: // server action 34: subscription
                handleAction34Subscription(payload, context);
                break;
            case 36: // server action 36: boards
                handleAction36Boards(payload, context);
                break;
            case 37: // server action 37: PvP
                handleAction37Pvp(payload, context);
                break;
            case 43: // server action 43: alliance
                handleAction43Alliance(payload, context);
                break;
            case 41: // server action 41: facility
                handleAction41Facility(payload, context);
                break;
            case 44: // server action 44: rebirth
                handleAction44Rebirth(payload, context);
                break;
            case 55: // server action 55: cafe
                handleAction55Cafe(payload, context);
                break;
            case 61: // server action 61: Lotto
                handleAction61Lotto(payload, context);
                break;
            case 69: // server action 69: PvP event notices
                handleAction69PvpEvents(payload, context);
                break;
            case 78: // server action 78: player kicked out
                handleAction78Kicked(payload, context);
                break;
            case 87: // server action 87: move to the PvP server
                handleAction87PvpServer(payload, context);
                break;
            case 199: // server action 199: GM
                handleAction199Gm(payload, context);
                break;
            case 42: // server action 42: accepted, nothing done
            case 46: // server action 46: accepted, nothing done
            case 56: // server action 56: accepted, nothing done
            case 73: // server action 73: accepted, nothing done
                break;
            case 66: // server action 66: Wishing God wish and time effects
                handleAction66Reborn(payload, context);
                break;
            case 72: // server action 72: house blackboard
                handleAction72BlackBoard(payload, context);
                break;
            case 77: // server action 77: flee-kill and survival event
                handleAction77FleeKill(payload, context);
                break;
            case 76: // server action 76: Wit Maid menu
                handleAction76Maid(payload, context);
                break;
            default: // unknown action: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
