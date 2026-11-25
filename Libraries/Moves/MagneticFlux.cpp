#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"

#include "abilities.h"
#include "server_flow.h"

// MAGNETIC FLUX
extern "C" void HandlerMagneticFlux(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        if (defendingSlot >= BATTLE_MAX_SLOTS)
            return;
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, defendingSlot);
        ABILITY ability = BattleMon_GetValue(defendingMon, VALUE_ABILITY);
        // Magnetic Flux / Gear Up use target "all" with the stat changes in MoveData,
        // so only the user and its allies holding Plus or Minus keep the effect.
        bool isUserSide = defendingSlot == pokemonSlot || MainModule_IsAllyMonID(pokemonSlot, defendingSlot);
        if (!isUserSide || (ability != ABIL_PLUS && ability != ABIL_MINUS)) {
            BattleEventVar_RewriteValue(VAR_NO_EFFECT_FLAG, 1);
        }
    }
}
// Hack PokeLobby: si ni el usuario ni sus aliados en el campo tienen Plus o Minus, el movimiento falla
// («¡Pero ha fallado!», como en los oficiales) en vez de mostrar «No afecta a...» con cada Pokémon.
extern "C" void HandlerMagneticFluxCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot != BattleEventVar_GetValue(VAR_MON_ID))
        return;
    for (u32 i = 0; i < 24; ++i) {
        BattleMon* currentMon = PokeCon_GetBattleMon(serverFlow->pokeCon, i);
        if (!currentMon)
            continue;
        u32 currentSlot = BattleMon_GetID(currentMon);
        if (currentSlot != pokemonSlot && !MainModule_IsAllyMonID(pokemonSlot, currentSlot))
            continue;
        if (Handler_PokeIDToPokePos(serverFlow, currentSlot) == NULL_BATTLE_POS)
            continue;
        if (BattleMon_IsFainted(currentMon))
            continue;
        ABILITY ability = BattleMon_GetValue(currentMon, VALUE_ABILITY);
        if (ability == ABIL_PLUS || ability == ABIL_MINUS)
            return;
    }
    BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
}
BattleEventHandlerTableEntry MagneticFluxHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerMagneticFluxCheckFail},
    {EVENT_ABILITY_CHECK_NO_EFFECT, HandlerMagneticFlux},
};
extern "C" BattleEventHandlerTableEntry* EventAddMagneticFlux(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(MagneticFluxHandlers);
    return MagneticFluxHandlers;
}

#endif // EXPAND_MOVES
