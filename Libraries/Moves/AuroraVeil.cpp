#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"

#include "server_flow.h"
#include "server_events.h"

// AURORA VEIL
// Implemented as Reflect + Light Screen at the same time (5 turns, 8 with Light Clay, removed by Brick Break, Defog, etc.)
// It can only be used while it is hailing (snowing)
extern "C" void HandlerAuroraVeilCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        WEATHER weather = Handler_CheckWeather(serverFlow, pokemonSlot, BATTLE_MAX_SLOTS, ServerEvent_GetWeather(serverFlow));
        if (weather != WEATHER_HAIL) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerAuroraVeil(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u8 side = (u8)GetSideFromMonID(pokemonSlot);

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_AURORA_VEIL_MSGID + side);
        BattleHandler_PopWork(serverFlow, message);

        HandlerParam_AddSideEffect* reflect;
        reflect = (HandlerParam_AddSideEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_SIDE_EFFECT, pokemonSlot);
        reflect->sideEffect = SIDEEFF_REFLECT;
        reflect->side = side;
        reflect->condData = Condition_MakeTurn(5);
        BattleHandler_PopWork(serverFlow, reflect);

        HandlerParam_AddSideEffect* lightScreen;
        lightScreen = (HandlerParam_AddSideEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_SIDE_EFFECT, pokemonSlot);
        lightScreen->sideEffect = SIDEEFF_LIGHT_SCREEN;
        lightScreen->side = side;
        lightScreen->condData = Condition_MakeTurn(5);
        BattleHandler_PopWork(serverFlow, lightScreen);
    }
}
BattleEventHandlerTableEntry AuroraVeilHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerAuroraVeilCheckFail},
    {EVENT_UNCATEGORIZED_MOVE_NO_TARGET, HandlerAuroraVeil},
};
extern "C" BattleEventHandlerTableEntry* EventAddAuroraVeil(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(AuroraVeilHandlers);
    return AuroraVeilHandlers;
}

#endif // EXPAND_MOVES
