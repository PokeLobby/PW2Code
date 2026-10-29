#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// POS THROAT CHOP
// work[0] -> battle slot of the Pokemon that can't use sound moves
// work[1] -> turn checks elapsed (the effect lasts the turn it is applied and the next one)
extern "C" void HandlerPosThroatChopCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 attackingSlot = BattleEventVar_GetValue(VAR_ATTACKING_MON);
    if (attackingSlot != work[0])
        return;
    if (BattleEventVar_GetValue(VAR_FAIL_CAUSE))
        return;

    MOVE_ID moveID = BattleEventVar_GetValue(VAR_MOVE_ID);
    if (!getMoveFlag(moveID, FLAG_SOUND))
        return;

    BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);

    HandlerParam_Message* message;
    message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, attackingSlot);
    BattleHandler_StrSetup(&message->str, 2u, BATTLE_THROAT_CHOP_FAIL_MSGID);
    BattleHandler_AddArg(&message->str, attackingSlot);
    BattleHandler_PopWork(serverFlow, message);
}
extern "C" void HandlerPosThroatChopTurnCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == BATTLE_MAX_SLOTS) {
        if (work[1] >= 1) {
            BattleEventItem_Remove(item);
        }
        ++work[1];
    }
}
extern "C" void HandlerPosThroatChopSwitchOut(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == work[0]) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosThroatChopFainted(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == work[0] && BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, slot))) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosThroatChopHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerPosThroatChopCheckFail},
    {EVENT_TURN_CHECK_DONE, HandlerPosThroatChopTurnCheck},
    {EVENT_SWITCH_OUT_END, HandlerPosThroatChopSwitchOut},
    {EVENT_NOTIFY_FAINTED, HandlerPosThroatChopFainted},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosThroatChop(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosThroatChopHandlers));
    return PosThroatChopHandlers;
}

// THROAT CHOP
extern "C" void HandlerThroatChop(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetCount = BattleEventVar_GetValue(VAR_TARGET_COUNT);

        for (u32 targetIdx = 0; targetIdx < targetCount; ++targetIdx) {
            u32 targetSlot = BattleEventVar_GetValue((BattleEventVar)(VAR_TARGET_MON_ID + targetIdx));
            if (targetSlot == BATTLE_MAX_SLOTS)
                continue;

            BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
            if (BattleMon_IsFainted(targetMon) || BattleMon_IsSubstituteActive(targetMon))
                continue;

            HandlerParam_AddPosEffect* addEffectPos;
            addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
            addEffectPos->posEffect = POSEFF_THROAT_CHOP;
            addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, targetSlot);
            addEffectPos->workToCopy[0] = targetSlot;
            addEffectPos->workToCopy[1] = 0;
            addEffectPos->workCount = 2;
            BattleHandler_PopWork(serverFlow, addEffectPos);
        }
    }
}
BattleEventHandlerTableEntry ThroatChopHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerThroatChop},
};
extern "C" BattleEventHandlerTableEntry* EventAddThroatChop(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ThroatChopHandlers);
    return ThroatChopHandlers;
}

#endif // EXPAND_MOVES
