#include "settings.h"
#if EXPAND_ABILITIES

#include "settings.h"
#include "Patches/BattleUpgrade/BattleEngine.h"

#include "server_flow.h"

// CURIOUS MEDICINE
// PokeLobby: reinicia TODAS las etapas (no solo las negativas) de los aliados en el campo (no de si mismo),
// y solo se activa cuando entra el propio Pokemon y tiene algun aliado (antes sacaba cartel y mensaje
// cada vez que entraba cualquier Pokemon y solo quitaba las negativas).
extern "C" void ResetAllStatStages(BattleMon * battleMon) {
    battleMon->statStageParam.AttackStage = 6;
    battleMon->statStageParam.DefenseStage = 6;
    battleMon->statStageParam.SpeedStage = 6;
    battleMon->statStageParam.SpAttackStage = 6;
    battleMon->statStageParam.SpDefenseStage = 6;
    battleMon->statStageParam.AccuracyStage = 6;
    battleMon->statStageParam.EvasionStage = 6;
}
extern "C" void HandlerCuriousMedicine(BattleEventItem * item, ServerFlow * serverFlow, u32 pokemonSlot, u32 * work) {
    if (pokemonSlot != BattleEventVar_GetValue(VAR_MON_ID)) {
        return;
    }

    b32 anyAlly = 0;
    for (u8 i = 0; i < 24; ++i) {
        BattleMon* affectedMon = PokeCon_GetBattleMon(serverFlow->pokeCon, i);
        if (affectedMon) {
            u32 affectedSlot = BattleMon_GetID(affectedMon);
            if (affectedSlot != pokemonSlot && MainModule_IsAllyMonID(pokemonSlot, affectedSlot)) {
                u32 affectedPos = Handler_PokeIDToPokePos(serverFlow, affectedSlot);
                // If it's currently in the battlefield
                if (affectedPos != NULL_BATTLE_POS &&
                    !BattleMon_IsFainted(affectedMon)) {
                    ResetAllStatStages(affectedMon);
                    anyAlly = 1;
                }
            }
        }
    }

    if (anyAlly) {
        BattleHandler_PushRun(serverFlow, EFFECT_ABILITY_POPUP_ADD, pokemonSlot);

        HandlerParam_Message* message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_CURIOUS_MEDICINE_MSGID);
        BattleHandler_PopWork(serverFlow, message);

        BattleHandler_PushRun(serverFlow, EFFECT_ABILITY_POPUP_REMOVE, pokemonSlot);
    }
}
BattleEventHandlerTableEntry CuriousMedicineHandlers[]{
    {EVENT_SWITCH_IN, HandlerCuriousMedicine},
    {EVENT_AFTER_ABILITY_CHANGE, HandlerCuriousMedicine},
};
extern "C" BattleEventHandlerTableEntry * EventAddCuriousMedicine(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(CuriousMedicineHandlers);
    return CuriousMedicineHandlers;
}
#endif // EXPAND_ABILITIES
