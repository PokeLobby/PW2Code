#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/ActionOrderManager.h"
#include "Patches/BattleUpgrade/include/BattleField.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "abilities.h"
#include "server_flow.h"

// Hack PokeLobby (lote 4): ataques de estado
//  750 Polvo Magico (Magic Powder): el objetivo pasa a ser de tipo Psiquico puro
//  867 Decalcomania (Doodle): el usuario y su aliado copian la habilidad del objetivo
//  810 Gas Corrosivo (Corrosive Gas): el objeto de cada Pokemon adyacente se destruye
//  829 Cambiapoder (Power Shift): intercambia Ataque con Defensa y At. Esp. con Def. Esp.
//  913 Bramido Dragon (Dragon Cheer): sube el indice de critico del aliado (+2 si es Dragon, +1 si no)
//  689 Mandato (Instruct): el objetivo repite inmediatamente su ultimo ataque

// ---------------- MAGIC POWDER (750)
extern "C" void HandlerMagicPowder(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;
        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        if (BattleMon_IsFainted(targetMon))
            return;

        u16 monotype = PokeTypePair_MakeMonotype(TYPE_PSY);
        if (monotype == BattleMon_GetPokeType(targetMon) && BattleField_GetExtraType(targetSlot) == TYPE_NULL) {
            ServerDisplay_MoveFail(serverFlow);
            return;
        }
        // Quita el tipo extra (Halloween / Maldicion del Bosque)
        BattleField_SetExtraType(targetSlot, TYPE_NULL);

        HandlerParam_ChangeType* changeType;
        changeType = (HandlerParam_ChangeType*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_TYPE, pokemonSlot);
        changeType->pokeType = monotype;
        changeType->pokeID = (u8)targetSlot;
        BattleHandler_PopWork(serverFlow, changeType);
    }
}
BattleEventHandlerTableEntry MagicPowderHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerMagicPowder},
};
extern "C" BattleEventHandlerTableEntry* EventAddMagicPowder(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(MagicPowderHandlers);
    return MagicPowderHandlers;
}

// ---------------- DOODLE (867)
static void CopyAbility(ServerFlow* serverFlow, u32 sourceSlot, u32 receiverSlot, u32 targetSlot, u32 ability) {
    BattleMon* receiver = Handler_GetBattleMon(serverFlow, receiverSlot);
    if (!receiver || BattleMon_IsFainted(receiver))
        return;
    u32 current = BattleMon_GetValue(receiver, VALUE_EFFECTIVE_ABILITY);
    if (current == ability || AbilityFailChange(current))
        return;

    HandlerParam_ChangeAbility* changeAbility;
    changeAbility = (HandlerParam_ChangeAbility*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_ABILITY, sourceSlot);
    changeAbility->ability = (u16)ability;
    changeAbility->pokeID = (u8)receiverSlot;
    BattleHandler_StrSetup(&changeAbility->exStr, 2u, 381);
    BattleHandler_AddArg(&changeAbility->exStr, targetSlot);
    BattleHandler_AddArg(&changeAbility->exStr, ability);
    BattleHandler_PopWork(serverFlow, changeAbility);
}
extern "C" void HandlerDoodle(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;
        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        u32 ability = BattleMon_GetValue(targetMon, VALUE_EFFECTIVE_ABILITY);
        if (AbilityFailChange(ability)) {
            ServerDisplay_MoveFail(serverFlow);
            return;
        }

        CopyAbility(serverFlow, pokemonSlot, pokemonSlot, targetSlot, ability);

        // Aliado en combate (dobles)
        for (u32 slot = 0; slot < 24; ++slot) {
            if (slot == pokemonSlot || slot == targetSlot || !MainModule_IsAllyMonID(pokemonSlot, slot))
                continue;
            if (Handler_PokeIDToPokePos(serverFlow, slot) == NULL_BATTLE_POS)
                continue;
            CopyAbility(serverFlow, pokemonSlot, slot, targetSlot, ability);
        }
    }
}
BattleEventHandlerTableEntry DoodleHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerDoodle},
};
extern "C" BattleEventHandlerTableEntry* EventAddDoodle(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(DoodleHandlers);
    return DoodleHandlers;
}

// ---------------- CORROSIVE GAS (810)
extern "C" void HandlerCorrosiveGas(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        // El evento se llama una sola vez con la lista de objetivos
        u32 targetCount = BattleEventVar_GetValue(VAR_TARGET_COUNT);
        for (u32 targetIdx = 0; targetIdx < targetCount; ++targetIdx) {
            u32 targetSlot = BattleEventVar_GetValue((BattleEventVar)(VAR_TARGET_MON_ID + targetIdx));
            if (targetSlot == BATTLE_MAX_SLOTS)
                continue;
            BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
            if (BattleMon_IsFainted(targetMon))
                continue;
            // Mismas comprobaciones que Desarme: Viscosidad, objetos que no se pueden quitar...
            if (HandlerCommon_CheckIfCanStealPokeItem(serverFlow, pokemonSlot, targetSlot))
                continue;
            ITEM heldItem = BattleMon_GetHeldItem(targetMon);
            if (!heldItem)
                continue;

            HandlerParam_SetItem* setItem;
            setItem = (HandlerParam_SetItem*)BattleHandler_PushWork(serverFlow, EFFECT_SET_HELD_ITEM, pokemonSlot);
            setItem->pokeID = (u8)targetSlot;
            setItem->itemID = 0;
            BattleHandler_StrSetup(&setItem->exStr, 2u, BATTLE_CORROSIVE_GAS_MSGID);
            BattleHandler_AddArg(&setItem->exStr, targetSlot);
            BattleHandler_AddArg(&setItem->exStr, heldItem);
            BattleHandler_PopWork(serverFlow, setItem);
        }
    }
}
BattleEventHandlerTableEntry CorrosiveGasHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerCorrosiveGas},
};
extern "C" BattleEventHandlerTableEntry* EventAddCorrosiveGas(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(CorrosiveGasHandlers);
    return CorrosiveGasHandlers;
}

// ---------------- POWER SHIFT (829)
extern "C" void HandlerPowerShift(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* mon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 attack = BattleMon_GetRealStat(mon, VALUE_ATTACK_STAT);
        u32 defense = BattleMon_GetRealStat(mon, VALUE_DEFENSE_STAT);
        u32 spAttack = BattleMon_GetRealStat(mon, VALUE_SPECIAL_ATTACK_STAT);
        u32 spDefense = BattleMon_GetRealStat(mon, VALUE_SPECIAL_DEFENSE_STAT);

        HandlerParam_SetStat* setStat;
        setStat = (HandlerParam_SetStat*)BattleHandler_PushWork(serverFlow, EFFECT_SET_STAT, pokemonSlot);
        setStat->battleSlot = (u8)pokemonSlot;
        setStat->attack = (u16)defense;
        setStat->defense = (u16)attack;
        setStat->spAttack = (u16)spDefense;
        setStat->spDefense = (u16)spAttack;
        setStat->flags = 0x0F;
        BattleHandler_StrSetup(&setStat->exStr, 2u, BATTLE_POWER_SHIFT_MSGID);
        BattleHandler_AddArg(&setStat->exStr, pokemonSlot);
        BattleHandler_PopWork(serverFlow, setStat);
    }
}
BattleEventHandlerTableEntry PowerShiftHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerPowerShift},
};
extern "C" BattleEventHandlerTableEntry* EventAddPowerShift(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PowerShiftHandlers);
    return PowerShiftHandlers;
}

// ---------------- DRAGON CHEER (913)
// POS effect: work[0] -> slot del Pokemon animado, work[1] -> bonus de critico (1 o 2)
extern "C" void HandlerPosDragonCheerCrit(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_ATTACKING_MON) == work[0]) {
        BattleEventVar_RewriteValue(VAR_CRIT_STAGE, BattleEventVar_GetValue(VAR_CRIT_STAGE) + work[1]);
    }
}
extern "C" void HandlerPosDragonCheerSwitchOut(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    if (BattleEventVar_GetValue(VAR_MON_ID) == work[0]) {
        BattleEventItem_Remove(item);
    }
}
extern "C" void HandlerPosDragonCheerFainted(BattleEventItem* item, ServerFlow* serverFlow, u32 targetPos, u32* work) {
    u32 slot = BattleEventVar_GetValue(VAR_MON_ID);
    if (slot == work[0] && BattleMon_IsFainted(Handler_GetBattleMon(serverFlow, slot))) {
        BattleEventItem_Remove(item);
    }
}
BattleEventHandlerTableEntry PosDragonCheerHandlers[]{
    {EVENT_CRITICAL_CHECK, HandlerPosDragonCheerCrit},
    {EVENT_SWITCH_OUT_END, HandlerPosDragonCheerSwitchOut},
    {EVENT_NOTIFY_FAINTED, HandlerPosDragonCheerFainted},
};
extern "C" BattleEventHandlerTableEntry* EventAddPosDragonCheer(u32* handlerAmount) {
    *handlerAmount = PosEffect_SetPriority(EVENTPRI_ABILITY_STALL, ARRAY_COUNT(PosDragonCheerHandlers));
    return PosDragonCheerHandlers;
}
extern "C" void HandlerDragonCheer(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;
        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        if (BattleMon_IsFainted(targetMon))
            return;

        HandlerParam_AddPosEffect* addEffectPos;
        addEffectPos = (HandlerParam_AddPosEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_POS_EFFECT, pokemonSlot);
        addEffectPos->posEffect = POSEFF_DRAGON_CHEER;
        addEffectPos->targetPos = Handler_PokeIDToPokePos(serverFlow, targetSlot);
        addEffectPos->workToCopy[0] = targetSlot;
        addEffectPos->workToCopy[1] = BattleMon_HasType(targetMon, TYPE_DRAGON) ? 2 : 1;
        addEffectPos->workCount = 2;
        BattleHandler_PopWork(serverFlow, addEffectPos);

        // Mismo mensaje que Foco Energia
        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, 1041);
        BattleHandler_AddArg(&message->str, targetSlot);
        BattleHandler_PopWork(serverFlow, message);
    }
}
BattleEventHandlerTableEntry DragonCheerHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerDragonCheer},
};
extern "C" BattleEventHandlerTableEntry* EventAddDragonCheer(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(DragonCheerHandlers);
    return DragonCheerHandlers;
}

// ---------------- INSTRUCT (689)
static b32 CannotBeInstructed(MOVE_ID moveID) {
    switch (moveID) {
    case MOVE_FOCUS_PUNCH: case MOVE_BIDE: case MOVE_SKY_DROP: case MOVE_MIMIC: case MOVE_MIRROR_MOVE:
    case MOVE_METRONOME: case MOVE_COPYCAT: case MOVE_ME_FIRST: case MOVE_ASSIST: case MOVE_SKETCH:
    case MOVE_TRANSFORM: case MOVE_STRUGGLE: case MOVE_SLEEP_TALK: case MOVE_UPROAR: case MOVE_ROLLOUT:
    case MOVE_ICE_BALL: case MOVE_OUTRAGE: case MOVE_THRASH: case MOVE_PETAL_DANCE: case MOVE_FLY:
    case MOVE_DIG: case MOVE_DIVE: case MOVE_BOUNCE: case MOVE_SHADOW_FORCE: case MOVE_SOLAR_BEAM:
    case MOVE_SKULL_BASH: case MOVE_RAZOR_WIND: case MOVE_SKY_ATTACK: case MOVE_FREEZE_SHOCK:
    case MOVE_ICE_BURN: case MOVE_NATURE_POWER: case MOVE_FUTURE_SIGHT: case MOVE_DOOM_DESIRE:
    case MOVE_INSTRUCT: case MOVE_BEAK_BLAST: case MOVE_SHELL_TRAP: case MOVE_SOLAR_BLADE: case MOVE_PHANTOM_FORCE:
        return 1;
    default:
        return 0;
    }
}
static MOVE_ID InstructMove(BattleMon* targetMon) {
    MOVE_ID moveID = BattleMon_GetPreviousMove(targetMon);
    if (!moveID || CannotBeInstructed(moveID))
        return (MOVE_ID)0;
    // Debe seguir en su lista de ataques y con PP
    u32 moveIdx = Move_SearchIndex(targetMon, moveID);
    if (moveIdx >= 4 || Move_GetPP(targetMon, moveIdx) == 0)
        return (MOVE_ID)0;
    return moveID;
}
extern "C" void HandlerInstruct(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot = BattleEventVar_GetValue(VAR_TARGET_MON_ID);
        if (targetSlot == BATTLE_MAX_SLOTS)
            return;
        BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
        MOVE_ID moveID = InstructMove(targetMon);
        if (!moveID || BattleMon_IsFainted(targetMon) || CheckExtraActionFlag()) {
            ServerDisplay_MoveFail(serverFlow);
            return;
        }

        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_INSTRUCT_MSGID);
        BattleHandler_AddArg(&message->str, targetSlot);
        BattleHandler_PopWork(serverFlow, message);

        // La accion extra se ejecuta justo despues de esta (igual que las de Bailarin)
        ShiftExtraActionOrders();
        ActionOrderWork* extraActionOrder = GetExtraActionOrder(0);
        extraActionOrder->battleMon = targetMon;
        for (u8 orderIdx = 0; orderIdx < 6; ++orderIdx) {
            if (serverFlow->actionOrderWork[orderIdx].battleMon == targetMon) {
                extraActionOrder->speed = serverFlow->actionOrderWork[orderIdx].speed;
                extraActionOrder->partyID = serverFlow->actionOrderWork[orderIdx].partyID;
                break;
            }
        }
        extraActionOrder->field_E = 0;
        extraActionOrder->field_F = 0;

        BattleAction_Fight* fight = (BattleAction_Fight*)&(extraActionOrder->action);
        fight->cmd = 1;
        fight->moveID = moveID;
        fight->pad = 0;
        MoveTarget moveTarget = (MoveTarget)PML_MoveGetParam(moveID, MVDATA_TARGET);
        switch (moveTarget) {
        case TARGET_OTHER_SELECT:
        case TARGET_ENEMY_SELECT:
        case TARGET_ENEMY_RANDOM:
        case TARGET_ALLY_USER_SELECT:
        case TARGET_ALLY_SELECT:
            fight->targetPos = Handler_ReqMoveTargetAuto(serverFlow, targetSlot, moveID);
            break;
        default:
            fight->targetPos = 6;
        }
    }
}
BattleEventHandlerTableEntry InstructHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerInstruct},
};
extern "C" BattleEventHandlerTableEntry* EventAddInstruct(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(InstructHandlers);
    return InstructHandlers;
}

#endif // EXPAND_MOVES
