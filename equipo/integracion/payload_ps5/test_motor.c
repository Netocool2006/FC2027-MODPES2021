/* Prueba el mismo motor C con memoria remota simulada. */
#define HOST_TEST
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
static unsigned char memory[0x10000];
static unsigned char state[0x80];
#define TEST_BASE 0x400000ULL
#define TEST_BLOCK (TEST_BASE + 0x15600000ULL)
static int writes, fail_read, fail_write, protection = 3;
static int kernel_proc_copyout(int pid,intptr_t a,void *b,size_t n) {
 (void)pid;
 if(!fail_read && (uint64_t)a>=TEST_BLOCK && (uint64_t)a+n<=TEST_BLOCK+sizeof(state)) {
  memcpy(b,state+(uint64_t)a-TEST_BLOCK,n); return 0;
 }
 if(fail_read || a<0x10000 || (uint64_t)a+n>0x20000) return -1;
 memcpy(b,memory+a-0x10000,n); return 0;
}
static int kernel_proc_copyin(int pid,const void *b,intptr_t a,size_t n) {
 (void)pid;
 if(!fail_write && (uint64_t)a>=TEST_BLOCK && (uint64_t)a+n<=TEST_BLOCK+sizeof(state)) {
  memcpy(state+(uint64_t)a-TEST_BLOCK,b,n); writes++; return 0;
 }
 if(fail_write || a<0x10000 || (uint64_t)a+n>0x20000) return -1;
 memcpy(memory+a-0x10000,b,n); writes++; return 0;
}
static int kernel_get_vmem_protection(int pid,intptr_t a,size_t n) { (void)pid;(void)a;(void)n;return protection; }
#include <io.h>
#define fsync _commit
static const char *test_path;
#define PES_MODO_PATH test_path
#include "main.c"
static void put64(uint64_t a,uint64_t v) { assert(!kernel_proc_copyin(1,&v,a,8)); }
static void put32(uint64_t a,uint32_t v) { assert(!kernel_proc_copyin(1,&v,a,4)); }
int main(int argc, char **argv) {
 assert(argc==2); test_path=argv[1];
 EstadoModoPes session={0}; int mode=-1, disk=-1;
 memcpy(state,"PESMODE bloque consola v1",25);
 put32(TEST_BLOCK+0x6c,1); put32(TEST_BLOCK+0x70,0xffffffff);
 put32(TEST_BLOCK+0x60,0); writes=0;
 assert(sincronizar_modo_pes(1,TEST_BASE,&session,&mode)==1 && mode==1 && writes==1);
 assert(cargar_modo_pes(&disk) && disk==1);
 /* Cambios reales del menu no generan escrituras en memoria. */
 for(int selected=0;selected<=2;selected++) {
  put32(TEST_BLOCK+0x60,selected); writes=0;
  assert(sincronizar_modo_pes(1,TEST_BASE,&session,&mode)==1 && mode==selected && writes==0);
  assert(cargar_modo_pes(&disk) && disk==selected);
  assert(sincronizar_modo_pes(1,TEST_BASE,&session,&mode)==1 && writes==0);
 }
 /* Reinicio con FC guardado y memoria inicialmente en PES. */
 assert(guardar_modo_pes(0)); memset(&session,0,sizeof session);
 assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==1 && mode==0);
 uint32_t value; assert(rd(1,TEST_BLOCK+0x70,&value,4) && value==0xffffffff);
 fail_read=1; writes=0;
 assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==0 && writes==0); fail_read=0;
 assert(guardar_modo_pes(1)); memset(&session,0,sizeof session);
 fail_write=1; assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==0); fail_write=0;
 protection=1; assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==0); protection=3;
 assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==1 && mode==1);
 state[0]='X'; writes=0; assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==-1 && writes==0); state[0]='P';
 put32(TEST_BLOCK+0x6c,2); assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==-1);
 put32(TEST_BLOCK+0x6c,1); put32(TEST_BLOCK+0x60,99);
 assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==-1);
 /* Guardado fallido: respetar menu y reintentar sin tocar memoria. */
 const char *real_path=test_path; test_path="Z:/no-existe/pes_modo.cfg";
 put32(TEST_BLOCK+0x60,0); writes=0;
 assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==1 && mode==0 && session.dirty && writes==0);
 test_path=real_path;
 assert(sincronizar_modo_pes(2,TEST_BASE,&session,&mode)==1 && !session.dirty && writes==0);
 assert(cargar_modo_pes(&disk) && disk==0);
 FILE *bad=fopen(test_path,"a"); assert(bad); fputs("\ninvalid\n",bad); assert(!fclose(bad));
 assert(cargar_modo_pes(&disk) && disk==1);
 const Patch *p=&patches[0]; uint64_t obj=0x18000, a=obj+p->offset;
 /* La base sintetica coloca la raiz en memoria del mock. */
 uint64_t base=0x10000ULL-0x11d9cae8ULL;
 put64(0x10000,0x11000); put64(0x11000,0x12000); put64(0x12008,0x13000);
 put64(0x13000+p->group,0x14004); put64(0x14000+p->instance,obj|4);
 assert(database(1,base)==0x13000);
 assert(instance(1,base,0x13000,p,0)==obj);
 assert(address(1,obj,p)==a);
 put32(a,p->accepted[0]); writes=0;
 assert(tick(1,base,1)>0); assert(saved[0][0].owned);
 uint32_t v=0; assert(rd(1,a,&v,4) && v==p->value);
 writes=0; tick(1,base,1); assert(writes==0);
 tick(1,base,0); assert(rd(1,a,&v,4) && v==p->accepted[0]);
 put32(a,0xdeadbeef); writes=0; tick(1,base,1); assert(writes==0);
 put32(a,p->accepted[0]); tick(1,base,1);
 put32(a,0xdeadbeef); writes=0; tick(1,base,0); assert(writes==0);
 put32(a,p->accepted[0]); fail_write=1; assert(!write_checked(1,a,p->accepted[0],p->value,4)); fail_write=0;
 fail_read=1; writes=0; tick(1,base,1); assert(writes==0); fail_read=0;
 for(size_t i=0;i<N;i++) if(patches[i].index>=0) {
  const Patch *arr=&patches[i]; put64(obj+arr->offset,0x1e000); put32(0x1dffc,arr->count);
  assert(address(1,obj,arr)==0x1e000+(unsigned)arr->index*arr->size);
  put32(0x1dffc,arr->count+1); assert(!address(1,obj,arr)); break;
 }
 put64(0x14000+p->instance,0); writes=0; tick(1,base,0); assert(writes==0 && !saved[0][0].owned);
 puts("OK: persistencia PES/FC/Holland, reinicio, selector respetado, fallo disco y reintento, firma/version, perfil intacto; rutas, arrays, idempotencia, conflictos y errores");
 return 0;
}
