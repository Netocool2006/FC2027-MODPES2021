/**
 * pes_c17_inyector.c
 * Inyector y despachador en memoria para la build C17
 * Aplica los 6 subsistemas calibrados a instancias vivas del motor Frostbite
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pes_c17_tabla.h"

/* Estructura para registrar punteros resueltos en memoria viva de consola */
typedef struct {
    const char *esquema;
    uintptr_t   direccion_base;
} InstanciaVivaC17;

static InstanciaVivaC17 g_instancias_vivas[] = {
    { "gp_defense_pushpull",                            0 },
    { "gp_rules_foul",                                  0 },
    { "gp_rules_refereestrictness",                     0 },
    { "gp_positioningformation_defensivelinelengthwidth", 0 },
    { "gp_positioningformation_attackinglinelengthwidth", 0 },
    { "gp_playerattribute_attributes",                  0 },
    { "gp_physics_physics",                             0 },
    { "gp_physics_soccerball",                          0 },
    { "gp_goalkeeper_be_a_goalkeeper",                  0 },
    { "gp_actor_movement",                              0 }
};

#define TOTAL_INSTANCIAS_VIVAS (sizeof(g_instancias_vivas) / sizeof(g_instancias_vivas[0]))

uintptr_t c17_obtener_direccion_base(const char *esquema) {
    for (size_t i = 0; i < TOTAL_INSTANCIAS_VIVAS; ++i) {
        if (g_instancias_vivas[i].direccion_base != 0) {
            /* Comparación de esquema */
            const char *a = g_instancias_vivas[i].esquema;
            const char *b = esquema;
            bool igual = true;
            while (*a && *b) {
                if (*a++ != *b++) { igual = false; break; }
            }
            if (igual && *a == '\0' && *b == '\0') {
                return g_instancias_vivas[i].direccion_base;
            }
        }
    }
    return 0;
}

int c17_registrar_instancia_viva(const char *esquema, uintptr_t direccion) {
    for (size_t i = 0; i < TOTAL_INSTANCIAS_VIVAS; ++i) {
        const char *a = g_instancias_vivas[i].esquema;
        const char *b = esquema;
        bool igual = true;
        while (*a && *b) {
            if (*a++ != *b++) { igual = false; break; }
        }
        if (igual && *a == '\0' && *b == '\0') {
            g_instancias_vivas[i].direccion_base = direccion;
            return 1;
        }
    }
    return 0;
}

int c17_aplicar_parches_memoria(void) {
    int parches_aplicados = 0;

    for (size_t i = 0; i < TOTAL_PARCHES_C17; ++i) {
        const EntradaParcheC17 *p = &TABLA_PARCHES_C17[i];
        uintptr_t base = c17_obtener_direccion_base(p->esquema);

        if (base == 0) {
            continue;
        }

        uintptr_t destino = base + p->offset;

        switch (p->tipo) {
            case TIPO_F32:
                *(volatile float *)destino = p->valor.val_f32;
                parches_aplicados++;
                break;
            case TIPO_U32:
                *(volatile uint32_t *)destino = p->valor.val_u32;
                parches_aplicados++;
                break;
            case TIPO_I32:
                *(volatile int32_t *)destino = p->valor.val_i32;
                parches_aplicados++;
                break;
            case TIPO_U8:
                *(volatile uint8_t *)destino = p->valor.val_u8;
                parches_aplicados++;
                break;
        }
    }

    return parches_aplicados;
}
