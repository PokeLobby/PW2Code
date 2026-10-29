#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 3): SPECTRAL THIEF (712)
// After the hit, the user keeps the target's stat boosts and the target loses them.
// Approximation: the boosts are stolen after the damage (the original steals them before and the damage uses them).
extern "C" void HandlerSpectralThief(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetCount = BattleEventVar_GetValue(VAR_TARGET_COUNT);

        for (u32 targetIdx = 0; targetIdx < targetCount; ++targetIdx) {
            u32 targetSlot = BattleEventVar_GetValue((BattleEventVar)(VAR_TARGET_MON_ID + targetIdx));
            if (targetSlot == BATTLE_MAX_SLOTS)
                continue;

            BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
            if (BattleMon_IsFainted(targetMon) || BattleMon_IsSubstituteActive(targetMon))
                continue;

            for (u32 stat = STATSTAGE_ATTACK; stat <= STATSTAGE_EVASION; ++stat) {
                // The stage value is 6 when there are no changes
                int boost = (int)BattleMon_GetValue(targetMon, (BattleMonValue)(VALUE_ATTACK_STAGE + (stat - STATSTAGE_ATTACK))) - 6;
                if (boost <= 0)
                    continue;

                HandlerParam_ChangeStatStage* remove;
                remove = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
                remove->pokeCount = 1;
                remove->pokeID[0] = targetSlot;
                remove->stat = (StatStage)stat;
                remove->volume = (s8)(-boost);
                BattleHandler_PopWork(serverFlow, remove);

                HandlerParam_ChangeStatStage* steal;
                steal = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
                steal->pokeCount = 1;
                steal->pokeID[0] = pokemonSlot;
                steal->stat = (StatStage)stat;
                steal->volume = (s8)boost;
                BattleHandler_PopWork(serverFlow, steal);
            }
        }
    }
}
BattleEventHandlerTableEntry SpectralThiefHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerSpectralThief},
};
extern "C" BattleEventHandlerTableEntry* EventAddSpectralThief(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SpectralThiefHandlers);
    return SpectralThiefHandlers;
}

#endif // EXPAND_MOVES
