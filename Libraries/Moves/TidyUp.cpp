#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// ServerControl_RemoveSubstitute (symbol added to ESDB.yml)
extern "C" void ServerControl_RemoveSubstitute(ServerFlow* serverFlow, BattleMon* battleMon);

// TIDY UP
// Removes the entry hazards and the substitutes of every Pokemon on the field, then raises the user's Attack and Speed
extern "C" void HandlerTidyUp(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u8 sides[2];
        sides[0] = (u8)GetSideFromMonID(pokemonSlot);
        sides[1] = GetSideFromOpposingMonID(pokemonSlot);

        for (u32 sideIdx = 0; sideIdx < 2; ++sideIdx) {
            HandlerParam_RemoveSideEffect* removeSide;
            removeSide = (HandlerParam_RemoveSideEffect*)BattleHandler_PushWork(serverFlow, EFFECT_REMOVE_SIDE_EFFECT, pokemonSlot);
            removeSide->side = sides[sideIdx];
            REMOVE_SIDE_EFFECT_SETUP(removeSide->flags);
            SET_REMOVE_SIDE_EFFEC_FLAG(SIDEEFF_SPIKES, removeSide->flags);
            SET_REMOVE_SIDE_EFFEC_FLAG(SIDEEFF_TOXIC_SPIKES, removeSide->flags);
            SET_REMOVE_SIDE_EFFEC_FLAG(SIDEEFF_STEALTH_ROCK, removeSide->flags);
            SET_REMOVE_SIDE_EFFEC_FLAG(SIDEEFF_STICKY_WEB, removeSide->flags);
            BattleHandler_PopWork(serverFlow, removeSide);
        }

        for (u32 i = 0; i < 24; ++i) {
            BattleMon* currentMon = PokeCon_GetBattleMon(serverFlow->pokeCon, i);
            if (!currentMon)
                continue;

            u32 currentSlot = BattleMon_GetID(currentMon);
            if (Handler_PokeIDToPokePos(serverFlow, currentSlot) == NULL_BATTLE_POS)
                continue;
            if (!BattleMon_IsFainted(currentMon) && BattleMon_IsSubstituteActive(currentMon)) {
                ServerControl_RemoveSubstitute(serverFlow, currentMon);
            }
        }

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_TIDY_UP_MSGID);
        BattleHandler_PopWork(serverFlow, message);

        HandlerParam_ChangeStatStage* attackBoost;
        attackBoost = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
        attackBoost->pokeCount = 1;
        attackBoost->pokeID[0] = pokemonSlot;
        attackBoost->moveAnimation = 1;
        attackBoost->stat = STATSTAGE_ATTACK;
        attackBoost->volume = 1;
        BattleHandler_PopWork(serverFlow, attackBoost);

        HandlerParam_ChangeStatStage* speedBoost;
        speedBoost = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
        speedBoost->pokeCount = 1;
        speedBoost->pokeID[0] = pokemonSlot;
        speedBoost->moveAnimation = 1;
        speedBoost->stat = STATSTAGE_SPEED;
        speedBoost->volume = 1;
        BattleHandler_PopWork(serverFlow, speedBoost);
    }
}
BattleEventHandlerTableEntry TidyUpHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerTidyUp},
};
extern "C" BattleEventHandlerTableEntry* EventAddTidyUp(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TidyUpHandlers);
    return TidyUpHandlers;
}

#endif // EXPAND_MOVES
