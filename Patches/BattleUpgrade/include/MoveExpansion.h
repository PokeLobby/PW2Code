#ifndef MOVE_EXPANSION_H
#define MOVE_EXPANSION_H

#include "battle_events.h"
#include "Lote4.h"

extern "C" BattleEventHandlerTableEntry * EventAddFlyingPress(u32 * handlerAmount);
extern "C" BattleEventHandlerTableEntry * EventAddMatBlock(u32 * handlerAmount); 
extern "C" BattleEventHandlerTableEntry* EventAddRototiller(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddStickyWeb(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddFellStinger(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTrickOrTreat(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddIonDeluge(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddForestsCurse(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddFreezeDry(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPartingShot(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTopsyTurvy(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddCraftyShield(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddGrassyTerrain(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMistyTerrain(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddElectrify(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddKingsShield(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSpikyShield(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddVenomDrench(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPowder(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMagneticFlux(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddElectricTerrain(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddThousandArrows(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddHyperspaceFury(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddShoreUp(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddBanefulBunker(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSparklingAria(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddFloralHealing(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddStrengthSap(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSpotlight(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddLaserFocus(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddThroatChop(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPollenPuff(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSpeedSwap(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddAuroraVeil(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddStuffCheeks(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTarShot(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddJungleHealing(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddRevivalBlessing(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSaltCure(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTidyUp(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddShedTail(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSnowscape(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSyrupBomb(u32* handlerAmount);

// Hack PokeLobby (lote 3)
extern "C" BattleEventHandlerTableEntry* EventAddBurnUp(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddRevelationDance(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddHyperspaceHole(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTrapHit(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddJawLock(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddStompingTantrum(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMoldBreakerMove(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPhotonGeyser(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMindBlown(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddBeakBlast(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddShellTrap(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSpectralThief(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSteelRoller(u32* handlerAmount);
extern "C" b32 BattleField_GetMoveFailedPrev(u32 battleSlot);
extern "C" void BattleField_SetMoveFailedNow(u32 battleSlot);
extern "C" void BattleField_BeginMoveAction(u32 battleSlot);
extern "C" void BattleField_ClearMoveFailed(u32 battleSlot);
extern "C" b32 BattleField_GetShellTrapHit(u32 battleSlot);
extern "C" void BattleField_SetShellTrapHit(u32 battleSlot, u32 value);

// Hack PokeLobby (lote 4)
extern "C" Lote4State* GetLote4State();
extern "C" u32 SwapSideEffects();
extern "C" b32 AbilityFailChange(u32 ability);
extern "C" BattleEventHandlerTableEntry* EventAddObstruct(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSilkTrap(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddBurningBulwark(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddExtremeEvoboost(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddClangorousSoulblaze(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddClangorousSoul(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddFilletAway(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddNoRetreat(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosObstruct(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosSilkTrap(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosBurningBulwark(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddFairyLock(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddOctolock(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddHappyHour(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTeatime(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddCourtChange(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddChillyReception(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosFairyLock(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosOctolock(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMagicPowder(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddDoodle(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddCorrosiveGas(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPowerShift(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddDragonCheer(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddInstruct(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPurify(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosDragonCheer(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddDireClaw(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddBurningJealousy(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddAlluringVoice(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddCoreEnforcer(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddGuardianOfAlola(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddGenesisSupernova(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddBoltBeak(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMeteorBeam(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddMistyExplosion(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddExpandingForce(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddRisingVoltage(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTerrainPulse(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddLashOut(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPoltergeist(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddEerieSpell(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddTeraBlast(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddLastRespects(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddRageFist(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddHardPress(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddSplinteredStormshards(u32* handlerAmount);

extern "C" BattleEventHandlerTableEntry* EventAddPsychicTerrain(u32* handlerAmount);

extern "C" BattleEventHandlerTableEntry* EventAddPlasmaFist(u32* handlerAmount);

extern "C" BattleEventHandlerTableEntry * EventAddCeaselessEdge(u32 * handlerAmount);

extern "C" BattleEventHandlerTableEntry* EventAddRapidSpinUpdated(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddDefogUpdated(u32* handlerAmount);

extern "C" BattleEventHandlerTableEntry* EventAddPosIonDeluge(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosCraftyShield(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosElectrify(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosKingsShield(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosSpikyShield(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosPowder(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosBanefulBunker(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosLaserFocus(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosThroatChop(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosSaltCure(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosSyrupBomb(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosTarShot(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosShedTail(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosBeakBlast(u32* handlerAmount);
extern "C" BattleEventHandlerTableEntry* EventAddPosShellTrap(u32* handlerAmount);

extern "C" void GetSideBattleSlots(ServerFlow * serverFlow, u32 side, u8 battleSlots[3]);
extern "C" u32 PosEffect_SetPriority(BattleEventPriority priority, u32 handlerCount);
extern "C" void HandlerPosTurnCheckDone(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);
extern "C" void HandlerPosProtectBroken(BattleEventItem* item, ServerFlow* serverFlow, u32 pokemonSlot, u32* work);

#endif // MOVE_EXPANSION_H
