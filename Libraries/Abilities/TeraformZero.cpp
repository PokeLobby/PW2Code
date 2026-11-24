#include "settings.h"
#if EXPAND_ABILITIES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/FieldEffectExpansion.h"

#include "server_flow.h"

// TERAFORM ZERO
extern "C" void HandlerTeraformZero(BattleEventItem * item, ServerFlow * serverFlow, u32 pokemonSlot, u32 * work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleHandler_PushRun(serverFlow, EFFECT_ABILITY_POPUP_ADD, pokemonSlot);

        HandlerParam_ChangeWeather* changeWeather;
        changeWeather = (HandlerParam_ChangeWeather*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_WEATHER, pokemonSlot);
        changeWeather->weather = WEATHER_NULL;
        BattleHandler_PopWork(serverFlow, changeWeather);

        b32 habiaCampo = ServerEvent_GetTerrain(serverFlow) != TERRAIN_NULL;
        HandlerParam_AddFieldEffect* addFieldEffect;
        addFieldEffect = (HandlerParam_AddFieldEffect*)BattleHandler_PushWork(serverFlow, EFFECT_REMOVE_FIELD_EFFECT, pokemonSlot);
        addFieldEffect->effect = FLDEFF_TERRAIN;
        BattleHandler_PopWork(serverFlow, addFieldEffect);
        // Como al acabarse los turnos: Mimetismo, Carga Cuantica... se enteran (streamers-serie-2#24)
        if (habiaCampo)
            ServerEvent_ChangeTerrain(serverFlow, TERRAIN_NULL);

        BattleHandler_PushRun(serverFlow, EFFECT_ABILITY_POPUP_REMOVE, pokemonSlot);
    }
}
BattleEventHandlerTableEntry TeraformZeroHandlers[]{
    {EVENT_AFTER_ABILITY_CHANGE, HandlerTeraformZero},
};
extern "C" BattleEventHandlerTableEntry * EventAddTeraformZero(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TeraformZeroHandlers);
    return TeraformZeroHandlers;
}
#endif // EXPAND_ABILITIES