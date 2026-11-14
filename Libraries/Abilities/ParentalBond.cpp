#include "settings.h"
#if EXPAND_ABILITIES

#include "Patches/BattleUpgrade/include/BattleField.h"

#include "server_flow.h"

// PARENTAL BOND
extern "C" void HandlerParentalBondPower(BattleEventItem * item, ServerFlow * serverFlow, u32 pokemonSlot, u32 * work) {
    if (pokemonSlot == BattleEventVar_GetValue(VAR_ATTACKING_MON) && BattleField_CheckParentalBondFlag()) {
        *work += 1;

        if (*work == 2) {
            *work = 0;
            // Multiplicador en vez de reescribir la potencia: los ataques que calculan su potencia en este mismo
            // evento (Poder Oculto, Retribucion, Giro Bola, Patada Baja...) la volvian a escribir completa y el
            // segundo golpe pegaba entero (PokeLobbyLauncher#108). El multiplicador se aplica al final, como el
            // de las demas habilidades (Megadisparador, Piel Feerica...).
#if GEN6_PARENTAL_BOND
            BattleEventVar_MulValue(VAR_MOVE_POWER_RATIO, 2048);
#else
            BattleEventVar_MulValue(VAR_MOVE_POWER_RATIO, 1024);
#endif
        }
    }
}
BattleEventHandlerTableEntry ParentalBondHandlers[]{
    {EVENT_MOVE_POWER, HandlerParentalBondPower},
    // Double hit implementation in [ServerControl_DamageRoot]
};
extern "C" BattleEventHandlerTableEntry * EventAddParentalBond(u32 * handlerAmount) {
    *handlerAmount = ARRAY_COUNT(ParentalBondHandlers);
    return ParentalBondHandlers;
}
#endif // EXPAND_ABILITIES