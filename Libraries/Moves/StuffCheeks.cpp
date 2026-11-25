#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// STUFF CHEEKS
// Fails if the user doesn't hold a berry. Eats the berry (with its effect) and raises the Defense by 2 stages.
extern "C" void HandlerStuffCheeksCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        ITEM itemID = BattleMon_GetHeldItem(currentMon);
        if (!itemID || !PML_ItemIsBerry(itemID)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
// Hack PokeLobby (streamers-serie-2#24): comer la baya a la fuerza, como Bicho Bocado:
// se quita y su efecto se aplica aunque no se cumpla su condicion (vida llena...).
// EFFECT_USE_HELD_ITEM la usaba "de forma normal" y con la vida llena no se la comia.
static void ComerBayaALaFuerza(ServerFlow* serverFlow, u32 slot) {
    BattleMon* mon = Handler_GetBattleMon(serverFlow, slot);
    ITEM berry = BattleMon_GetHeldItem(mon);
    if (!berry || !PML_ItemIsBerry(berry))
        return;

    HandlerParam_SetItem* setItem;
    setItem = (HandlerParam_SetItem*)BattleHandler_PushWork(serverFlow, EFFECT_SET_HELD_ITEM, slot);
    setItem->pokeID = (u8)slot;
    setItem->itemID = 0;
    setItem->callConsumedEvent = 1;
    BattleHandler_PopWork(serverFlow, setItem);

    HandlerParam_ActivateItem* forceUseItem;
    forceUseItem = (HandlerParam_ActivateItem*)BattleHandler_PushWork(serverFlow, EFFECT_FORCE_USE_HELD_ITEM, slot);
    forceUseItem->itemID = berry;
    forceUseItem->pokeID = (u8)slot;
    BattleHandler_PopWork(serverFlow, forceUseItem);
}
extern "C" void HandlerStuffCheeks(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        ITEM itemID = BattleMon_GetHeldItem(currentMon);
        if (!itemID || !PML_ItemIsBerry(itemID))
            return;

        // Se come la baya aunque no se cumpla su condicion (como en los oficiales)
        ComerBayaALaFuerza(serverFlow, pokemonSlot);

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
