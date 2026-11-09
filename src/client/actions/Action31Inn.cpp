#include "client/actions/Action31Inn.h"

#include "core/ByteReader.h"
#include "protocol/PetLists.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction31Inn(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: { // 31/1: u8 how the rest went
                const int answer = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onInnAnswer(answer);
                break;
            }
            case 2: { // 31/2: i32 what a rest costs
                const std::int32_t price = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onInnPrice(price);
                break;
            }
            case 10: // 31/10: the player sleeps
                context.events.onInnSleep();
                break;
            case 8: // 31/8: ignored (form arrows)
            case 9: // 31/9: ignored (form arrows)
                break;
            case 15: { // 31/15: u8 pet slot, u8 stored place: pets swapped (RecvInnPetSwapped)
                const int petSlot = reader.u8();
                const int place = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onInnPetSwapped(petSlot, place);
                break;
            }
            case 6: // 31/6: stored pet records
                context.events.onStoredPets(protocol::decodeStoredPets(payload));
                break;
            case 7: // 31/7: the pet storage form opens
                context.events.onPetStorageOpened();
                break;
            case 3: // 31/3: u8 place, u8 pet slot: that pet stored
            case 5: // 31/5: u8 place, u8 pet slot: the stored pet into that slot
            case 11: { // 31/11: same as 31/5
                const int place = reader.u8();
                const int petSlot = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                if (id.sub == 3) { // 31/3: stored
                    context.events.onPetStored(place, petSlot);
                } else {
                    context.events.onStoredPetTaken(place, petSlot);
                }
                break;
            }
            case 4: { // 31/4: u8 place: emptied
                const int place = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onStoredPetCleared(place);
                break;
            }
            case 12: // 31/12: the storage's answer is done
                context.events.onPetStorageDone();
                break;
            case 13: // 31/13: refused (the storer has the kind)
            case 14: // 31/14: refused (you have the kind)
                context.events.onPetStorageRefused(id.sub == 13); // sub 13 = the storer
                break;
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
