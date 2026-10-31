/* Prueba el mismo motor C con memoria remota simulada. */
#define HOST_TEST
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
static unsigned char memory[0x10000];
static int writes, fail_read, fail_write;
static int kernel_proc_copyout(int pid,intptr_t a,void *b,size_t n) {
 (void)pid;
 if(fail_read || a<0x10000 || (uint64_t)a+n>0x20000) return -1;
 memcpy(b,memory+a-0x10000,n); return 0;
}
static int kernel_proc_copyin(int pid,const void *b,intptr_t a,size_t n) {
 (void)pid;
 if(fail_write || a<0x10000 || (uint64_t)a+n>0x20000) return -1;
 memcpy(memory+a-0x10000,b,n); writes++; return 0;
}
static int kernel_get_vmem_protection(int pid,intptr_t a,size_t n) { (void)pid;(void)a;(void)n;return 3; }
#include "main.c"
static void put64(uint64_t a,uint64_t v) { assert(!kernel_proc_copyin(1,&v,a,8)); }
static void put32(uint64_t a,uint32_t v) { assert(!kernel_proc_copyin(1,&v,a,4)); }
int main(void) {
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
 puts("OK: rutas, bits etiquetados, arrays, modo FC, idempotencia, conflictos, errores y descarga de instancia");
 return 0;
}
