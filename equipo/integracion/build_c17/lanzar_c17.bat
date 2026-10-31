@echo off
REM Lanzador rapido de inyeccion C17 para PS5
set IP_DEFECTO=192.168.0.44

if "%1"=="" (
    set CONSOLA_IP=%IP_DEFECTO%
) else (
    set CONSOLA_IP=%1
)

echo ===================================================
echo     LANZANDO INYECTOR C17 (PES 2021 SOBRE FC 27)
echo     IP Destino: %CONSOLA_IP%
echo ===================================================

python pes_c17_inyector.py --ip %CONSOLA_IP%

pause
