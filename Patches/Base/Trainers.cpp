#include "settings.h"

#include "gfl/core/gfl_random.h"

#include "poke_party.h"
#include "trainer_data.h"

#if TRAINER_EVS
#include "nds/fs.h"
#endif

#ifdef PERFECT_TRAINERS

struct SWAN_PACKED TrainerPokemonExt
{
	u32 genetic; // 2 bits [sex] + 30 bits [IVs (HP, ATK, DEF, SPA, SPD, SPE)]
	u16 species;
	u8 form;
	u8 level;
	u16 heldItem;
	u16 moves[4]; // 1 bit [PP Max] + 15 bits [Move ID]
	u16 ability;
	u8 evs[6]; // (HP, ATK, DEF, SPE, SPA, SPD)
	u8 nature;
	u8 happiness;
	u16 state; // 1 bit [shiny] + 3 bits [status] + 7 bits [hp]
};
#define GET_TRPOKE_SEX(trainerPokemon) (trainerPokemon.genetic >> 30)
#define GET_TRPOKE_IV(trainerPokemon) (trainerPokemon.genetic & 0x3FFFFFFF)
#define GET_TRMOVE_PPMAX(trainerPokemon, slot) (trainerPokemon.moves[slot] & 0x8000 != 0) 
#define GET_TRMOVE_ID(trainerPokemon, slot) (trainerPokemon.moves[slot] & 0x7FFF) 

#define GET_TRPOKE_SHINY(trainerPokemon) ((trainerPokemon.state & 0x0400) != 0)
#define GET_TRPOKE_STATUS(trainerPokemon) ((trainerPokemon.state & 0x0380) >> 7)
#define GET_TRPOKE_HP(trainerPokemon) (trainerPokemon.state & 0x007F)


#if TRAINER_EVS

// Hack PokeLobby: EV de los Pokemon de entrenador. La ficha de entrenador de Gen 5 no tiene EV, asi que se leen de un
// fichero de la ROM de tamano fijo (raiz, TRAINER_EVS_FILE) escrito por el randomizador. Contrato: tools/evs/CONTRATO.md
//   cabecera 8 B: 'PLEV', u16 version (1), u16 numero de entrenadores N
//   despues, por id de entrenador (0..N-1): 6 huecos x 6 B (PS, Atq, Def, Vel, AtEsp, DefEsp), cada EV 0-252 y suma <= 510.
// Hueco todo a cero, con algun EV > 252 o con suma > 510 = se ignora (el Pokemon queda como siempre).
#define TRAINER_EVS_HEADER 8
#define TRAINER_EVS_SLOT 6
#define TRAINER_EVS_ENTRY (6 * TRAINER_EVS_SLOT)

static b32 ReadTrainerEVs(u32 trainerId, u8* out) {
	FSFile file;
	finit(&file);
	if (!romfs_fopen(&file, TRAINER_EVS_FILE))
		return 0;

	u8 header[TRAINER_EVS_HEADER];
	b32 ok = romfs_fread(&file, header, TRAINER_EVS_HEADER) == TRAINER_EVS_HEADER;
	if (ok)
		ok = header[0] == 'P' && header[1] == 'L' && header[2] == 'E' && header[3] == 'V' &&
			(header[4] | (header[5] << 8)) == 1 && trainerId < (u32)(header[6] | (header[7] << 8));
	if (ok)
		ok = romfs_fseek(&file, TRAINER_EVS_HEADER + trainerId * TRAINER_EVS_ENTRY, IO_SEEK_SET) &&
			romfs_fread(&file, out, TRAINER_EVS_ENTRY) == TRAINER_EVS_ENTRY;
	romfs_fclose(&file);
	return ok;
}

static void ApplyTrainerEVs(PokeParty* party, u32 trainerId) {
	u8 table[TRAINER_EVS_ENTRY];
	if (!ReadTrainerEVs(trainerId, table))
		return;

	u32 count = party->PokemonCount;
	if (count > 6)
		count = 6;
	for (u32 slot = 0; slot < count; ++slot) {
		const u8* evs = &table[slot * TRAINER_EVS_SLOT];
		u32 sum = 0;
		b32 valid = 1;
		for (u32 ev = 0; ev < 6; ++ev) {
			sum += evs[ev];
			if (evs[ev] > 252)
				valid = 0;
		}
		if (!valid || sum == 0 || sum > 510)
			continue;

		PartyPkm* pkm = &party->Pokemon[slot];
		u32 oldNow = PokeParty_GetParam(pkm, PF_NowHP, nullptr);
		u32 oldMax = PokeParty_GetParam(pkm, PF_MaxHP, nullptr);

		for (u32 ev = 0; ev < 6; ++ev)
			PokeParty_SetParam(pkm, (PkmField)(PF_EvHP + ev), evs[ev]);
		PokeParty_RecalcStats(pkm);

		// la vida actual conserva su proporcion (llena = llena)
		u32 newMax = PokeParty_GetParam(pkm, PF_MaxHP, nullptr);
		u32 newNow = newMax;
		if (oldNow != oldMax && oldMax != 0)
			newNow = (oldNow * newMax) / oldMax;
		PokeParty_SetParam(pkm, PF_NowHP, newNow);
	}
}

#endif // TRAINER_EVS

extern "C" void TrainerUtil_CalcBasePID(u16 species, u16 form, u8 abilAndSex, u32* dest, HeapID heapId);
extern "C" void THUMB_BRANCH_LINK_TrainerUtil_LoadParty_0x4C8(PokeParty* party, PartyPkm* partyPkm, 
	TrainerData* trainerData, TrainerPokemonExt* team, u32 trainerId) {

	if (trainerData->trainerType == PERFECT_TRAINER) {
		for (u32 slot = 0; slot < trainerData->pokemonCount; ++slot) {
			u32 PID = 0;
			TrainerUtil_CalcBasePID(team[slot].species, team[slot].form, GET_TRPOKE_SEX(team[slot]), &PID, 0x1/*HEAPID_BATTLE_LOAD*/);
			PID += (team[slot].species + team[slot].form + team[slot].level + GFL_RandomLC(0xFF)) << 8;

			PokeParty_CreatePkm(partyPkm, team[slot].species, team[slot].level, 0, -1, GET_TRPOKE_IV(team[slot]), PID, 0);
		
			
			PokeParty_SetParam(partyPkm, PF_Item, team[slot].heldItem);
			for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot) {
				
				PokeParty_SetParam(partyPkm, (PkmField)(PF_Move1PPMax + moveSlot), GET_TRMOVE_PPMAX(team[slot], moveSlot));
				PokeParty_SetMove(partyPkm, GET_TRMOVE_ID(team[slot], moveSlot), moveSlot);
			}

			PokeParty_SetParam(partyPkm, PF_Forme, team[slot].form);
			PokeParty_SetParam(partyPkm, PF_Ability, team[slot].ability);
			PokeParty_SetParam(partyPkm, PF_Nature, team[slot].nature);
			PokeParty_SetParam(partyPkm, PF_Happiness, team[slot].happiness);

			for (u32 ev = 0; ev < 6; ++ev)
				PokeParty_SetParam(partyPkm, (PkmField)(PF_EvHP + ev), team[slot].evs[ev]);

			PokeParty_RecalcStats(partyPkm);

			if (GET_TRPOKE_SHINY(team[slot])) {
				// This ensures the Pokémon being shiny
				PokeParty_SetParam(partyPkm, PF_IdSet, PokeParty_GetParam(partyPkm, PF_PID, nullptr));
			}
			else {
				// This ensures the Pokémon not being shiny
				PokeParty_SetParam(partyPkm, PF_IdSet, 0xFFFF0000);
			}

			PokeParty_SetParam(partyPkm, PF_StatusCond, GET_TRPOKE_STATUS(team[slot]));
			
			u32 maxHP = PokeParty_GetParam(partyPkm, PF_MaxHP, nullptr);
			PokeParty_SetParam(partyPkm, PF_NowHP, (GET_TRPOKE_HP(team[slot]) * maxHP) / 100);
			
			PokeParty_AddPkm(party, partyPkm);
#if 0
			DPRINTF("--- TEAM SLOT %d --- \n", slot);
			DPRINTF("SPECIES: %d \n", team[slot].species);
			DPRINTF("FORM: %d \n", team[slot].form);
			DPRINTF("LEVEL: %d \n", team[slot].level);
			DPRINTF("ITEM: %d \n", team[slot].heldItem);
			DPRINTF("ABILITY: %d \n", team[slot].ability);
			DPRINTF("NATURE: %d \n", team[slot].nature);
			DPRINTF("HAPPINESS: %d \n", team[slot].happiness);
			DPRINT("MOVES: \n");
			for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
				DPRINTF("    %d (%d) \n", GET_TRMOVE_ID(team[slot], moveSlot), GET_TRMOVE_PPMAX(team[slot], moveSlot));
			DPRINTF("STATUS: %d \n", GET_TRPOKE_STATUS(team[slot]));
			DPRINTF("HP: %d \n", (GET_TRPOKE_HP(team[slot]) * maxHP) / 100);
#endif
		}
	}

#if TRAINER_EVS
	ApplyTrainerEVs(party, trainerId);
#endif

	GFL_HeapFree(trainerData);
	GFL_HeapFree(team);
	GFL_HeapFree(partyPkm);
}

#endif // PERFECT_TRAINERS