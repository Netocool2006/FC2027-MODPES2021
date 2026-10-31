/* Politica autonoma compartida por los dos daemons. Kernel API o mocks del test. */
#ifndef MODO_PES_H
#define MODO_PES_H
#define DEFAULT_PES 1

/* No falsear perfil_cargado: el hook del integrador carga parametros y bits
 * cuando modo != perfil_cargado. Reintentar en cada ciclo y cada proceso nuevo.
 * Retorno: 1 confirmado; 0 pendiente/error; -1 bloque incompatible. */
static int asegurar_modo_pes(int pid, uint64_t base) {
    const uint64_t block = base + 0x15600000ULL;
    const uint64_t addr = block + 0x60;
    char marca[25];
    int32_t version = 0, mode = -1;
    const int32_t wanted = DEFAULT_PES;
    if (block < base || addr < block || addr >= 0x800000000000ULL - 4)
        return -1;
    if (kernel_proc_copyout(pid, (intptr_t)block, marca, sizeof(marca)) ||
        kernel_proc_copyout(pid, (intptr_t)(block + 0x6c), &version, 4)) return 0;
    if (memcmp(marca, "PESMODE bloque consola v1", sizeof(marca)) || version != 1)
        return -1;
    if (kernel_proc_copyout(pid, (intptr_t)addr, &mode, 4)) return 0;
    if (mode == wanted) return 1;
    if (mode < 0 || mode > 2) return -1;
    int prot = kernel_get_vmem_protection(pid, (intptr_t)addr, 4);
    if (prot < 0 || (prot & 3) != 3) return 0;
    if (kernel_proc_copyin(pid, &wanted, (intptr_t)addr, 4)) return 0;
    return !kernel_proc_copyout(pid, (intptr_t)addr, &mode, 4) && mode == wanted;
}
#endif
