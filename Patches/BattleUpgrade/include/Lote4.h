#ifndef __LOTE4_H
#define __LOTE4_H

#include "defs.h"

// Hack PokeLobby (lote 4). Estado compartido entre el motor (BattleUpgrade) y las DLL de ataques.
// Vive dentro de BattleFieldExt y se pone a cero al empezar el combate.
#define LOTE4_SLOTS 31

struct Lote4State {
    // turnCount + 1 del ultimo turno en el que al Pokemon le han subido / bajado una estadistica (Envidia Ardiente,
    // Canto Encantador, Desahogo). 0 = nunca.
    u32 statRaisedTurn[LOTE4_SLOTS];
    u32 statLoweredTurn[LOTE4_SLOTS];
    // Golpes recibidos en el combate (Puno Furia). No se borra al cambiar de Pokemon.
    u8 timesHit[LOTE4_SLOTS];
    // 1 si el Pokemon ya ha usado Sin Retirada desde que entro en combate
    u8 noRetreat[LOTE4_SLOTS];
    // 1 si ya se ha usado Hora Feliz en este combate
    u8 happyHour;
    u8 pad[2];
};

#endif // __LOTE4_H
