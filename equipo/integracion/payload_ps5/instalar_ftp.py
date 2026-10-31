"""Instala por FTP; conserva copias locales y no borra archivos remotos."""
import argparse
import ftplib
import hashlib
import io
import time
from pathlib import Path
HERE = Path(__file__).resolve().parent
REMOTE = '/data/pldmgr/payloads/pes2021/pes2021_fc.elf'
AUTO = '/data/pldmgr/autoload.txt'

def updated(old):
    lines = old.decode('utf-8-sig').splitlines()
    if any(x.strip() in (REMOTE, 'pes2021_fc.elf') for x in lines):
        return old
    return old + (b'\n' if old and not old.endswith(b'\n') else b'') + REMOTE.encode() + b'\n'

def retrieve(ftp, path):
    out = io.BytesIO()
    ftp.retrbinary('RETR ' + path, out.write)
    return out.getvalue()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--host', required=True)
    ap.add_argument('--port', type=int, default=2121)
    ap.add_argument('--user', default='anonymous')
    ap.add_argument('--password', default='anonymous@')
    args=ap.parse_args()
    elf=(HERE/'pes2021_fc.elf').read_bytes()
    if elf[:4]!=b'\x7fELF': raise ValueError('ELF invalido')
    with ftplib.FTP() as ftp:
        ftp.connect(args.host,args.port,timeout=15)
        ftp.login(args.user,args.password)
        # Exigir una lista legible; un 550 puede significar falta de permisos.
        old=retrieve(ftp,AUTO)
        stamp=time.strftime('%Y%m%d-%H%M%S')+'-'+str(time.time_ns())
        (HERE/('autoload-respaldo-'+stamp+'.txt')).write_bytes(old)
        for path in ('/data/pldmgr/payloads','/data/pldmgr/payloads/pes2021'):
            try: ftp.cwd(path)
            except ftplib.error_perm:
                ftp.mkd(path)
        names=ftp.nlst('/data/pldmgr/payloads/pes2021')
        if any(x.rsplit('/',1)[-1]=='pes2021_fc.elf' for x in names):
            prior=retrieve(ftp,REMOTE)
            (HERE/('payload-respaldo-'+stamp+'.elf')).write_bytes(prior)
        ftp.storbinary('STOR '+REMOTE,io.BytesIO(elf))
        if retrieve(ftp,REMOTE)!=elf: raise RuntimeError('Verificacion ELF fallida; autoload no modificado')
        new=updated(old)
        if new!=old:
            if retrieve(ftp,AUTO)!=old: raise RuntimeError('Autoload cambio durante la instalacion; repetir')
            # Append preserva los bytes existentes incluso si el envio se interrumpe.
            ftp.storbinary('APPE '+AUTO,io.BytesIO(new[len(old):]))
        if retrieve(ftp,AUTO)!=new: raise RuntimeError('Autoload no coincide; consultar respaldo')
    print('Instalacion FTP verificada. SHA256:',hashlib.sha256(elf).hexdigest())
    print('Activar Autoload en Payload Manager y cargar/reiniciar el manager. Ejecucion aun no verificada.')

if __name__=='__main__': main()
