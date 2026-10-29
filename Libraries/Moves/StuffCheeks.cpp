#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// STUFF CHEEKS
// Fails if the user doesn't hold a berry. Eats the berry (with its effect) and raises the Defense by 2 stages.
extern "C" void HandlerStuffCheeksCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        ITEM itemID = BattleMon_GetHeldItem(currentMon);
        if (!itemID || !PML_ItemIsBerry(itemID)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerStuffCheeks(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        ITEM itemID = BattleMon_GetHeldItem(currentMon);
        if (!itemID || !PML_ItemIsBerry(itemID))
            return;

        // Eat the held berry, its effect is applied as if the Pokemon had used it normally
        BattleHandler_PushRun(serverFlow, EFFECT_USE_HELD_ITEM, pokemonSlot);

        HandlerParam_ChangeStatStage* statChange;
        statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
        statChange->pokeCount = 1;
        statChange->pokeID[0] = pokemonSlot;
        statChange->moveAnimation = 1;
        statChange->stat = STATSTAGE_DEFENSE;
        statChange->volume = 2;
        BattleHandler_PopWork(serverFlow, statChange);
    }
}
BattleEventHandlerTableEntry StuffCheeksHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerStuffCheeksCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerStuffCheeks},
};
extern "C" BattleEventHandlerTableEntry* EventAddStuffCheeks(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(StuffCheeksHandlers);
    return StuffCheeksHandlers;
}

#endif // EXPAND_MOVES
