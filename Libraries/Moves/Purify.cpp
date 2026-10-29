#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// PURIFY
extern "C" void HandlerPurifyCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        if (defendingSlot >= BATTLE_MAX_SLOTS)
            return;
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, defendingSlot);
        work[0] = BattleMon_GetStatus(defendingMon);
        if (!work[0]) {
            BattleEventVar_RewriteValue(VAR_NO_EFFECT_FLAG, 1);
        }
    }
}
extern "C" void HandlerPurify(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);

        HandlerParam_CureCondition* cureCondition;
        cureCondition = (HandlerParam_CureCondition*)BattleHandler_PushWork(serverFlow, EFFECT_CURE_STATUS, pokemonSlot);
        cureCondition->condition = work[0];
        cureCondition->pokeID[0] = targetSlot;
        cureCondition->pokeCount = 1;
        BattleHandler_PopWork(serverFlow, cureCondition);

        BattleMon* attackingMon = Handler_GetBattleMon(serverFlow, pokemonSlot);

        HandlerParam_RecoverHP* recoverHP;
        recoverHP = (HandlerParam_RecoverHP*)BattleHandler_PushWork(serverFlow, EFFECT_RECOVER_HP, pokemonSlot);
        recoverHP->recoverHP = DivideMaxHPZeroCheck(attackingMon, 2);
        recoverHP->pokeID = pokemonSlot;
        BattleHandler_PopWork(serverFlow, recoverHP);
    }
}
BattleEventHandlerTableEntry PurifyHandlers[]{
    {EVENT_ABILITY_CHECK_NO_EFFECT, HandlerPurifyCheck},
    {EVENT_UNCATEGORIZED_MOVE, HandlerPurify},
};
extern "C" BattleEventHandlerTableEntry* EventAddPurify(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PurifyHandlers);
    return PurifyHandlers;
}

#endif // EXPAND_MOVES
