#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "abilities.h"
#include "server_flow.h"

// Hack PokeLobby (lote 3): MIND BLOWN (720)
// After hitting, the user loses half of its max HP (rounded up). Magic Guard avoids it.
// work[0] -> 1 if the move has hit somebody
#define BATTLE_MIND_BLOWN_MSGID 430 // file 18, 3 variants: "X se ha herido!"
extern "C" void HandlerMindBlownHit(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        work[0] = 1;
    }
}
extern "C" void HandlerMindBlownRecoil(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID) && work[0] == 1) {
        work[0] = 0;
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        if (BattleMon_IsFainted(currentMon))
            return;
        if (BattleMon_GetValue(currentMon, VALUE_EFFECTIVE_ABILITY) == ABIL_MAGIC_GUARD)
            return;

        HandlerParam_Damage* damage;
        damage = (HandlerParam_Damage*)BattleHandler_PushWork(serverFlow, EFFECT_DAMAGE, pokemonSlot);
        damage->pokeID = pokemonSlot;
        damage->damage = (BattleMon_GetValue(currentMon, VALUE_MAX_HP) + 1) / 2;
        BattleHandler_StrSetup(&damage->exStr, 2u, BATTLE_MIND_BLOWN_MSGID);
        BattleHandler_AddArg(&damage->exStr, pokemonSlot);
        BattleHandler_PopWork(serverFlow, damage);
    }
}
BattleEventHandlerTableEntry MindBlownHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerMindBlownHit},
    {EVENT_MOVE_SEQUENCE_END, HandlerMindBlownRecoil},
};
extern "C" BattleEventHandlerTableEntry* EventAddMindBlown(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(MindBlownHandlers);
    return MindBlownHandlers;
}

#endif // EXPAND_MOVES
