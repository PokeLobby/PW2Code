.thumb

@ Texto instantaneo del Randomizer (misc.fastestText, patches/instant_text/w2_instant_text.ips) en
@ Negro 2/Blanco 2. El IPS cambia el salto de 0x2022628 de la funcion que escribe el texto en las ventanas
@ para que, tras cada letra, vuelva al principio del bucle y escriba todo el mensaje en el mismo fotograma.
@ Asi se salta el camino normal de los codigos de control, y los mensajes con "esperar boton" (0xBE01)
@ salian en blanco (p. ej. "No tienes suficiente dinero." en la tienda). PokeLobbyLauncher#109.
@
@ Este gancho sustituye ese salto (y el "movs r0, #2" de detras):
@ - siguiente letra = fin (0xFFFF): lo de siempre, r0 = 2 y vuelta a 0x202262C (str r0, [r5]).
@ - ROM sin texto instantaneo (0x2022570 no esta anulado con nops): camino original, 0x2022630.
@ - con texto instantaneo: las letras normales siguen de golpe (0x20225B6, inicio del bucle); los codigos
@   de control (0xF000) y los saltos de linea (r7 = 0xFFFE) van por el camino original.
@ Entra con r0 = 0xFFFF, r1 = siguiente letra, r5 = impresora, r7 = 0xFFFE. Solo toca r0 (r2 se guarda).

.type THUMB_BRANCH_LINK_TextPrinter_Update_0x198, %function
THUMB_BRANCH_LINK_TextPrinter_Update_0x198:
    cmp     r1, r0
    bne     1f
    movs    r0, #2
    bx      lr                  @ 0x202262C: str r0, [r5]; b 0x2022648
1:
    push    {r2}
    ldr     r2, =0x2022570
    ldrh    r2, [r2]
    ldr     r0, =0x46C0         @ nop: el IPS de texto instantaneo esta puesto
    cmp     r2, r0
    bne     2f
    ldr     r0, =0xF000
    cmp     r1, r0
    beq     2f
    cmp     r1, r7
    beq     2f
    pop     {r2}
    ldr     r0, =0x20225B7      @ inicio del bucle (Thumb)
    bx      r0
2:
    pop     {r2}
    ldr     r0, =0x2022631      @ camino original (Thumb)
    bx      r0
    .pool
    .size THUMB_BRANCH_LINK_TextPrinter_Update_0x198, . - THUMB_BRANCH_LINK_TextPrinter_Update_0x198
