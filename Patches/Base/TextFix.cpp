#include "settings.h"

// Hack PokeLobby (PokeLobbyLauncher#124): mensajes de la tienda en blanco tras comprar.
//
// La base del hack trae el texto instantáneo: PrintStream_Update (0x2022490) cambia el «bne» de +0x198 para
// pintar la letra siguiente al momento, sin pasar por BmpWin_FlushChar, que es la que copia el cuadro a la
// pantalla. Solo se copiaba al llegar al final del texto. Un mensaje con una pausa a mitad (esperar un botón o
// un sonido: «¡Aquí tienes! ¡Gracias!» de la tienda) se quedaba parado antes del final sin haber copiado nada,
// y el cuadro se veía vacío. Los cuadros del mapa no lo notan porque se refrescan solos cada fotograma.
//
// Aquí, en la llamada de +0x152 que trata los códigos de control (pausas incluidas), se copia primero lo que
// ya está escrito y después se trata el código como siempre. El texto sigue siendo instantáneo.

extern "C" void PrintStream_ProcessControlCode(void* printStream);
extern "C" void BmpWin_FlushChar(void* bmpWin);

extern "C" void THUMB_BRANCH_LINK_PrintStream_Update_0x152(void* printStream) {
    BmpWin_FlushChar(*(void**)((char*)printStream + 8));
    PrintStream_ProcessControlCode(printStream);
}
