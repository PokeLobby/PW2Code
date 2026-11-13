#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/Contact.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): BEAK BLAST (690, priority -3)
// At the start of the turn the user heats its beak (message + position effect). While it is heated, every
// Pokemon that hits it with a contact move gets burned. The heat ends when the user moves or at the end of the turn.

// POS BEAK BLAST
// work[0] -> battle slot of the Pokemon with the heated beak
extern "C" void HandlerPosBeakBlastReaction(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
    if (defendingSlot != work[0] || BattleEventVar_GetValue(VAR_SUBSTITUTE_FLAG))
        return;

    MOVE_ID moveID = BattleEventVar_GetValue(VAR_MOVE_ID);
    u32 attackingSlot = BattleEventVar_GetValue(VAR_ATTACKING_MON);
    if (attackingSlot == defendingSlot)
        return;

    if (MakesContact(serverFlow, moveID, attackingSlot, defendingSlot)) {
        HandlerParam_AddCondition* addCondition;
        addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, defendingSlot);
        addCondition->condition = CONDITION_BURN;
        addCondition->condData = MakeBasicStatus(CONDITION_BURN);
        addCondition->almost = 0;
        addCondition->pokeID = attackingSlot;
        BattleHandler_PopWork(serverFlow, addCondition);
    }
}
extern "C" void HandlerPosBeakBlastMoveStart(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_ATTACKING_MON) == work[0]) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosBeakBlastHandlers[]{
    {EVENT_MOVE_DAMAGE_REACTION_1, HandlerPosBeakBlastReaction},
    {EVENT_MOVE_SEQUENCE_START, HandlerPosBeakBlastMoveStart},
    {EVENT_TURN_CHECK_DONE, HandlerPosTurnCheckDone},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosBeakBlast(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PosBeakBlastHandlers);
    return PosBeakBlastHandlers;
}

// BEAK BLAST
extern "C" void HandlerBeakBlastBeforeAttacks(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_BEAK_BLAST_MSGID);
        BattleHandler_AddArg(&message->str, pokemonSlot);
        BattleHandler_PopWork(serverFlow, message);

        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_BEAK_BLAST;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, pokemonSlot);
        addEffectPos->workToCopy[0] = pokemonSlot;
        addEffectPos->workCount = 1;
        BattleHandler_PopWork(serverFlow, addEffectPos);
    }
}
BattleEventHandlerTableEntry BeakBlastHandlers[]{
    {EVENT_BEFORE_ATTACKS, HandlerBeakBlastBeforeAttacks},
};
extern "C" BattleEventHandlerTableEntry* EventAddBeakBlast(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(BeakBlastHandlers);
    return BeakBlastHandlers;
}

#endif // EXPAND_MOVES
