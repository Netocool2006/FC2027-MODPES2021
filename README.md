# FC 27 -> PES 2021 GAMEPLAY PORT (PS5 Jailbreak)
### Conversión completa de físicas, IA táctica y sensaciones de juego de eFootball PES 2021 sobre EA Sports FC 27 (`PPSA34015`)

Este proyecto implementa mediante ingeniería inversa profunda y modificación en memoria nativa la traslación completa del motor de físicas, peso de balón, disputas corporales, compactación táctica y arbitraje de **eFootball PES 2021 (FOX Engine)** sobre el motor **Frostbite de EA Sports FC 27 en PlayStation 5**.

Incluye:
1. **`pes2021_fc.elf`**: Daemon nativo en C para PS5 (Payload Manager / `autoload.txt`) que se ejecuta de forma autónoma en segundo plano sin requerir PC.
2. **`eboot.bin`**: Binario modificado con inyección de hooks en el menú de ajustes de partido para alternar entre "PES 2021 Original", "PES 2021 Holland" y "FC 27 Nativo" en vivo.
3. **Documentación Técnica**: Mapeo completo de ingeniería inversa cruzada entre archivos binarios de Konami y esquemas Frostbite de EA.

---

## 🎮 CARACTERÍSTICAS DEL PORT (46 PARÁMETROS CALIBRADOS)

- **Física del Balón PES:** Balón con masa de 0.470 kg, inercia de 0.76, resistencia aerodinámica natural y fricción de césped elevada (`0.82` dinámica, `0.90` estática). Elimina el efecto de "disco de hockey deslizante" y el rebote plástico excesivo.
- **Inercia y Primer Toque:** Ventana de amortiguación ampliada (`75.0` y `125.0` ticks) y penalización por giro brusco; se elimina el balón imantado al pie durante el sprint.
- **Bloque Defensivo Compacto:** Líneas longitudinales reducidas de 44 m a **31.5 m** (fórmula exacta de PES `constant_team`: `dfLineRate 0.2` sobre 35 m); centrales juntos (`0.85`) sin huecos irreales entre defensa y mediocampo.
- **Porteros Orgánicos:** Desmagnetización de la cobertura milagrosa asistida (`AutoSaveRadius` reducido de 4.50 a 2.80) y aumento del umbral de rechace en tiros fuertes (evita blocajes imposibles a bocajarro).
- **Fatiga Dinámica Real:** El jugador controlado activamente por el usuario se cansa igual que la IA (`DisableFatigueOnControlledPlayer = 0`). La velocidad punta en fatiga alta cae al 85%.
- **Duelos Aéreos y Cabeceo:** Neutralización de saltos artificiales generados por PlayStyles (`AerialMaster_HeightBoost = 0.0`), basando el salto únicamente en altura y corpulencia.
- **Arbitraje Justo:** Tolerancia ampliada en los 11 niveles de severidad (`1.15`), permitiendo choques hombro con hombro legales de hasta 135° sin tarjetas absurdas.

---

## 📦 INSTALACIÓN EN CUALQUIER PS5 (JAILBREAK)

Para instalar este MOD en cualquier PS5 jailbreak (FW 3.xx - 4.xx - 13.00 con Payload Manager activo):

### Paso 1: Copiar el `eboot.bin` modificado
Copia el archivo `distribucion/eboot.bin` a la ruta de ShadowMount / backports de tu PS5 mediante FTP:
```text
/data/homebrew/backports/PPSA34015/eboot.bin
```

### Paso 2: Instalar el Payload Autónomo (`pes2021_fc.elf`)
1. Sube por FTP el archivo `distribucion/pes2021_fc.elf` a:
   ```text
   /data/pldmgr/payloads/pes2021_fc.elf
   ```
2. Descarga `/data/pldmgr/autoload.txt`, añade al final:
   ```text
   !3000
   pes2021_fc.elf
   ```
   y vuelve a subirlo por FTP.

---

## 🕹️ CÓMO JUGAR
1. Enciende la PS5 y entra en modo Jailbreak. El Payload Manager cargará automáticamente `pes2021_fc.elf`.
2. Inicia EA Sports FC 27.
3. En la pantalla previa al partido, ve a **Ajustes de Juego > Estilo de Jugabilidad**:
   - Selecciona **"PES2021 Original"** o **"PES2021 Holland"**.
   - Aparecerá la notificación en pantalla: *"PES 2021 Original Activado"*.
   - ¡Todos los parámetros entrarán en vigor de inmediato!
   - Si seleccionas *"Predeterminado / FC 27"*, el sistema restaura automáticamente la jugabilidad original de EA.

---

Para más detalles sobre la ingeniería inversa y el código fuente, consulta [DOC_TECNICA.md](DOC_TECNICA.md).

### Persistencia del selector

El daemon recuerda la ultima seleccion en `/data/pes_modo.cfg`: PES 2021
por defecto si no hay estado guardado, o FC si fue la ultima eleccion.
Al abrir FC aplica el estado una sola vez; despues respeta el menu y guarda
los cambios automaticamente. Espera al menos 2 segundos tras cambiar la
opcion antes de cerrar el juego o reiniciar la consola. El archivo conserva
un historial: la ultima linea no vacia indica el modo vigente (0 FC, 1 PES
Original, 2 Holland compatible). Requiere Payload Manager en autoload tras
activar el jailbreak.
