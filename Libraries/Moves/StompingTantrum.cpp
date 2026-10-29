#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): STOMPING TANTRUM (707). Power x2 if the user's previous action failed.
// The result is stored in BattleEngine.cpp (BattleField_GetMoveFailedPrev): "But it failed", missed move,
// blocked by protection, immune target or no effect.
extern "C" void HandlerStompingTantrumPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (BattleField_GetMoveFailedPrev(pokemonSlot)) {
            BattleEventVar_RewriteValue(VAR_MOVE_POWER, BattleEventVar_GetValue(VAR_MOVE_POWER) * 2);
        }
    }
}
BattleEventHandlerTableEntry StompingTantrumHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerStompingTantrumPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddStompingTantrum(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(StompingTantrumHandlers);
    return StompingTantrumHandlers;
}

#endif // EXPAND_MOVES
