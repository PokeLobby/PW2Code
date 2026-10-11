#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// SPEED SWAP
extern "C" void HandlerSpeedSwap(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);

        BattleMon* attackingMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);

        u32 speed = BattleMon_GetRealStat(attackingMon, VALUE_SPEED_STAT);
        u32 targetSpeed = BattleMon_GetRealStat(targetMon, VALUE_SPEED_STAT);

        HandlerParam_SetStat* setStat;
        setStat = (HandlerParam_SetStat*)BattleHandler_PushWork(serverFlow, EFFECT_SET_STAT, pokemonSlot);
        setStat->battleSlot = pokemonSlot;
        setStat->speed = targetSpeed;
        setStat->flags = 0x10;
        BattleHandler_PopWork(serverFlow, setStat);

        setStat = (HandlerParam_SetStat*)BattleHandler_PushWork(serverFlow, EFFECT_SET_STAT, pokemonSlot);
        setStat->battleSlot = targetSlot;
        setStat->speed = speed;
        setStat->flags = 0x10;
        BattleHandler_PopWork(serverFlow, setStat);
    }
}
BattleEventHandlerTableEntry SpeedSwapHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerSpeedSwap},
};
extern "C" BattleEventHandlerTableEntry* EventAddSpeedSwap(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SpeedSwapHandlers);
    return SpeedSwapHandlers;
}

#endif // EXPAND_MOVES
