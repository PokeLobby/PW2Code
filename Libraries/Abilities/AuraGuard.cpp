#include "settings.h"
#if EXPAND_ABILITIES

#include "Patches/BattleUpgrade/include/Contact.h"

#include "server_flow.h"

// AURA GUARD
extern "C" void HandlerAuraGuard(BattleEventItem * item, ServerFlow * serverFlow, u32 pokemonSlot, u32 * work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_DEFENDING_MON)) {

        MOVE_ID moveID = BattleEventVar_GetValue(VAR_MOVE_ID);
        u32 attackingSlot = BattleEventVar_GetValue(VAR_ATTACKING_MON);
        if (MakesContact(serverFlow, moveID, attackingSlot, pokemonSlot)) {

            BattleEventVar_MulValue(VAR_RATIO, 2048);
        }

    }
}
BattleEventHandlerTableEntry AuraGuardHandlers[]{
    {EVENT_MOVE_DAMAGE_PROCESSING_2, HandlerAuraGuard},
};
extern "C" BattleEventHandlerTableEntry * EventAddAuraGuard(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(AuraGuardHandlers);
    return AuraGuardHandlers;
}
#endif // EXPAND_ABILITIES
