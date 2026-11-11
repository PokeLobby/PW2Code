#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"
#include "Patches/BattleUpgrade/include/FieldEffectExpansion.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "abilities.h"
#include "server_events.h"
#include "server_flow.h"

// Hack PokeLobby (lote 4b): ataques con reglas propias que la auditoria #53 daba por buenos solo con sus datos
//  813 Triple Axel (Triple Axel)          3 golpes con potencia 20 / 40 / 60 y un chequeo de punteria por golpe
//  860 Bomba Poblacion (Population Bomb)  10 golpes con un chequeo de punteria por golpe
//  817 Golpe Oscuro / 818 Azote Torrencial / 870 Truco Floral: siempre critico
//  830 Hachazo Petreo (Stone Axe)         pone Trampa Rocas tras el golpe
//  862 Asalto Espadon (Glaive Rush)       el usuario recibe doble dano hasta que vuelva a actuar
//  875 Psicohojas (Psyblade)              +50 % con campo electrico
//  878 Nitrochoque / 879 Electroderrape   +33 % si es supereficaz
//  893 Martillo Colosal / 901 Luna Roja   no se pueden usar dos veces seguidas
//  905 Electrorrayo (Electro Shot)        At. Esp. +1 al cargar; no carga con lluvia
//  907 Laser Veleidoso (Fickle Beam)      30 % de doble potencia
//  918 Palma Rauda (Upper Hand)           solo funciona si el objetivo va a usar un ataque con prioridad
//  788 Fuerza G (Grav Apple)              +50 % con Gravedad
//  801 Moluscanon (Shell Side Arm)     fisico si el Ataque supera al At. Esp. (aproximado)
//  892 Electropalmas (Double Shock) esta en RemoveType.cpp
//  92  Toxico (Toxic)                     no falla si lo usa un tipo Veneno (Gen 6+, issue-82)
//  282 Desarme (Knock Off)                x1,5 de potencia si el objetivo lleva un objeto que se le puede quitar (Gen 6+, issue-82)

// ---------------- utilidades
static b32 TargetHasActed(BattleMon* targetMon) {
    return BattleMon_GetTurnFlag(targetMon, TURNFLAG_ACTIONDONE) || BattleMon_GetTurnFlag(targetMon, TURNFLAG_MOVED) ||
        BattleMon_GetTurnFlag(targetMon, TURNFLAG_MOVEPROCDONE);
}
static void MulPower(u32 numerator, u32 denominator) {
    BattleEventVar_RewriteValue(VAR_MOVE_POWER, BattleEventVar_GetValue(VAR_MOVE_POWER) * numerator / denominator);
}

// ---------------- TRIPLE AXEL (813) / POPULATION BOMB (860)
// Se reutiliza el manejador de Triple Patada del juego que hace el chequeo de punteria en cada golpe (ESDB.yml)
extern "C" void HandlerTripleKickHitCount(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
extern "C" void HandlerTripleAxelPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        ++work[0];
        BattleEventVar_RewriteValue(VAR_MOVE_POWER, 20 * work[0]);
    }
}
BattleEventHandlerTableEntry TripleAxelHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerTripleAxelPower},
    {EVENT_MOVE_HIT_COUNT, HandlerTripleKickHitCount},
};
extern "C" BattleEventHandlerTableEntry* EventAddTripleAxel(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TripleAxelHandlers);
    return TripleAxelHandlers;
}
// Bomba Poblacion: solo el chequeo de punteria por golpe, sin la potencia creciente
BattleEventHandlerTableEntry PopulationBombHandlers[]{
    {EVENT_MOVE_HIT_COUNT, HandlerTripleKickHitCount},
};
extern "C" BattleEventHandlerTableEntry* EventAddPopulationBomb(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PopulationBombHandlers);
    return PopulationBombHandlers;
}

// ---------------- ALWAYS CRIT (817, 818, 870)
extern "C" void HandlerAlwaysCrit(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleEventVar_RewriteValue(VAR_CRIT_STAGE, 4);
    }
}
BattleEventHandlerTableEntry AlwaysCritHandlers[]{
    {EVENT_CRITICAL_CHECK, HandlerAlwaysCrit},
};
extern "C" BattleEventHandlerTableEntry* EventAddAlwaysCrit(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(AlwaysCritHandlers);
    return AlwaysCritHandlers;
}

// ---------------- STONE AXE (830)
extern "C" void HandlerStoneAxe(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        ConditionData condData = Condition_MakePermanent();
        u8 opposingSide = GetSideFromOpposingMonID(pokemonSlot);

        HandlerParam_AddSideEffect* addSideEffect;
        addSideEffect = (HandlerParam_AddSideEffect*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_SIDE_EFFECT, pokemonSlot);
        addSideEffect->header.flags |= SIDE_HANDLER_MOVE_POPUP_FLAG;
        addSideEffect->sideEffect = SIDEEFF_STEALTH_ROCK;
        addSideEffect->side = opposingSide;
        addSideEffect->condData = condData;
        BattleHandler_StrSetup(&addSideEffect->exStr, 1u, 156u);
        BattleHandler_AddArg(&addSideEffect->exStr, opposingSide);
        BattleHandler_PopWork(serverFlow, addSideEffect);
    }
}
BattleEventHandlerTableEntry StoneAxeHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerStoneAxe},
};
extern "C" BattleEventHandlerTableEntry* EventAddStoneAxe(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(StoneAxeHandlers);
    return StoneAxeHandlers;
}

// ---------------- GLAIVE RUSH (862)
// El motor (ServerEvent_CalcDamage) duplica el dano que recibe un Pokemon con glaiveRush; se apaga cuando vuelve a actuar.
extern "C" void HandlerGlaiveRush(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        GetLote4State()->glaiveRush[pokemonSlot] = 1;
    }
}
BattleEventHandlerTableEntry GlaiveRushHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerGlaiveRush},
};
extern "C" BattleEventHandlerTableEntry* EventAddGlaiveRush(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(GlaiveRushHandlers);
    return GlaiveRushHandlers;
}

// ---------------- PSYBLADE (875)
extern "C" void HandlerPsybladePower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) && ServerEvent_GetTerrain(serverFlow) == TERRAIN_ELECTRIC) {
        MulPower(3, 2);
    }
}
BattleEventHandlerTableEntry PsybladeHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerPsybladePower},
};
extern "C" BattleEventHandlerTableEntry* EventAddPsyblade(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PsybladeHandlers);
    return PsybladeHandlers;
}

// ---------------- COLLISION COURSE (878) / ELECTRO DRIFT (879)
extern "C" void HandlerSuperEffectiveBoost(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 effectiveness = BattleEventVar_GetValue(VAR_TYPE_EFFECTIVENESS);
        if (effectiveness > EFFECTIVENESS_1) {
            BattleEventVar_MulValue(VAR_RATIO, 5461);
        }
    }
}
BattleEventHandlerTableEntry SuperEffectiveBoostHandlers[]{
    {EVENT_MOVE_DAMAGE_PROCESSING_2, HandlerSuperEffectiveBoost},
};
extern "C" BattleEventHandlerTableEntry* EventAddSuperEffectiveBoost(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(SuperEffectiveBoostHandlers);
    return SuperEffectiveBoostHandlers;
}

// ---------------- GIGATON HAMMER (893) / BLOOD MOON (901): no dos veces seguidas
extern "C" void HandlerNotTwiceCheckFail(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        if (BattleMon_GetPreviousMove(currentMon) == (MOVE_ID)BattleEventVar_GetValue(VAR_MOVE_ID)) {
            BattleEventVar_RewriteValue(VAR_FAIL_CAUSE, MOVEFAIL_OTHER);
        }
    }
}
BattleEventHandlerTableEntry NotTwiceHandlers[]{
    {EVENT_MOVE_EXECUTE_CHECK2, HandlerNotTwiceCheckFail},
};
extern "C" BattleEventHandlerTableEntry* EventAddNotTwice(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(NotTwiceHandlers);
    return NotTwiceHandlers;
}

// ---------------- ELECTRO SHOT (905): la carga viene de la marca de MoveData
extern "C" void HandlerElectroShotSkip(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (Handler_CheckWeather(serverFlow, pokemonSlot, BATTLE_MAX_SLOTS, ServerEvent_GetWeather(serverFlow)) == WEATHER_RAIN) {
            BattleEventVar_RewriteValue(VAR_GENERAL_USE_FLAG, 1);
        }
    }
}
extern "C" void HandlerElectroShotBoost(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerParam_ChangeStatStage* statChange;
        statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
        statChange->pokeCount = 1;
        statChange->pokeID[0] = (u8)pokemonSlot;
        statChange->moveAnimation = 1;
        statChange->stat = STATSTAGE_SPECIAL_ATTACK;
        statChange->volume = 1;
        BattleHandler_PopWork(serverFlow, statChange);
    }
}
BattleEventHandlerTableEntry ElectroShotHandlers[]{
    {EVENT_CHECK_CHARGE_UP_SKIP, HandlerElectroShotSkip},
    {EVENT_CHARGE_UP_START_DONE, HandlerElectroShotBoost},
};
extern "C" BattleEventHandlerTableEntry* EventAddElectroShot(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ElectroShotHandlers);
    return ElectroShotHandlers;
}

// ---------------- FICKLE BEAM (907)
extern "C" void HandlerFickleBeamPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) && BattleRandom(100u) < 30) {
        MulPower(2, 1);
    }
}
BattleEventHandlerTableEntry FickleBeamHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerFickleBeamPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddFickleBeam(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(FickleBeamHandlers);
    return FickleBeamHandlers;
}

// ---------------- UPPER HAND (918): solo si el objetivo va a usar un ataque con prioridad
extern "C" void HandlerUpperHandCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        if (defendingSlot >= BATTLE_MAX_SLOTS)
            return;
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, defendingSlot);
        b32 works = 0;
        if (defendingMon && !TargetHasActed(defendingMon)) {
            for (u8 i = 0; i < 6; ++i) {
                if (serverFlow->actionOrderWork[i].battleMon == defendingMon &&
                    BattleAction_GetAction(&serverFlow->actionOrderWork[i].action) == 1) {
                    MOVE_ID moveID = (MOVE_ID)((BattleAction_Fight*)&serverFlow->actionOrderWork[i].action)->moveID;
                    if (PML_MoveIsDamaging(moveID) && (s8)PML_MoveGetParam(moveID, MVDATA_PRIORITY) > 0)
                        works = 1;
                    break;
                }
            }
        }
        if (!works) {
            BattleEventVar_RewriteValue(VAR_NO_EFFECT_FLAG, 1);
        }
    }
}
BattleEventHandlerTableEntry UpperHandHandlers[]{
    {EVENT_ABILITY_CHECK_NO_EFFECT, HandlerUpperHandCheck},
};
extern "C" BattleEventHandlerTableEntry* EventAddUpperHand(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(UpperHandHandlers);
    return UpperHandHandlers;
}

// ---------------- GRAV APPLE (788)
extern "C" void HandlerGravApplePower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) && BattleField_CheckEffect(FLDEFF_GRAVITY)) {
        MulPower(3, 2);
    }
}
BattleEventHandlerTableEntry GravAppleHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerGravApplePower},
};
extern "C" BattleEventHandlerTableEntry* EventAddGravApple(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(GravAppleHandlers);
    return GravAppleHandlers;
}

// ---------------- TAKE HEART (850): At. Esp. y Def. Esp. +1 y cura el estado del usuario
extern "C" void HandlerTakeHeart(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        static const StatStage stats[2] = { STATSTAGE_SPECIAL_ATTACK, STATSTAGE_SPECIAL_DEFENSE };
        for (u32 i = 0; i < 2; ++i) {
            HandlerParam_ChangeStatStage* statChange;
            statChange = (HandlerParam_ChangeStatStage*)BattleHandler_PushWork(serverFlow, EFFECT_CHANGE_STAT_STAGE, pokemonSlot);
            statChange->pokeCount = 1;
            statChange->pokeID[0] = (u8)pokemonSlot;
            statChange->moveAnimation = 1;
            statChange->stat = stats[i];
            statChange->volume = 1;
            BattleHandler_PopWork(serverFlow, statChange);
        }

        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        CONDITION status = BattleMon_GetStatus(currentMon);
        if (status != CONDITION_NONE) {
            HandlerParam_CureCondition* cureCondition;
            cureCondition = (HandlerParam_CureCondition*)BattleHandler_PushWork(serverFlow, EFFECT_CURE_STATUS, pokemonSlot);
            cureCondition->condition = status;
            cureCondition->pokeID[0] = (u8)pokemonSlot;
            cureCondition->pokeCount = 1;
            BattleHandler_PopWork(serverFlow, cureCondition);
        }
    }
}
BattleEventHandlerTableEntry TakeHeartHandlers[]{
    {EVENT_UNCATEGORIZED_MOVE, HandlerTakeHeart},
};
extern "C" BattleEventHandlerTableEntry* EventAddTakeHeart(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TakeHeartHandlers);
    return TakeHeartHandlers;
}

// ---------------- SHELL SIDE ARM (801): aproximado, fisico si el Ataque del usuario supera a su At. Esp.
extern "C" void HandlerShellSideArmCategory(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 attack = BattleMon_GetRealStat(currentMon, VALUE_ATTACK_STAT);
        u32 specialAttack = BattleMon_GetRealStat(currentMon, VALUE_SPECIAL_ATTACK_STAT);
        if (attack > specialAttack) {
            BattleEventVar_RewriteValue(VAR_MOVE_CATEGORY, CATEGORY_PHYSICAL);
        }
    }
}
BattleEventHandlerTableEntry ShellSideArmHandlers[]{
    {EVENT_MOVE_PARAM, HandlerShellSideArmCategory},
};
extern "C" BattleEventHandlerTableEntry* EventAddShellSideArm(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ShellSideArmHandlers);
    return ShellSideArmHandlers;
}

// ---------------- TOXIC (92): un usuario de tipo Veneno nunca falla
extern "C" void HandlerToxicNeverMiss(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) &&
        BattleMon_HasType(Handler_GetBattleMon(serverFlow, pokemonSlot), TYPE_POIS)) {
        BattleEventVar_RewriteValue(VAR_GENERAL_USE_FLAG, 1);
    }
}
BattleEventHandlerTableEntry ToxicHandlers[]{
    {EVENT_BYPASS_ACCURACY_CHECK, HandlerToxicNeverMiss},
};
extern "C" BattleEventHandlerTableEntry* EventAddToxic(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ToxicHandlers);
    return ToxicHandlers;
}

// ---------------- KNOCK OFF (282): la entrada de abajo es la de Desarme del juego (HandlerKnockOff, ESDB.yml) + x1,5 de potencia
extern "C" void HandlerKnockOff(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
extern "C" void HandlerKnockOffPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, defendingSlot);
        if (defendingMon && BattleMon_GetHeldItem(defendingMon) != ITEM_NULL &&
            !HandlerCommon_CheckIfCanStealPokeItem(serverFlow, pokemonSlot, defendingSlot)) {
            MulPower(3, 2);
        }
    }
}
BattleEventHandlerTableEntry KnockOffHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerKnockOff},
    {EVENT_MOVE_BASE_POWER, HandlerKnockOffPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddKnockOffUpdated(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(KnockOffHandlers);
    return KnockOffHandlers;
}

#endif // EXPAND_MOVES
