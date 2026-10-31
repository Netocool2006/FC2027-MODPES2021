#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <ps5/kernel.h>
#include <sys/file.h>
#include <fcntl.h>
#include "../modo_pes.h"

typedef struct __attribute__((packed)) {
    uint32_t app_id;
    uint64_t desconocido1;
    uint32_t app_type;
    char     title_id[10];
    char     desconocido2[0x3c];
} app_info_t;

int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
int sceKernelSendNotificationRequest(int, void *, size_t, int);

static void notificar(const char *msg) {
    struct { char pad[0x2d]; char msg[0x3ff]; } __attribute__((packed)) req;
    memset(&req, 0, sizeof req);
    strncpy(req.msg, msg, sizeof req.msg - 1);
    sceKernelSendNotificationRequest(0, &req, sizeof req, 0);
}

static void reg(const char *fmt, ...) {
    FILE *f = fopen("/data/pes2021_fc.log", "a");
    if (!f) return;
    time_t t = time(NULL); struct tm tm; localtime_r(&t, &tm);
    fprintf(f, "%02d:%02d:%02d ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc(10, f); fclose(f);
}

static void *sysctl_todo(int *mib, unsigned n, size_t *tam) {
    for (int intento = 0; intento < 4; intento++) {
        size_t t = 0;
        if (sysctl(mib, n, NULL, &t, NULL, 0) || !t) return NULL;
        t += t / 4 + 4096;
        void *b = malloc(t);
        if (!b) return NULL;
        if (!sysctl(mib, n, b, &t, NULL, 0)) { *tam = t; return b; }
        free(b);
    }
    return NULL;
}

static uint64_t base_ejecutable(pid_t pid) {
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_VMMAP, pid};
    size_t tam = 0;
    char *b = sysctl_todo(mib, 4, &tam);
    uint64_t base = 0;
    if (!b) return 0;
    for (size_t pos = 0; pos + sizeof(struct kinfo_vmentry) <= tam;) {
        struct kinfo_vmentry *v = (void *)(b + pos);
        if (v->kve_structsize < (int)sizeof(*v) || (size_t)v->kve_structsize > tam - pos) break;
        if (!strcmp(v->kve_path, "executable") && (!base || v->kve_start < base)) base = v->kve_start;
        pos += v->kve_structsize;
    }
    free(b);
    return base;
}

static inline bool addr_valida(uint64_t a) {
    return a >= 0x400000ULL && a < 0x800000000000ULL;
}

static uint64_t leer_q(pid_t pid, uint64_t addr) {
    uint64_t v = 0;
    if (kernel_proc_copyout(pid, (intptr_t)addr, &v, sizeof(v)) == 0) return v;
    return 0;
}

static void escribir_mem(pid_t pid, uint64_t addr, const void *src, size_t tam) {
    kernel_proc_copyin(pid, src, (intptr_t)addr, tam);
}

static uint64_t resolver_db(pid_t pid, uint64_t base) {
    uint64_t X = leer_q(pid, base + 0x11d9cae8ULL);
    if (!addr_valida(X)) return 0;
    uint64_t X1 = leer_q(pid, X);
    if (!addr_valida(X1)) return 0;
    uint64_t DB = leer_q(pid, X1 + 8);
    if (addr_valida(DB)) return DB;
    return 0;
}

static uint64_t resolver_instancia(pid_t pid, uint64_t db, uint32_t off_g, uint32_t off_i) {
    if (!addr_valida(db)) return 0;
    uint64_t G = leer_q(pid, db + off_g) & ~4ULL;
    if (!addr_valida(G)) return 0;
    uint64_t I = leer_q(pid, G + off_i) & ~4ULL;
    if (addr_valida(I)) return I;
    return 0;
}

typedef enum { T_F32, T_U8, T_I32, T_ARR11_F32 } TipoParche;

typedef struct {
    uint32_t   off_grupo;
    uint32_t   off_inst;
    uint32_t   off_campo;
    TipoParche tipo;
    float      val_pes_f32;
    float      val_fc_f32;
} DefParche;

static const DefParche PARCHES[] = {
    /* 1. Arbitraje */
    { 0x138, 0x30, 0xa0,  T_F32, 0.92f, 0.85f },  // PushPull_FallThreshold
    { 0x138, 0x30, 0xa8,  T_F32, 0.50f, 0.40f },  // PushPull_MedIntensity
    { 0x138, 0x30, 0xb0,  T_F32, 0.88f, 0.75f },  // PushPull_HighIntensity
    { 0xb8,  0x50, 0x70,  T_I32, 210.0f, 180.0f }, // AdvantageWindowTimeLength
    { 0xb8,  0x50, 0x94,  T_F32, 135.0f, 90.0f },  // ShoulderChallengesAngleGap
    { 0xb8,  0x68, 0x70,  T_ARR11_F32, 1.15f, 1.50f }, // referee_strictness_foul_intensity
    { 0xb8,  0x68, 0x118, T_F32, 66.0f, 50.0f },  // referee_strictness_yellowcard

    /* 2. Bloque Tactico y Compactacion */
    { 0x128, 0x30, 0x30,  T_F32, 0.72f, 1.00f },  // Def LineLength Balanced
    { 0x128, 0x30, 0x50,  T_F32, 0.72f, 1.00f },  // Def LineLength Possession
    { 0x128, 0x30, 0x2c,  T_F32, 0.75f, 1.00f },  // Def LineLength LongBall
    { 0x128, 0x30, 0x54,  T_F32, 0.75f, 1.00f },  // Def LineLength FastBuildUp
    { 0x128, 0x30, 0x28,  T_F32, 0.88f, 1.00f },  // Def lineWidthModifier
    { 0x128, 0x30, 0x3c,  T_F32, 0.85f, 1.00f },  // Def lineWidthModifierForCB
    { 0x128, 0x28, 0x30,  T_F32, 0.72f, 0.84f },  // Atk LineLength Balanced
    { 0x128, 0x28, 0x34,  T_F32, 0.70f, 0.80f },  // Atk LineLength Possession
    { 0x128, 0x28, 0x28,  T_F32, 0.90f, 1.00f },  // Atk lineWidthModifier

    /* 3. Fatiga y Resistencia */
    { 0x168, 0x20, 0x101, T_U8,  0.0f,  1.0f },   // DisableFatigueOnControlledPlayer
    { 0x168, 0x20, 0xe8,  T_F32, 0.70f, 0.84f },  // ShortTermFatigueSlowdownThreshold
    { 0x168, 0x20, 0xe0,  T_F32, 0.38f, 0.25f },  // ShortTermFatigueSlowdownRate
    { 0x168, 0x20, 0xec,  T_F32, 0.85f, 0.95f },  // LongTermFatigueSpeedFactor
    { 0x168, 0x20, 0xc0,  T_F32, 1.15f, 0.95f },  // fixedShortTermFatigueModifier
    { 0x168, 0x20, 0xf8,  T_F32, 1.10f, 0.90f },  // fixedLongTermFatigueModifier
    { 0x168, 0x20, 0x10c, T_F32, 8.00f, 1.00f },  // fatigueModifierForPressureTactics

    /* 4. Balistica y Friccion */
    { 0x130, 0x58, 0x3c,  T_F32, 0.82f, 0.72f },  // FieldDynamicFriction
    { 0x130, 0x58, 0x68,  T_F32, 0.90f, 0.82f },  // FieldStaticFriction
    { 0x130, 0x58, 0x34,  T_F32, 0.985f, 0.998f }, // BallBounceLinearDamping
    { 0x130, 0x58, 0x58,  T_F32, 0.80f, 0.90f },  // BallRestitution
    { 0x130, 0x58, 0x50,  T_F32, 0.0045f, 0.003f }, // BallLinearDrag
    { 0x130, 0xb0, 0x50,  T_F32, 0.470f, 0.454f }, // Soccerball_Mass
    { 0x130, 0xb0, 0x34,  T_F32, 0.76f, 0.66f },  // Soccerball_Inertia
    { 0x130, 0xb0, 0x4c,  T_F32, 0.85f, 0.92f },  // Soccerball_GripConstant

    /* 5. Porteros */
    { 0x160, 0x148, 0x120, T_F32, 2.80f, 4.50f }, // AutoSaveRadiusFullAssist
    { 0x160, 0x148, 0x11c, T_F32, 2.20f, 3.50f }, // AutoSaveRadiusPartiallAssist
    { 0x160, 0x148, 0x198, T_F32, 0.95f, 1.20f }, // CrossCatchExtendedCoverage
    { 0x160, 0x148, 0x138, T_F32, 1.35f, 1.80f }, // CrossDeflectAndPunchExtendedCoverage
    { 0x160, 0x148, 0x19c, T_F32, 40.0f, 60.0f }, // SaveAssistReflexIncreaseCoverageFullAssist
    { 0x160, 0x148, 0x150, T_F32, 0.35f, 0.20f }, // GkCatchExclusionDifficultyThreshold

    /* 6. Primer Toque e Inercia */
    { 0x48,  0x18, 0x250, T_F32, 75.0f, 65.0f },  // TrappingControlledFirstTouchBallTargetTimeShort
    { 0x48,  0x18, 0x3d4, T_F32, 125.0f, 110.0f }, // TrappingControlledFirstTouchBallTargetTimeLong
    { 0x48,  0x18, 0x20c, T_F32, 3.00f, 2.00f },  // ATTR_DribbleMinTouchInterruptPenalty
    { 0x48,  0x18, 0x214, T_F32, 52.0f, 45.0f },  // DribbleControlledSprintBallTargetTimeRunTouch

    /* 7. Duelos Aereos */
    { 0x40,  0x18, 0x34,  T_F32, 0.0f, 15.0f },   // TPH_INTERACTION_Master_Aerial_Loser_TouchAdvantage
    { 0x168, 0x18, 0x38,  T_F32, 0.0f, 0.25f },   // AerialMaster_HeightBoost
    { 0x168, 0x18, 0x34,  T_F32, 0.0f, 0.50f },   // AerialMaster_Icon_HeightBoost
    { 0x118, 0x120, 0x94, T_F32, 1.0f, 1.10f },   // ATTR_BallTouchHeadSpeedModifierForPowerHeader

    /* 8. Pases Manuales */
    { 0xd0,  0xa8, 0x584, T_F32, 50.0f, 50.0f }   // PASS_MaxExpectedGroundPassDistForManual
};

#define TOTAL_PARCHES (sizeof(PARCHES) / sizeof(PARCHES[0]))

static void aplicar_parches_nativo(pid_t pid, uint64_t db, bool activar_pes) {
    if (!addr_valida(db)) return;
    int aplicados = 0;

    for (size_t i = 0; i < TOTAL_PARCHES; i++) {
        const DefParche *p = &PARCHES[i];
        uint64_t inst = resolver_instancia(pid, db, p->off_grupo, p->off_inst);
        if (!addr_valida(inst)) continue;

        uint64_t destino = inst + p->off_campo;
        float v_flt = activar_pes ? p->val_pes_f32 : p->val_fc_f32;

        if (p->tipo == T_F32) {
            escribir_mem(pid, destino, &v_flt, sizeof(float));
            aplicados++;
        } else if (p->tipo == T_U8) {
            uint8_t v_u8 = (uint8_t)v_flt;
            escribir_mem(pid, destino, &v_u8, sizeof(uint8_t));
            aplicados++;
        } else if (p->tipo == T_I32) {
            int32_t v_i32 = (int32_t)v_flt;
            escribir_mem(pid, destino, &v_i32, sizeof(int32_t));
            aplicados++;
        } else if (p->tipo == T_ARR11_F32) {
            uint64_t p_arr = leer_q(pid, destino);
            if (addr_valida(p_arr)) {
                uint32_t n_elem = 0;
                kernel_proc_copyout(pid, (intptr_t)(p_arr - 4), &n_elem, sizeof(uint32_t));
                n_elem &= 0x7fffffff;
                float arr[16];
                for (size_t j = 0; j < 16; j++) arr[j] = v_flt;
                size_t num = n_elem > 16 ? 16 : n_elem;
                escribir_mem(pid, p_arr, arr, num * sizeof(float));
                aplicados++;
            }
        }
    }

    reg("PPSA34015: Parches aplicados (%s): %d/%zu sobre BD 0x%llx", 
        activar_pes ? "PES2021" : "EA FC 27", aplicados, TOTAL_PARCHES, (unsigned long long)db);
}

int main(void) {
    int lock = open("/data/pes2021_fc.lock", O_CREAT | O_RDWR, 0600);
    if (lock < 0 || flock(lock, LOCK_EX | LOCK_NB)) return 1;
    EstadoModoPes modo_estado = {0};
    int inicial;
    reg("Persistencia /data/pes_modo.cfg: %s", cargar_modo_pes(&inicial) ? "lista" : "error; se reintentara");
    notificar("PES 2021 Daemon Nativo Iniciado");
    reg("Daemon persistencia v1 PID=%d cargado con exito", (int)getpid());

    int ultimo_modo = -1;
    uint64_t ultima_db = 0;
    pid_t ultimo_pid = 0;
    uint64_t ultima_base = 0;

    for (;;) {
        int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
        size_t tam = 0;
        char *b = sysctl_todo(mib, 4, &tam);
        pid_t fc_pid = 0;

        if (b) {
            for (char *p = b; p < b + tam;) {
                struct kinfo_proc *ki = (struct kinfo_proc *)p;
                if (ki->ki_structsize <= 0) break;
                p += ki->ki_structsize;
                if (strcmp(ki->ki_comm, "eboot.bin")) continue;
                pid_t pid = ki->ki_pid;
                app_info_t ai; memset(&ai, 0, sizeof ai);
                if (sceKernelGetAppInfo(pid, &ai) == 0) {
                    if (!strncmp(ai.title_id, "PPSA34015", 9)) {
                        fc_pid = pid;
                        break;
                    }
                }
            }
            free(b);
        }

        if (fc_pid > 0) {
            uint64_t base = base_ejecutable(fc_pid);
            if (!base) { usleep(500000); continue; }
            if (fc_pid != ultimo_pid || base != ultima_base) {
                memset(&modo_estado, 0, sizeof modo_estado);
                ultimo_modo = -1;
                ultima_db = 0;
                ultimo_pid = fc_pid;
                ultima_base = base;
            }
            int modo_actual = DEFAULT_PES;

            /* Disco solo al conectar; despues el selector manda. */
            if (sincronizar_modo_pes(fc_pid, base, &modo_estado, &modo_actual) != 1) {
                usleep(500000);
                continue;
            }

            if (modo_estado.dirty) reg("Error guardando modo %d; se reintentara", modo_actual);
            if (modo_actual != ultimo_modo) reg("Selector modo=%d persistido=%s", modo_actual, modo_estado.dirty ? "no" : "si");

            /* Asegurar g_ia_mtp.activos = 0 siempre para evitar congelacion de los 3 defensas */
            uint32_t val_ia_mtp = 0;
            kernel_proc_copyin(fc_pid, &val_ia_mtp, (intptr_t)(base + 0x1561dee0ULL), sizeof(val_ia_mtp));

            {
                uint64_t db_addr = resolver_db(fc_pid, base);

                if (modo_actual != ultimo_modo || (db_addr != 0 && db_addr != ultima_db)) {
                    if (modo_actual == 1 || modo_actual == 2) {
                        aplicar_parches_nativo(fc_pid, db_addr, true);
                        notificar(modo_actual == 1 ? "PES 2021 Original Activado" : "PES 2021 Holland Activado");
                    } else {
                        aplicar_parches_nativo(fc_pid, db_addr, false);
                        notificar("EA FC 27 Modo Nativo Restaurado");
                    }
                    ultimo_modo = modo_actual;
                    ultima_db = db_addr;
                }
            }
            usleep(500000);
        } else {
            memset(&modo_estado, 0, sizeof modo_estado);
            ultimo_modo = -1;
            ultima_db = 0;
            ultimo_pid = 0;
            ultima_base = 0;
            usleep(2000000);
        }
    }

    return 0;
}
