# DOCUMENTACIÓN TÉCNICA E INGENIERÍA INVERSA
## Port Gameplay PES 2021 sobre EA Sports FC 27 (PS5)

---

### 1. FUENTES PRIMARIAS DE INGENIERÍA INVERSA

#### A. Extracción y Desensamblado de eFootball PES 2021 (PC / FOX Engine)
Los datos de referencia física de Konami fueron extraídos directamente de los archivos binarios compilados del motor:
* **`dt18\common\match\constant\constant_match.bin`** (y su formato descomprimido `.dec`):
  * Sección `ball.o` (inicio `0x3C0`): Define la física de rebote por superficie (`boundRate`), coeficiente de fricción de cesped y coeficientes de rozamiento aerodinámico.
  * Desensamblado ASM de vuelo de balón: `analisis/pes/balon/f_865f20.asm`.
* **`dt18\common\match\constant\constant_team.bin` / `.dec`**:
  * Define el comportamiento táctico del bloque: `lengthOf = 35.0 m`, tasa de línea defensiva `dfLineRate = 0.2`, compactación longitudinal objetivo = **31.5 metros**.
* **`dt18\common\match\constant\constant_player.bin` / `.dec`**:
  * Curvas de resistencia y cansancio a 60 Hz, penalización de sprint prolongado e inercias de giro corporal.

#### B. Arquitectura Frostbite en EA Sports FC 27 (`PPSA34015`, PS5)
El motor de EA organiza las propiedades de jugabilidad en estructuras `gp_*` dentro de una base de datos dinámica en memoria:
* **Localizador de Base de Datos (DB Pointer):** `base_eboot + 0x11d9cae8`.
* **Desreferenciación:**
  ```c
  uint64_t X  = *(uint64_t*)(base + 0x11d9cae8);
  uint64_t X1 = *(uint64_t*)(X);
  uint64_t DB = *(uint64_t*)(X1 + 8);
  ```
* **Esquemas Primarios Identificados:**
  * `gp_physics` (`0x130`): Instancias `gp_physics_physics` (`0x58`) y `gp_physics_soccerball` (`0xb0`).
  * `gp_positioningformation` (`0x128`): Instancias `defensivelinelengthwidth` (`0x30`) y `attackinglinelengthwidth` (`0x28`).
  * `gp_playerattribute` (`0x168`): Instancia `gp_playerattribute_attributes` (`0x20`).
  * `gp_goalkeeper` (`0x160`): Instancia `gp_goalkeeper_be_a_goalkeeper` (`0x148`).
  * `gp_actor` (`0x48`): Instancia `gp_actor_movement` (`0x18`).
  * `gp_defense` (`0x138`): Instancia `gp_defense_pushpull` (`0x30`).
  * `gp_rules` (`0xb8`): Instancia `gp_rules_refereestrictness` (`0x68`) y `gp_rules_foul` (`0x50`).

---

### 2. MATRIZ DE PARÁMETROS CALIBRADOS (46 VALORES)

| Subsistema | Esquema e Instancia Frostbite | Offset | Tipo | Valor FC Nativo | Valor PES 2021 | Justificación Técnica |
|---|---|---|---|---|---|---|
| **Balística** | `gp_physics_physics` | `0x3c` | `f32` | 0.72 | **0.82** | `FieldDynamicFriction`: Frenado orgánico del balón sobre el césped. |
| **Balística** | `gp_physics_physics` | `0x68` | `f32` | 0.82 | **0.90** | `FieldStaticFriction`: Mayor agarre en controles rasos y balón parado. |
| **Balística** | `gp_physics_physics` | `0x34` | `f32` | 0.998 | **0.985** | `BallBounceLinearDamping`: Amortiguación del bote vertical. |
| **Balística** | `gp_physics_physics` | `0x58` | `f32` | 0.90 | **0.80** | `BallRestitution`: Elimina el rebote elástico de "globo de plástico". |
| **Balística** | `gp_physics_physics` | `0x50` | `f32` | 0.003 | **0.0045**| `BallLinearDrag`: Resistencia aerodinámica y parábola pesada en pases largos. |
| **Balística** | `gp_physics_soccerball` | `0x50` | `f32` | 0.454 | **0.470**| `Soccerball_Mass`: Peso percibido de cuero tradicional. |
| **Balística** | `gp_physics_soccerball` | `0x34` | `f32` | 0.66 | **0.76** | `Soccerball_Inertia`: Inercia de giro independiente del pie. |
| **Balística** | `gp_physics_soccerball` | `0x4c` | `f32` | 0.92 | **0.85** | `Soccerball_GripConstant`: Evita el imantado excesivo bota-balón. |
| **Táctica** | `gp_positioningformation_def` | `0x30` | `f32` | 1.00 | **0.72** | Compacta la longitud de línea en Balanceada (31.5 m). |
| **Táctica** | `gp_positioningformation_def` | `0x50` | `f32` | 1.00 | **0.72** | Longitud en Posesión. |
| **Táctica** | `gp_positioningformation_def` | `0x2c` | `f32` | 1.00 | **0.75** | Longitud en Pase Largo. |
| **Táctica** | `gp_positioningformation_def` | `0x54` | `f32` | 1.00 | **0.75** | Longitud en Construcción Rápida. |
| **Táctica** | `gp_positioningformation_def` | `0x28` | `f32` | 1.00 | **0.88** | Ancho de bloque defensivo. |
| **Táctica** | `gp_positioningformation_def` | `0x3c` | `f32` | 1.00 | **0.85** | Cierra la brecha interna entre la pareja de centrales. |
| **Fatiga** | `gp_playerattribute_attributes` | `0x101`| `u8` | 1 (on)| **0 (off)**| `DisableFatigueOnControlledPlayer`: Quita sprint infinito humano. |
| **Fatiga** | `gp_playerattribute_attributes` | `0xe8` | `f32` | 0.84 | **0.70** | Umbral de desaceleración por fatiga inmediata. |
| **Fatiga** | `gp_playerattribute_attributes` | `0xe0` | `f32` | 0.25 | **0.38** | Ratio de pérdida de aceleración en cansancio. |
| **Fatiga** | `gp_playerattribute_attributes` | `0xec` | `f32` | 0.95 | **0.85** | Velocidad máxima en tramos finales con fatiga acumulada. |
| **Porteros** | `gp_goalkeeper_be_a_goalkeeper`| `0x120`| `f32` | 4.50 | **2.80** | `AutoSaveRadiusFullAssist`: Reduce salvada teledirigida asistida. |
| **Porteros** | `gp_goalkeeper_be_a_goalkeeper`| `0x11c`| `f32` | 3.50 | **2.20** | Radio de salvada en asistencia parcial. |
| **Porteros** | `gp_goalkeeper_be_a_goalkeeper`| `0x198`| `f32` | 1.20 | **0.95** | Salidas aéreas por alto más disputadas con atacantes. |
| **Porteros** | `gp_goalkeeper_be_a_goalkeeper`| `0x138`| `f32` | 1.80 | **1.35** | Cobertura en despejes de puños. |
| **Porteros** | `gp_goalkeeper_be_a_goalkeeper`| `0x19c`| `f32` | 60.0 | **40.0** | Reduce estiradas milagrosas reflejas a bocajarro. |
| **Porteros** | `gp_goalkeeper_be_a_goalkeeper`| `0x150`| `f32` | 0.20 | **0.35** | `CatchExclusion`: Ante disparos duros opta por despeje/rechace. |
| **Control** | `gp_actor_movement` | `0x250`| `f32` | 65.0 | **75.0** | Ventana de amortiguación en primer toque corto. |
| **Control** | `gp_actor_movement` | `0x3d4`| `f32` | 110.0| **125.0**| Ventana de control en balones aéreos largos. |
| **Control** | `gp_actor_movement` | `0x20c`| `f32` | 2.00 | **3.00** | Penalización de inercia por cambio brusco de dirección. |
| **Control** | `gp_actor_movement` | `0x214`| `f32` | 45.0 | **52.0** | Toque de balón más despegado al correr al máximo. |
| **Aéreo** | `gp_twoplayerheader_twoplayer` | `0x34` | `f32` | 15.0 | **0.0** | Elimina ventaja artificial del perdedor aéreo. |
| **Aéreo** | `gp_playerattribute_traits` | `0x38` | `f32` | 0.25 | **0.0** | Neutraliza el boost de altura ficticio del PlayStyle. |
| **Aéreo** | `gp_playerattribute_traits` | `0x34` | `f32` | 0.50 | **0.0** | Neutraliza el boost de altura de PlayStyle Icon/Plus. |
| **Arbitraje** | `gp_defense_pushpull` | `0xa0` | `f32` | 0.85 | **0.92** | Umbral de caída: requiere más forcejeo real para derribar. |
| **Arbitraje** | `gp_defense_pushpull` | `0xa8` | `f32` | 0.40 | **0.50** | Umbral de disputa media cuerpo a cuerpo. |
| **Arbitraje** | `gp_defense_pushpull` | `0xb0` | `f32` | 0.75 | **0.88** | Umbral de forcejeo de alta intensidad. |
| **Arbitraje** | `gp_rules_foul` | `0x70` | `i32` | 180  | **210**  | Ventana de ley de la ventaja (3.5 segundos). |
| **Arbitraje** | `gp_rules_foul` | `0x94` | `f32` | 90.0 | **135.0**| Legaliza choques hombro con hombro laterales hasta 135°. |
| **Arbitraje** | `gp_rules_refereestrictness` | `0x70` | `arr11`| 1.50 | **1.15** | Severidad de falta en las 11 escalas de árbitro. |

---

### 3. INTEGRACIÓN DEL MENÚ Y SELECTOR DINÁMICO
El eboot modificado intercepta la función de renderizado de la UI de ajustes de partido y enlaza con un nuevo segmento de datos (`PF_R|PF_W`) en `0x15600000`:
* Dirección de variable de estado: **`base + 0x15600060` (`g_pes.modo`)**.
  * `0`: Modo Nativo EA Sports FC 27.
  * `1`: Modo PES 2021 Original.
  * `2`: Modo PES 2021 Holland.
* El daemon `pes2021_fc.elf` lee periódicamente esta dirección usando `kernel_proc_copyout`. Al detectar transición a `1` o `2`, aplica la tabla completa vía llamadas directas de kernel a la base de datos de Frostbite y emite la notificación en pantalla mediante `sceKernelSendNotificationRequest`.
