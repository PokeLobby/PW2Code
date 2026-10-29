#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"
#include "Patches/BattleUpgrade/include/FieldEffectExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): STEEL ROLLER (798)
// Fails if no terrain is active; if it hits, it removes the terrain.
extern "C" void HandlerSteelRollerCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (ServerEvent_GetTerrain(serverFlow) == TERRAIN_NULL) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerSteelRoller(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (ServerEvent_GetTerrain(serverFlow) != TERRAIN_NULL) {
            HandlerParam_RemoveFieldEffect* removeFieldEffect;
            removeFieldEffect = (HandlerParam_RemoveFieldEffect*)BattleHandler_PushWork(serverFlow, EFFECT_REMOVE_FIELD_EFFECT, pokemonSlot);
            removeFieldEffect->effect = FLDEFF_TERRAIN;
            BattleHandler_PopWork(serverFlow, removeFieldEffect);
        }
    }
}
BattleEventHandlerTableEntry SteelRollerHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerSteelRollerCheckFail},
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerSteelRoller},
};
extern "C" BattleEventHandlerTableEntry* EventAddSteelRoller(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SteelRollerHandlers);
    return SteelRollerHandlers;
}

#endif // EXPAND_MOVES
