/* ia_productor_consola.c - productor por fotograma (54 Hz) de los objetivos PES para el enganche de destino
 * (ia_loco_consola.c). Agente ia_tactica, fase 9. Bloque de CONSOLA (no se llama pes_*.c: lee memoria de FC).
 *
 * Entradas (todas validadas; si falta cualquiera -> no se escribe ningun objetivo = salida NEUTRA):
 *   - mundo = [balon_entidad+0x10]: lo fija pes_ia_prod_fijar_balon(rdi de 0x4ab8e30), que el integrador llama en el
 *     enganche de toques junto a jugador_fc_fijar_balon. Antes del primer toque: neutro.
 *   - jugadores: lista de movimiento (integracion\entradas\jugador_fc.c, DEMOSTRADO): G = [[mundo+0x980]+0x20],
 *     56 ranuras S = G+0x18+k*0x58 (activa si S+0x20 == 1), J = [S+0x18] (id +0, valido (u8)+8, equipo +0xc segun
 *     movimiento -> configurable cfg.off_equipo), Q = [[S+0x08]] (pos +0x1640, vel +0x1650; pies, pies/tick 60 Hz).
 *   - balon: cuerpo rigido del balon real de pes_fc.c (g_ctl+8), posicion en +0x60 (pies). DEMOSTRADO (pesmode).
 * Ejes y unidades: g_ia_prod.cfg (como g_porteros.cfg): escala pies->m, eje largo, eje ancho, signo del ancho,
 *   semilargo/semiancho PES, fps. Sentido de ataque: automatico (equipo con la media de X menor ataca hacia +X;
 *   solo se fija/cambia con los equipos claramente separados, p. ej. en los saques de centro) o fijo (cfg.lado_modo).
 *   - (fase 10) formación: PlayerPosition en J+0x10 (cfg.off_posicion; PROBABLE, ver AVISO_IA_LOCO.md §7): el núcleo
 *     asigna portero/DF/MF/FW con la tabla de FC 0x1164aca0 y el orden lateral por el lado de la posición.
 * Llaves: g_pes.modo != 0 y cargado, g_pes.activos bit 2. La escritura final en FC la decide ia_loco_consola.c
 *   con sus propias llaves (g_ia_loco.activos). */
#include <stdint.h>
#include "pes_estado.h"
#include "ia_loco_consola.h"
#include "pes_ia_productor.h"

extern uint8_t g_ctl[];                 /* pes_fc.c: +8 = cuerpo del balon real */
#define CTL_BALON 8
#define CUERPO_POS 0x60

typedef struct {
    float escala;                       /* pies -> m (0.3048) */
    int32_t eje_largo, eje_ancho;       /* indice del eje de FC (0 x, 2 z) que es la X / la Z de PES */
    float signo_ancho;                  /* +1 / -1 */
    float semilargo_m, semiancho_m;     /* campo PES (52.5 / 34) */
    float fps;                          /* 54 */
    float k_vel;                        /* vel FC (pies/tick60) -> m/paso54: 0.3048*60/54 */
    float radio_posesion_m, alto_posesion_m;
    int32_t off_equipo;                 /* J + off: equipo 0/1 (movimiento: 0xc) */
    int32_t lado_modo;                  /* 0 automatico, 1 fijo: equipo 0 ataca hacia lado0 */
    int32_t lado0;
    float sep_lado_m;                   /* separacion minima de medias para decidir el sentido (8 m) */
    uint8_t nivel[2];                   /* nivel tactico 0..4 */
    int32_t escribir_bloque;            /* 0: no se sustituye el "volver a su sitio" de FC */
    /* fase 10: formación real de FC */
    int32_t usar_formacion;             /* 1 = líneas/portero por la PlayerPosition de J (respaldo: regla por X) */
    int32_t off_posicion;               /* J + off: PlayerPosition 0..27 (0x10, PROBABLE: registro de 0x488 B del equipo) */
    int32_t derecha_z;                  /* signo de la Z propia del lado derecho (+1, PROBABLE) */
} ia_prod_cfg;

typedef struct {
    char marca[16];                     /* "PESIAPROD v1" */
    ia_prod_cfg cfg;
    void *mundo;
    int8_t lado[2], pad[2];
    uint32_t n_ticks, n_neutros, n_escritos, n_sin_mundo, n_sin_balon, n_lista;
    int32_t ultimo_motivo;              /* PES_IA_PROD_* o 100 + motivo de entrada */
    int32_t n_jug[2];                   /* jugadores por equipo del ultimo paso */
    PesIaProdEstado st;
    PesIaProdEntrada ent;
    PesIaProdSalida sal[PES_IA_PROD_MAX];
    float y_fc[PES_IA_PROD_MAX];
} ia_prod;

__attribute__((used, aligned(16))) ia_prod g_ia_prod = {
    .marca = "PESIAPROD v1",
    .cfg = { 0.3048f, 0, 2, 1.0f, 52.5f, 34.0f, 54.0f, 0.33866667f, 1.5f, 2.0f, 0xc, 0, 1, 8.0f, {2, 2}, 0, 1, 0x10, 1 } };

static int ok_ptr(uintptr_t p) { return p >= 0x10000u && p < 0x0000800000000000ull && (p & 7u) == 0; }
static int lee_ptr(uintptr_t b, uintptr_t o, uintptr_t *r) {
    if (!ok_ptr(b)) return 0;
    uintptr_t v = *(const uintptr_t *)(b + o);
    if (!ok_ptr(v)) return 0;
    *r = v; return 1;
}
static int fin(float v) { return v == v && v < 1.0e6f && v > -1.0e6f; }

void pes_ia_prod_fijar_balon(const void *balon) {
    uintptr_t m, sub, g;
    if (!lee_ptr((uintptr_t)balon, 0x10, &m) || !lee_ptr(m, 0x980, &sub) || !lee_ptr(sub, 0x20, &g)) return;
    g_ia_prod.mundo = (void *)m;
}

static PesIaV3 a_pes(const float *v) {
    const ia_prod_cfg *c = &g_ia_prod.cfg;
    int il = c->eje_largo == 2 ? 2 : 0, ia = c->eje_ancho == 0 ? 0 : 2;
    PesIaV3 r = { v[il] * c->escala, v[1] * c->escala, v[ia] * c->escala * c->signo_ancho };
    return r;
}

static int neutro(int motivo) { g_ia_prod.n_neutros++; g_ia_prod.ultimo_motivo = motivo; return 0; }

/* rellena g_ia_prod.ent desde FC; 1 si todo cuadra */
static int leer_fc(void) {
    ia_prod *P = &g_ia_prod; const ia_prod_cfg *c = &P->cfg; PesIaProdEntrada *e = &P->ent;
    uintptr_t sub, g;
    if (!P->mundo) { P->n_sin_mundo++; return neutro(101); }
    if (!lee_ptr((uintptr_t)P->mundo, 0x980, &sub) || !lee_ptr(sub, 0x20, &g)) { P->n_sin_mundo++; return neutro(102); }
    uintptr_t cuerpo = *(const uintptr_t *)(g_ctl + CTL_BALON);
    if (!ok_ptr(cuerpo)) { P->n_sin_balon++; return neutro(103); }
    const float *bp = (const float *)(cuerpo + CUERPO_POS);
    if (!fin(bp[0]) || !fin(bp[1]) || !fin(bp[2])) { P->n_sin_balon++; return neutro(104); }
    e->balon = a_pes(bp); e->balon_valido = 1;
    e->n = 0; P->n_jug[0] = P->n_jug[1] = 0;
    float sx[2] = {0, 0};
    for (uint32_t k = 0; k < 56u; k++) {
        uintptr_t s = g + 0x18 + k * 0x58, j, y, q;
        if (*(const uint8_t *)(s + 0x20) != 1 || !lee_ptr(s, 0x18, &j)) continue;
        if (*(const uint8_t *)(j + 8) != 1) continue;
        if (!lee_ptr(s, 0x08, &y) || !lee_ptr(y, 0, &q) || (q & 15u)) continue;
        int32_t id = *(const int32_t *)j, eq = *(const int32_t *)(j + (uint32_t)c->off_equipo);
        if (id < 0 || (uint32_t)id >= LOCO_MAX_JUG || (uint32_t)eq > 1u) return neutro(105);   /* lista rara: neutro */
        const float *pq = (const float *)(q + 0x1640), *vq = (const float *)(q + 0x1650);
        if (!fin(pq[0]) || !fin(pq[1]) || !fin(pq[2]) || !fin(vq[0]) || !fin(vq[1]) || !fin(vq[2])) return neutro(106);
        if (e->n >= PES_IA_PROD_MAX) return neutro(107);
        PesIaProdJugador *pj = &e->jug[e->n];
        pj->valido = 1; pj->equipo = eq; pj->id = id; pj->pos = a_pes(pq);
        pj->posicion = *(const int32_t *)(j + (uint32_t)c->off_posicion);
        if ((uint32_t)pj->posicion > 27u) pj->posicion = -1;
        PesIaV3 v = a_pes(vq); float kv = c->k_vel / c->escala;   /* a_pes ya aplico la escala */
        pj->vel.x = v.x * kv; pj->vel.y = v.y * kv; pj->vel.z = v.z * kv;
        P->y_fc[e->n] = pq[1];
        sx[eq] += pj->pos.x; P->n_jug[eq]++; e->n++;
    }
    P->n_lista++;
    if (P->n_jug[0] < 7 || P->n_jug[1] < 7) return neutro(108);
    /* sentido de ataque */
    if (c->lado_modo == 1) { P->lado[0] = (int8_t)(c->lado0 < 0 ? -1 : 1); P->lado[1] = (int8_t)-P->lado[0]; }
    else {
        float m0 = sx[0] / (float)P->n_jug[0], m1 = sx[1] / (float)P->n_jug[1], d = m1 - m0;
        if (d > c->sep_lado_m) { P->lado[0] = 1; P->lado[1] = -1; }
        else if (d < -c->sep_lado_m) { P->lado[0] = -1; P->lado[1] = 1; }
    }
    e->lado[0] = P->lado[0]; e->lado[1] = P->lado[1];
    e->perfil = g_pes.modo == 2 ? 1 : 0;
    e->nivel[0] = c->nivel[0] > 4 ? 2 : c->nivel[0]; e->nivel[1] = c->nivel[1] > 4 ? 2 : c->nivel[1];
    e->fps = c->fps; e->semilargo = c->semilargo_m; e->semiancho = c->semiancho_m;
    e->radio_posesion = c->radio_posesion_m; e->alto_posesion = c->alto_posesion_m; e->escribir_bloque = c->escribir_bloque;
    e->usar_formacion = c->usar_formacion; e->derecha_z = (int8_t)(c->derecha_z < 0 ? -1 : 1);
    return 1;
}

/* un paso de 1/54 s (pes_tick_54_n lo llama n veces) */
void pes_ia_prod_tick54(void) {
    ia_prod *P = &g_ia_prod; const ia_prod_cfg *c = &P->cfg;
    if (g_pes.modo == 0 || g_pes.perfil_cargado != g_pes.modo || !(g_pes.activos & 4u)) return;
    P->n_ticks++;
    g_ia_loco.fotograma++;                       /* todo objetivo anterior queda viejo: neutro salvo que se escriba */
    if (!(c->escala > 0.1f && c->escala < 1.0f) || (c->signo_ancho != 1.0f && c->signo_ancho != -1.0f) || !(c->k_vel > 0.0f && c->k_vel < 2.0f) ||
        (uint32_t)c->off_equipo > 0x1000u ||
        (uint32_t)c->off_posicion > 0x484u || (c->off_posicion & 3)) { neutro(109); return; }
    if (!leer_fc()) return;
    int n = pes_ia_producir(&P->ent, &P->st, P->sal);
    if (n <= 0) { neutro(P->st.ultimo_motivo); return; }
    int il = c->eje_largo == 2 ? 2 : 0, ia = c->eje_ancho == 0 ? 0 : 2;
    for (int k = 0; k < P->ent.n && k < PES_IA_PROD_MAX; k++) {
        const PesIaProdSalida *s = &P->sal[k];
        if (!s->valido || (uint32_t)s->id >= LOCO_MAX_JUG) continue;
        float f[3];
        f[il] = s->obj.punto.x / c->escala;
        f[ia] = s->obj.punto.z / (c->escala * c->signo_ancho);
        f[1] = P->y_fc[k];
        if (!fin(f[0]) || !fin(f[1]) || !fin(f[2])) continue;
        ia_loco_objetivo *o = &g_ia_loco.objetivo[s->id];
        o->punto[0] = f[0]; o->punto[1] = f[1]; o->punto[2] = f[2];
        o->reservado = s->obj.ritmo;
        o->valido = 1; o->fotograma = g_ia_loco.fotograma;
        P->n_escritos++;
    }
    P->ultimo_motivo = 0;
}
