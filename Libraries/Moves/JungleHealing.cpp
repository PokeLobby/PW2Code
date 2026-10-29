#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// JUNGLE HEALING & LUNAR BLESSING
// The user and its allies on the field recover 1/4 of their max HP and are cured of their status condition
extern "C" void HandlerJungleHealing(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        for (u32 i = 0; i < 24; ++i) {
            BattleMon* currentMon = PokeCon_GetBattleMon(serverFlow->pokeCon, i);
            if (!currentMon)
                continue;

            u32 currentSlot = BattleMon_GetID(currentMon);
            if (currentSlot != pokemonSlot && !MainModule_IsAllyMonID(pokemonSlot, currentSlot))
                continue;
            // Only the Pokemon on the field are affected
            if (Handler_PokeIDToPokePos(serverFlow, currentSlot) == NULL_BATTLE_POS)
                continue;
            if (BattleMon_IsFainted(currentMon))
                continue;

            if (!BattleMon_IsFullHP(currentMon) && !BattleMon_CheckIfMoveCondition(currentMon, CONDITION_HEALBLOCK)) {
                HandlerParam_RecoverHP* recoverHP;
                recoverHP = (HandlerParam_RecoverHP*)BattleHandler_PushWork(serverFlow, EFFECT_RECOVER_HP, pokemonSlot);
                recoverHP->pokeID = (u8)currentSlot;
                recoverHP->recoverHP = (u16)DivideMaxHPZeroCheck(currentMon, 4u);
                BattleHandler_StrSetup(&recoverHP->exStr, 2u, 387);
                BattleHandler_AddArg(&recoverHP->exStr, currentSlot);
                BattleHandler_PopWork(serverFlow, recoverHP);
            }

            CONDITION status = BattleMon_GetStatus(currentMon);
            if (status != CONDITION_NONE) {
                HandlerParam_CureCondition* cureCondition;
                cureCondition = (HandlerParam_CureCondition*)BattleHandler_PushWork(serverFlow, EFFECT_CURE_STATUS, pokemonSlot);
                cureCondition->condition = status;
                cureCondition->pokeCount = 1;
                cureCondition->pokeID[0] = (u8)currentSlot;
                BattleHandler_PopWork(serverFlow, cureCondition);
            }
        }
    }
}
BattleEventHandlerTableEntry JungleHealingHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE_NO_TARGET, HandlerJungleHealing},
};
extern "C" BattleEventHandlerTableEntry* EventAddJungleHealing(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(JungleHealingHandlers);
    return JungleHealingHandlers;
}

#endif // EXPAND_MOVES
