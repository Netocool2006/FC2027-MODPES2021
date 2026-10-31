/*
 * jugador_fc.c — id de jugador en el partido (ecx de 0x4ab8e30) -> posición, velocidad, rumbo y atributos en FC 27.
 * Agente: movimiento. Código propio (sin código descompilado). Eboot PS5 v1.0.3, direcciones VA del eboot.
 *
 * CAMINO (análisis estático, 2026-10-24; detalles en equipo\movimiento\TRADUCCION_FC.md §0.ter):
 *   balon  = rdi de 0x4ab8e30 (BallEntity_AplicarToque); id = ecx                       DEMOSTRADO
 *   mundo  = [balon+0x10]                                                                 DEMOSTRADO (0x4abea60)
 *   G      = [[mundo+0x980]+0x20]   (G+0x1360 = índice de entidades, accesor 0x4a8a360)   DEMOSTRADO
 *   ranura S_k = G + 0x18 + k*0x58, k = 0..55 (bucle de 0x4ce8070 / 0x4ce2c90)            DEMOSTRADO
 *     S+0x20 (u8)  == 1 -> ranura activa                                                  DEMOSTRADO
 *     J = [S+0x18]: J+0 = id (int), (u8)J+8 == 1 -> jugador válido                        DEMOSTRADO (uso)
 *     L = [S+0x10] = LOCO (rdi del getter 0x4cb0e80; 0x4a4d070 lo toma así)               DEMOSTRADO
 *     Q = [[S+0x08]] = objeto jugador de la IA (0x4a4d070, 0x4e7d1f0, acciones GK)        DEMOSTRADO
 *     P = [S+0x28] (P+0xb68 = componente de animación, accesor 0x49eb9f0)                 DEMOSTRADO (no se usa)
 *   J = [LOCO+0x18] (medido en consola), así que se exige [L+0x18] == J                   DEMOSTRADO + comprobación
 *   Q+0x1640 vec4 posición (x largo, y alto, z ancho; pies)                               DEMOSTRADO (uso, porteros)
 *   Q+0x1650 vec4 velocidad (pies/tick, 60 Hz): FC predice pos + 15*Q+0x1650 (0x48d9d77)   PROBABLE alto
 *   Q+0x17e0 float rumbo (FC lo usa para construir una dirección cuando no usa Q+0x1650)  PROBABLE (unidad: rad)
 *   J+0xf4+4*i atributo i (1..99) en el orden del enum PlayerAttribute                    DEMOSTRADO (consola)
 *
 * "mundo" no tiene global conocido (DESCONOCIDO): se toma del enganche existente de 0x4ab8e30
 * (jugador_fc_fijar_balon(rdi) al entrar) o, si se prefiere, de cualquier sitio que ya tenga G.
 *
 * Todas las lecturas comprueban puntero no nulo, alineación y rango de usuario; los valores se validan
 * (finitos y dentro del campo). Si algo no cuadra, la función devuelve 0 y no escribe nada.
 * Límite: no se puede capturar un fallo de página dentro del juego; las comprobaciones lo evitan para punteros
 * nulos/basura típicos, pero un objeto ya liberado con puntero plausible no es detectable aquí.
 */
#include <stdint.h>
#define memcpy(d, s, n) __builtin_memcpy((d), (s), (n))   /* tamaños fijos: se expande en línea */

/* ---- desplazamientos (cambiar aquí si la medición de confirmación m7 dice otra cosa) ---- */
#define FC_BALON_MUNDO      0x10
#define FC_MUNDO_SUB        0x980
#define FC_SUB_GESTOR       0x20
#define FC_GESTOR_RANURA0   0x18
#define FC_RANURA_PASO      0x58
#define FC_RANURAS          56
#define FC_RANURA_IA        0x08   /* -> Y; [Y] = Q */
#define FC_RANURA_LOCO      0x10
#define FC_RANURA_JUG       0x18
#define FC_RANURA_ACTIVA    0x20
#define FC_LOCO_JUG         0x18
#define FC_JUG_ID           0x00
#define FC_JUG_VALIDO       0x08
#define FC_JUG_ATRIB        0xf4
#define FC_N_ATRIB          34     /* 0 Acceleration .. 33 GkPositioning (LOCO+0x120..0x1a4); PROBABLE */
#define FC_Q_POS            0x1640
#define FC_Q_VEL            0x1650
#define FC_Q_RUMBO          0x17e0

/* límites de plausibilidad (pies, pies/tick) */
#define FC_MAX_X   450.0f   /* campo ~345 x 223 pies; margen amplio */
#define FC_MAX_Z   300.0f
#define FC_MIN_Y   -10.0f
#define FC_MAX_Y    30.0f
#define FC_MAX_V     2.0f   /* 2 pies/tick = 120 pies/s; un sprint es ~0.5 */
#define FC_MAX_RUMBO 7.0f   /* |ángulo| <= 2*pi con margen */

typedef struct { float x, y, z, w; } fc_vec4;

static void *fc_mundo_cache;   /* [balon+0x10] del último toque */

static int ptr_ok(uintptr_t p, uintptr_t alin)
{
    return p >= 0x10000u && p < 0x0000800000000000ull && (p & (alin - 1)) == 0;
}

static int leer_ptr(uintptr_t base, uintptr_t ofs, uintptr_t *out)
{
    uintptr_t v;
    if (!ptr_ok(base, 8)) return 0;
    memcpy(&v, (const void *)(base + ofs), sizeof v);
    if (!ptr_ok(v, 8)) return 0;
    *out = v;
    return 1;
}

static int finito(float f)
{
    uint32_t b;
    memcpy(&b, &f, 4);
    return (b & 0x7f800000u) != 0x7f800000u;
}

static float fabs_f(float f) { return f < 0 ? -f : f; }

/* Llamar al entrar en el enganche de 0x4ab8e30 con su rdi (balón). Devuelve 1 si el mundo parece válido. */
int jugador_fc_fijar_balon(const void *balon)
{
    uintptr_t m, sub, g;
    if (!leer_ptr((uintptr_t)balon, FC_BALON_MUNDO, &m)) return 0;
    if (!leer_ptr(m, FC_MUNDO_SUB, &sub) || !leer_ptr(sub, FC_SUB_GESTOR, &g)) return 0;
    fc_mundo_cache = (void *)m;
    return 1;
}

/* Busca la ranura del id. Devuelve 1 y rellena J, LOCO y Q. */
static int fc_ranura(const void *mundo, int32_t id, uintptr_t *pj, uintptr_t *pl, uintptr_t *pq)
{
    uintptr_t sub, g, k;
    if (id < 0) return 0;
    if (!leer_ptr((uintptr_t)mundo, FC_MUNDO_SUB, &sub) || !leer_ptr(sub, FC_SUB_GESTOR, &g)) return 0;
    for (k = 0; k < FC_RANURAS; k++) {
        uintptr_t s = g + FC_GESTOR_RANURA0 + k * FC_RANURA_PASO, j, l, y, q, lj;
        int32_t jid;
        uint8_t act, val;
        memcpy(&act, (const void *)(s + FC_RANURA_ACTIVA), 1);
        if (act != 1 || !leer_ptr(s, FC_RANURA_JUG, &j)) continue;
        memcpy(&jid, (const void *)(j + FC_JUG_ID), 4);
        if (jid != id) continue;
        memcpy(&val, (const void *)(j + FC_JUG_VALIDO), 1);
        if (val != 1) return 0;
        if (!leer_ptr(s, FC_RANURA_LOCO, &l) || !leer_ptr(l, FC_LOCO_JUG, &lj) || lj != j) return 0;
        if (!leer_ptr(s, FC_RANURA_IA, &y) || !leer_ptr(y, 0, &q) || !ptr_ok(q, 16)) return 0;
        *pj = j; *pl = l; *pq = q;
        return 1;
    }
    return 0;
}

/*
 * jugador_fc_en: con mundo explícito ([balon+0x10]). pos/vel en pies y pies/tick (FC, 60 Hz), rumbo tal cual
 * (Q+0x17e0), atributos enteros 1..99 (hasta n_atrib, máx. FC_N_ATRIB). Cualquier salida puede ser NULL.
 * Devuelve 1 si todo cuadra; 0 si no (sin escribir nada).
 */
int jugador_fc_en(const void *mundo, int32_t id, fc_vec4 *pos, fc_vec4 *vel, float *rumbo,
                  int32_t *atributos, int n_atrib)
{
    uintptr_t j, l, q;
    fc_vec4 p, v;
    float r;
    int32_t at[FC_N_ATRIB];
    int i, n;

    if (!fc_ranura(mundo, id, &j, &l, &q)) return 0;
    memcpy(&p, (const void *)(q + FC_Q_POS), sizeof p);
    memcpy(&v, (const void *)(q + FC_Q_VEL), sizeof v);
    memcpy(&r, (const void *)(q + FC_Q_RUMBO), sizeof r);
    if (!finito(p.x) || !finito(p.y) || !finito(p.z)) return 0;
    if (fabs_f(p.x) > FC_MAX_X || fabs_f(p.z) > FC_MAX_Z || p.y < FC_MIN_Y || p.y > FC_MAX_Y) return 0;
    if (vel && (!finito(v.x) || !finito(v.y) || !finito(v.z) ||
                fabs_f(v.x) > FC_MAX_V || fabs_f(v.y) > FC_MAX_V || fabs_f(v.z) > FC_MAX_V)) return 0;
    if (rumbo && (!finito(r) || fabs_f(r) > FC_MAX_RUMBO)) return 0;
    n = n_atrib < 0 ? 0 : (n_atrib > FC_N_ATRIB ? FC_N_ATRIB : n_atrib);
    if (atributos && n > 0) {
        for (i = 0; i < n; i++) memcpy(&at[i], (const void *)(j + FC_JUG_ATRIB + 4 * (uintptr_t)i), 4);
        for (i = 0; i < n; i++) if (at[i] < 1 || at[i] > 99) return 0;
    }
    if (pos) *pos = p;
    if (vel) *vel = v;
    if (rumbo) *rumbo = r;
    if (atributos) for (i = 0; i < n; i++) atributos[i] = at[i];
    (void)l;
    return 1;
}

/* Firma pedida: usa el mundo guardado por jugador_fc_fijar_balon. */
int jugador_fc(int32_t id, fc_vec4 *pos, fc_vec4 *vel, float *rumbo, int32_t *atributos, int n_atrib)
{
    if (!fc_mundo_cache) return 0;
    return jugador_fc_en(fc_mundo_cache, id, pos, vel, rumbo, atributos, n_atrib);
}

/* Para quien ya tenga Q (p. ej. acciones de portero: Q = **(acción+0x18)). */
int jugador_fc_desde_q(const void *q, fc_vec4 *pos, fc_vec4 *vel, float *rumbo)
{
    fc_vec4 p, v;
    float r;
    if (!ptr_ok((uintptr_t)q, 16)) return 0;
    memcpy(&p, (const char *)q + FC_Q_POS, sizeof p);
    memcpy(&v, (const char *)q + FC_Q_VEL, sizeof v);
    memcpy(&r, (const char *)q + FC_Q_RUMBO, sizeof r);
    if (!finito(p.x) || !finito(p.y) || !finito(p.z) ||
        fabs_f(p.x) > FC_MAX_X || fabs_f(p.z) > FC_MAX_Z || p.y < FC_MIN_Y || p.y > FC_MAX_Y) return 0;
    if (vel && (!finito(v.x) || !finito(v.z) || fabs_f(v.x) > FC_MAX_V || fabs_f(v.z) > FC_MAX_V)) return 0;
    if (rumbo && (!finito(r) || fabs_f(r) > FC_MAX_RUMBO)) return 0;
    if (pos) *pos = p;
    if (vel) *vel = v;
    if (rumbo) *rumbo = r;
    return 1;
}
