/**
 * pes_porteros_c17.h
 * Modulo de Guardametas y Paradas Organicas - Build C17
 * Modelo de reacciones y coberturas segun PES 2021
 */

#ifndef PES_PORTEROS_C17_H
#define PES_PORTEROS_C17_H

/* Instancia gp_goalkeeper_be_a_goalkeeper (Grupo 0x160, Instancia 0x148) */
#define OFFSET_GK_AUTO_SAVE_RADIUS_FULL_ASSIST             0x120
#define OFFSET_GK_AUTO_SAVE_RADIUS_PARTIAL_ASSIST          0x11c
#define OFFSET_GK_CROSS_CATCH_EXTENDED_COVERAGE            0x198
#define OFFSET_GK_CROSS_DEFLECT_PUNCH_EXTENDED_COVERAGE    0x138
#define OFFSET_GK_SAVE_REFLEX_COVERAGE_FULL_ASSIST         0x19c
#define OFFSET_GK_CATCH_EXCLUSION_DIFFICULTY_THRESHOLD     0x150

#define VAL_GK_AUTO_SAVE_RADIUS_FULL_ASSIST                2.80f   /* FC: 4.50f -> Menos asistencia magnetica */
#define VAL_GK_AUTO_SAVE_RADIUS_PARTIAL_ASSIST             2.20f   /* FC: 3.50f */
#define VAL_GK_CROSS_CATCH_EXTENDED_COVERAGE               0.95f   /* FC: 1.20f -> Salidas aéreas disputadas */
#define VAL_GK_CROSS_DEFLECT_PUNCH_EXTENDED_COVERAGE       1.35f   /* FC: 1.80f -> Despejes naturales */
#define VAL_GK_SAVE_REFLEX_COVERAGE_FULL_ASSIST            40.0f   /* FC: 60.0f -> Menos cobertura refleja irreal */
#define VAL_GK_CATCH_EXCLUSION_DIFFICULTY_THRESHOLD        0.35f   /* FC: 0.20f -> Mas desvíos en tiros potentes */

#endif /* PES_PORTEROS_C17_H */
