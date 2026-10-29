#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/Contact.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// Hack PokeLobby (lote 4): protecciones nuevas que castigan el contacto.
// Obstruccion (792): Defensa -2 al atacante. Telatrampa (852): Velocidad -1. Llama Protectora (908): quema.
// Igual que Escudo Real: solo bloquean ataques de dano (los de estado pasan).

// Comun: el golpe de contacto bloqueado
static b32 ContactBlocked(ServerFlow* serverFlow, u32 targetPos, u32* attackingSlot, u32* defendingSlot) {
    if (IS_NOT_NEW_EVENT)
        return 0;

    *defendingSlot = Handler_PokePosToPokeID(serverFlow, targetPos);
    MOVE_ID moveID = BattleEventVar_GetValue(VAR_MOVE_ID);
    if (*defendingSlot == BattleEventVar_GetValue(NEW_VAR_DEFENDING_MON) &&
            getMoveFlag(moveID, FLAG_CONTACT)) {
        *attackingSlot = BattleEventVar_GetValue(NEW_VAR_ATTACKING_MON);
        return MakesContact(serverFlow, moveID, *attackingSlot, *defendingSlot);
    }
    return 0;
}
// Comun: los de estado atraviesan la proteccion
static void LetStatusThrough(ServerFlow* serverFlow, u32 targetPos) {
    u32 pokemonSlot = Handler_PokePosToPokeID(serverFlow, targetPos);
    if (pokemonSlot == BattleEventVar_GetValue(VAR_DEFENDING_MON) &&
            BattleEventVar_GetValue(VAR_MOVE_CATEGORY) == CATEGORY_STATUS) {
        BattleEventVar_RewriteValue(VAR_GENERAL_USE_FLAG, 2);
    }
}
static void LowerStat(ServerFlow* serverFlow, u32 sourceSlot, u32 targetSlot, StatStage stat, int volume) {
    HandlerParam_ChangeStatStage* statChange;
    statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, sourceSlot);
    statChange->pokeCount = 1;
    statChange->pokeID[0] = targetSlot;
    statChange->stat = stat;
    statChange->volume = (s8)volume;
    BattleHandler_PopWork(serverFlow, statChange);
}
static void AddProtectPos(ServerFlow* serverFlow, u32 pokemonSlot, u32 posEffect) {
    HandlerParam_AddPosEffect* addEffectPos;
    addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
    addEffectPos->posEffect = posEffect;
    addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, pokemonSlot);
    BattleHandler_PopWork(serverFlow, addEffectPos);
}

// ---------------- OBSTRUCT (792)
extern "C" void HandlerPosObstruct(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 attackingSlot, defendingSlot;
    if (ContactBlocked(serverFlow, targetPos, &attackingSlot, &defendingSlot))
        LowerStat(serverFlow, defendingSlot, attackingSlot, STATSTAGE_DEFENSE, -2);
}
extern "C" void HandlerPosObstructStatus(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    LetStatusThrough(serverFlow, targetPos);
}
BattleEventHandlerTableEntry PosObstructHandlers[]{
    {EVENT_PROTECT_SUCCESS, HandlerPosObstruct},
    {EVENT_CHECK_PROTECT_BREAK, HandlerPosObstructStatus},
    {EVENT_TURN_CHECK_DONE, HandlerPosTurnCheckDone},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosObstruct(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PosObstructHandlers);
    return PosObstructHandlers;
}
extern "C" void HandlerObstruct(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        AddProtectPos(serverFlow, pokemonSlot, POSEFF_OBSTRUCT);
        HandlerProtectStart(item, serverFlow, pokemonSlot, work);
    }
}
BattleEventHandlerTableEntry ObstructHandlers[]{
    {EVENT_MOVE_SEQUENCE_START, HandlerObstruct},
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerProtectCheckFail},
    {EVENT_MOVE_EXECUTE_FAIL, HandlerProtectResetCounter},
    {EVENT_UNCATEGORIZED_MOVE, HandlerProtect},
};
extern "C" BattleEventHandlerTableEntry* EventAddObstruct(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ObstructHandlers);
    return ObstructHandlers;
}

// ---------------- SILK TRAP (852)
extern "C" void HandlerPosSilkTrap(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 attackingSlot, defendingSlot;
    if (ContactBlocked(serverFlow, targetPos, &attackingSlot, &defendingSlot))
        LowerStat(serverFlow, defendingSlot, attackingSlot, STATSTAGE_SPEED, -1);
}
extern "C" void HandlerPosSilkTrapStatus(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    LetStatusThrough(serverFlow, targetPos);
}
BattleEventHandlerTableEntry PosSilkTrapHandlers[]{
    {EVENT_PROTECT_SUCCESS, HandlerPosSilkTrap},
    {EVENT_CHECK_PROTECT_BREAK, HandlerPosSilkTrapStatus},
    {EVENT_TURN_CHECK_DONE, HandlerPosTurnCheckDone},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosSilkTrap(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PosSilkTrapHandlers);
    return PosSilkTrapHandlers;
}
extern "C" void HandlerSilkTrap(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        AddProtectPos(serverFlow, pokemonSlot, POSEFF_SILK_TRAP);
        HandlerProtectStart(item, serverFlow, pokemonSlot, work);
    }
}
BattleEventHandlerTableEntry SilkTrapHandlers[]{
    {EVENT_MOVE_SEQUENCE_START, HandlerSilkTrap},
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerProtectCheckFail},
    {EVENT_MOVE_EXECUTE_FAIL, HandlerProtectResetCounter},
    {EVENT_UNCATEGORIZED_MOVE, HandlerProtect},
};
extern "C" BattleEventHandlerTableEntry* EventAddSilkTrap(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SilkTrapHandlers);
    return SilkTrapHandlers;
}

// ---------------- BURNING BULWARK (908)
extern "C" void HandlerPosBurningBulwark(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 attackingSlot, defendingSlot;
    if (ContactBlocked(serverFlow, targetPos, &attackingSlot, &defendingSlot)) {
        HandlerParam_AddCondition* addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, defendingSlot);
        addCondition->pokeID = attackingSlot;
        addCondition->condition = CONDITION_BURN;
        addCondition->condData = MakeBasicStatus(CONDITION_BURN);
        BattleHandler_PopWork(serverFlow, addCondition);
    }
}
extern "C" void HandlerPosBurningBulwarkStatus(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    LetStatusThrough(serverFlow, targetPos);
}
BattleEventHandlerTableEntry PosBurningBulwarkHandlers[]{
    {EVENT_PROTECT_SUCCESS, HandlerPosBurningBulwark},
    {EVENT_CHECK_PROTECT_BREAK, HandlerPosBurningBulwarkStatus},
    {EVENT_TURN_CHECK_DONE, HandlerPosTurnCheckDone},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosBurningBulwark(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PosBurningBulwarkHandlers);
    return PosBurningBulwarkHandlers;
}
extern "C" void HandlerBurningBulwark(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        AddProtectPos(serverFlow, pokemonSlot, POSEFF_BURNING_BULWARK);
        HandlerProtectStart(item, serverFlow, pokemonSlot, work);
    }
}
BattleEventHandlerTableEntry BurningBulwarkHandlers[]{
    {EVENT_MOVE_SEQUENCE_START, HandlerBurningBulwark},
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerProtectCheckFail},
    {EVENT_MOVE_EXECUTE_FAIL, HandlerProtectResetCounter},
    {EVENT_UNCATEGORIZED_MOVE, HandlerProtect},
};
extern "C" BattleEventHandlerTableEntry* EventAddBurningBulwark(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(BurningBulwarkHandlers);
    return BurningBulwarkHandlers;
}

#endif // EXPAND_MOVES
