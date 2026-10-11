#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// POLLEN PUFF
extern "C" void HandlerPollenPuffAllyCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);

        if (MainModule_IsAllyMonID(pokemonSlot, defendingSlot)) {
            BattleEventVar_RewriteValue(VAR_GENERAL_USE_FLAG, 1);
        }
    }
}
extern "C" void HandlerPollenPuffHeal(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, defendingSlot);

        if (BattleMon_IsFullHP(defendingMon)) {
            HandlerParam_Message* message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
            BattleHandler_StrSetup(&message->str, 2, 210);
            BattleHandler_AddArg(&message->str, defendingSlot);
            BattleHandler_PopWork(serverFlow, message);
        }
        else {
            HandlerParam_RecoverHP* recoverHP = (HandlerParam_RecoverHP*)BattleHandler_PushWork(serverFlow, EFFECT_RECOVER_HP, pokemonSlot);
            recoverHP->pokeID = defendingSlot;
            recoverHP->recoverHP = DivideMaxHPZeroCheck(defendingMon, 2u);
            BattleHandler_StrSetup(&recoverHP->exStr, 2, 387);
            BattleHandler_AddArg(&recoverHP->exStr, defendingSlot);
            BattleHandler_PopWork(serverFlow, recoverHP);
        }
    }
}
BattleEventHandlerTableEntry PollenPuffHandlers[] {
    {EVENT_CHECK_DAMAGE_TO_RECOVER, HandlerPollenPuffAllyCheck},
    {EVENT_APPLY_DAMAGE_TO_RECOVER, HandlerPollenPuffHeal},
};

extern "C" BattleEventHandlerTableEntry* EventAddPollenPuff(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PollenPuffHandlers);
    return PollenPuffHandlers;
}
#endif // EXPAND_MOVES
