.thumb

#include "settings.h"

#if PERFECT_TRAINERS

FULL_COPY_TrainerUtil_LoadParty_0x3E:
    movs    r1, #0xC0
    .size FULL_COPY_TrainerUtil_LoadParty_0x3E, . - FULL_COPY_TrainerUtil_LoadParty_0x3E

FULL_COPY_TrainerUtil_LoadParty_0x4BA:
#if TRAINER_EVS
    ldr     r0, [sp,#0x10]  // trainer id (5th argument of the hook, on the stack)
    str     r0, [sp,#0]
    nop
#else
    nop
    nop
    nop
#endif
    ldr     r0, [sp,#0x14]  // PokemonParty
    movs    r1, r4          // PartyPkm
    ldr     r2, [sp,#0x70]  // TrainerData
    ldr     r3, [sp,#0x68]  // TrainerTeamData
    .size FULL_COPY_TrainerUtil_LoadParty_0x4BA, . - FULL_COPY_TrainerUtil_LoadParty_0x4BA

#endif // PERFECT_TRAINERS