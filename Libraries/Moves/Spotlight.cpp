#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// SPOTLIGHT
// work[0] -> battleSlot of the Pokemon targeted by Spotlight
// work[6] -> unload prevention flag for Move Events
extern "C" void HandlerSpotlightMessage(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;

        HandlerParam_Message *message = (HandlerParam_Message *)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, 670u);
        BattleHandler_AddArg(&message->str, targetSlot);
        BattleHandler_PopWork(serverFlow, message);

        work[0] = targetSlot;
        // The sixth work value of a Move Event prevents it from being unloaded after the action ends.
        // We need the turn check to force the removal of the event at the end of the turn.
        work[6] = 1;
    }
}
extern "C" void HandlerSpotlightRedirect(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (Handler_GetBattleStyle(serverFlow) != BTL_STYLE_SINGLE
            && Handler_GetBattleStyle(serverFlow) != BTL_STYLE_ROTATION) {
        u32 attackingSlot = BattleEventVar_GetValue(VAR_ATTACKING_MON);
        u32 targetSlot = work[0];
        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);

        if (!MainModule_IsAllyMonID(attackingSlot, targetSlot)
                && BattleEventVar_GetValue(VAR_MOVE_ID) != MOVE_SKY_DROP
                && !BattleMon_CheckIfMoveCondition(targetMon, CONDITION_SKYDROP)
                && !Handler_IsMonSwitchOutInterrupted(serverFlow)) { 
            MOVE_ID moveID = BattleEventVar_GetValue(VAR_MOVE_ID);
            if (!Handler_IsTargetInRange(serverFlow, attackingSlot, targetSlot, moveID)) {
                BattleEventVar_RewriteValue(VAR_DEFENDING_MON, targetSlot);
            }
        }
    }
}

BattleEventHandlerTableEntry SpotlightHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerFollowMeCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerSpotlightMessage},
    {EVENT_REDIRECT_TARGET, HandlerSpotlightRedirect},
    {EVENT_TURN_CHECK_BEGIN, HandlerFollowMeTurnCheck},
};
extern "C" BattleEventHandlerTableEntry* EventAddSpotlight(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SpotlightHandlers);
    return SpotlightHandlers;
}

#endif // EXPAND_MOVES
