# Arranque autonomo PES Original (2026-10-31)

## Causa comprobada

El ELF distribuido y el instalado en 192.168.0.44:2121 tienen SHA256
`7a976c9d5a668d556b971370cd10c46f4f1ca84a7df368eb5afb10aac0be8419`.
Corresponden a la ruta `src/main.c` / `compilar.sh`. Esa ruta esperaba
que el menu escribiera modo 1/2. En `distribucion/eboot.bin` el bloque
`PESMODE bloque consola v1` contiene modo 0, version 1 y perfil_cargado -1.
La otra ruta (`main.c`) usaba DEFAULT_PES=0, solo como fallback de lectura.
La consola ya incluye `pes2021_fc.elf` en `/data/pldmgr/autoload.txt`.

## Cambio y alcance

Ambas rutas usan `modo_pes.h`: DEFAULT_PES=1 y escritura verificada de
`base + 0x15600060` en cada ciclo, tras comprobar firma, version, modo conocido
y permisos RW. Fallos temporales se reintentan. La ruta distribuida resuelve
ahora la base del ejecutable y reinicia su estado al cambiar PID/base.
No se escribe `perfil_cargado`: el integrador C16 detecta la diferencia
con modo y llama `pes_bloque_iniciar_partido`, que carga parametros y bits.
No existe un campo `perfil_pes` en `pes_estado`.

Esta politica mantiene PES Original: una seleccion posterior 0/2 vuelve a 1
en el siguiente ciclo. Se conserva la correccion existente de IA MTP.
No instala hooks nuevos: requiere el eboot modificado compatible ya instalado.
No garantiza latencia de un milisegundo: sondeo de 2 s sin juego y 500 ms con
juego en src/main.c; 2 s en main.c. Los hooks cargan el perfil al ejecutarse.

## Compilacion y candidato

`compilar.ps1` y `compilar.sh` compilan por defecto `src/main.c` en una carpeta
nueva y rechazan reemplazar salidas existentes. Para el motor alternativo:
`compilar.ps1 -Source main.c`. Se utilizo el PS5 Payload SDK local con Clang,
`-Wall -Wextra -Werror`; ambas variantes compilaron y readelf valido ELF64 DYN
x86-64 FreeBSD. El test_motor ampliado paso con mocks de memoria remota:
modos 0/1/2, reenganche, idempotencia, firma/version, permisos, errores y
reintentos, perfil sin falsificar, y regresiones del motor de parches.

Candidato de la ruta distribuida: `distribucion/pes2021_fc.default-pes.elf`.
SHA256: `7d315d1dbe14499700b62f52ec7d264eaafa8bc616938d2bf814c6570b3d690e`.

## Despliegue pendiente de confirmacion

No se reemplazaron binarios existentes ni se modifico la consola. La norma
del usuario exige confirmacion antes de sustituirlos. Destinos pendientes:

- `G:\ProyFIFA\distribucion\pes2021_fc.elf`
- `G:\ProyFIFA\equipo\integracion\payload_ps5\salida\pes2021_fc.elf`
- PS5 `192.168.0.44`: `/data/pldmgr/payloads/pes2021_fc.elf`

El ELF alternativo `payload_ps5/pes2021_fc.elf` tambien sigue siendo el anterior;
su compilacion nueva esta en `payload_ps5/build-default-pes-20261031/`.
No ejecutar instalar_ftp.py para este despliegue: apunta a una subcarpeta
distinta de la ruta activa comprobada en la consola.

Tras autorizar la sustitucion: conservar respaldos, instalar el candidato,
comparar SHA256 por RETR y verificar arranque frio y reapertura del juego.
FTP no sustituye un proceso ya residente. Falta verificar en consola que
modo=1, perfil_cargado=1 y errores_carga=0, junto con parametros y bits activos.
Las pruebas con mocks no acreditan la ejecucion real de todos los hooks.
