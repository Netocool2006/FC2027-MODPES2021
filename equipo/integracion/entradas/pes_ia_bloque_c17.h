/**
 * pes_ia_bloque_c17.h
 * Moduladores de longitud y anchura del bloque tactico para C17 (Modo PES 2021)
 */
#ifndef PES_IA_BLOQUE_C17_H
#define PES_IA_BLOQUE_C17_H

#include <stdint.h>

/* gp_positioningformation_defensivelinelengthwidth (grupo: gp_positioningformation, inst: 0x30) */
#define OFF_DEF_LINE_LENGTH_BALANCED       0x0030 /* f32: FC=1.00f -> PES=0.72f */
#define OFF_DEF_LINE_LENGTH_POSSESSION     0x0050 /* f32: FC=1.00f -> PES=0.72f */
#define OFF_DEF_LINE_LENGTH_LONGBALL       0x002c /* f32: FC=1.00f -> PES=0.75f */
#define OFF_DEF_LINE_LENGTH_FASTBUILDU     0x0054 /* f32: FC=1.00f -> PES=0.75f */
#define OFF_DEF_LINE_WIDTH_MOD             0x0028 /* f32: FC=1.00f -> PES=0.88f */
#define OFF_DEF_LINE_WIDTH_CB              0x003c /* f32: FC=1.00f -> PES=0.85f */

#define VAL_DEF_LINE_LENGTH_BALANCED       0.72f
#define VAL_DEF_LINE_LENGTH_POSSESSION     0.72f
#define VAL_DEF_LINE_LENGTH_LONGBALL       0.75f
#define VAL_DEF_LINE_LENGTH_FASTBUILDU     0.75f
#define VAL_DEF_LINE_WIDTH_MOD             0.88f
#define VAL_DEF_LINE_WIDTH_CB              0.85f

/* gp_positioningformation_attackinglinelengthwidth (grupo: gp_positioningformation, inst: 0x28) */
#define OFF_ATK_LINE_LENGTH_BALANCED       0x0030 /* f32: FC=0.84f -> PES=0.72f */
#define OFF_ATK_LINE_LENGTH_POSSESSION     0x0050 /* f32: FC=0.80f -> PES=0.70f */
#define OFF_ATK_LINE_WIDTH_MOD             0x0028 /* f32: FC=1.00f -> PES=0.90f */

#define VAL_ATK_LINE_LENGTH_BALANCED       0.72f
#define VAL_ATK_LINE_LENGTH_POSSESSION     0.70f
#define VAL_ATK_LINE_WIDTH_MOD             0.90f

#endif /* PES_IA_BLOQUE_C17_H */
