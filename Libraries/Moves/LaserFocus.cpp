#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// POS LASER FOCUS
extern "C" void HandlerPosLaserFocus(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_ATTACKING_MON) == Handler_PokePosToPokeID(serverFlow, targetPos)) {
        BattleEventVar_RewriteValue(VAR_CRIT_STAGE, 4);
    }
}
extern "C" void HandlerPosTurnCheckDoneNext(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == BATTLE_MAX_SLOTS) {
        // We know the turn Laser Focus is used it will be 0,
        // thus being removed the next turn
        if (work[0] >= 1) {
            BattleEventItem_Remove(item);
        }
        ++work[0];
    }
}

BattleEventHandlerTableEntry PosLaserFocusHandlers[]{
    {EVENT_CRITICAL_CHECK, HandlerPosLaserFocus},
    {EVENT_TURN_CHECK_DONE, HandlerPosTurnCheckDoneNext},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosLaserFocus(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosLaserFocusHandlers));
    return PosLaserFocusHandlers;
}

// LASER FOCUS
extern "C" void HandlerLaserFocus(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_LASER_FOCUS;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, pokemonSlot);
        // Make sure to initialize work to 0, so the turn count works properly
        addEffectPos->workToCopy[0] = 0;
        addEffectPos->workCount = 1;
        BattleHandler_PopWork(serverFlow, addEffectPos);

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_LASER_FOCUS_MSGID);
        BattleHandler_PopWork(serverFlow, message);
    }
}
BattleEventHandlerTableEntry LaserFocusHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerLaserFocus},
};
extern "C" BattleEventHandlerTableEntry* EventAddLaserFocus(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(LaserFocusHandlers);
    return LaserFocusHandlers;
}

#endif // EXPAND_MOVES
