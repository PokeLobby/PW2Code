#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "abilities.h"
#include "server_flow.h"

// Hack PokeLobby (lote 4): ataques que suben estadisticas al usuario, algunos a cambio de PS o de poder cambiar
//  702 Novena Potencia (Extreme Evoboost): las 5 estadisticas +2
//  728 Estruendo Implacable Z (Clangorous Soulblaze): tras el golpe, las 5 estadisticas +1
//  775 Estruendo Implacable (Clangorous Soul): pierde 33 % de PS y las 5 estadisticas +1
//  868 Deslome (Fillet Away): pierde 50 % de PS, Ataque, At. Esp. y Velocidad +2
//  748 Bastion Final (No Retreat): las 5 estadisticas +1, no puede huir ni cambiar mientras este en combate

static void Boost(ServerFlow* serverFlow, u32 slot, StatStage stat, int volume) {
    HandlerParam_ChangeStatStage* statChange;
    statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, slot);
    statChange->pokeCount = 1;
    statChange->pokeID[0] = slot;
    statChange->moveAnimation = 1;
    statChange->stat = stat;
    statChange->volume = (s8)volume;
    BattleHandler_PopWork(serverFlow, statChange);
}
static void BoostAllFive(ServerFlow* serverFlow, u32 slot, int volume) {
    Boost(serverFlow, slot, STATSTAGE_ATTACK, volume);
    Boost(serverFlow, slot, STATSTAGE_DEFENSE, volume);
    Boost(serverFlow, slot, STATSTAGE_SPECIAL_ATTACK, volume);
    Boost(serverFlow, slot, STATSTAGE_SPECIAL_DEFENSE, volume);
    Boost(serverFlow, slot, STATSTAGE_SPEED, volume);
}
static void LoseHP(ServerFlow* serverFlow, u32 slot, u32 amount) {
    if (amount == 0)
        amount = 1;
    HandlerParam_Damage* damage;
    damage = (HandlerParam_Damage*)BattleHandler_PushWork(serverFlow, EFFECT_DAMAGE, slot);
    damage->pokeID = slot;
    damage->damage = (u16)amount;
    BattleHandler_PopWork(serverFlow, damage);
}

// ---------------- EXTREME EVOBOOST (702)
extern "C" void HandlerExtremeEvoboost(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BoostAllFive(serverFlow, pokemonSlot, 2);
    }
}
BattleEventHandlerTableEntry ExtremeEvoboostHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerExtremeEvoboost},
};
extern "C" BattleEventHandlerTableEntry* EventAddExtremeEvoboost(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ExtremeEvoboostHandlers);
    return ExtremeEvoboostHandlers;
}

// ---------------- CLANGOROUS SOULBLAZE (728)
// work[0] -> 1 si el golpe ha alcanzado a alguien
extern "C" void HandlerSoulblazeHit(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        work[0] = 1;
    }
}
extern "C" void HandlerSoulblazeBoost(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID) && work[0] == 1) {
        work[0] = 0;
        if (!BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, pokemonSlot)))
            BoostAllFive(serverFlow, pokemonSlot, 1);
    }
}
BattleEventHandlerTableEntry ClangorousSoulblazeHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerSoulblazeHit},
    {EVENT_MOVE_SEQUENCE_END, HandlerSoulblazeBoost},
};
extern "C" BattleEventHandlerTableEntry* EventAddClangorousSoulblaze(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ClangorousSoulblazeHandlers);
    return ClangorousSoulblazeHandlers;
}

// ---------------- CLANGOROUS SOUL (775)
extern "C" void HandlerClangorousSoulCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 maxHP = BattleMon_GetValue(currentMon, VALUE_MAX_HP);
        u32 currentHP = BattleMon_GetValue(currentMon, VALUE_CURRENT_HP);
        if (maxHP == 1 || currentHP <= maxHP * 33 / 100) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerClangorousSoul(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        LoseHP(serverFlow, pokemonSlot, BattleMon_GetValue(currentMon, VALUE_MAX_HP) * 33 / 100);
        BoostAllFive(serverFlow, pokemonSlot, 1);
    }
}
BattleEventHandlerTableEntry ClangorousSoulHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerClangorousSoulCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerClangorousSoul},
};
extern "C" BattleEventHandlerTableEntry* EventAddClangorousSoul(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ClangorousSoulHandlers);
    return ClangorousSoulHandlers;
}

// ---------------- FILLET AWAY (868)
extern "C" void HandlerFilletAwayCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 maxHP = BattleMon_GetValue(currentMon, VALUE_MAX_HP);
        u32 currentHP = BattleMon_GetValue(currentMon, VALUE_CURRENT_HP);
        if (maxHP == 1 || currentHP * 2 <= maxHP) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerFilletAway(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        LoseHP(serverFlow, pokemonSlot, BattleMon_GetValue(currentMon, VALUE_MAX_HP) / 2);
        Boost(serverFlow, pokemonSlot, STATSTAGE_ATTACK, 2);
        Boost(serverFlow, pokemonSlot, STATSTAGE_SPECIAL_ATTACK, 2);
        Boost(serverFlow, pokemonSlot, STATSTAGE_SPEED, 2);
    }
}
BattleEventHandlerTableEntry FilletAwayHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerFilletAwayCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerFilletAway},
};
extern "C" BattleEventHandlerTableEntry* EventAddFilletAway(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(FilletAwayHandlers);
    return FilletAwayHandlers;
}

// ---------------- NO RETREAT (748)
extern "C" void HandlerNoRetreatCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        // Solo se puede usar una vez por entrada en combate
        if (GetLote4State()->noRetreat[pokemonSlot]) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
extern "C" void HandlerNoRetreat(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        GetLote4State()->noRetreat[pokemonSlot] = 1;
        BoostAllFive(serverFlow, pokemonSlot, 1);

        // El usuario queda atrapado (como Mal de Ojo, pero sobre si mismo). Los fantasmas ignoran las trampas.
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        if (!BattleMon_HasType(currentMon, TYPE_GHOST) && !BattleMon_CheckIfMoveCondition(currentMon, CONDITION_BLOCK)) {
            HandlerParam_AddCondition* addCondition;
            addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, pokemonSlot);
            addCondition->condition = CONDITION_BLOCK;
            addCondition->condData = ConditionData_MakePoke(pokemonSlot);
            addCondition->almost = 0;
            addCondition->pokeID = pokemonSlot;
            BattleHandler_StrSetup(&addCondition->exStr, 2u, 872);
            BattleHandler_AddArg(&addCondition->exStr, pokemonSlot);
            BattleHandler_PopWork(serverFlow, addCondition);
        }
    }
}
BattleEventHandlerTableEntry NoRetreatHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerNoRetreatCheckFail},
    {EVENT_UNCATEGORIZED_MOVE, HandlerNoRetreat},
};
extern "C" BattleEventHandlerTableEntry* EventAddNoRetreat(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(NoRetreatHandlers);
    return NoRetreatHandlers;
}

#endif // EXPAND_MOVES
