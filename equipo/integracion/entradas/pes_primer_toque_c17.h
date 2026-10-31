/**
 * pes_primer_toque_c17.h
 * Modulo de Primer Toque, Control e Inercia de Conduccion - Build C17
 * Modelo organico de recepcion y control de balon estilo PES 2021
 */

#ifndef PES_PRIMER_TOQUE_C17_H
#define PES_PRIMER_TOQUE_C17_H

/* Instancia gp_actor_movement (Grupo 0x48, Instancia 0x18) */
#define OFFSET_ACTOR_TRAP_BALL_TARGET_TIME_SHORT         0x250
#define OFFSET_ACTOR_TRAP_BALL_TARGET_TIME_LONG          0x3d4
#define OFFSET_ACTOR_DRIBBLE_MIN_TOUCH_INTERRUPT_PENALTY 0x20c
#define OFFSET_ACTOR_DRIBBLE_SPRINT_BALL_TARGET_TIME_RUN 0x214

#define VAL_ACTOR_TRAP_BALL_TARGET_TIME_SHORT           75.0f  /* FC: 65.0f -> Amortiguacion natural */
#define VAL_ACTOR_TRAP_BALL_TARGET_TIME_LONG            125.0f /* FC: 110.0f -> No queda imantado en largo */
#define VAL_ACTOR_DRIBBLE_MIN_TOUCH_INTERRUPT_PENALTY   3.00f  /* FC: 2.00f -> Inercia en cortes secos */
#define VAL_ACTOR_DRIBBLE_SPRINT_BALL_TARGET_TIME_RUN   52.0f  /* FC: 45.0f -> Toque mas despegado en sprint */

#endif /* PES_PRIMER_TOQUE_C17_H */
