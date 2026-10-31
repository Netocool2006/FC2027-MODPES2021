/**
 * pes_fatiga_c17.h
 * Curva de Fatiga y Resistencia Dinamica para C17 (Modo PES 2021)
 *
 * Mapeo extraido de gp_playerattribute_attributes (grupo: gp_playerattribute, offset_instancia: 0x20)
 * Catalogo Frostbite FC 27.
 */
#ifndef PES_FATIGA_C17_H
#define PES_FATIGA_C17_H

#include <stdint.h>
#include <stdbool.h>

/* Instancia gp_playerattribute_attributes en grupo gp_playerattribute (offset 0x20) */
#define OFF_FATIGA_DISABLE_CONTROLLED_PLAYER   0x0101 /* bool: FC=true (0x01) -> PES=false (0x00) */
#define OFF_FATIGA_ST_SLOWDOWN_THRESHOLD       0x00e8 /* f32:  FC=0.84f        -> PES=0.70f       */
#define OFF_FATIGA_ST_SLOWDOWN_RATE            0x00e0 /* f32:  FC=0.25f        -> PES=0.38f       */
#define OFF_FATIGA_LT_SPEED_FACTOR             0x00ec /* f32:  FC=0.95f        -> PES=0.85f       */
#define OFF_FATIGA_FIXED_ST_MODIFIER           0x00c0 /* f32:  FC=0.95f        -> PES=1.15f       */
#define OFF_FATIGA_FIXED_LT_MODIFIER           0x00f8 /* f32:  FC=0.90f        -> PES=1.10f       */
#define OFF_FATIGA_PRESS_TACTICS_MODIFIER      0x00d8 /* f32:  FC=6.50f        -> PES=8.00f       */

/* Valores PES 2021 listos para inyeccion / compilacion C17 */
#define VAL_FATIGA_DISABLE_CONTROLLED_PLAYER   ((uint8_t)0) /* Activa la fatiga en el jugador controlado */
#define VAL_FATIGA_ST_SLOWDOWN_THRESHOLD       0.70f
#define VAL_FATIGA_ST_SLOWDOWN_RATE            0.38f
#define VAL_FATIGA_LT_SPEED_FACTOR             0.85f
#define VAL_FATIGA_FIXED_ST_MODIFIER           1.15f
#define VAL_FATIGA_FIXED_LT_MODIFIER           1.10f
#define VAL_FATIGA_PRESS_TACTICS_MODIFIER      8.00f

#endif /* PES_FATIGA_C17_H */
