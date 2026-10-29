#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): damaging moves that trap the target
// Spirit Shackle (662), Anchor Shot (677), Thousand Waves (615) and Jaw Lock (746, also traps the user).
// Applies the same "can't escape" condition as Mean Look / Spider Web, but after the hit.
static void AddTrapCondition(ServerFlow* serverFlow, u32 sourceSlot, u32 trappedSlot, u32 trapperSlot) {
    HandlerParam_AddCondition* addCondition;
    addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, sourceSlot);
    addCondition->condition = CONDITION_BLOCK;
    addCondition->condData = ConditionData_MakePoke(trapperSlot);
    addCondition->almost = 0;
    addCondition->pokeID = trappedSlot;
    BattleHandler_StrSetup(&addCondition->exStr, 2u, 872);
    BattleHandler_AddArg(&addCondition->exStr, trappedSlot);
    BattleHandler_PopWork(serverFlow, addCondition);
}

static b32 CanBeTrapped(BattleMon* mon) {
    if (BattleMon_IsFainted(mon))
        return 0;
    if (BattleMon_CheckIfMoveCondition(mon, CONDITION_BLOCK))
        return 0;
    // Ghost types ignore traps (settings.h GHOST_IGNORES_TRAPS)
    if (BattleMon_HasType(mon, TYPE_GHOST))
        return 0;
    return 1;
}

static void TrapTargets(ServerFlow* serverFlow, u32 pokemonSlot, b32 trapUser) {
    u32 targetCount = BattleEventVar_GetValue(VAR_TARGET_COUNT);
    for (u32 targetIdx = 0; targetIdx < targetCount; ++targetIdx) {
        u32 targetSlot = BattleEventVar_GetValue((BattleEventVar)(VAR_TARGET_MON_ID + targetIdx));
        if (targetSlot == BATTLE_MAX_SLOTS)
            continue;

        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        if (BattleMon_IsFainted(targetMon) || BattleMon_IsSubstituteActive(targetMon))
            continue;

        if (CanBeTrapped(targetMon))
            AddTrapCondition(serverFlow, pokemonSlot, targetSlot, pokemonSlot);

        if (trapUser) {
            BattleMon* userMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
            if (CanBeTrapped(userMon))
                AddTrapCondition(serverFlow, pokemonSlot, pokemonSlot, targetSlot);
        }
    }
}

extern "C" void HandlerTrapHit(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        TrapTargets(serverFlow, pokemonSlot, 0);
    }
}
BattleEventHandlerTableEntry TrapHitHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerTrapHit},
};
extern "C" BattleEventHandlerTableEntry* EventAddTrapHit(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TrapHitHandlers);
    return TrapHitHandlers;
}

extern "C" void HandlerJawLock(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        TrapTargets(serverFlow, pokemonSlot, 1);
    }
}
BattleEventHandlerTableEntry JawLockHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerJawLock},
};
extern "C" BattleEventHandlerTableEntry* EventAddJawLock(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(JawLockHandlers);
    return JawLockHandlers;
}

#endif // EXPAND_MOVES
