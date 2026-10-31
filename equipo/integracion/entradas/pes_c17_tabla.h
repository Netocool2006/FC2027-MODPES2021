/**
 * pes_c17_tabla.h
 * Tabla Maestra Unificada de Parches Estaticos C17 (6 Subsistemas PES 2021)
 *
 * Mapeo de parches directos sobre instancias vivas de gameplay (gp_*)
 */

#ifndef PES_C17_TABLA_H
#define PES_C17_TABLA_H

#include <stdint.h>
#include "pes_arbitraje_c17.h"
#include "pes_ia_bloque_c17.h"
#include "pes_fatiga_c17.h"
#include "pes_balistica_c17.h"
#include "pes_porteros_c17.h"
#include "pes_primer_toque_c17.h"

typedef enum {
    TIPO_F32,
    TIPO_U32,
    TIPO_I32,
    TIPO_U8
} TipoDatoC17;

typedef struct {
    const char   *subsistema;
    const char   *esquema;
    uint32_t      instancia_hash_o_id;
    uint32_t      offset;
    TipoDatoC17   tipo;
    union {
        float    val_f32;
        uint32_t val_u32;
        int32_t  val_i32;
        uint8_t  val_u8;
    } valor;
} EntradaParcheC17;

static const EntradaParcheC17 TABLA_PARCHES_C17[] = {
    /* --- SUBSISTEMA 1: Arbitraje y Faltas --- */
    { "Arbitraje", "gp_defense_pushpull", 0, OFF_PUSHPULL_FALL_THRESHOLD, TIPO_F32, { .val_f32 = VAL_PUSHPULL_FALL_THRESHOLD } },
    { "Arbitraje", "gp_defense_pushpull", 0, OFF_PUSHPULL_MED_INTENSITY,  TIPO_F32, { .val_f32 = VAL_PUSHPULL_MED_INTENSITY } },
    { "Arbitraje", "gp_defense_pushpull", 0, OFF_PUSHPULL_HIGH_INTENSITY, TIPO_F32, { .val_f32 = VAL_PUSHPULL_HIGH_INTENSITY } },
    { "Arbitraje", "gp_rules_foul",       0, OFF_ADVANTAGE_WINDOW_TICKS,  TIPO_I32, { .val_i32 = VAL_ADVANTAGE_WINDOW_TICKS } },
    { "Arbitraje", "gp_rules_foul",       0, OFF_SHOULDER_CHALLENGE_ANGLE_GAP, TIPO_F32, { .val_f32 = VAL_SHOULDER_CHALLENGE_ANGLE_GAP } },
    { "Arbitraje", "gp_rules_refereestrictness", 0, OFF_REF_STRICTNESS_FOUL_INTENSITY, TIPO_F32, { .val_f32 = VAL_REF_STRICTNESS_FOUL_INTENSITY } },
    { "Arbitraje", "gp_rules_refereestrictness", 0, OFF_REF_STRICTNESS_YELLOW_THRESH,  TIPO_F32, { .val_f32 = VAL_REF_STRICTNESS_YELLOW_THRESH } },

    /* --- SUBSISTEMA 2: IA Tactica y Compactacion de Lineas --- */
    { "IA_Bloque", "gp_positioningformation_defensivelinelengthwidth", 0x30, OFF_DEF_LINE_LENGTH_BALANCED,   TIPO_F32, { .val_f32 = VAL_DEF_LINE_LENGTH_BALANCED } },
    { "IA_Bloque", "gp_positioningformation_defensivelinelengthwidth", 0x30, OFF_DEF_LINE_LENGTH_POSSESSION, TIPO_F32, { .val_f32 = VAL_DEF_LINE_LENGTH_POSSESSION } },
    { "IA_Bloque", "gp_positioningformation_defensivelinelengthwidth", 0x30, OFF_DEF_LINE_LENGTH_LONGBALL,   TIPO_F32, { .val_f32 = VAL_DEF_LINE_LENGTH_LONGBALL } },
    { "IA_Bloque", "gp_positioningformation_defensivelinelengthwidth", 0x30, OFF_DEF_LINE_LENGTH_FASTBUILDU, TIPO_F32, { .val_f32 = VAL_DEF_LINE_LENGTH_FASTBUILDU } },
    { "IA_Bloque", "gp_positioningformation_defensivelinelengthwidth", 0x30, OFF_DEF_LINE_WIDTH_MOD,         TIPO_F32, { .val_f32 = VAL_DEF_LINE_WIDTH_MOD } },
    { "IA_Bloque", "gp_positioningformation_defensivelinelengthwidth", 0x30, OFF_DEF_LINE_WIDTH_CB,          TIPO_F32, { .val_f32 = VAL_DEF_LINE_WIDTH_CB } },
    { "IA_Bloque", "gp_positioningformation_attackinglinelengthwidth", 0x28, OFF_ATK_LINE_LENGTH_BALANCED,   TIPO_F32, { .val_f32 = VAL_ATK_LINE_LENGTH_BALANCED } },
    { "IA_Bloque", "gp_positioningformation_attackinglinelengthwidth", 0x28, OFF_ATK_LINE_LENGTH_POSSESSION, TIPO_F32, { .val_f32 = VAL_ATK_LINE_LENGTH_POSSESSION } },
    { "IA_Bloque", "gp_positioningformation_attackinglinelengthwidth", 0x28, OFF_ATK_LINE_WIDTH_MOD,         TIPO_F32, { .val_f32 = VAL_ATK_LINE_WIDTH_MOD } },

    /* --- SUBSISTEMA 3: Fatiga Dinamica --- */
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_DISABLE_CONTROLLED_PLAYER, TIPO_U8,  { .val_u8 = VAL_FATIGA_DISABLE_CONTROLLED_PLAYER } },
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_ST_SLOWDOWN_THRESHOLD,     TIPO_F32, { .val_f32 = VAL_FATIGA_ST_SLOWDOWN_THRESHOLD } },
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_ST_SLOWDOWN_RATE,          TIPO_F32, { .val_f32 = VAL_FATIGA_ST_SLOWDOWN_RATE } },
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_LT_SPEED_FACTOR,             TIPO_F32, { .val_f32 = VAL_FATIGA_LT_SPEED_FACTOR } },
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_FIXED_ST_MODIFIER,           TIPO_F32, { .val_f32 = VAL_FATIGA_FIXED_ST_MODIFIER } },
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_FIXED_LT_MODIFIER,           TIPO_F32, { .val_f32 = VAL_FATIGA_FIXED_LT_MODIFIER } },
    { "Fatiga", "gp_playerattribute_attributes", 0x20, OFF_FATIGA_PRESS_TACTICS_MODIFIER,      TIPO_F32, { .val_f32 = VAL_FATIGA_PRESS_TACTICS_MODIFIER } },

    /* --- SUBSISTEMA 4: Fisica de Balon y Balistica --- */
    { "Balistica", "gp_physics_physics",    0x58, OFFSET_PHYSICS_FIELD_DYNAMIC_FRICTION, TIPO_F32, { .val_f32 = VAL_PHYSICS_FIELD_DYNAMIC_FRICTION } },
    { "Balistica", "gp_physics_physics",    0x58, OFFSET_PHYSICS_FIELD_STATIC_FRICTION,  TIPO_F32, { .val_f32 = VAL_PHYSICS_FIELD_STATIC_FRICTION } },
    { "Balistica", "gp_physics_physics",    0x58, OFFSET_PHYSICS_BALL_BOUNCE_DAMPING,    TIPO_F32, { .val_f32 = VAL_PHYSICS_BALL_BOUNCE_DAMPING } },
    { "Balistica", "gp_physics_physics",    0x58, OFFSET_PHYSICS_BALL_RESTITUTION,       TIPO_F32, { .val_f32 = VAL_PHYSICS_BALL_RESTITUTION } },
    { "Balistica", "gp_physics_physics",    0x58, OFFSET_PHYSICS_BALL_LINEAR_DRAG,       TIPO_F32, { .val_f32 = VAL_PHYSICS_BALL_LINEAR_DRAG } },
    { "Balistica", "gp_physics_soccerball", 0xb0, OFFSET_SOCCERBALL_MASS,                TIPO_F32, { .val_f32 = VAL_SOCCERBALL_MASS } },
    { "Balistica", "gp_physics_soccerball", 0xb0, OFFSET_SOCCERBALL_INERTIA,             TIPO_F32, { .val_f32 = VAL_SOCCERBALL_INERTIA } },
    { "Balistica", "gp_physics_soccerball", 0xb0, OFFSET_SOCCERBALL_GRIP_CONSTANT,        TIPO_F32, { .val_f32 = VAL_SOCCERBALL_GRIP_CONSTANT } },

    /* --- SUBSISTEMA 5: Guardametas --- */
    { "Porteros", "gp_goalkeeper_be_a_goalkeeper", 0x148, OFFSET_GK_AUTO_SAVE_RADIUS_FULL_ASSIST,          TIPO_F32, { .val_f32 = VAL_GK_AUTO_SAVE_RADIUS_FULL_ASSIST } },
    { "Porteros", "gp_goalkeeper_be_a_goalkeeper", 0x148, OFFSET_GK_AUTO_SAVE_RADIUS_PARTIAL_ASSIST,       TIPO_F32, { .val_f32 = VAL_GK_AUTO_SAVE_RADIUS_PARTIAL_ASSIST } },
    { "Porteros", "gp_goalkeeper_be_a_goalkeeper", 0x148, OFFSET_GK_CROSS_CATCH_EXTENDED_COVERAGE,         TIPO_F32, { .val_f32 = VAL_GK_CROSS_CATCH_EXTENDED_COVERAGE } },
    { "Porteros", "gp_goalkeeper_be_a_goalkeeper", 0x148, OFFSET_GK_CROSS_DEFLECT_PUNCH_EXTENDED_COVERAGE, TIPO_F32, { .val_f32 = VAL_GK_CROSS_DEFLECT_PUNCH_EXTENDED_COVERAGE } },
    { "Porteros", "gp_goalkeeper_be_a_goalkeeper", 0x148, OFFSET_GK_SAVE_REFLEX_COVERAGE_FULL_ASSIST,      TIPO_F32, { .val_f32 = VAL_GK_SAVE_REFLEX_COVERAGE_FULL_ASSIST } },
    { "Porteros", "gp_goalkeeper_be_a_goalkeeper", 0x148, OFFSET_GK_CATCH_EXCLUSION_DIFFICULTY_THRESHOLD,  TIPO_F32, { .val_f32 = VAL_GK_CATCH_EXCLUSION_DIFFICULTY_THRESHOLD } },

    /* --- SUBSISTEMA 6: Primer Toque e Inercia --- */
    { "PrimerToque", "gp_actor_movement", 0x18, OFFSET_ACTOR_TRAP_BALL_TARGET_TIME_SHORT,         TIPO_F32, { .val_f32 = VAL_ACTOR_TRAP_BALL_TARGET_TIME_SHORT } },
    { "PrimerToque", "gp_actor_movement", 0x18, OFFSET_ACTOR_TRAP_BALL_TARGET_TIME_LONG,          TIPO_F32, { .val_f32 = VAL_ACTOR_TRAP_BALL_TARGET_TIME_LONG } },
    { "PrimerToque", "gp_actor_movement", 0x18, OFFSET_ACTOR_DRIBBLE_MIN_TOUCH_INTERRUPT_PENALTY, TIPO_F32, { .val_f32 = VAL_ACTOR_DRIBBLE_MIN_TOUCH_INTERRUPT_PENALTY } },
    { "PrimerToque", "gp_actor_movement", 0x18, OFFSET_ACTOR_DRIBBLE_SPRINT_BALL_TARGET_TIME_RUN,   TIPO_F32, { .val_f32 = VAL_ACTOR_DRIBBLE_SPRINT_BALL_TARGET_TIME_RUN } }
};

#define TOTAL_PARCHES_C17 (sizeof(TABLA_PARCHES_C17) / sizeof(TABLA_PARCHES_C17[0]))

#endif /* PES_C17_TABLA_H */
