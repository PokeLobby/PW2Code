.thumb

#include "settings.h"

@ PC del juego (overlay 255): la ficha del Pokemon senalado pinta el segundo tipo solo si es
@ distinto del primero Y distinto de 0 (cmp r4,#0 / beq en 0x021D0A42). En Blanco 2 ningun
@ Pokemon tenia Normal (0) de segundo tipo, pero en el hack si: Ursaluna (Tierra/Normal),
@ Litleo/Pyroar, Helioptile/Heliolisk, Indeedee... salian solo con el primero. Se quita el
@ salto del 0; el de "igual al primero" (monotipo) se queda.
FULL_COPY_255_0x21D0A44:
    .hword 0x46C0               @ nop (era beq 0x021D0A66)
    .size FULL_COPY_255_0x21D0A44, . - FULL_COPY_255_0x21D0A44
