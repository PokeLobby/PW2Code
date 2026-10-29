#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"

#include "abilities.h"
#include "server_flow.h"

// REMOVE TYPE
// work[0] -> stores the type to become
// work[1] -> flag to remove extra type
void HandlerRemoveTypeCheck(ServerFlow *serverFlow, u32 pokemonSlot, PokeType type, u32* work, u32 msgID) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        if (!BattleMon_HasType(currentMon, type)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
            return;
        }

        work[0] = TYPE_NULL;
        work[1] = 0;
        work[2] = msgID;

        if (BattleField_GetExtraType(pokemonSlot) == type) {
            work[1] = 1;
            return;
        }

        if (currentMon->type[0] != currentMon->type[1]) {
            work[0] = (currentMon->type[0] == type) ? currentMon->type[1] : currentMon->type[0];
        }
    }
}
extern "C" void HandlerRemoveType(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {

        if (work[1] == 0) {
            HandlerParam_ChangeType* changeType = (HandlerParam_ChangeType*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_TYPE, pokemonSlot);
            changeType->pokeType = work[0];
            changeType->pokeID = pokemonSlot;
            changeType->skipText = 1;
            BattleHandler_PopWork(serverFlow, changeType);
        } else {
            BattleField_SetExtraType(pokemonSlot, TYPE_NULL);
        }

        HandlerParam_Message* message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2, work[2]);
        BattleHandler_AddArg(&message->str, pokemonSlot);
        BattleHandler_PopWork(serverFlow, message);
    }
}

// BURN UP
extern "C" void HandlerBurnUpCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    HandlerRemoveTypeCheck(serverFlow, pokemonSlot, TYPE_FIRE, work, BATTLE_BURN_UP_MSGID);
}
BattleEventHandlerTableEntry BurnUpHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerBurnUpCheck},
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerRemoveType},
};
extern "C" BattleEventHandlerTableEntry* EventAddBurnUp(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(BurnUpHandlers);
    return BurnUpHandlers;
}

// DOUBLE SHOCK
extern "C" void HandlerDoubleShockCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    HandlerRemoveTypeCheck(serverFlow, pokemonSlot, TYPE_ELEC, work, BATTLE_DOUBLE_SHOCK_MSGID);
}
BattleEventHandlerTableEntry DoubleShockHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerDoubleShockCheck},
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerRemoveType},
};
extern "C" BattleEventHandlerTableEntry* EventAddDoubleShock(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(DoubleShockHandlers);
    return DoubleShockHandlers;
}

#endif // EXPAND_MOVES
