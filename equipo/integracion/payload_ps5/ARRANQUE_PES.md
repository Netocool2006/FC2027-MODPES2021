# Persistencia del selector PES / FC (2026-10-31)

La politica anterior escribia modo=1 en cada ciclo e impedia elegir FC.
Ambos daemons usan ahora modo_pes.h para aplicar el estado del disco una
sola vez por PID/base del juego. Despues leen el selector sin sobrescribirlo.

/data/pes_modo.cfg se crea con 1 (PES Original) si falta o es invalido.
0 conserva FC; 1 conserva PES; tambien se mantiene compatibilidad con 2
(PES Holland), presente en el menu existente. Se guarda solo cuando cambia
el selector, mediante append y fsync. La ultima linea no vacia es el valor
vigente: las lineas anteriores se conservan para no destruir informacion.
Un archivo manual con un solo digito tambien es valido.

Los errores de lectura de memoria no se convierten en cambios de modo.
Los errores de escritura a disco se registran y reintentan mientras el juego
sigue conectado, sin restaurar un valor viejo en memoria. Firma, version,
permisos y perfil_cargado conservan sus comprobaciones. Un lock impide dos
instancias nuevas simultaneas; no bloquea daemons antiguos sin ese lock.

La variante distribuida es src/main.c. Su sondeo es de 500 ms con FC abierto
y 2 s sin juego; main.c sondea cada 2 s. El modo FC tambien restaura los
parches cuando la BD aparece despues del cambio de selector. Se conserva
la correccion de IA MTP. Requiere el eboot modificado compatible y autoload
del payload tras activar el jailbreak.

Compilar con compilar.ps1 en una carpeta nueva. Test_motor usa mocks de
memoria y un archivo real en una ruta nueva pasada como argumento; cubre
arranque por defecto, FC persistido tras reconectar, cambios 0/1/2 sin
escrituras forzadas, archivo invalido, fallo de disco/reintento, firma,
version, permisos y regresiones del motor de parches.

Limites: esperar al menos 2 segundos despues de cambiar el selector antes
de cerrar/reiniciar. Un cierre antes del siguiente sondeo o un fallo de disco
persistente puede perder el ultimo cambio. El historial crece 3 bytes por
cambio y no se compacta. La comprobacion de reinicio fisico y del selector
real requiere abrir FC en consola; los mocks no acreditan esos pasos.

## Despliegue verificado

Distribucion/pes2021_fc.elf y PS5 /data/pldmgr/payloads/pes2021_fc.elf:
SHA256 89f7c8d0548ffdff178a92efdf54545f656a5f53022f917912d12a5f6d25266f
(111992 bytes). Respaldos conservados en
G:\ProyFIFA\respaldo_consola\persistencia-3a5eef9b98ae y en PS5
/data/pes2021_fc.antes-persistencia-3a5eef9b98ae.bak.

Payload Manager confirmo parada de PID 95 y carga del nuevo ELF. Su log
identifica PID 120 y confirma persistencia lista; FTP comprobo el archivo
creado por el daemon con valor 1. Autoload ya contiene pes2021_fc.elf.
FC estaba cerrado. Pendiente prueba manual del selector y reinicio fisico.

Clang compilo ambas variantes con -Wall -Wextra -Werror. Readelf identifica
ELF64 DYN FreeBSD x86-64; advierte del tamano de PT_DYNAMIC del ELF distribuido
(el segmento del SDK incluye otras secciones). Payload Manager lo cargo y
el proceso ejecuto correctamente la creacion y sincronizacion del archivo.
