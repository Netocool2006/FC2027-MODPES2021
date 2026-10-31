#ifndef PES_ARBITRAJE_C17_H
#define PES_ARBITRAJE_C17_H

#include <stdint.h>

/*
 * ==============================================================================
 * SUBSISTEMA 1: SISTEMA ARBITRAL Y FALTAS (C17 - MODO PES 2021)
 * ==============================================================================
 * Define los offsets y valores calibrados para relajar el criterio de faltas
 * hiper-sensible de FC 27 y ajustarlo al forcejeo, ventaja y tarjetas de PES.
 */

/* 1. Forcejeo y Desequilibrio Fisico: gp_defense_pushpull */
#define OFFSET_PUSHPULL_FALL_THRESHOLD            0x00A0  /* f32: FC=0.85 -> PES=0.92 */
#define OFFSET_PUSHPULL_MED_INTENSITY_THRESHOLD   0x00A8  /* f32: FC=0.40 -> PES=0.50 */
#define OFFSET_PUSHPULL_HIGH_INTENSITY_THRESHOLD  0x00B0  /* f32: FC=0.85 -> PES=0.88 */

#define VAL_PUSHPULL_FALL_THRESHOLD_FC            0.85f
#define VAL_PUSHPULL_FALL_THRESHOLD_PES           0.92f

#define VAL_PUSHPULL_MED_INTENSITY_FC             0.40f
#define VAL_PUSHPULL_MED_INTENSITY_PES            0.50f

#define VAL_PUSHPULL_HIGH_INTENSITY_FC            0.85f
#define VAL_PUSHPULL_HIGH_INTENSITY_PES           0.88f

/* 2. Criterio de Faltas y Ley de la Ventaja: gp_rules_foul */
#define OFFSET_ADVANTAGE_WINDOW_TIME_LENGTH       0x0070  /* i32: FC=180  -> PES=210 ticks (~3.5s) */
#define OFFSET_SHOULDER_CHALLENGES_ANGLE_GAP      0x0094  /* f32: FC=115.5 -> PES=135.0 deg */
#define OFFSET_ADVANTAGE_WINDOW_EXTRA_TIME_LENGTH 0x00A8  /* i32: FC=30   -> PES=45 ticks */

#define VAL_ADVANTAGE_WINDOW_TIME_FC              180
#define VAL_ADVANTAGE_WINDOW_TIME_PES             210

#define VAL_SHOULDER_CHALLENGES_ANGLE_FC          115.5f
#define VAL_SHOULDER_CHALLENGES_ANGLE_PES         135.0f

#define VAL_ADVANTAGE_WINDOW_EXTRA_FC             30
#define VAL_ADVANTAGE_WINDOW_EXTRA_PES            45

/* 3. Severidad y Umbrales Disciplinarios: gp_rules_refereestrictness */
#define OFFSET_REFEREE_STRICTNESS_FOUL_INTENSITY  0x0070  /* arr_f32[11]: FC=1.50 -> PES=1.15 */
#define REFEREE_STRICTNESS_INTENSITY_COUNT        11

#define OFFSET_REFEREE_REDCARD_THRESHOLD          0x00FC  /* f32: FC=85.0 -> PES=90.0 */
#define OFFSET_REFEREE_YELLOWCARD_THRESHOLD       0x0118  /* f32: FC=60.5 -> PES=66.0 */

#define VAL_REFEREE_FOUL_INTENSITY_FC             1.50f
#define VAL_REFEREE_FOUL_INTENSITY_PES            1.15f

#define VAL_REFEREE_YELLOWCARD_FC                 60.5f
#define VAL_REFEREE_YELLOWCARD_PES                66.0f

#define VAL_REFEREE_REDCARD_FC                    85.0f
#define VAL_REFEREE_REDCARD_PES                   90.0f

/* Registro de aplicacion de parches */
typedef struct {
    uint32_t aplicados;
    uint32_t fallidos;
    uint32_t modo_pes_activo;
} pes_arbitraje_stats_t;

#endif /* PES_ARBITRAJE_C17_H */
