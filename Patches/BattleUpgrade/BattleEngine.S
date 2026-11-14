.thumb

#include "settings.h"

#if GEN7_INFILTRATOR
.type THUMB_BRANCH_LINK_CALCDMG_Set_0xFA, %function
#endif
#if EXPAND_MOVES
.type THUMB_BRANCH_LINK_LoadAnimationScriptFile_0xB6, %function
.type THUMB_BRANCH_LINK_BtlvEffect_LoadAnimationScript_0x1A, %function
#endif

@ El montón 0x12 del combate pasa de 128 KB a 256 KB (lsls r2, r4, #0x11 -> #0x12, r4 = 1).
@ Con el código extra del hack, en los combates múltiples largos se quedaba troceado y
@ GFL_HeapAllocate acababa en el error fatal al cargar el gráfico de un Pokémon que entra
@ (23.988 bytes de /a/1/6/9). El montón padre (0x1) tiene más de 500 KB libres en combate.
@ PokeLobbyLauncher#102.
FULL_COPY_BattleProc_Init_0x26:
    .hword  0x04A2          @ lsls r2, r4, #0x12
    .size FULL_COPY_BattleProc_Init_0x26, . - FULL_COPY_BattleProc_Init_0x26

#if WEATHER_ABILITY_TURNS != 255

@ This changes the amount of turns weather will last after being summoned by an ability
FULL_COPY_CommonWeatherChangeAbility_0x2A:
    movs    r0, #WEATHER_ABILITY_TURNS
    .size FULL_COPY_CommonWeatherChangeAbility_0x2A, . - FULL_COPY_CommonWeatherChangeAbility_0x2A

#endif // WEATHER_ABILITY_TURNS != 255

#if EXPAND_FIELD_EFFECTS
 
FULL_COPY_BattleHandler_RemoveSideEffectCore_0x58:
    cmp     r4, #MAX_SIDE_EFFECT_AMOUNT
    .size FULL_COPY_BattleHandler_RemoveSideEffectCore_0x58, . - FULL_COPY_BattleHandler_RemoveSideEffectCore_0x58
 
#endif // EXPAND_FIELD_EFFECTS

#if SNOW_WEATHER && SNOW_NO_DAMAGE

@ This prevents any Pokemon from taking hail/snow damage
@ El original estaba en +0xE, que es la comprobacion de tipo Roca de la TORMENTA DE ARENA (tiempo 4): la arena dejaba
@ de hacer dano a todos y el granizo seguia haciendolo. La del granizo (tiempo 3, tipo Hielo) es la de +0x3C.
FULL_COPY_CalcWeatherDamage_0x3C:
    nop
    movs    r0, #1
    .size FULL_COPY_CalcWeatherDamage_0x3C, . - FULL_COPY_CalcWeatherDamage_0x3C

#endif // SNOW_WEATHER && SNOW_NO_DAMAGE

#if DRAIN_ANY_CATEGORY

@ ServerControl_DamageDrain (0x21A58AC) solo drenaba si la categoria del ataque era 8 (dano + absorber). Canon Batidor
@ (902) es dano + estado (quema) con 50 de drenaje y no curaba. Ahora drena cualquier ataque con drenaje (parametro 25)
@ mayor que 0; los de retroceso lo tienen negativo. Era el unico ataque con drenaje fuera de la categoria 8.
@ Antes: bl PML_MoveGetCategory; cmp r0,#8; bne fin; ldrh r0,[r4]; movs r1,#25; bl PML_MoveGetParam; adds r1,r0,#0; ldr r0,[sp,#24]
FULL_COPY_167_0x21A58B8:
    .hword 0x2119           @ movs r1, #25            (r0 = ataque, cargado en 0x21A58B2)
    .hword 0xF67B, 0xFCF7   @ bl   0x20212AC          (PML_MoveGetParam)
    .hword 0x0600           @ lsls r0, r0, #24
    .hword 0x1600           @ asrs r0, r0, #24        (con signo)
    .hword 0x2800           @ cmp  r0, #0
    .hword 0xDD13           @ ble  0x21A58EE          (sin drenaje: fin)
    .hword 0x1C01           @ adds r1, r0, #0
    .hword 0x9806           @ ldr  r0, [sp, #24]
    .hword 0x46C0           @ nop                     (sigue en 0x21A58CC: bl del calculo de lo drenado)
    .size FULL_COPY_167_0x21A58B8, . - FULL_COPY_167_0x21A58B8

#endif // DRAIN_ANY_CATEGORY

#if EXPAND_ABILITIES || ADD_MEGA_EVOLUTION

@ Remove the if statements that check for Unnerve and add the checked ability as 
@ the third function parameter so that Unnerve and As One can be checked check inside 
@ the ServerControl_UnnerveAction function
FULL_COPY_ServerControl_AddCondition_0x132:
    nop
    movs    r2, r0
    .size FULL_COPY_ServerControl_AddCondition_0x132, . - FULL_COPY_ServerControl_AddCondition_0x132

FULL_COPY_ServerDisplay_SkillSwap_0x148:
    nop
    movs    r2, r4
    .size FULL_COPY_ServerDisplay_SkillSwap_0x148, . - FULL_COPY_ServerDisplay_SkillSwap_0x148

FULL_COPY_ServerDisplay_SkillSwap_0x16E:
    nop
    movs    r2, r6
    .size FULL_COPY_ServerDisplay_SkillSwap_0x16E, . - FULL_COPY_ServerDisplay_SkillSwap_0x16E

FULL_COPY_ServerControl_ClearMonDependentEffects_0xB6:
    nop
    movs    r2, r0
    .size FULL_COPY_ServerControl_ClearMonDependentEffects_0xB6, . - FULL_COPY_ServerControl_ClearMonDependentEffects_0xB6

FULL_COPY_BattleHandler_ChangeAbility_0x126:
    nop
    movs    r2, r6
    .size FULL_COPY_BattleHandler_ChangeAbility_0x126, . - FULL_COPY_BattleHandler_ChangeAbility_0x126

#endif // EXPAND_ABILITIES || ADD_MEGA_EVOLUTION

#if GEN7_WEAK_ARMOR

FULL_COPY_HandlerWeakArmor_0x9E:
    strb    r7, [r1,#0xC]
    .size FULL_COPY_HandlerWeakArmor_0x9E, . - FULL_COPY_HandlerWeakArmor_0x9E

#endif // GEN7_WEAK_ARMOR

#if GEN8_MOODY

FULL_COPY_HandlerMoody_0x76:
    cmp r5, #5
    .size FULL_COPY_HandlerMoody_0x76, . - FULL_COPY_HandlerMoody_0x76

#endif // GEN8_MOODY

#if GEN7_INFILTRATOR

THUMB_BRANCH_LINK_CALCDMG_Set_0xFA:
    ldr     r3, [sp,#0x18]  @ attackingMon
    push    {r7, lr}
    movs    r7, r0
    movs    r0, r3
    movs    r1, #0x11       @ VALUE_EFFECTIVE_ABILITY
    bl      BattleMon_GetValue
    cmp     r0, #151        @ ABIL_INFILTRATOR
    bne     Check_Substitute
    movs    r0, #0
    b       Exit_CALCDMG_Set
Check_Substitute:
    movs    r0, r7          @ targetMon
    bl      BattleMon_IsSubstituteActive
Exit_CALCDMG_Set:
    pop     {r7, pc}
    .size THUMB_BRANCH_LINK_CALCDMG_Set_0xFA, . - THUMB_BRANCH_LINK_CALCDMG_Set_0xFA

#endif // GEN7_INFILTRATOR

#if EXPAND_ITEMS

FULL_COPY_HandlerDrySkinWeather_0x1A:
    movs    r0, r5
    .size FULL_COPY_HandlerDrySkinWeather_0x1A, . - FULL_COPY_HandlerDrySkinWeather_0x1A

FULL_COPY_HandlerSolarPowerWeather_0x12:
    movs    r0, r5
    .size FULL_COPY_HandlerSolarPowerWeather_0x12, . - FULL_COPY_HandlerSolarPowerWeather_0x12

FULL_COPY_HandlerSolarPowerPower_0x12:
    movs    r1, r5
    .size FULL_COPY_HandlerSolarPowerPower_0x12, . - FULL_COPY_HandlerSolarPowerPower_0x12

FULL_COPY_BattleHandler_ChangeAbility_0x10:
    movs    r1, r5
    .size FULL_COPY_BattleHandler_ChangeAbility_0x10, . - FULL_COPY_BattleHandler_ChangeAbility_0x10

FULL_COPY_ServerDisplay_SkillSwap_0x14:
    movs    r1, r5
    .size FULL_COPY_ServerDisplay_SkillSwap_0x14, . - FULL_COPY_ServerDisplay_SkillSwap_0x14

FULL_COPY_ServerDisplay_SkillSwap_0x1E:
    movs    r1, r5
    .size FULL_COPY_ServerDisplay_SkillSwap_0x1E, . - FULL_COPY_ServerDisplay_SkillSwap_0x1E

#endif // EXPAND_ITEMS

#if ADD_MEGA_EVOLUTION || (EXPAND_ABILITIES && EXPAND_ITEMS)

FULL_COPY_HandlerTrick_0x6C:
    nop
    .size FULL_COPY_HandlerTrick_0x6C, . - FULL_COPY_HandlerTrick_0x6C

FULL_COPY_HandlerTrick_0x82:
    ldr r0, [sp,#0x4]
    .size FULL_COPY_HandlerTrick_0x82, . - FULL_COPY_HandlerTrick_0x82

FULL_COPY_HandlerTrick_0x9A:
    ldr r0, [sp,#0x4]
    .size FULL_COPY_HandlerTrick_0x9A, . - FULL_COPY_HandlerTrick_0x9A

FULL_COPY_HandlerBestow_0x66:
    nop
    nop     @ el "bl GetSpecies" original ocupa 4 bytes (dos mitades Thumb): hay que anular ambas
    .size FULL_COPY_HandlerBestow_0x66, . - FULL_COPY_HandlerBestow_0x66

FULL_COPY_HandlerCommon_CheckUnchangeableItem_0x18:
    movs r0, r5
    .size FULL_COPY_HandlerCommon_CheckUnchangeableItem_0x18, . - FULL_COPY_HandlerCommon_CheckUnchangeableItem_0x18

FULL_COPY_HandlerCommon_CheckIfCanStealPokeItem_0x4A:
    movs r0, r4
    .size FULL_COPY_HandlerCommon_CheckIfCanStealPokeItem_0x4A, . - FULL_COPY_HandlerCommon_CheckIfCanStealPokeItem_0x4A

#endif // ADD_MEGA_EVOLUTION || (EXPAND_ABILITIES && EXPAND_ITEMS)

#if EXPAND_MOVES

THUMB_BRANCH_LINK_LoadAnimationScriptFile_0xB6:
    push    {lr}
    ldr     r3, =0x7FFF
    cmp     r1, r0
    blt     Load_Move_Animation
    ldr     r0, =FIRST_NEW_MOVE
    cmp     r1, r0
    bge     Load_New_Move_Animation
    movs    r1, #1
    b       Return_LoadAnimationScriptFile
Load_New_Move_Animation:
    sub     r6, #BATTLE_ANIMATIONS_COUNT
Load_Move_Animation:
    movs    r1, #0
Return_LoadAnimationScriptFile:
    movs    r0, #1
    cmp     r1, r0
    pop     {pc}
    .size THUMB_BRANCH_LINK_LoadAnimationScriptFile_0xB6, . - THUMB_BRANCH_LINK_LoadAnimationScriptFile_0xB6

THUMB_BRANCH_LINK_BtlvEffect_LoadAnimationScript_0x1A:
    push    {r6, r7, lr}
    ldr     r3, =0x7FFF
    cmp     r4, r0
    blt     Load_Move_Animation_2
    ldr     r0, =FIRST_NEW_MOVE
    cmp     r4, r0
    bge     Load_Move_Animation_2
    movs    r6, #1
    b       Return_BtlvEffect_LoadAnimationScript
Load_New_Move_Animation_2:
    sub     r4, #BATTLE_ANIMATIONS_COUNT
Load_Move_Animation_2:
    movs    r6, #0
Return_BtlvEffect_LoadAnimationScript:
    movs    r7, #1
    cmp     r6, r7
    pop     {r6, r7, pc}
    .size THUMB_BRANCH_LINK_BtlvEffect_LoadAnimationScript_0x1A, . - THUMB_BRANCH_LINK_BtlvEffect_LoadAnimationScript_0x1A

#endif // EXPAND_MOVES

#if GEN6_CRIT

FULL_COPY_167_0x21D74E0:
    .byte 16
    .byte 8
    .byte 2
    .byte 1
    .byte 1
    .size FULL_COPY_167_0x21D74E0, . - FULL_COPY_167_0x21D74E0

#endif // GEN6_CRIT

#if GEN9_PARALYSIS_SPEED

@ ServerEvent_CalculateSpeed: la rama de paralisis hacia speed = speed * 25 / 100; ahora * 50 / 100
FULL_COPY_ServerEvent_CalculateSpeed_0x80:
    movs    r0, #50
    .size FULL_COPY_ServerEvent_CalculateSpeed_0x80, . - FULL_COPY_ServerEvent_CalculateSpeed_0x80

#endif // GEN9_PARALYSIS_SPEED

#if GEN9_BURN_DAMAGE

@ BattleMon_GetConditionDamage (tabla de saltos por estado): el caso de la quemadura (4) hacia PS maximos / 8; ahora / 16
FULL_COPY_BattleMon_GetConditionDamage_0x62:
    movs    r1, #16
    .size FULL_COPY_BattleMon_GetConditionDamage_0x62, . - FULL_COPY_BattleMon_GetConditionDamage_0x62

#endif // GEN9_BURN_DAMAGE

#if GEN9_BIND_DAMAGE

@ Dano de los ataques que atrapan (PokeLobbyLauncher#112), 1/8 y 1/6 con Banda Atadura (Gen 5: 1/16 y 1/8).
@ Lo calcula Condition_BindSetup (overlay 169) llamando a DivideMaxHPZeroCheck; los FULL_COPY del overlay 169 no se
@ aplican (se carga en 0x0689xxxx por otro camino), asi que se sustituye DivideMaxHPZeroCheck: igual que la original
@ (PS maximos / divisor, minimo 1, con DivideMaxHp) salvo que la llame Condition_BindSetup, que cambia el divisor.
@ El salto de PW2Code al principio de la funcion es "push {lr}; bl aqui; pop {pc}", asi que la vuelta del que
@ llamo esta en la pila: [sp] al entrar, [sp, #8] tras el push de aqui. El .balign hace falta: la tabla de .byte de
@ arriba deja la seccion en direccion impar y el codigo Thumb quedaba desalineado (el juego se colgaba).
.balign 4
.type THUMB_BRANCH_DivideMaxHPZeroCheck, %function
THUMB_BRANCH_DivideMaxHPZeroCheck:
    push    {r4, lr}
    ldr     r2, [sp, #8]
    ldr     r3, =0x689BE5B      @ vuelta a Condition_BindSetup (Thumb)
    cmp     r2, r3
    bne     1f
    cmp     r1, #16
    bne     2f
    movs    r1, #8
    b       1f
2:
    cmp     r1, #8
    bne     1f
    movs    r1, #6
1:
    bl      DivideMaxHp
    cmp     r0, #0
    bne     3f
    movs    r0, #1
3:
    pop     {r4, pc}
    .pool
    .size THUMB_BRANCH_DivideMaxHPZeroCheck, . - THUMB_BRANCH_DivideMaxHPZeroCheck

#endif // GEN9_BIND_DAMAGE

#if GEN9_PROTECT_CHAIN

@ Tabla de HandlerProtectCheckFail (0x21c93dc): N de "exito 1 de cada N" segun usos seguidos (indices 0 a 9). Gen 5: 1,2,4,8,16,32,64,128,0,0.
@ Gen 9: 1, 3, 9, 27, 81, 243 (el maximo de un byte; a partir de ahi se queda en 1/243)
FULL_COPY_167_0x21D991A:
    .byte 1
    .byte 3
    .byte 9
    .byte 27
    .byte 81
    .byte 243
    .byte 243
    .byte 243
    .byte 243
    .byte 243
    .size FULL_COPY_167_0x21D991A, . - FULL_COPY_167_0x21D991A

#endif // GEN9_PROTECT_CHAIN

#if GEN9_CONFUSION_CHANCE

@ ServerFlow_CheckConfusionHitSelf (0x21a3e00): si el Pokemon sigue confuso, IsOccurPer(50) decide si se pega a si mismo. Gen 9: 33 %.
FULL_COPY_ServerFlow_CheckConfusionHitSelf_0x74:
    movs    r0, #33
    .size FULL_COPY_ServerFlow_CheckConfusionHitSelf_0x74, . - FULL_COPY_ServerFlow_CheckConfusionHitSelf_0x74

#endif // GEN9_CONFUSION_CHANCE

#if GEN9_SHEER_COLD_ACCURACY

.type THUMB_BRANCH_LINK_ServerEvent_CheckOHKOHit_0x94, %function

@ ServerEvent_CheckOHKOHit (0x21aa5f4, tirada de los ataques de KO): en +0x94 lee la precision base del ataque con PML_MoveGetParam(ataque, 4)
@ y la suma al nivel de diferencia. Se redirige a Gen9_OHKOAccuracy pasandole tambien el atacante (r6, callee-saved) para poder mirar su tipo.
.balign 4
THUMB_BRANCH_LINK_ServerEvent_CheckOHKOHit_0x94:
    push    {lr}
    movs    r2, r6
    bl      Gen9_OHKOAccuracy
    pop     {pc}
    .size THUMB_BRANCH_LINK_ServerEvent_CheckOHKOHit_0x94, . - THUMB_BRANCH_LINK_ServerEvent_CheckOHKOHit_0x94

#endif // GEN9_SHEER_COLD_ACCURACY
