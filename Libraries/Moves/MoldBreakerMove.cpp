#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): moves that ignore the target's ability (Mold Breaker only during the hit)
// Sunsteel Strike (713), Moongeist Beam (714) and Photon Geyser (722).
// They use the game's own Mold Breaker handlers; work[0] marks them as active.
extern "C" void HandlerMoldBreakerStart(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
extern "C" void HandlerMoldBreakerEnd(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);

BattleEventHandlerTableEntry MoldBreakerMoveHandlers[]{
    {EVENT_MOVE_SEQUENCE_START, HandlerMoldBreakerStart},
    {EVENT_MOVE_SEQUENCE_END, HandlerMoldBreakerEnd},
};
extern "C" BattleEventHandlerTableEntry* EventAddMoldBreakerMove(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(MoldBreakerMoveHandlers);
    return MoldBreakerMoveHandlers;
}

// PHOTON GEYSER: physical if the user's Attack is higher than its Special Attack
extern "C" void HandlerPhotonGeyserCategory(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 attack = BattleMon_GetRealStat(currentMon, VALUE_ATTACK_STAT);
        u32 specialAttack = BattleMon_GetRealStat(currentMon, VALUE_SPECIAL_ATTACK_STAT);
        if (attack > specialAttack) {
            BattleEventVar_RewriteValue(VAR_MOVE_CATEGORY, CATEGORY_PHYSICAL);
        }
    }
}
BattleEventHandlerTableEntry PhotonGeyserHandlers[]{
    {EVENT_MOVE_PARAM, HandlerPhotonGeyserCategory},
    {EVENT_MOVE_SEQUENCE_START, HandlerMoldBreakerStart},
    {EVENT_MOVE_SEQUENCE_END, HandlerMoldBreakerEnd},
};
extern "C" BattleEventHandlerTableEntry* EventAddPhotonGeyser(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PhotonGeyserHandlers);
    return PhotonGeyserHandlers;
}

#endif // EXPAND_MOVES
