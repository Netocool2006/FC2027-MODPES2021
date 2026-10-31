/* ia_loco_consola.c - Enganche de la IA tactica PES sobre la decision de destino de movimiento de FC (agente ia_tactica,
 * fase 8). Para el bloque de consola del integrador (equipo\integracion\consola\). NO se llama pes_*.c a proposito:
 * compilar_todo.sh / probar_todo.py no deben recogerlo (usa un simbolo de FC y no es una funcion pura).
 *
 * QUE SE ENGANCHA (analisis estatico, ver equipo\integracion\AVISO_IA_LOCO.md):
 *   0x4a91d60 = "fc_loco_candidatos": rellena 3 ranuras de 64 B de destino candidato de un jugador (salida = 13.er
 *   argumento, 7.o de pila). Ranura: +0x00 punto (x,y,z,w; pies), +0x10 desvio angular [0,pi], +0x14/+0x18/+0x1c/
 *   +0x20/+0x24/+0x30 parametros, +0x28 byte "ranura usada", +0x2c = 0, +0x3c id del jugador ([[arg6]+0x10]).
 *   Llamada desde 2 sitios (rel32):
 *     sitio 1  0x4f22ebb  e8 a0 ee b6 ff   en 0x4f21910 (agente por jugador: lee gp_defense; salida = agente+0x10)
 *     sitio 2  0x53c07b8  e8 a3 15 6d ff   en 0x53c02d0 (salida = 7.o argumento de pila del llamador)
 *   PROBABLE: es la eleccion de destino de la IA de un jugador (fase de decision, ANTES de LocoTargetGS);
 *   DESCONOCIDO: si corre solo para jugadores de la CPU y si su punto acaba en LocoTargetGS.TargetPosition.
 *   Por eso: salida NEUTRA por defecto (solo observar) y la sustitucion exige 3 llaves (ver abajo).
 *
 * Enganche: se cambia el rel32 de las llamadas para que vayan a pes_ia_loco_entrada, que llama al ORIGINAL con los
 *   mismos argumentos (registros intactos y copia de los 7 argumentos de pila), y DESPUES llama a pes_ia_loco_registro
 *   (observar y, si todo esta activado, sustituir el punto por el objetivo PES del jugador).
 *
 * Llaves de la sustitucion (las tres; por defecto ninguna):
 *   1) g_pes.modo != 0 y g_pes.perfil_cargado == g_pes.modo (modo PES elegido y cargado);
 *   2) g_pes.activos bit 2 (IA tactica / LocoTargetGS, ya reservado en pes_estado.h);
 *   3) g_ia_loco.activos bit (sitio-1) (bit 0 sitio 1, bit 1 sitio 2).
 *   y ademas un objetivo PES FRESCO para ese jugador: g_ia_loco.objetivo[j].fotograma == g_ia_loco.fotograma, finito y
 *   dentro del campo. Nadie rellena hoy esa tabla (falta medir ejes/unidades: peticiones ia-1/ia-7), asi que aun con
 *   todas las llaves la salida sigue siendo la de FC. El productor (gestor PES por fotograma) solo tiene que escribir
 *   objetivo[j] y subir g_ia_loco.fotograma.
 *   Bit 30 de g_ia_loco.activos = "sombra": anota en el anillo lo que se habria escrito, sin escribir. */
#include <stdint.h>
#include "pes_estado.h"

extern void fc_loco_candidatos(void);       /* 0x4a91d60 (simbolo del enlazador: regiones.json simbolos_fc) */

#include "ia_loco_consola.h"

__attribute__((used, aligned(16))) ia_loco g_ia_loco = { .marca = "PESIALOCO v1" };

static int loco_finito(float v) { return v == v && v < 3.0e38f && v > -3.0e38f; }

/* salida: 3 ranuras; id: arg6 (objeto con puntero al id en +0x10); ret: direccion de retorno del sitio de llamada */
void pes_ia_loco_registro(uint8_t *salida, const uint8_t *id, uintptr_t ret) {
    uintptr_t d = ret - (uintptr_t)&fc_loco_candidatos;
    uint32_t sitio = d == LOCO_RET_SITIO1 ? 1u : d == LOCO_RET_SITIO2 ? 2u : 0u;
    g_ia_loco.n_total++; g_ia_loco.n_sitio[sitio]++;
    if (!salida) return;
    uint32_t jug = 0xffffffffu;
    if (id) { const uint32_t *p = *(const uint32_t *const *)(id + 0x10); if (p) jug = *p; }
    ia_loco_obs *o = &g_ia_loco.anillo[g_ia_loco.siguiente++ & 31u];
    o->sitio = sitio; o->jugador = jug; o->usadas = 0; o->aplicado = 0;
    for (int r = 0; r < LOCO_RANURAS; r++) {
        const uint8_t *s = salida + r * LOCO_TAM_RANURA;
        const float *pt = (const float *)s;
        if (s[0x28]) o->usadas |= 1u << r;
        o->punto[r][0] = pt[0]; o->punto[r][1] = pt[1]; o->punto[r][2] = pt[2];
        o->desvio[r] = *(const float *)(s + 0x10);
    }
    o->pes[0] = o->pes[1] = o->pes[2] = 0.0f;
    /* ---- llaves ---- */
    if (sitio == 0u || g_pes.modo == 0 || g_pes.perfil_cargado != g_pes.modo) return;
    if (!(g_pes.activos & 4u)) return;
    uint32_t sombra = g_ia_loco.activos & 0x40000000u;
    if (!(g_ia_loco.activos & ((1u << (sitio - 1u)) | 0x40000000u))) return;
    if (jug >= LOCO_MAX_JUG || !o->usadas) { g_ia_loco.n_sin_objetivo++; return; }
    const ia_loco_objetivo *ob = &g_ia_loco.objetivo[jug];
    if (!ob->valido || ob->fotograma != g_ia_loco.fotograma) { g_ia_loco.n_sin_objetivo++; return; }
    float x = ob->punto[0], y = ob->punto[1], z = ob->punto[2];
    if (!loco_finito(x) || !loco_finito(z) || x > LOCO_LIMITE_CAMPO || x < -LOCO_LIMITE_CAMPO ||
        z > LOCO_LIMITE_CAMPO || z < -LOCO_LIMITE_CAMPO) { g_ia_loco.n_rechazados++; return; }
    o->pes[0] = x; o->pes[1] = y; o->pes[2] = z;
    if (!(g_ia_loco.activos & (1u << (sitio - 1u))) || sombra) { g_ia_loco.n_sombra++; return; }
    /* sustitucion: el punto de cada ranura usada pasa a ser el objetivo PES (y y w de FC si y no es finita) */
    for (int r = 0; r < LOCO_RANURAS; r++) {
        if (!(o->usadas & (1u << r))) continue;
        float *pt = (float *)(salida + r * LOCO_TAM_RANURA);
        pt[0] = x; pt[2] = z; if (loco_finito(y)) pt[1] = y;
    }
    o->aplicado = 1; g_ia_loco.n_aplicados++;
}

/* Entrada de los sitios 1 y 2 (rel32 de la llamada -> aqui). Al entrar: rsp = 8 (mod 16), [rsp] retorno,
 * [rsp+8..+0x38] = 7 argumentos de pila (el ultimo, +0x38, es la salida). No se toca ningun registro de argumento
 * (rdi, rsi, rdx, rcx, r8, r9, xmm0..xmm7, al) antes de la llamada al original. rbx/r12 (preservados por el original)
 * guardan salida y arg6 para despues. Tras el original se conservan rax, rdx, xmm0, xmm1 (por si devolviera algo). */
__attribute__((naked, used)) void pes_ia_loco_entrada(void) {
    __asm__(
        "push %rbp\n\tmov %rsp, %rbp\n\t"
        "push %rbx\n\tpush %r12\n\t"                     /* rsp = 0 (mod 16) */
        "mov 0x40(%rbp), %rbx\n\t"                       /* salida (7.o argumento de pila) */
        "mov %r9, %r12\n\t"                              /* arg6: objeto con el id del jugador */
        "sub $8, %rsp\n\t"                               /* 8 + 7*8 = 64 -> rsp = 0 (mod 16) en la llamada */
        "pushq 0x40(%rbp)\n\tpushq 0x38(%rbp)\n\tpushq 0x30(%rbp)\n\tpushq 0x28(%rbp)\n\t"
        "pushq 0x20(%rbp)\n\tpushq 0x18(%rbp)\n\tpushq 0x10(%rbp)\n\t"
        "call fc_loco_candidatos\n\t"
        "add $0x40, %rsp\n\t"
        "sub $0x30, %rsp\n\t"                            /* rsp = 0 (mod 16) */
        "mov %rax, 0x00(%rsp)\n\tmov %rdx, 0x08(%rsp)\n\t"
        "movdqu %xmm0, 0x10(%rsp)\n\tmovdqu %xmm1, 0x20(%rsp)\n\t"
        "mov %rbx, %rdi\n\tmov %r12, %rsi\n\tmov 8(%rbp), %rdx\n\t"
        "call pes_ia_loco_registro\n\t"
        "mov 0x00(%rsp), %rax\n\tmov 0x08(%rsp), %rdx\n\t"
        "movdqu 0x10(%rsp), %xmm0\n\tmovdqu 0x20(%rsp), %xmm1\n\t"
        "add $0x30, %rsp\n\t"
        "pop %r12\n\tpop %rbx\n\tpop %rbp\n\t"
        "ret");
}
