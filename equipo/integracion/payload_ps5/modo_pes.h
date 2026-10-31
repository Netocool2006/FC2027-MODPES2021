/* Politica autonoma compartida por los dos daemons. Kernel API o mocks del test. */
#ifndef MODO_PES_H
#define MODO_PES_H
#define DEFAULT_PES 1
#include <errno.h>
#ifndef PES_MODO_PATH
#define PES_MODO_PATH "/data/pes_modo.cfg"
#endif
/* Registro append-only: la ultima linea completa es el estado vigente.
 * No trunca archivos; fsync confirma el guardado antes de marcarlo limpio. */
static int guardar_modo_pes(int mode) {
    FILE *f = fopen(PES_MODO_PATH, "a");
    if (!f) return 0;
    int ok = fprintf(f, "\n%d\n", mode) > 0;
    if (fflush(f)) ok = 0;
    if (fsync(fileno(f))) ok = 0;
    if (fclose(f)) ok = 0;
    return ok;
}
static int cargar_modo_pes(int *mode) {
    FILE *f = fopen(PES_MODO_PATH, "r");
    *mode = DEFAULT_PES;
    if (!f) return errno == ENOENT ? guardar_modo_pes(*mode) : 0;
    int c, value = -1, length = 0, last = -1;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') {
            if (length) last = length == 1 ? value : -1;
            length = 0; value = -1;
        } else {
            if (!length && c >= '0' && c <= '2') value = c - '0';
            if (length < 2) length++;
        }
    }
    int ok = !ferror(f);
    if (fclose(f)) ok = 0;
    if (!ok) return 0;
    /* Tambien aceptar un archivo manual con un solo digito sin salto. */
    if (length) last = length == 1 ? value : -1;
    if (last >= 0) { *mode = last; return 1; }
    return guardar_modo_pes(*mode);
}
typedef struct { int initialized, last, dirty; } EstadoModoPes;


/* No falsear perfil_cargado. Aplicar disco una vez por proceso; luego observar.
 * Los fallos de guardado se reintentan sin escribir en memoria del juego.
 * Retorno: 1 confirmado; 0 pendiente/error; -1 bloque incompatible. */
static int sincronizar_modo_pes(int pid, uint64_t base, EstadoModoPes *s, int *actual) {
    const uint64_t block = base + 0x15600000ULL;
    const uint64_t addr = block + 0x60;
    char marca[25];
    int32_t version = 0, mode = -1;
    int32_t wanted = DEFAULT_PES;
    if (block < base || addr < block || addr >= 0x800000000000ULL - 4)
        return -1;
    if (kernel_proc_copyout(pid, (intptr_t)block, marca, sizeof(marca)) ||
        kernel_proc_copyout(pid, (intptr_t)(block + 0x6c), &version, 4)) return 0;
    if (memcmp(marca, "PESMODE bloque consola v1", sizeof(marca)) || version != 1)
        return -1;
    if (kernel_proc_copyout(pid, (intptr_t)addr, &mode, 4)) return 0;
    if (mode < 0 || mode > 2) return -1;
    if (s->initialized) {
        if (mode != s->last) { s->last = mode; s->dirty = 1; }
        if (s->dirty && guardar_modo_pes(mode)) s->dirty = 0;
        *actual = mode;
        return 1;
    }
    int loaded;
    if (!cargar_modo_pes(&loaded)) return 0;
    wanted = loaded;
    if (mode == wanted) {
        s->initialized = 1; s->last = mode; *actual = mode; return 1;
    }
    int prot = kernel_get_vmem_protection(pid, (intptr_t)addr, 4);
    if (prot < 0 || (prot & 3) != 3) return 0;
    if (kernel_proc_copyin(pid, &wanted, (intptr_t)addr, 4)) return 0;
    /* Una escritura exitosa se aplica solo una vez, incluso si falla la
     * lectura posterior: el siguiente ciclo ya observa cambios del menu. */
    s->initialized = 1; s->last = wanted;
    if (kernel_proc_copyout(pid, (intptr_t)addr, &mode, 4)) return 0;
    if (mode < 0 || mode > 2) return -1;
    if (mode != wanted) { s->last = mode; s->dirty = !guardar_modo_pes(mode); }
    *actual = mode;
    return 1;
}
#endif
