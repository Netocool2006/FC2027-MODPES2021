/**
 * pes_arbitraje_c17.h
 * Definicion de parches y offsets para el Subsistema 1: Faltas y Arbitraje (C17)
 */
#ifndef PES_ARBITRAJE_C17_H
#define PES_ARBITRAJE_C17_H

#include <stdint.h>

/* gp_defense_pushpull (grupo: gp_defense) */
#define OFF_PUSHPULL_FALL_THRESHOLD        0x00a0 /* f32: FC=0.85f -> PES=0.92f */
#define OFF_PUSHPULL_MED_INTENSITY         0x00a8 /* f32: FC=0.40f -> PES=0.50f */
#define OFF_PUSHPULL_HIGH_INTENSITY        0x00b0 /* f32: FC=0.85f -> PES=0.88f */

#define VAL_PUSHPULL_FALL_THRESHOLD        0.92f
#define VAL_PUSHPULL_MED_INTENSITY         0.50f
#define VAL_PUSHPULL_HIGH_INTENSITY        0.88f

/* gp_rules_foul (grupo: gp_rules) */
#define OFF_ADVANTAGE_WINDOW_TICKS         0x0070 /* i32: FC=180 -> PES=210 (~3.5 s a 60 fps) */
#define OFF_SHOULDER_CHALLENGE_ANGLE_GAP   0x0094 /* f32: FC=115.5f -> PES=135.0f */

#define VAL_ADVANTAGE_WINDOW_TICKS         210
#define VAL_SHOULDER_CHALLENGE_ANGLE_GAP   135.0f

/* gp_rules_refereestrictness (grupo: gp_rules) */
#define OFF_REF_STRICTNESS_FOUL_INTENSITY  0x0070 /* arr_f32[11]: FC=1.5f -> PES=1.15f */
#define OFF_REF_STRICTNESS_YELLOW_THRESH   0x0118 /* f32: FC=60.5f -> PES=66.0f */

#define VAL_REF_STRICTNESS_FOUL_INTENSITY  1.15f
#define VAL_REF_STRICTNESS_YELLOW_THRESH   66.0f

#endif /* PES_ARBITRAJE_C17_H */
