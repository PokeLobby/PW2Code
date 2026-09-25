#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// STRENGTH SAP
extern "C" void HandlerStrengthSap(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
            u32 affectedSlot = BattleEventVar_GetValue(VAR_MON_ID);
            if (affectedSlot == BATTLE_MAX_SLOTS)
                return;

            BattleMon* affectedMon = Handler_GetBattleMon(serverFlow, affectedSlot);
            if (BattleMon_IsStatChangeValid(affectedMon, STATSTAGE_ATTACK, BattleEventVar_GetValue(VAR_VOLUME))) {
                HandlerParam_Drain* drain = (HandlerParam_Drain*)BattleHandler_PushWork(serverFlow, EFFECT_DRAIN, pokemonSlot);
                drain->recoverHP = BattleMon_GetValue(affectedMon, VALUE_ATTACK_STAT);
                drain->recoverSlot = pokemonSlot;
                drain->damagedSlot = affectedSlot;
                BattleHandler_PopWork(serverFlow, drain);
            }
    }
}

BattleEventHandlerTableEntry StrengthSapHandlers[] {
    {EVENT_STAT_STAGE_CHANGE_LAST_CHECK, HandlerStrengthSap},
};

extern "C" BattleEventHandlerTableEntry* EventAddStrengthSap(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(StrengthSapHandlers);
    return StrengthSapHandlers;
}

#endif // EXPAND_MOVES
