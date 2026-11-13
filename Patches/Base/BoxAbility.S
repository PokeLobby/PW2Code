.thumb

#include "settings.h"

#if EXPAND_ABILITIES

.type THUMB_BRANCH_LINK_BoxView_PrintAbilityName_0x30, %function

@ PC del juego (overlay 255): la ficha del Pokemon senalado guarda la habilidad en un byte
@ (strb r0,[r4,#0xE] en 0x021BF0CE), asi que una habilidad nueva (>255) salia como otra
@ (General Supremo 293 -> Potencia 37). Aqui se escribe su nombre: en vez del byte recortado
@ (r2), se vuelve a pedir la habilidad al Pokemon, cuyo puntero esta en el +0 de la ficha (r6).
@ PokeLobbyLauncher#99.
@ Entra con r0 = wordset, r1 = 0 (hueco), r2 = habilidad recortada, r6 = ficha.
THUMB_BRANCH_LINK_BoxView_PrintAbilityName_0x30:
    push    {r4, lr}
    movs    r4, r0
    ldr     r0, [r6]            @ BoxPkm*
    movs    r1, #0xA            @ PF_Ability
    movs    r2, #0
    bl      PML_PkmGetParam
    movs    r2, r0
    movs    r0, r4
    movs    r1, #0
    bl      GFL_WordSetRegisterAbilityName
    pop     {r4, pc}
    .size THUMB_BRANCH_LINK_BoxView_PrintAbilityName_0x30, . - THUMB_BRANCH_LINK_BoxView_PrintAbilityName_0x30

#endif // EXPAND_ABILITIES
