#include "settings.h"
#ifdef EXPAND_MOVES

#include "Patches/BattleUpgrade/BattleEngine.h"
#include "Patches/BattleUpgrade/include/BattleField.h"
#include "Patches/BattleUpgrade/include/FieldEffectExpansion.h"
#include "Patches/BattleUpgrade/include/MoveExpansion.h"

#include "abilities.h"
#include "server_flow.h"

// Hack PokeLobby (lote 4): ataques de dano con reglas propias
//  827 Garra Nociva (Dire Claw)               50 % de veneno / paralisis / sueno
//  807 Envidia Ardiente (Burning Jealousy)    quema al objetivo si le han subido una estadistica este turno
//  914 Canto Encantador (Alluring Voice)      confunde al objetivo si le han subido una estadistica este turno
//  687 Nucleo Castigo (Core Enforcer)         anula la habilidad del objetivo si ya ha actuado
//  698 Colera del Guardian (Guardian of Alola) 75 % de los PS actuales del objetivo
//  703 Supernova Original (Genesis Supernova) campo psiquico tras el golpe
//  754 Electropico / 755 Branquibocado        doble de potencia si el usuario actua antes que el objetivo
//  800 Rayo Meteorico (Meteor Beam)           Ataque Especial +1 el turno de carga
//  802 Bruma Explosiva (Misty Explosion)      Explosion con +50 % de potencia en el campo de niebla
//  797 Vasta Fuerza (Expanding Force)         +50 % de potencia en el campo psiquico
//  804 Alto Voltaje (Rising Voltage)          doble de potencia si el objetivo esta en el campo electrico
//  805 Pulso de Campo (Terrain Pulse)         con campo: potencia x2 y tipo del campo
//  808 Desahogo (Lash Out)                    doble de potencia si al usuario le han bajado una estadistica este turno
//  809 Poltergeist                            falla si el objetivo no lleva objeto
//  826 Conjuro Funesto (Eerie Spell)          -3 PP al ultimo ataque del objetivo
//  851 Teraexplosion (Tera Blast)             fisico si el Ataque del usuario supera su At. Esp.
//  854 Homenaje Postumo (Last Respects)       +50 de potencia por cada companero debilitado
//  889 Punyo Furia (Rage Fist)                +50 de potencia por cada golpe recibido (max. 350)
//  912 Prensa Metalica (Hard Press)           potencia segun los PS restantes del objetivo
// Cuerpo a Cuerpo con Defensa (776 Plancha Corporal) esta en BattleEngine.cpp (ServerEvent_CalcDamage).

// ---- utilidades
static b32 EachTarget(u32 idx, u32* targetSlot) {
    u32 targetCount = BattleEventVar_GetValue(VAR_TARGET_COUNT);
    if (idx >= targetCount)
        return 0;
    *targetSlot = BattleEventVar_GetValue((BattleEventVar)(VAR_TARGET_MON_ID + idx));
    return 1;
}
static BattleMon* LiveTarget(ServerFlow* serverFlow, u32 targetSlot) {
    if (targetSlot == BATTLE_MAX_SLOTS)
        return nullptr;
    BattleMon* mon = Handler_GetBattleMon(serverFlow, targetSlot);
    if (!mon || BattleMon_IsFainted(mon) || BattleMon_IsSubstituteActive(mon))
        return nullptr;
    return mon;
}
static void AddStatus(ServerFlow* serverFlow, u32 sourceSlot, u32 targetSlot, CONDITION condition, ConditionData condData) {
    HandlerParam_AddCondition* addCondition;
    addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, sourceSlot);
    addCondition->condition = condition;
    addCondition->condData = condData;
    addCondition->almost = 0;
    addCondition->pokeID = (u8)targetSlot;
    BattleHandler_PopWork(serverFlow, addCondition);
}
static b32 TargetHasActed(BattleMon* targetMon) {
    return BattleMon_GetTurnFlag(targetMon, TURNFLAG_ACTIONDONE) || BattleMon_GetTurnFlag(targetMon, TURNFLAG_MOVED);
}
static b32 TerrainActiveForUser(ServerFlow* serverFlow, u32 userSlot, TERRAIN terrain) {
    if (ServerEvent_GetTerrain(serverFlow) != terrain)
        return 0;
    return !ServerControl_CheckFloating(serverFlow, Handler_GetBattleMon(serverFlow, userSlot), 1);
}
static void MulPower(u32 numerator, u32 denominator) {
    BattleEventVar_RewriteValue(VAR_MOVE_POWER, BattleEventVar_GetValue(VAR_MOVE_POWER) * numerator / denominator);
}

// ---------------- DIRE CLAW (827)
extern "C" void HandlerDireClaw(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot;
        for (u32 idx = 0; EachTarget(idx, &targetSlot); ++idx) {
            BattleMon* targetMon = LiveTarget(serverFlow, targetSlot);
            if (!targetMon || BattleMon_GetStatus(targetMon) != CONDITION_NONE)
                continue;
            if (!AbilityEvent_RollEffectChance(serverFlow, 50u))
                continue;
            CONDITION condition;
            switch (BattleRandom(3u)) {
            case 0: condition = CONDITION_POISON; break;
            case 1: condition = CONDITION_PARALYSIS; break;
            default: condition = CONDITION_SLEEP; break;
            }
            AddStatus(serverFlow, pokemonSlot, targetSlot, condition, MakeBasicStatus((MoveCondition)condition));
        }
    }
}
BattleEventHandlerTableEntry DireClawHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerDireClaw},
};
extern "C" BattleEventHandlerTableEntry* EventAddDireClaw(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(DireClawHandlers);
    return DireClawHandlers;
}

// ---------------- BURNING JEALOUSY (807) / ALLURING VOICE (914)
extern "C" void HandlerBurningJealousy(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot;
        for (u32 idx = 0; EachTarget(idx, &targetSlot); ++idx) {
            BattleMon* targetMon = LiveTarget(serverFlow, targetSlot);
            if (!targetMon || BattleMon_GetStatus(targetMon) != CONDITION_NONE)
                continue;
            if (GetLote4State()->statRaisedTurn[targetSlot] != serverFlow->turnCount + 1)
                continue;
            AddStatus(serverFlow, pokemonSlot, targetSlot, CONDITION_BURN, MakeBasicStatus(CONDITION_BURN));
        }
    }
}
BattleEventHandlerTableEntry BurningJealousyHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerBurningJealousy},
};
extern "C" BattleEventHandlerTableEntry* EventAddBurningJealousy(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(BurningJealousyHandlers);
    return BurningJealousyHandlers;
}
extern "C" void HandlerAlluringVoice(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot;
        for (u32 idx = 0; EachTarget(idx, &targetSlot); ++idx) {
            BattleMon* targetMon = LiveTarget(serverFlow, targetSlot);
            if (!targetMon || BattleMon_CheckIfMoveCondition(targetMon, CONDITION_CONFUSION))
                continue;
            if (GetLote4State()->statRaisedTurn[targetSlot] != serverFlow->turnCount + 1)
                continue;
            ConditionData condData;
            MakeCondition(CONDITION_CONFUSION, targetMon, &condData);
            AddStatus(serverFlow, pokemonSlot, targetSlot, CONDITION_CONFUSION, condData);
        }
    }
}
BattleEventHandlerTableEntry AlluringVoiceHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerAlluringVoice},
};
extern "C" BattleEventHandlerTableEntry* EventAddAlluringVoice(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(AlluringVoiceHandlers);
    return AlluringVoiceHandlers;
}

// ---------------- CORE ENFORCER (687)
extern "C" void HandlerCoreEnforcer(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot;
        for (u32 idx = 0; EachTarget(idx, &targetSlot); ++idx) {
            BattleMon* targetMon = Handler_GetBattleMon(serverFlow, targetSlot);
            if (!targetMon || BattleMon_IsFainted(targetMon) || !TargetHasActed(targetMon))
                continue;
            if (BattleMon_CheckIfMoveCondition(targetMon, CONDITION_GASTROACID))
                continue;
            if (AbilityFailChange(BattleMon_GetValue(targetMon, VALUE_EFFECTIVE_ABILITY)))
                continue;

            HandlerParam_AddCondition* addCondition;
            addCondition = (HandlerParam_AddCondition*)BattleHandler_PushWork(serverFlow, EFFECT_ADD_CONDITION, pokemonSlot);
            addCondition->condition = CONDITION_GASTROACID;
            addCondition->condData = Condition_MakePermanent();
            addCondition->almost = 0;
            addCondition->pokeID = (u8)targetSlot;
            BattleHandler_StrSetup(&addCondition->exStr, 2u, 565);
            BattleHandler_AddArg(&addCondition->exStr, targetSlot);
            BattleHandler_PopWork(serverFlow, addCondition);
        }
    }
}
BattleEventHandlerTableEntry CoreEnforcerHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerCoreEnforcer},
};
extern "C" BattleEventHandlerTableEntry* EventAddCoreEnforcer(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(CoreEnforcerHandlers);
    return CoreEnforcerHandlers;
}

// ---------------- GUARDIAN OF ALOLA (698): 75 % de los PS actuales del objetivo
extern "C" void HandlerGuardianOfAlola(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, BattleEventVar_GetValue(VAR_DEFENDING_MON));
        u32 damage = BattleMon_GetValue(defendingMon, VALUE_CURRENT_HP) * 3 / 4;
        if (damage == 0)
            damage = 1;
        BattleEventVar_RewriteValue(VAR_FIXED_DAMAGE, damage);
    }
}
BattleEventHandlerTableEntry GuardianOfAlolaHandlers[]{
    {EVENT_MOVE_DAMAGE_PROCESSING_1, HandlerGuardianOfAlola},
};
extern "C" BattleEventHandlerTableEntry* EventAddGuardianOfAlola(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(GuardianOfAlolaHandlers);
    return GuardianOfAlolaHandlers;
}

// ---------------- GENESIS SUPERNOVA (703): campo psiquico tras el golpe
extern "C" void HandlerGenesisSupernova(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerCommon_TerrainChange(serverFlow, pokemonSlot, TERRAIN_PSYCHIC, 5, false, BATTLE_PSYCHIC_TERRAIN_MSGID);
    }
}
BattleEventHandlerTableEntry GenesisSupernovaHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerGenesisSupernova},
};
extern "C" BattleEventHandlerTableEntry* EventAddGenesisSupernova(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(GenesisSupernovaHandlers);
    return GenesisSupernovaHandlers;
}

// ---------------- BOLT BEAK (754) / FISHIOUS REND (755): x2 si el usuario actua antes que el objetivo
extern "C" void HandlerBoltBeakPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, BattleEventVar_GetValue(VAR_DEFENDING_MON));
        if (defendingMon && !TargetHasActed(defendingMon)) {
            MulPower(2, 1);
        }
    }
}
BattleEventHandlerTableEntry BoltBeakHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerBoltBeakPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddBoltBeak(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(BoltBeakHandlers);
    return BoltBeakHandlers;
}

// ---------------- METEOR BEAM (800): At. Esp. +1 el turno de carga (la carga viene de la marca de MoveData)
extern "C" void HandlerMeteorBeamCharge(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        HandlerParam_Message* message;
        message = (HandlerParam_Message*)BattleHandler_PushWork(serverFlow, EFFECT_MESSAGE, pokemonSlot);
        BattleHandler_StrSetup(&message->str, 2u, BATTLE_METEOR_BEAM_MSGID);
        BattleHandler_AddArg(&message->str, pokemonSlot);
        BattleHandler_PopWork(serverFlow, message);

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
BattleEventHandlerTableEntry MeteorBeamHandlers[]{
    {EVENT_CHARGE_UP_START_DONE, HandlerMeteorBeamCharge},
};
extern "C" BattleEventHandlerTableEntry* EventAddMeteorBeam(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(MeteorBeamHandlers);
    return MeteorBeamHandlers;
}

// ---------------- MISTY EXPLOSION (802): Explosion + 50 % de potencia con campo de niebla
extern "C" void HandlerMistyExplosionPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) &&
        TerrainActiveForUser(serverFlow, pokemonSlot, TERRAIN_MISTY)) {
        MulPower(3, 2);
    }
}
// Las tres primeras entradas son las de Explosion del juego (sus manejadores estan en ESDB.yml)
extern "C" void HandlerExplosionDamage(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
extern "C" void HandlerExplosionStart(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
extern "C" void HandlerExplosionEnd(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
BattleEventHandlerTableEntry MistyExplosionHandlers[]{
    {EVENT_DETERMINE_MOVE_DAMAGE, HandlerExplosionDamage},
    {EVENT_MOVE_EXECUTE_START, HandlerExplosionStart},
    {EVENT_MOVE_EXECUTE_END, HandlerExplosionEnd},
    {EVENT_MOVE_BASE_POWER, HandlerMistyExplosionPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddMistyExplosion(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(MistyExplosionHandlers);
    return MistyExplosionHandlers;
}

// ---------------- EXPANDING FORCE (797): +50 % con campo psiquico (no se extiende a todos los rivales)
extern "C" void HandlerExpandingForcePower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) &&
        TerrainActiveForUser(serverFlow, pokemonSlot, TERRAIN_PSYCHIC)) {
        MulPower(3, 2);
    }
}
BattleEventHandlerTableEntry ExpandingForceHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerExpandingForcePower},
};
extern "C" BattleEventHandlerTableEntry* EventAddExpandingForce(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ExpandingForceHandlers);
    return ExpandingForceHandlers;
}

// ---------------- RISING VOLTAGE (804): x2 si el objetivo esta en el suelo con campo electrico
extern "C" void HandlerRisingVoltagePower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        if (TerrainActiveForUser(serverFlow, defendingSlot, TERRAIN_ELECTRIC)) {
            MulPower(2, 1);
        }
    }
}
BattleEventHandlerTableEntry RisingVoltageHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerRisingVoltagePower},
};
extern "C" BattleEventHandlerTableEntry* EventAddRisingVoltage(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(RisingVoltageHandlers);
    return RisingVoltageHandlers;
}

// ---------------- TERRAIN PULSE (805)
static b32 TerrainPulseType(ServerFlow* serverFlow, u32 userSlot, u32* type) {
    TERRAIN terrain = ServerEvent_GetTerrain(serverFlow);
    if (terrain == TERRAIN_NULL || terrain == TERRAIN_NULLIFY)
        return 0;
    if (ServerControl_CheckFloating(serverFlow, Handler_GetBattleMon(serverFlow, userSlot), 1))
        return 0;
    switch (terrain) {
    case TERRAIN_ELECTRIC: *type = TYPE_ELEC; return 1;
    case TERRAIN_GRASSY: *type = TYPE_GRASS; return 1;
    case TERRAIN_MISTY: *type = TYPE_FAIRY; return 1;
    case TERRAIN_PSYCHIC: *type = TYPE_PSY; return 1;
    default: return 0;
    }
}
extern "C" void HandlerTerrainPulseType(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        u32 type;
        if (TerrainPulseType(serverFlow, pokemonSlot, &type))
            BattleEventVar_RewriteValue(VAR_MOVE_TYPE, type);
    }
}
extern "C" void HandlerTerrainPulsePower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 type;
        if (TerrainPulseType(serverFlow, pokemonSlot, &type))
            MulPower(2, 1);
    }
}
BattleEventHandlerTableEntry TerrainPulseHandlers[]{
    {EVENT_MOVE_PARAM, HandlerTerrainPulseType},
    {EVENT_MOVE_BASE_POWER, HandlerTerrainPulsePower},
};
extern "C" BattleEventHandlerTableEntry* EventAddTerrainPulse(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TerrainPulseHandlers);
    return TerrainPulseHandlers;
}

// ---------------- LASH OUT (808)
extern "C" void HandlerLashOutPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        if (GetLote4State()->statLoweredTurn[pokemonSlot] == serverFlow->turnCount + 1) {
            MulPower(2, 1);
        }
    }
}
BattleEventHandlerTableEntry LashOutHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerLashOutPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddLashOut(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(LashOutHandlers);
    return LashOutHandlers;
}

// ---------------- POLTERGEIST (809): falla si el objetivo no lleva objeto
extern "C" void HandlerPoltergeistCheck(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 defendingSlot = BattleEventVar_GetValue(VAR_DEFENDING_MON);
        if (defendingSlot >= BATTLE_MAX_SLOTS)
            return;
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, defendingSlot);
        if (!BattleMon_GetHeldItem(defendingMon)) {
            BattleEventVar_RewriteValue(VAR_NO_EFFECT_FLAG, 1);
        }
    }
}
BattleEventHandlerTableEntry PoltergeistHandlers[]{
    {EVENT_ABILITY_CHECK_NO_EFFECT, HandlerPoltergeistCheck},
};
extern "C" BattleEventHandlerTableEntry* EventAddPoltergeist(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(PoltergeistHandlers);
    return PoltergeistHandlers;
}

// ---------------- EERIE SPELL (826): -3 PP al ultimo ataque del objetivo
extern "C" void HandlerEerieSpell(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 targetSlot;
        for (u32 idx = 0; EachTarget(idx, &targetSlot); ++idx) {
            BattleMon* targetMon = LiveTarget(serverFlow, targetSlot);
            if (!targetMon)
                continue;
            MOVE_ID lastMove = BattleMon_GetPreviousMove(targetMon);
            if (!lastMove)
                continue;
            u32 moveIdx = Move_SearchIndex(targetMon, lastMove);
            if (moveIdx >= 4)
                continue;
            u32 pp = Move_GetPP(targetMon, moveIdx);
            if (pp == 0)
                continue;
            u32 amount = pp < 3 ? pp : 3;

            HandlerParam_PP* decrementPP;
            decrementPP = (HandlerParam_PP*)BattleHandler_PushWork(serverFlow, EFFECT_DECREMENT_PP, pokemonSlot);
            decrementPP->pokeID = (u8)targetSlot;
            decrementPP->volume = (u8)amount;
            decrementPP->moveSlot = (u8)moveIdx;
            BattleHandler_StrSetup(&decrementPP->exStr, 2u, 641);
            BattleHandler_AddArg(&decrementPP->exStr, targetSlot);
            BattleHandler_AddArg(&decrementPP->exStr, lastMove);
            BattleHandler_AddArg(&decrementPP->exStr, amount);
            BattleHandler_PopWork(serverFlow, decrementPP);
        }
    }
}
BattleEventHandlerTableEntry EerieSpellHandlers[]{
    {EVENT_DAMAGE_PROCESSING_END_HIT_REAL, HandlerEerieSpell},
};
extern "C" BattleEventHandlerTableEntry* EventAddEerieSpell(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(EerieSpellHandlers);
    return EerieSpellHandlers;
}

// ---------------- TERA BLAST (851): sin teracristalizacion, Normal; fisico si el Ataque supera al At. Esp.
extern "C" void HandlerTeraBlastCategory(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_MON_ID)) {
        BattleMon* currentMon = Handler_GetBattleMon(serverFlow, pokemonSlot);
        u32 attack = BattleMon_GetRealStat(currentMon, VALUE_ATTACK_STAT);
        u32 specialAttack = BattleMon_GetRealStat(currentMon, VALUE_SPECIAL_ATTACK_STAT);
        if (attack > specialAttack) {
            BattleEventVar_RewriteValue(VAR_MOVE_CATEGORY, CATEGORY_PHYSICAL);
        }
    }
}
BattleEventHandlerTableEntry TeraBlastHandlers[]{
    {EVENT_MOVE_PARAM, HandlerTeraBlastCategory},
};
extern "C" BattleEventHandlerTableEntry* EventAddTeraBlast(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(TeraBlastHandlers);
    return TeraBlastHandlers;
}

// ---------------- LAST RESPECTS (854)
extern "C" void HandlerLastRespectsPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 clientID = MainModule_PokeIDToClientID(pokemonSlot);
        u32 fainted = 0;
        for (u32 i = 0; i < 24; ++i) {
            BattleMon* partyMon = PokeCon_GetBattleMon(serverFlow->pokeCon, i);
            if (partyMon && MainModule_PokeIDToClientID(BattleMon_GetID(partyMon)) == clientID && BattleMon_IsFainted(partyMon))
                ++fainted;
        }
        BattleEventVar_RewriteValue(VAR_MOVE_POWER, BattleEventVar_GetValue(VAR_MOVE_POWER) + 50 * fainted);
    }
}
BattleEventHandlerTableEntry LastRespectsHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerLastRespectsPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddLastRespects(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(LastRespectsHandlers);
    return LastRespectsHandlers;
}

// ---------------- RAGE FIST (889)
extern "C" void HandlerRageFistPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        u32 hits = GetLote4State()->timesHit[pokemonSlot];
        u32 power = 50 + 50 * hits;
        if (power > 350)
            power = 350;
        BattleEventVar_RewriteValue(VAR_MOVE_POWER, power);
    }
}
BattleEventHandlerTableEntry RageFistHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerRageFistPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddRageFist(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(RageFistHandlers);
    return RageFistHandlers;
}

// ---------------- HARD PRESS (912)
extern "C" void HandlerHardPressPower(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON)) {
        BattleMon* defendingMon = Handler_GetBattleMon(serverFlow, BattleEventVar_GetValue(VAR_DEFENDING_MON));
        u32 maxHP = BattleMon_GetValue(defendingMon, VALUE_MAX_HP);
        u32 currentHP = BattleMon_GetValue(defendingMon, VALUE_CURRENT_HP);
        u32 power = maxHP ? 100 * currentHP / maxHP : 1;
        if (power == 0)
            power = 1;
        BattleEventVar_RewriteValue(VAR_MOVE_POWER, power);
    }
}
BattleEventHandlerTableEntry HardPressHandlers[]{
    {EVENT_MOVE_BASE_POWER, HandlerHardPressPower},
};
extern "C" BattleEventHandlerTableEntry* EventAddHardPress(u32* handlerAmount) {
    *handlerAmount = ARRAY_COUNT(HardPressHandlers);
    return HardPressHandlers;
}

#endif // EXPAND_MOVES
