#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// REVELATION DANCE
extern "C" void HandlerRevelationDance(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        BattleEventVar_RewriteValue(VAR_MOVE_TYPE, currentMon->type[0]);
    }
}
BattleEventHandlerTableEntry RevelationDanceHandlers[]{
    {EVENT_MOVE_PARAM, HandlerRevelationDance},
};
extern "C" BattleEventHandlerTableEntry* EventAddRevelationDance(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(RevelationDanceHandlers);
    return RevelationDanceHandlers;
}

#endif // EXPAND_MOVES
