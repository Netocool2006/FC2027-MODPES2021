/* ia_loco_consola.h - tipos compartidos del enganche de destino (ia_loco_consola.c) y del productor
 * (ia_productor_consola.c). Agente ia_tactica. */
#pragma once
#include <stdint.h>
#define LOCO_RANURAS 3
#define LOCO_TAM_RANURA 0x40
#define LOCO_MAX_JUG 64u
#define LOCO_LIMITE_CAMPO 600.0f            /* pies: |x|,|z| mayores se rechazan (campo FC ~ 345 x 225 pies) */
/* retorno - fc_loco_candidatos: identifica el sitio sin direcciones absolutas (ASLR) */
#define LOCO_RET_SITIO1 (0x4f22ec0u - 0x4a91d60u)
#define LOCO_RET_SITIO2 (0x53c07bdu - 0x4a91d60u)

typedef struct {
    uint32_t sitio, jugador, usadas, aplicado;   /* usadas: bit i = ranura i con byte +0x28 != 0 */
    float punto[LOCO_RANURAS][3];               /* +0x00 de cada ranura (FC, pies) */
    float desvio[LOCO_RANURAS];                 /* +0x10 */
    float pes[3];                               /* objetivo PES aplicado o en sombra */
} ia_loco_obs;                                  /* 76 B */

typedef struct {
    uint32_t fotograma;                         /* = g_ia_loco.fotograma cuando se escribio */
    uint32_t valido;
    float punto[3];                             /* mundo FC (pies), eje y = alto (se conserva el de FC si y != y) */
    float reservado;
} ia_loco_objetivo;

typedef struct {
    char marca[16];                             /* "PESIALOCO v1" */
    uint32_t activos;                           /* bit0 sitio 1, bit1 sitio 2, bit30 sombra. 0 = solo observar */
    uint32_t fotograma;                         /* lo sube el productor de objetivos PES */
    uint32_t n_total, n_sitio[3];               /* n_sitio[0] = sitio desconocido */
    uint32_t n_aplicados, n_rechazados, n_sin_objetivo, n_sombra;
    uint32_t siguiente;
    uint32_t reservado;
    ia_loco_obs anillo[32];
    ia_loco_objetivo objetivo[LOCO_MAX_JUG];    /* por id de jugador ([[arg6]+0x10]); rango sin medir -> < 64 */
} ia_loco;

extern ia_loco g_ia_loco;
