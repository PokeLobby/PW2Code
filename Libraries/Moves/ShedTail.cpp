#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// BattleMon_CreateSubstitute (symbol added to ESDB.yml): sets the substitute HP of a Pokemon
extern "C" void BattleMon_CreateSubstitute(BattleMon* battleMon, u16 hp);

// POS SHED TAIL
// The substitute is not created in the user: it is handed to the Pokemon that takes its place.
// work[0] -> HP of the substitute
// work[1] -> battle slot of the user (it must not receive the substitute)
// work[2] -> turn checks elapsed (safety, the effect is removed if nobody comes in)
extern "C" void HandlerPosShedTailSwitchIn(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == BATTLE_MAX_SLOTS || slot == work[1])
        return;
    if (Handler_PokeIDToPokePos(serverFlow, slot) != targetPos)
        return;

    BattleMon* incomingMon = Handler_GetBattleMon(serverFlow, slot);
    if (!BattleMon_IsFainted(incomingMon) && !BattleMon_IsSubstituteActive(incomingMon)) {
        BattleMon_CreateSubstitute(incomingMon, (u16)work[0]);
        ServerDisplay_AddCommon(serverFlow->serverCommandQueue, SCID_CreateSubstitute, slot, work[0]);
        ServerDisplay_AddCommon(serverFlow->serverCommandQueue, SCID_SubstituteCreate, targetPos);
    }
    BattleEventItem_Remove(item);
}
extern "C" void HandlerPosShedTailTurnCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == BATTLE_MAX_SLOTS) {
        if (work[2] >= 1) {
            BattleEventItem_Remove(item);
        }
        ++work[2];
    }
}
BattleEventHandlerTableEntry PosShedTailHandlers[]{
    {EVENT_SWITCH_IN, HandlerPosShedTailSwitchIn},
    {EVENT_TURN_CHECK_DONE, HandlerPosShedTailTurnCheck},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosShedTail(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosShedTailHandlers));
    return PosShedTailHandlers;
}

// SHED TAIL
// work[0] -> True if the user should switch out when the move ends
extern "C" void HandlerShedTailCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 maxHP = BattleMon_GetValue(currentMon, VALUE_MAX_HP);
        u32 currentHP = BattleMon_GetValue(currentMon, VALUE_CURRENT_HP);

        // Needs more than half of the HP, no substitute and somebody to switch to
        if (currentHP * 2 <= maxHP ||
            BattleMon_IsSubstituteActive(currentMon) ||
            !Handler_GetFightEnableBenchPokeNum(serverFlow, pokemonSlot)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerShedTail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_SHED_TAIL_MSGID);
        BattleHandler_AddArg(&message->str, pokemonSlot);
        BattleHandler_PopWork(serverFlow, message);

        // The user loses half of its max HP
        HandlerParam_Damage* damage;
        damage = (HandlerParam_Damage*)BattleHandler_PushWork(serverFlow, EFFECT_DAMAGE, pokemonSlot);
        damage->pokeID = pokemonSlot;
        damage->damage = DivideMaxHPZeroCheck(currentMon, 2u);
        BattleHandler_PopWork(serverFlow, damage);

        // The substitute has a quarter of the max HP and goes to the next Pokemon
        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_SHED_TAIL;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, pokemonSlot);
        addEffectPos->workToCopy[0] = DivideMaxHPZeroCheck(currentMon, 4u);
        addEffectPos->workToCopy[1] = pokemonSlot;
        addEffectPos->workToCopy[2] = 0;
        addEffectPos->workCount = 3;
        BattleHandler_PopWork(serverFlow, addEffectPos);

        // The user switches out when the move sequence ends
        work[0] = 1;
    }
}
extern "C" void HandlerShedTailSwitch(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID) && work[0] == 1) {
        work[0] = 0;
        if (Handler_GetFightEnableBenchPokeNum(serverFlow, pokemonSlot)
            && Handler_CheckReservedMemberChangeAction(serverFlow)) {

            HandlerParam_Switch* switchOut;
            switchOut = (HandlerParam_Switch*)BattleHandler_PushWork(serverFlow, EFFECT_SWITCH, pokemonSlot);
            switchOut->pokeID = pokemonSlot;
            BattleHandler_PopWork(serverFlow, switchOut);
        }
    }
}
BattleEventHandlerTableEntry ShedTailHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerShedTailCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerShedTail},
    {EVENT_MOVE_SEQUENCE_END, HandlerShedTailSwitch},
};
extern "C" BattleEventHandlerTableEntry* EventAddShedTail(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ShedTailHandlers);
    return ShedTailHandlers;
}

#endif // EXPAND_MOVES
