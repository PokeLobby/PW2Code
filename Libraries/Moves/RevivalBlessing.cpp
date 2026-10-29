#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// REVIVAL BLESSING
// There is no party selection menu in the battle engine, so the first fainted Pokemon of the user's party is revived
// with half of its max HP.
extern "C" u32 RevivalBlessingFindTarget(ServerFlow* serverFlow, u32 pokemonSlot) {
    u32 clientID = MainModule_PokeIDToClientID(pokemonSlot);
    for (u32 i = 0; i < 24; ++i) {
        BattleMon* partyMon = PokeCon_GetBattleMon(serverFlow->pokeCon, i);
        if (partyMon) {
            u32 partySlot = BattleMon_GetID(partyMon);
            if (MainModule_PokeIDToClientID(partySlot) == clientID && BattleMon_IsFainted(partyMon)) {
                return partySlot;
            }
        }
    }
    return BATTLE_MAX_SLOTS;
}
extern "C" void HandlerRevivalBlessingCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (RevivalBlessingFindTarget(serverFlow, pokemonSlot) == BATTLE_MAX_SLOTS) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerRevivalBlessing(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = RevivalBlessingFindTarget(serverFlow, pokemonSlot);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;

        BattleMon* targetMon = PokeCon_GetBattleMon(serverFlow->pokeCon, targetSlot);

        HandlerParam_Revive* revive;
        revive = (HandlerParam_Revive*)BattleHandler_PushWork(serverFlow, EFFECT_REVIVE, pokemonSlot);
        revive->pokeID = (u8)targetSlot;
        revive->hp = (u16)DivideMaxHPZeroCheck(targetMon, 2u);
        BattleHandler_StrSetup(&revive->exStr, 2u, BATTLE_REVIVED_MSGID);
        BattleHandler_AddArg(&revive->exStr, targetSlot);
        BattleHandler_PopWork(serverFlow, revive);
    }
}
BattleEventHandlerTableEntry RevivalBlessingHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerRevivalBlessingCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerRevivalBlessing},
};
extern "C" BattleEventHandlerTableEntry* EventAddRevivalBlessing(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(RevivalBlessingHandlers);
    return RevivalBlessingHandlers;
}

#endif // EXPAND_MOVES
