#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"
#include "abilities.h"

// POS SALT CURE
// work[0] -> battle slot of the salt cured Pokemon
extern "C" void HandlerPosSaltCureTurnCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot != work[0])
        return;

    BattleMon* currentMon = Handler_GetBattleMon(serverFlow, slot);
    if (BattleMon_IsFainted(currentMon))
        return;
    if (BattleMon_GetValue(currentMon, VALUE_EFFECTIVE_ABILITY) == ABIL_MAGIC_GUARD)
        return;

    // Salt Cure deals 1/8 of the max HP each turn, 1/4 if the target is Water or Steel type
    u32 denominator = 8u;
    if (BattleMon_HasType(currentMon, TYPE_WATER) || BattleMon_HasType(currentMon, TYPE_STEEL))
        denominator = 4u;

    HandlerParam_Damage* damage;
    damage = (HandlerParam_Damage*)BattleHandler_PushWork(serverFlow, EFFECT_DAMAGE, slot);
    damage->pokeID = slot;
    damage->damage = DivideMaxHPZeroCheck(currentMon, denominator);
    BattleHandler_StrSetup(&damage->exStr, 2u, BATTLE_SALT_CURE_DAMAGE_MSGID);
    BattleHandler_AddArg(&damage->exStr, slot);
    BattleHandler_PopWork(serverFlow, damage);
}
extern "C" void HandlerPosSaltCureSwitchOut(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == work[0]) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosSaltCureFainted(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == work[0] && BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, slot))) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosSaltCureHandlers[]{
    {EVENT_TURN_CHECK_END, HandlerPosSaltCureTurnCheck},
    {EVENT_SWITCH_OUT_END, HandlerPosSaltCureSwitchOut},
    {EVENT_NOTIFY_FAINTED, HandlerPosSaltCureFainted},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosSaltCure(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosSaltCureHandlers));
    return PosSaltCureHandlers;
}

// SALT CURE
extern "C" void HandlerSaltCure(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
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
            addEffectPos->posEffect = POSEFF_SALT_CURE;
            addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, targetSlot);
            addEffectPos->workToCopy[0] = targetSlot;
            addEffectPos->workCount = 1;
            BattleHandler_PopWork(serverFlow, addEffectPos);

            HandlerParam_Message* message;
            message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
            BattleHandler_StrSetup(&message->str, 2u, BATTLE_SALT_CURE_START_MSGID);
            BattleHandler_AddArg(&message->str, targetSlot);
            BattleHandler_PopWork(serverFlow, message);
        }
    }
}
BattleEventHandlerTableEntry SaltCureHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerSaltCure},
};
extern "C" BattleEventHandlerTableEntry* EventAddSaltCure(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SaltCureHandlers);
    return SaltCureHandlers;
}

#endif // EXPAND_MOVES
