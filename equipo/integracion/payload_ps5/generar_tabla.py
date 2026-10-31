import ast,json,struct
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
def generate():
 tree=ast.parse((HERE.parent/'build_c17/pes_c17_inyector.py').read_text(encoding='utf-8-sig'))
 patches=next(ast.literal_eval(n.value) for n in tree.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='PARCHES_C17' for t in n.targets))
 cat=json.loads((ROOT/'equipo/datos_fc/catalogo_gp.json').read_text(encoding='utf-8-sig'))
 rows=[]
 for name,field,value,requested in patches:
  inst=cat['instancias'][name]; c=next(c for c in inst['campos'] if c['n']==field)
  array=c['tipo'].startswith('arr_'); typ=c['tipo'][4:] if array else c['tipo']
  assert typ==requested or (typ=='bool' and requested=='u8'),(name,field,typ,requested)
  fmt={'f32':'<f','i32':'<i','u32':'<I','bool':'<B','u8':'<B'}[typ]
  pack=lambda v:int.from_bytes(struct.pack(fmt,v),'little')
  defaults=c['defecto'] if array else [c['defecto']]
  assert defaults and len(defaults)<=32
  for idx,default in enumerate(defaults):
   accepted={pack(default)}
   for variant in (inst.get('variantes') or {}).values():
    v=variant.get(field,c['defecto'])
    if array:
     if idx>=len(v):continue
     v=v[idx]
    accepted.add(pack(v))
   assert len(accepted)<=16
   rows.append('{%s,%s,%s,%s,%d,%d,%d,0x%x,%d,{%s}}, /* %s.%s */'%(hex(inst['off_grupo']),hex(inst['off_inst']),inst.get('estatico') or '0',hex(c['off']),idx if array else -1,len(defaults) if array else 0,struct.calcsize(fmt),pack(value),len(accepted),','.join(hex(v) for v in sorted(accepted)),name,field))
 return '/* Generado desde C17 y catalogo_gp.json. */\nstatic const Patch patches[] = {\n'+'\n'.join(rows)+'\n};\n'
if __name__=='__main__':
 (HERE/'tabla.h').write_text(generate(),encoding='utf-8'); print('tabla.h generada')
