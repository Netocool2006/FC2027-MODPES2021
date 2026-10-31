/**
 * pes_balistica_c17.h
 * Modulo de Balistica, Fisicas del Balon y Friccion de Cesped - Build C17
 * Modelo de contacto e inercia segun fisica de PES 2021
 */

#ifndef PES_BALISTICA_C17_H
#define PES_BALISTICA_C17_H

/* Instancia gp_physics_physics (Grupo 0x130, Instancia 0x58) */
#define OFFSET_PHYSICS_FIELD_DYNAMIC_FRICTION  0x3c
#define OFFSET_PHYSICS_FIELD_STATIC_FRICTION   0x68
#define OFFSET_PHYSICS_BALL_BOUNCE_DAMPING     0x34
#define OFFSET_PHYSICS_BALL_RESTITUTION        0x58
#define OFFSET_PHYSICS_BALL_LINEAR_DRAG        0x50

#define VAL_PHYSICS_FIELD_DYNAMIC_FRICTION     0.82f   /* FC: 0.72f -> Mayor frenado en cesped */
#define VAL_PHYSICS_FIELD_STATIC_FRICTION      0.90f   /* FC: 0.82f */
#define VAL_PHYSICS_BALL_BOUNCE_DAMPING        0.985f  /* FC: 0.998f -> Mayor absorcion de rebote */
#define VAL_PHYSICS_BALL_RESTITUTION           0.80f   /* FC: 0.90f -> Menos bote plastico */
#define VAL_PHYSICS_BALL_LINEAR_DRAG           0.0045f /* FC: 0.003f -> Resistencia en el aire */

/* Instancia gp_physics_soccerball (Grupo 0x130, Instancia 0xb0) */
#define OFFSET_SOCCERBALL_MASS                 0x50
#define OFFSET_SOCCERBALL_INERTIA              0x34
#define OFFSET_SOCCERBALL_GRIP_CONSTANT        0x4c

#define VAL_SOCCERBALL_MASS                    0.470f  /* FC: 0.454f -> Balon con mayor peso de cuero */
#define VAL_SOCCERBALL_INERTIA                 0.76f   /* FC: 0.66f -> Inercia organica */
#define VAL_SOCCERBALL_GRIP_CONSTANT           0.85f   /* FC: 0.92f -> Despegue natural pie-balon */

#endif /* PES_BALISTICA_C17_H */
