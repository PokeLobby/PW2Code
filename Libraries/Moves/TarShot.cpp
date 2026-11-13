#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "server_flow.h"

// POS TAR SHOT
// work[0] -> battle slot of the Pokemon covered in tar
// Fire-type moves deal double damage to it until it leaves the field
extern "C" void HandlerPosTarShotPower(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_DEFENDING_MON) == work[0] &&
        BattleEventVar_GetValue(VAR_MOVE_TYPE) == TYPE_FIRE) {
        BattleEventVar_MulValue(VAR_RATIO, 8192);
    }
}
extern "C" void HandlerPosTarShotSwitchOut(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == work[0]) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosTarShotFainted(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == work[0] && BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, slot))) {
        BattleEventItem_Remove(item);
    }
}
// Self cleaning (issue #93, same as Salt Cure #92): the item must not outlive the tarred Pokemon, a stale one
// blocks every new Tar Shot of the battle. Called once with VAR_MON_ID = 31 and then once per Pokemon on the field.
extern "C" void HandlerPosTarShotTurnCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (Handler_PokePosToPokeID(serverFlow, targetPos) != work[0] ||
        BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, work[0]))) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosTarShotHandlers[]{
    {EVENT_TURN_CHECK_END, HandlerPosTarShotTurnCheck},
    {EVENT_ATTACKER_POWER, HandlerPosTarShotPower},
    {EVENT_SWITCH_OUT_END, HandlerPosTarShotSwitchOut},
    {EVENT_NOTIFY_FAINTED, HandlerPosTarShotFainted},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosTarShot(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosTarShotHandlers));
    return PosTarShotHandlers;
}

// TAR SHOT
extern "C" void HandlerTarShot(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;

        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        if (BattleMon_IsFainted(targetMon) || BattleMon_IsSubstituteActive(targetMon))
            return;

        HandlerParam_ChangeStatStage* statChange;
        statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
        statChange->pokeCount = 1;
        statChange->pokeID[0] = targetSlot;
        statChange->moveAnimation = 1;
        statChange->stat = STATSTAGE_SPEED;
        statChange->volume = -1;
        BattleHandler_PopWork(serverFlow, statChange);

        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_TAR_SHOT;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, targetSlot);
        addEffectPos->workToCopy[0] = targetSlot;
        addEffectPos->workCount = 1;
        BattleHandler_PopWork(serverFlow, addEffectPos);

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_TAR_SHOT_START_MSGID);
        BattleHandler_AddArg(&message->str, targetSlot);
        BattleHandler_PopWork(serverFlow, message);
    }
}
BattleEventHandlerTableEntry TarShotHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerTarShot},
};
extern "C" BattleEventHandlerTableEntry* EventAddTarShot(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TarShotHandlers);
    return TarShotHandlers;
}

#endif // EXPAND_MOVES
