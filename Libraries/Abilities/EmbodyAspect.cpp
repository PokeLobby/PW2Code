#include "settings.h"
#if EXPAND_ABILITIES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"

#include "server_flow.h"

// EMBODY ASPECT (Teal, Hearthflame, Wellspring, Cornerstone)
// Hack PokeLobby: las 4 variantes hacen lo mismo. Una vez por combate y por Pokemon (al entrar o al adquirir la
// habilidad en mitad del combate) sube +1 una estadistica al azar entre Ataque, Defensa, At. Esp., Def. Esp. y
// Velocidad, ignorando las que ya estan en +6.
extern "C" void CommonEmbodyAspectHandler(BattleEventItem * item, ServerFlow * serverFlow, u32 pokemonSlot, u32 * work) {
    if (pokemonSlot != BattleEventVar_GetValue(VAR_MON_ID)) {
        return;
    }
    if (BattleField_CheckEmbodyAspectFlag(pokemonSlot)) {
        return;
    }

    BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
    // BattleMonValue y StatStage comparten numeracion 1-5 (Atk, Def, SpA, SpD, Spe); la etapa neutra vale 6
    u32 candidates[5];
    u32 count = 0;
    for (u32 stat = STATSTAGE_ATTACK; stat <= STATSTAGE_SPEED; ++stat) {
        if (BattleMon_GetValue(currentMon, (BattleMonValue)stat) < 12) {
            candidates[count++] = stat;
        }
    }
    if (count == 0) {
        return;
    }
    BattleField_SetEmbodyAspectFlag(pokemonSlot);

    HandlerParam_ChangeStatStage* statChange;
    statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
    statChange->header.flags |= HANDLER_ABILITY_POPUP_FLAG;
    statChange->pokeCount = 1;
    statChange->pokeID[0] = pokemonSlot;
    statChange->stat = (StatStage)candidates[BattleRandom(count)];
    statChange->volume = 1;
    BattleHandler_PopWork(serverFlow, statChange);
}
BattleEventHandlerTableEntry EmbodyAspectHandlers[]{
    {EVENT_SWITCH_IN, CommonEmbodyAspectHandler},
    {EVENT_AFTER_ABILITY_CHANGE, CommonEmbodyAspectHandler},
};

extern "C" BattleEventHandlerTableEntry * EventAddEmbodyAspectTeal(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(EmbodyAspectHandlers);
    return EmbodyAspectHandlers;
}

extern "C" BattleEventHandlerTableEntry * EventAddEmbodyAspectHearthflame(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(EmbodyAspectHandlers);
    return EmbodyAspectHandlers;
}

extern "C" BattleEventHandlerTableEntry * EventAddEmbodyAspectWellspring(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(EmbodyAspectHandlers);
    return EmbodyAspectHandlers;
}

extern "C" BattleEventHandlerTableEntry * EventAddEmbodyAspectCornerstorne(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(EmbodyAspectHandlers);
    return EmbodyAspectHandlers;
}
#endif // EXPAND_ABILITIES