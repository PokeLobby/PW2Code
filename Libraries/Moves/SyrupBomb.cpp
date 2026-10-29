#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// POS SYRUP BOMB
// work[0] -> battle slot of the Pokemon covered in syrup
// work[1] -> amount of times the Speed has been lowered
extern "C" void HandlerPosSyrupBombTurnCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot != work[0])
        return;

    BattleMon* currentMon = Handler_GetBattleMon(serverFlow, slot);
    if (BattleMon_IsFainted(currentMon))
        return;

    HandlerParam_ChangeStatStage* statChange;
    statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, slot);
    statChange->pokeCount = 1;
    statChange->pokeID[0] = slot;
    statChange->moveAnimation = 1;
    statChange->stat = STATSTAGE_SPEED;
    statChange->volume = -1;
    BattleHandler_PopWork(serverFlow, statChange);

    // The Speed is lowered during 3 turns
    ++work[1];
    if (work[1] >= 3) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosSyrupBombSwitchOut(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == work[0]) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosSyrupBombFainted(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == work[0] && BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, slot))) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosSyrupBombHandlers[]{
    {EVENT_TURN_CHECK_END, HandlerPosSyrupBombTurnCheck},
    {EVENT_SWITCH_OUT_END, HandlerPosSyrupBombSwitchOut},
    {EVENT_NOTIFY_FAINTED, HandlerPosSyrupBombFainted},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosSyrupBomb(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosSyrupBombHandlers));
    return PosSyrupBombHandlers;
}

// SYRUP BOMB
extern "C" void HandlerSyrupBomb(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
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
            addEffectPos->posEffect = POSEFF_SYRUP_BOMB;
            addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, targetSlot);
            addEffectPos->workToCopy[0] = targetSlot;
            addEffectPos->workToCopy[1] = 0;
            addEffectPos->workCount = 2;
            BattleHandler_PopWork(serverFlow, addEffectPos);

            HandlerParam_Message* message;
            message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
            BattleHandler_StrSetup(&message->str, 2u, BATTLE_SYRUP_BOMB_START_MSGID);
            BattleHandler_AddArg(&message->str, targetSlot);
            BattleHandler_PopWork(serverFlow, message);
        }
    }
}
BattleEventHandlerTableEntry SyrupBombHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerSyrupBomb},
};
extern "C" BattleEventHandlerTableEntry* EventAddSyrupBomb(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SyrupBombHandlers);
    return SyrupBombHandlers;
}

#endif // EXPAND_MOVES
