#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "main_module.h"
#include "server_flow.h"

// Hack PokeLobby (lote 4): ataques de campo y de trampa
//  587 Cerrojo Feerico (Fairy Lock): nadie puede huir ni cambiar durante el turno siguiente
//  753 Octopresa (Octolock): atrapa al objetivo y le baja Defensa y Def. Esp. al final de cada turno
//  603 Paga Extra (Happy Hour): dinero extra tras el combate
//  752 Hora del Te (Teatime): todos los Pokemon en combate se comen su baya
//  756 Cambio de Cancha (Court Change): los dos bandos intercambian sus efectos de campo
//  881 Fria Acogida (Chilly Reception): nieve 5 turnos y el usuario se retira

// ---- utilidades
static void AddBlock(ServerFlow* serverFlow, u32 sourceSlot, u32 trappedSlot, u32 trapperSlot, b32 message) {
    HandlerParam_AddCondition* addCondition;
    addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, sourceSlot);
    addCondition->condition = CONDITION_BLOCK;
    addCondition->condData = ConditionData_MakePoke(trapperSlot);
    addCondition->almost = 0;
    addCondition->pokeID = trappedSlot;
    if (message) {
        BattleHandler_StrSetup(&addCondition->exStr, 2u, 872);
        BattleHandler_AddArg(&addCondition->exStr, trappedSlot);
    }
    BattleHandler_PopWork(serverFlow, addCondition);
}
static b32 IsOnField(ServerFlow* serverFlow, u32 slot) {
    return Handler_PokeIDToPokePos(serverFlow, slot) != NULL_BATTLE_POS;
}

// ---------------- FAIRY LOCK (587)
// work[0] -> mascara de los Pokemon a los que se ha puesto la trampa (un bit por slot)
// work[1] -> turnos completos transcurridos
extern "C" void HandlerPosFairyLockTurnEnd(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) != BATTLE_MAX_SLOTS || work[1] < 1 || work[0] == 0)
        return;

    // Segundo final de turno: se quitan las trampas que puso Cerrojo Feerico
    for (u32 slot = 0; slot < 24; ++slot) {
        if (!(work[0] & (1u << slot)))
            continue;
        BattleMon* mon = PokeCon_GetBattleMon(serverFlow->pokeCon, slot);
        if (!mon || BattleMon_IsFainted(mon) || !IsOnField(serverFlow, slot))
            continue;
        if (!BattleMon_CheckIfMoveCondition(mon, CONDITION_BLOCK))
            continue;

        HandlerParam_CureCondition* cure;
        cure = (HandlerParam_CureCondition*)BattleHandler_PushWork(serverFlow, EFFECT_CURE_STATUS, slot);
        cure->pokeCount = 1;
        cure->pokeID[0] = (u8)slot;
        cure->condition = CONDITION_BLOCK;
        BattleHandler_PopWork(serverFlow, cure);
    }
    work[0] = 0;
}
extern "C" void HandlerPosFairyLockTurnDone(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == BATTLE_MAX_SLOTS) {
        if (work[1] >= 1) {
            BattleEventItem_Remove(item);
        }
        else {
            ++work[1];
        }
    }
}
BattleEventHandlerTableEntry PosFairyLockHandlers[]{
    {EVENT_TURN_CHECK_END, HandlerPosFairyLockTurnEnd},
    {EVENT_TURN_CHECK_DONE, HandlerPosFairyLockTurnDone},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosFairyLock(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosFairyLockHandlers));
    return PosFairyLockHandlers;
}
extern "C" void HandlerFairyLock(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 mask = 0;
        for (u32 slot = 0; slot < 24; ++slot) {
            BattleMon* mon = PokeCon_GetBattleMon(serverFlow->pokeCon, slot);
            if (!mon || BattleMon_IsFainted(mon) || !IsOnField(serverFlow, slot))
                continue;
            // Los fantasmas ignoran las trampas
            if (BattleMon_HasType(mon, TYPE_GHOST) || BattleMon_CheckIfMoveCondition(mon, CONDITION_BLOCK))
                continue;
            AddBlock(serverFlow, pokemonSlot, slot, slot, 0);
            mask |= (1u << slot);
        }

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_FAIRY_LOCK_MSGID);
        BattleHandler_PopWork(serverFlow, message);

        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_FAIRY_LOCK;
        addEffectPos->targetPos = NULL_BATTLE_POS;
        addEffectPos->workToCopy[0] = mask;
        addEffectPos->workToCopy[1] = 0;
        addEffectPos->workCount = 2;
        BattleHandler_PopWork(serverFlow, addEffectPos);
    }
}
BattleEventHandlerTableEntry FairyLockHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerFairyLock},
};
extern "C" BattleEventHandlerTableEntry* EventAddFairyLock(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(FairyLockHandlers);
    return FairyLockHandlers;
}

// ---------------- OCTOLOCK (753)
// work[0] -> slot del Pokemon atrapado
// work[1] -> slot del usuario
extern "C" void HandlerPosOctolockTurnEnd(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot != work[0])
        return;

    BattleMon* target = PokeCon_GetBattleMon(serverFlow->pokeCon, work[0]);
    BattleMon* user = PokeCon_GetBattleMon(serverFlow->pokeCon, work[1]);
    if (!target || !user || BattleMon_IsFainted(target) || BattleMon_IsFainted(user))
        return;
    if (!IsOnField(serverFlow, work[0]) || !IsOnField(serverFlow, work[1]))
        return;

    static const StatStage stats[2] = { STATSTAGE_DEFENSE, STATSTAGE_SPECIAL_DEFENSE };
    for (u32 i = 0; i < 2; ++i) {
        HandlerParam_ChangeStatStage* statChange;
        statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, work[1]);
        statChange->pokeCount = 1;
        statChange->pokeID[0] = (u8)work[0];
        statChange->moveAnimation = 1;
        statChange->stat = stats[i];
        statChange->volume = -1;
        BattleHandler_PopWork(serverFlow, statChange);
    }
}
extern "C" void HandlerPosOctolockSwitchOut(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == work[0] || slot == work[1]) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosOctolockFainted(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if ((slot == work[0] || slot == work[1]) && BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, slot))) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosOctolockHandlers[]{
    {EVENT_TURN_CHECK_END, HandlerPosOctolockTurnEnd},
    {EVENT_SWITCH_OUT_END, HandlerPosOctolockSwitchOut},
    {EVENT_NOTIFY_FAINTED, HandlerPosOctolockFainted},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosOctolock(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosOctolockHandlers));
    return PosOctolockHandlers;
}
extern "C" void HandlerOctolock(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;

        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        if (BattleMon_IsFainted(targetMon) || BattleMon_HasType(targetMon, TYPE_GHOST))
            return;

        if (!BattleMon_CheckIfMoveCondition(targetMon, CONDITION_BLOCK))
            AddBlock(serverFlow, pokemonSlot, targetSlot, pokemonSlot, 1);

        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_OCTOLOCK;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, targetSlot);
        addEffectPos->workToCopy[0] = targetSlot;
        addEffectPos->workToCopy[1] = pokemonSlot;
        addEffectPos->workCount = 2;
        BattleHandler_PopWork(serverFlow, addEffectPos);
    }
}
BattleEventHandlerTableEntry OctolockHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerOctolock},
};
extern "C" BattleEventHandlerTableEntry* EventAddOctolock(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(OctolockHandlers);
    return OctolockHandlers;
}

// ---------------- HAPPY HOUR (603)
// El premio en dinero se calcula al final del combate, fuera del motor de ataques: aqui se anade un extra al dinero
// "de bonificacion" (el mismo que usa Dia de Pago): 40 x nivel del usuario, una vez por combate y solo para el jugador.
extern "C" void HandlerHappyHour(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 clientID = MainModule_PokeIDToClientID(pokemonSlot);
        u8 playerSide = (clientID == serverFlow->mainModule->playerClientID);
        Lote4State* state = GetLote4State();
        u8 bit = playerSide ? 1 : 2;
        if (state->happyHour & bit)
            return;
        state->happyHour |= bit;

        if (playerSide) {
            BattleMon* mon = Handler_GetBattleMon(serverFlow, pokemonSlot);
            u32 extra = 40 * BattleMon_GetValue(mon, VALUE_LEVEL);
            u32 bonus = serverFlow->mainModule->bonusMoney + extra;
            serverFlow->mainModule->bonusMoney = bonus > 99999 ? 99999 : bonus;
        }

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_HAPPY_HOUR_MSGID);
        BattleHandler_PopWork(serverFlow, message);
    }
}
BattleEventHandlerTableEntry HappyHourHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerHappyHour},
};
extern "C" BattleEventHandlerTableEntry* EventAddHappyHour(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(HappyHourHandlers);
    return HappyHourHandlers;
}

// ---------------- TEATIME (752)
static b32 HoldsBerry(ServerFlow* serverFlow, u32 slot) {
    BattleMon* mon = PokeCon_GetBattleMon(serverFlow->pokeCon, slot);
    if (!mon || BattleMon_IsFainted(mon) || !IsOnField(serverFlow, slot))
        return 0;
    ITEM itemID = BattleMon_GetHeldItem(mon);
    return itemID && PML_ItemIsBerry(itemID);
}
extern "C" void HandlerTeatimeCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        for (u32 slot = 0; slot < 24; ++slot) {
            if (HoldsBerry(serverFlow, slot))
                return;
        }
        BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
    }
}
extern "C" void HandlerTeatime(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 1u, BATTLE_TEATIME_MSGID);
        BattleHandler_PopWork(serverFlow, message);

        for (u32 slot = 0; slot < 24; ++slot) {
            if (HoldsBerry(serverFlow, slot))
                BattleHandler_PushRun(serverFlow, EFFECT_USE_HELD_ITEM, slot);
        }
    }
}
BattleEventHandlerTableEntry TeatimeHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerTeatimeCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerTeatime},
};
extern "C" BattleEventHandlerTableEntry* EventAddTeatime(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TeatimeHandlers);
    return TeatimeHandlers;
}

// ---------------- COURT CHANGE (756)
extern "C" void HandlerCourtChange(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        // SwapSideEffects (BattleEngine.cpp) intercambia los efectos de los dos lados; devuelve cuantos ha movido
        if (SwapSideEffects()) {
            HandlerParam_Message* message;
            message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
            BattleHandler_StrSetup(&message->str, 1u, BATTLE_COURT_CHANGE_MSGID);
            BattleHandler_PopWork(serverFlow, message);
        }
        else {
            // Sin nada que intercambiar el ataque falla
            ServerDisplay_MoveFail(serverFlow);
        }
    }
}
BattleEventHandlerTableEntry CourtChangeHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerCourtChange},
};
extern "C" BattleEventHandlerTableEntry* EventAddCourtChange(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(CourtChangeHandlers);
    return CourtChangeHandlers;
}

// ---------------- CHILLY RECEPTION (881)
// work[0] -> 1 si el usuario debe retirarse al acabar el ataque
extern "C" void HandlerChillyReception(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_CHILLY_RECEPTION_MSGID);
        BattleHandler_AddArg(&message->str, pokemonSlot);
        BattleHandler_PopWork(serverFlow, message);

        // En este juego el granizo es la nieve
        if (CanWeatherBeChanged(serverFlow, WEATHER_HAIL, 5, 0)) {
            HandlerParam_ChangeWeather* changeWeather;
            changeWeather = (HandlerParam_ChangeWeather*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_WEATHER, pokemonSlot);
            changeWeather->weather = WEATHER_HAIL;
            changeWeather->turn = 5;
            BattleHandler_PopWork(serverFlow, changeWeather);
        }
        work[0] = 1;
    }
}
extern "C" void HandlerChillyReceptionSwitch(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID) && work[0] == 1) {
        work[0] = 0;
        if (Handler_GetFightEnableBenchPokeNum(serverFlow, pokemonSlot)
            && Handler_CheckReservedMemberChangeAction(serverFlow)) {

            HandlerParam_Switch* switchOut;
            switchOut = (HandlerParam_Switch*)BattleHandler_PushWork(serverFlow, EFFECT_SWITCH, pokemonSlot);
            switchOut->pokeID = pokemonSlot;
            BattleHandler_PopWork(serverFlow, switchOut);
        }
    }
}
BattleEventHandlerTableEntry ChillyReceptionHandlers[]{
    {EVENT_CALL_FIELD_EFFECT, HandlerChillyReception},
    {EVENT_MOVE_SEQUENCE_END, HandlerChillyReceptionSwitch},
};
extern "C" BattleEventHandlerTableEntry* EventAddChillyReception(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ChillyReceptionHandlers);
    return ChillyReceptionHandlers;
}

#endif // EXPAND_MOVES
