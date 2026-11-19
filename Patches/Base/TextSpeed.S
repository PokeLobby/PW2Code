.thumb

#include "settings.h"

@ Hack PokeLobby (pedido de Poxi, 2026-11-19): el texto respeta otra vez la opción «Velocidad del texto».
@
@ La base del hack (White 2 Upgrade) hacía el texto instantáneo cambiando tres instrucciones de
@ PrintStream_Update (0x2022490) en el ARM9: quitaba la espera entre letras (+0xE0 y +0xE2, ahora nop)
@ y hacía que el bucle de letras siguiera hasta el final del texto (+0x198). Aquí se ponen las del juego
@ original (Blanco 2 español): Lenta, Media y Rápida vuelven a ir como en el juego.
@ El arreglo de los mensajes de la tienda (TextFix.cpp, +0x152) sigue valiendo con esto.

FULL_COPY_PrintStream_Update_0xE0:
    .hword 0xD000   @ beq 0x2022574: sin espera pendiente, pinta
    .hword 0xE081   @ b   0x2022678: con espera pendiente, la descuenta y sale
    .size FULL_COPY_PrintStream_Update_0xE0, . - FULL_COPY_PrintStream_Update_0xE0

FULL_COPY_PrintStream_Update_0x198:
    .hword 0xD102   @ bne 0x2022630: tras cada letra, vuelve a poner la espera
    .size FULL_COPY_PrintStream_Update_0x198, . - FULL_COPY_PrintStream_Update_0x198
