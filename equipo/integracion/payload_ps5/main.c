/* FC27 v1.0.3: rutas verificadas por gp_vivo.py. Sin direcciones PS4 fijas. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef HOST_TEST
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/file.h>
#include <fcntl.h>
#include <ps5/kernel.h>
#endif
#ifndef DEFAULT_PES
#define DEFAULT_PES 0
#endif
typedef struct {
 uint32_t group, instance; uint64_t stat; uint32_t offset;
 int index; unsigned count, size; uint32_t value; unsigned n;
 uint32_t accepted[16];
} Patch;
#include "tabla.h"
#define N (sizeof(patches)/sizeof(patches[0]))
typedef struct { uint64_t address, object; uint32_t original; int owned; } Saved;
static Saved saved[N][2];
static int valid(uint64_t p) { return p>=0x10000 && p<0x800000000000ULL; }
static int rd(int pid,uint64_t a,void *b,size_t n) {
 return valid(a) && n && a<=0x800000000000ULL-n && !kernel_proc_copyout(pid,(intptr_t)a,b,n);
}
static uint64_t ptr(int pid,uint64_t a) {
 uint64_t p=0; return rd(pid,a,&p,8) && valid(p&~4ULL) ? p : 0;
}
static uint64_t database(int pid,uint64_t base) {
 uint64_t x=ptr(pid,base+0x11d9cae8), y=ptr(pid,x);
 return ptr(pid,y+8);
}
static uint64_t instance(int pid,uint64_t base,uint64_t db,const Patch *p,int slot) {
 if(slot) return p->stat ? ptr(pid,base+p->stat)&~4ULL : 0;
 uint64_t g=ptr(pid,db+p->group)&~4ULL;
 return ptr(pid,g+p->instance)&~4ULL;
}
static uint64_t address(int pid,uint64_t obj,const Patch *p) {
 if(!valid(obj)) return 0;
 if(p->index<0) return obj+p->offset;
 uint64_t a=ptr(pid,obj+p->offset); uint32_t count=0;
 if(!a || !rd(pid,a-4,&count,4) || (count&0x7fffffff)!=p->count) return 0;
 return a+(unsigned)p->index*p->size;
}
static int write_checked(int pid,uint64_t a,uint32_t before,uint32_t after,unsigned size) {
 uint32_t now=0;
 if(!rd(pid,a,&now,size) || now!=before) return 0;
 /* Datos solamente: nunca abrir paginas de codigo ni inventar permisos. */
 int prot=kernel_get_vmem_protection(pid,(intptr_t)a,size);
 if(prot<0 || (prot&3)!=3) return 0;
 if(kernel_proc_copyin(pid,&after,(intptr_t)a,size)) return 0;
 now=0; return rd(pid,a,&now,size) && now==after;
}
static unsigned tick(int pid,uint64_t base,int enabled) {
 uint64_t db=database(pid,base); unsigned changes=0;
 for(size_t i=0;i<N;i++) for(int slot=0;slot<2;slot++) {
  const Patch *p=&patches[i]; Saved *s=&saved[i][slot];
  uint64_t obj=instance(pid,base,db,p,slot), a=address(pid,obj,p);
  if(slot && obj==instance(pid,base,db,p,0)) a=0;
  if(s->address!=a || s->object!=obj) { memset(s,0,sizeof(*s)); s->address=a; s->object=obj; }
  uint32_t now=0;
  if(!a || !rd(pid,a,&now,p->size)) continue;
  if(!enabled) {
   if(s->owned && now==p->value) {
    if(write_checked(pid,a,now,s->original,p->size)) { s->owned=0; changes++; }
   } else if(now!=p->value) s->owned=0;
   continue;
  }
  if(now==p->value) continue;
  /* Otro escritor cambio el dato: no sobrescribirlo salvo defecto conocido. */
  int accepted=0;
  for(unsigned j=0;j<p->n;j++) if(now==p->accepted[j]) accepted=1;
  if(!accepted) { s->owned=0; continue; }
  s->original=now; s->owned=1;
  if(write_checked(pid,a,now,p->value,p->size)) changes++;
 }
 return changes;
}
#ifndef HOST_TEST
typedef struct __attribute__((packed)) {
 uint32_t app_id; uint64_t unk; uint32_t type; char title[10]; char pad[0x3c];
} AppInfo;
int sceKernelGetAppInfo(pid_t,AppInfo*);
static void logline(int pid,const char *msg,unsigned n) {
 FILE *f=fopen("/data/pes2021_fc.log","a");
 if(f) { fprintf(f,"pid=%d %s %u\n",pid,msg,n); fclose(f); }
}
static void *query(int kind,int pid,size_t *size) {
 int mib[4]={CTL_KERN,KERN_PROC,kind,pid};
 for(int i=0;i<3;i++) {
  size_t n=0;
  if(sysctl(mib,4,NULL,&n,NULL,0) || !n || n>32*1024*1024) return NULL;
  n+=4096; void *b=malloc(n); if(!b) return NULL;
  if(!sysctl(mib,4,b,&n,NULL,0)) { *size=n; return b; } free(b);
 }
 return NULL;
}
static int find_game(void) {
 size_t n=0; char *b=query(KERN_PROC_PROC,0,&n); int pid=0;
 if(!b) return 0;
 for(size_t pos=0;pos+sizeof(struct kinfo_proc)<=n;) {
  struct kinfo_proc *p=(void*)(b+pos);
  if(p->ki_structsize<(int)sizeof(*p) || (size_t)p->ki_structsize>n-pos) break;
  AppInfo ai={0};
  if(!strcmp(p->ki_comm,"eboot.bin") && !sceKernelGetAppInfo(p->ki_pid,&ai) && !memcmp(ai.title,"PPSA34015",9)) { pid=p->ki_pid; break; }
  pos+=p->ki_structsize;
 }
 free(b); return pid;
}
static uint64_t find_base(int pid) {
 size_t n=0; char *b=query(KERN_PROC_VMMAP,pid,&n); uint64_t base=0;
 if(!b) return 0;
 for(size_t pos=0;pos+sizeof(struct kinfo_vmentry)<=n;) {
  struct kinfo_vmentry *v=(void*)(b+pos);
  if(v->kve_structsize<(int)sizeof(*v) || (size_t)v->kve_structsize>n-pos) break;
  if(!strcmp(v->kve_path,"executable") && (!base || v->kve_start<base)) base=v->kve_start;
  pos+=v->kve_structsize;
 }
 free(b); return base;
}
int main(void) {
 int lock=open("/data/pes2021_fc.lock",O_CREAT|O_RDWR,0600);
 if(lock<0 || flock(lock,LOCK_EX|LOCK_NB)) return 1;
 logline(0,"daemon activo; default",DEFAULT_PES);
 int previous=0,last_mode=-9; uint64_t old_base=0;
 for(;;) {
  int pid=find_game(); uint64_t base=pid?find_base(pid):0;
  if(pid!=previous || base!=old_base) { memset(saved,0,sizeof(saved)); previous=pid; old_base=base; last_mode=-9; }
  if(pid && valid(base)) {
   int mode=-1; int readable=rd(pid,base+0x15600060,&mode,4);
   int enabled=readable && (mode==1 || mode==2);
   if(!readable) enabled=DEFAULT_PES;
   if(mode!=last_mode) { logline(pid,"modo",(unsigned)mode); last_mode=mode; }
   unsigned changed=tick(pid,base,enabled);
   if(changed) logline(pid,enabled?"escritos":"restaurados",changed);
  }
  sleep(2);
 }
}
#endif
