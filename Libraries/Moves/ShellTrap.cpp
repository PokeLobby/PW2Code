#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): SHELL TRAP (704, priority -3)
// At the start of the turn the user sets the trap (message + position effect). The move only works if the user
// has been hit by a physical move during the turn (flag stored in the BattleField by the position effect).

// POS SHELL TRAP
// work[0] -> battle slot of the Pokemon with the shell trap
extern "C" void HandlerPosShellTrapReaction(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_DEFENDING_MON) == work[0] &&
        BattleEventVar_GetValue(VAR_ATTACKING_MON) != work[0] &&
        !BattleEventVar_GetValue(VAR_SUBSTITUTE_FLAG) &&
        BattleEventVar_GetValue(VAR_MOVE_CATEGORY) == CATEGORY_PHYSICAL) {
        BattleField_SetShellTrapHit(work[0], 1);
    }
}
BattleEventHandlerTableEntry PosShellTrapHandlers[]{
    {EVENT_MOVE_DAMAGE_REACTION_1, HandlerPosShellTrapReaction},
    {EVENT_TURN_CHECK_DONE, HandlerPosTurnCheckDone},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosShellTrap(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PosShellTrapHandlers);
    return PosShellTrapHandlers;
}

// SHELL TRAP
extern "C" void HandlerShellTrapBeforeAttacks(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleField_SetShellTrapHit(pokemonSlot, 0);

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_SHELL_TRAP_MSGID);
        BattleHandler_AddArg(&message->str, pokemonSlot);
        BattleHandler_PopWork(serverFlow, message);

        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_SHELL_TRAP;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, pokemonSlot);
        addEffectPos->workToCopy[0] = pokemonSlot;
        addEffectPos->workCount = 1;
        BattleHandler_PopWork(serverFlow, addEffectPos);
    }
}
extern "C" void HandlerShellTrapCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (!BattleField_GetShellTrapHit(pokemonSlot)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
        else {
            BattleField_SetShellTrapHit(pokemonSlot, 0);
        }
    }
}
BattleEventHandlerTableEntry ShellTrapHandlers[]{
    {EVENT_BEFORE_ATTACKS, HandlerShellTrapBeforeAttacks},
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerShellTrapCheckFail},
};
extern "C" BattleEventHandlerTableEntry* EventAddShellTrap(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ShellTrapHandlers);
    return ShellTrapHandlers;
}

#endif // EXPAND_MOVES
