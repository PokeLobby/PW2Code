#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"

#include "server_flow.h"

// SNOWSCAPE
// In this game Hail is replaced by Snow (SNOW_WEATHER), so Snowscape sets the same weather as Hail for 5 turns
extern "C" void HandlerSnowscapeCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (!CanWeatherBeChanged(serverFlow, WEATHER_HAIL, 5, 0)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerSnowscape(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerParam_ChangeWeather* changeWeather;
        changeWeather = (HandlerParam_ChangeWeather*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_WEATHER, pokemonSlot);
        changeWeather->weather = WEATHER_HAIL;
        changeWeather->turn = 5;
        BattleHandler_PopWork(serverFlow, changeWeather);
    }
}
BattleEventHandlerTableEntry SnowscapeHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerSnowscapeCheckFail},
    {EVENT_CALL_FIELD_EFFECT, HandlerSnowscape},
};
extern "C" BattleEventHandlerTableEntry* EventAddSnowscape(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SnowscapeHandlers);
    return SnowscapeHandlers;
}

#endif // EXPAND_MOVES
