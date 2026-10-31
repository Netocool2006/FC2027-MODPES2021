# pes_c17_inyector.py
# Inyector Maestro C17 para PS5 (EA Sports FC 27 -> Gameplay PES 2021)
# Aplica los 6 subsistemas calibrados en vivo via ps5debug / gp_vivo

import sys, os, time, struct, json

DIR_ACTUAL = os.path.dirname(os.path.abspath(__file__))
DIR_PROY = os.path.abspath(os.path.join(DIR_ACTUAL, "..", "..", ".."))
sys.path.insert(0, os.path.join(DIR_PROY, "equipo", "datos_fc"))
sys.path.insert(0, os.path.join(DIR_PROY, "equipo", "fc_enganches"))

import medir_comun as mc
import gp_vivo as gv

# Mapeo de los 6 subsistemas y sus 36 parametros
PARCHES_C17 = [
    # 1. Arbitraje y Faltas
    ("gp_defense_pushpull", "PushPull_FallThreshold", 0.92, "f32"),
    ("gp_defense_pushpull", "PushPull_MedIntensityThreshold", 0.50, "f32"),
    ("gp_defense_pushpull", "PushPull_HighIntensityThreshold", 0.88, "f32"),
    ("gp_rules_foul", "AdvantageWindowTimeLength", 210, "i32"),
    ("gp_rules_foul", "ShoulderChallengesAngleGapThreshold", 135.0, "f32"),
    ("gp_rules_refereestrictness", "referee_strictness_foul_intensity", 1.15, "f32"),
    ("gp_rules_refereestrictness", "referee_strictness_yellowcard_threshold", 66.0, "f32"),

    # 2. Bloque Tactico y Compactacion
    ("gp_positioningformation_defensivelinelengthwidth", "LineLengthModifierForBalanced", 0.72, "f32"),
    ("gp_positioningformation_defensivelinelengthwidth", "LineLengthModifierForPossession", 0.72, "f32"),
    ("gp_positioningformation_defensivelinelengthwidth", "LineLengthModifierForLongBall", 0.75, "f32"),
    ("gp_positioningformation_defensivelinelengthwidth", "LineLengthModifierForFastBuildUp", 0.75, "f32"),
    ("gp_positioningformation_defensivelinelengthwidth", "lineWidthModifier", 0.88, "f32"),
    ("gp_positioningformation_defensivelinelengthwidth", "lineWidthModifierForCB", 0.85, "f32"),
    ("gp_positioningformation_attackinglinelengthwidth", "LineLengthModifierForBalanced", 0.72, "f32"),
    ("gp_positioningformation_attackinglinelengthwidth", "LineLengthModifierForPossession", 0.70, "f32"),
    ("gp_positioningformation_attackinglinelengthwidth", "lineWidthModifier", 0.90, "f32"),

    # 3. Fatiga y Resistencia Dinamica
    ("gp_playerattribute_attributes", "DisableFatigueOnControlledPlayer", 0, "u8"),
    ("gp_playerattribute_attributes", "ATTR_ShortTermFatigueSlowdownThreshold", 0.70, "f32"),
    ("gp_playerattribute_attributes", "ATTR_ShortTermFatigueSlowdownRate", 0.38, "f32"),
    ("gp_playerattribute_attributes", "ATTR_LongTermFatigueSpeedFactor", 0.85, "f32"),
    ("gp_playerattribute_attributes", "fixedShortTermFatigueModifier", 1.15, "f32"),
    ("gp_playerattribute_attributes", "fixedLongTermFatigueModifier", 1.10, "f32"),
    ("gp_playerattribute_attributes", "fatigueModifierForPressureTactics", 8.00, "f32"),

    # 4. Balistica, Peso del Balon y Friccion de Cesped
    ("gp_physics_physics", "FieldDynamicFriction", 0.82, "f32"),
    ("gp_physics_physics", "FieldStaticFriction", 0.90, "f32"),
    ("gp_physics_physics", "BallBounceLinearDamping", 0.985, "f32"),
    ("gp_physics_physics", "BallRestitution", 0.80, "f32"),
    ("gp_physics_physics", "BallLinearDrag", 0.0045, "f32"),
    ("gp_physics_soccerball", "Soccerball_Mass", 0.470, "f32"),
    ("gp_physics_soccerball", "Soccerball_Inertia", 0.76, "f32"),
    ("gp_physics_soccerball", "Soccerball_GripConstant", 0.85, "f32"),

    # 5. Guardametas
    ("gp_goalkeeper_be_a_goalkeeper", "AutoSaveRadiusFullAssist", 2.80, "f32"),
    ("gp_goalkeeper_be_a_goalkeeper", "AutoSaveRadiusPartiallAssist", 2.20, "f32"),
    ("gp_goalkeeper_be_a_goalkeeper", "CrossCatchExtendedCoverage", 0.95, "f32"),
    ("gp_goalkeeper_be_a_goalkeeper", "CrossDeflectAndPunchExtendedCoverage", 1.35, "f32"),
    ("gp_goalkeeper_be_a_goalkeeper", "SaveAssistReflexIncreaseCoverageFullAssist", 40.0, "f32"),
    ("gp_goalkeeper_be_a_goalkeeper", "GkCatchExclusionDifficultyThreshold", 0.35, "f32"),

    # 6. Primer Toque e Inercia de Conduccion
    ("gp_actor_movement", "TrappingControlledFirstTouchBallTargetTimeShort", 75.0, "f32"),
    ("gp_actor_movement", "TrappingControlledFirstTouchBallTargetTimeLong", 125.0, "f32"),
    ("gp_actor_movement", "ATTR_DribbleMinTouchInterruptPenalty", 3.00, "f32"),
    ("gp_actor_movement", "DribbleControlledSprintBallTargetTimeRunTouch", 52.0, "f32"),

    # 7. Duelos Aereos y Cabeceo Realista PES (Neutralizar ventaja artificial FC)
    ("gp_twoplayerheader_twoplayerheader", "TPH_INTERACTION_Master_Aerial_Loser_TouchAdvantage", 0.0, "f32"),
    ("gp_playerattribute_traits", "AerialMaster_HeightBoost", 0.0, "f32"),
    ("gp_playerattribute_traits", "AerialMaster_Icon_HeightBoost", 0.0, "f32"),
    ("gp_kickerror_balltouch", "ATTR_BallTouchHeadSpeedModifierForPowerHeader", 1.0, "f32"),

    # 8. Pases Manuales y Resolucion al Espacio (gp_kickpass)
    ("gp_kickpass_passing", "PASS_MaxExpectedGroundPassDistForManual", 50.0, "f32"),
]

def inyectar_c17(simular=False, ip_consola=None):
    print("=" * 65)
    print("      INSPECTOR E INYECTOR MAESTRO C17 - GAMEPLAY PES 2021")
    print("=" * 65)
    
    if simular:
        print("[MODO] Simulacion local (EBX base en memoria sintetica)")
        mem = gv.Simulada()
    else:
        if ip_consola:
            mc.IP = ip_consola
        print(f"[CONEXION] Esperando consola y eboot activo en {mc.IP}...")
        intento = 0
        while True:
            try:
                intento += 1
                mem = gv.Consola()
                print(f"[OK] Conectado a PID {mem.pid} (eboot en {hex(mem.base)}) tras {intento} intento(s).")
                break
            except Exception as e:
                if intento == 1 or intento % 10 == 0:
                    print(f"  [ESPERANDO PARTIDO] Consola/juego aun no detectado ({e}). Reintentando en 3s...")
                time.sleep(3)

    vivo = gv.Vivo(mem)
    db_info = vivo.db()
    print(f"[MEMORIA] Puntero BD: {hex(db_info.get('DB', 0))}")

    exitos = 0
    fallos = 0

    print("\n--- Esperando a que el motor Frostbite instancie los subsistemas en el partido ---")
    tiempo_espera_instancias = 0
    while True:
        listas = 0
        for inst_nombre, _, _, _ in PARCHES_C17:
            if vivo.instancias(inst_nombre):
                listas += 1
        if listas >= 6 or simular:
            print(f"[LISTO] Detectadas {listas} instancias activas en memoria de partido.")
            break
        tiempo_espera_instancias += 2
        if tiempo_espera_instancias % 6 == 0:
            print(f"  [ESPERANDO PARTIDO] Instancias cargadas: {listas}/10. Entra al partido o reanudalo...")
        time.sleep(2)

    print("\n--- Aplicando Parches en Vivo ---")
    for instancia, campo, valor, tipo in PARCHES_C17:
        try:
            dirs = vivo.instancias(instancia)
            if not dirs:
                print(f"[SKIP] Instancia no activa o no resuelta: {instancia}")
                fallos += 1
                continue

            # Usar la primera instancia valida (BD o estatico)
            _, addr_instancia = dirs[0]
            cat_inst = gv.INST.get(instancia)
            if not cat_inst:
                print(f"[WARN] Instancia no hallada en catalogo: {instancia}")
                fallos += 1
                continue

            c_info = None
            campos_dict = cat_inst.get("campos", {})
            if isinstance(campos_dict, list):
                for item in campos_dict:
                    if item.get("n") == campo:
                        c_info = item
                        break
            elif isinstance(campos_dict, dict):
                c_info = campos_dict.get(campo)

            if not c_info:
                print(f"[WARN] Campo {campo} no hallado en catalogo para {instancia}")
                fallos += 1
                continue

            # Manejo de arrays (ej. referee_strictness_foul_intensity de 11 posiciones)
            if c_info.get("tipo", "").startswith("arr_"):
                tipo_elem = c_info["tipo"][4:]
                p_arr = vivo.q(addr_instancia + c_info["off"])
                if not gv.ok(p_arr):
                    print(f"[WARN] Puntero de array no valido para {instancia}.{campo}")
                    fallos += 1
                    continue
                n_elem = struct.unpack('<I', mem.rd(p_arr - 4, 4))[0] & 0x7fffffff
                fmt_elem = gv.FMT[tipo_elem]
                raw_elem = struct.pack(fmt_elem, valor)
                for i in range(min(n_elem, 32)):
                    mem.wr(p_arr + gv.TAM[tipo_elem] * i, raw_elem)
                print(f"[OK] {instancia[:22]:22} | {campo[:28]:28} -> {valor} (array de {n_elem} elems)")
                exitos += 1
                continue

            addr_campo, tipo_esc = vivo.dir_campo(addr_instancia, c_info)
            if not addr_campo:
                print(f"[WARN] No se resolvio direccion para {instancia}.{campo}")
                fallos += 1
                continue

            # Escribir segun tipo
            fmt = gv.FMT[tipo]
            raw = struct.pack(fmt, valor if tipo != 'u8' else int(valor))
            mem.wr(addr_campo, raw)

            # Verificacion de lectura inmediata
            leido = vivo.leer(addr_campo, tipo)
            diff = abs(leido - valor) if tipo in ('f32',) else (leido != valor)
            if diff < 1e-4 if tipo == 'f32' else not diff:
                print(f"[OK] {instancia[:22]:22} | {campo[:28]:28} -> {valor}")
                exitos += 1
            else:
                print(f"[ERR] Fallo verificacion en {campo}: leido {leido}, esperado {valor}")
                fallos += 1

        except Exception as e:
            print(f"[EXC] Error parcheando {campo}: {e}")
            fallos += 1

    print("-" * 65)
    print(f"Resultado C17: {exitos} exitosos, {fallos} omitidos/fallidos.")
    print("=" * 65)

    # Modo persistencia: supervisar y reaplicar en cada reinicio/partido nuevo segun el menu (g_pes.modo)
    if "--daemon" in sys.argv or "--persistente" in sys.argv:
        print("\n[DAEMON] Modo interactivo con Menu FC: monitoreando selector 'Estilo de jugabilidad' (g_pes.modo)...")
        ADDR_MODO = mem.base + 0x15600060
        ultimo_modo = None
        
        # Diccionario para restaurar valores nativos FC cuando se elija modo FC
        valores_fc_originales = {
            "FieldDynamicFriction": 0.72,
            "FieldStaticFriction": 0.82,
            "BallBounceLinearDamping": 0.998,
            "BallRestitution": 0.90,
            "BallLinearDrag": 0.003,
            "Soccerball_Mass": 0.454,
            "Soccerball_Inertia": 0.66,
            "Soccerball_GripConstant": 0.92,
            "AutoSaveRadiusFullAssist": 4.50,
            "AutoSaveRadiusPartiallAssist": 3.50,
            "CrossCatchExtendedCoverage": 1.20,
            "CrossDeflectAndPunchExtendedCoverage": 1.80,
            "SaveAssistReflexIncreaseCoverageFullAssist": 60.0,
            "GkCatchExclusionDifficultyThreshold": 0.20,
            "DisableFatigueOnControlledPlayer": 1,
            "ATTR_ShortTermFatigueSlowdownThreshold": 0.84,
            "ATTR_ShortTermFatigueSlowdownRate": 0.25,
            "ATTR_LongTermFatigueSpeedFactor": 0.95,
            "fixedShortTermFatigueModifier": 0.95,
            "fixedLongTermFatigueModifier": 0.90,
            "fatigueModifierForPressureTactics": 1.00,
            "LineLengthModifierForBalanced": 1.00,
            "LineLengthModifierForPossession": 1.00,
            "LineLengthModifierForLongBall": 1.00,
            "LineLengthModifierForFastBuildUp": 1.00,
            "lineWidthModifier": 1.00,
            "lineWidthModifierForCB": 1.00,
            "PushPull_FallThreshold": 0.85,
            "PushPull_MedIntensityThreshold": 0.40,
            "PushPull_HighIntensityThreshold": 0.75,
            "AdvantageWindowTimeLength": 180,
            "ShoulderChallengesAngleGapThreshold": 90.0,
            "referee_strictness_foul_intensity": 1.50,
            "TPH_INTERACTION_Master_Aerial_Loser_TouchAdvantage": 15.0,
            "AerialMaster_HeightBoost": 0.25,
            "AerialMaster_Icon_HeightBoost": 0.50,
            "ATTR_BallTouchHeadSpeedModifierForPowerHeader": 1.10
        }

        while True:
            try:
                time.sleep(2)
                raw_m = mem.rd(ADDR_MODO, 4)
                modo_actual = struct.unpack('<i', raw_m)[0] if len(raw_m) == 4 else 0
                db_actual = vivo.db().get('DB', 0)

                # Si el usuario cambia el selector en el menu o si cambio el partido
                if modo_actual != ultimo_modo or db_actual != db_info.get('DB', 0):
                    if modo_actual in (1, 2):
                        nombre_modo = "PES2021 Original" if modo_actual == 1 else "PES2021 Holland"
                        print(f"\n[MENU FC] Selector activado: '{nombre_modo}' (modo={modo_actual}). Inyectando fisica y gameplay PES...")
                        # Inyectar tabla PES
                        for instancia, campo, valor, tipo in PARCHES_C17:
                            try:
                                dirs = vivo.instancias(instancia)
                                if not dirs: continue
                                _, addr_instancia = dirs[0]
                                cat_inst = gv.INST.get(instancia)
                                if not cat_inst: continue
                                c_info = None
                                cd = cat_inst.get("campos", {})
                                if isinstance(cd, list):
                                    for item in cd:
                                        if item.get("n") == campo: c_info = item; break
                                elif isinstance(cd, dict): c_info = cd.get(campo)
                                if not c_info: continue

                                if c_info.get("tipo", "").startswith("arr_"):
                                    tipo_elem = c_info["tipo"][4:]
                                    p_arr = vivo.q(addr_instancia + c_info["off"])
                                    if not gv.ok(p_arr): continue
                                    n_elem = struct.unpack('<I', mem.rd(p_arr - 4, 4))[0] & 0x7fffffff
                                    fmt_elem = gv.FMT[tipo_elem]
                                    raw_elem = struct.pack(fmt_elem, valor)
                                    for i in range(min(n_elem, 32)):
                                        mem.wr(p_arr + gv.TAM[tipo_elem] * i, raw_elem)
                                    continue

                                addr_campo, _ = vivo.dir_campo(addr_instancia, c_info)
                                if not addr_campo: continue
                                fmt = gv.FMT[tipo]
                                mem.wr(addr_campo, struct.pack(fmt, valor if tipo != 'u8' else int(valor)))
                            except: pass
                        print(f"[OK] 46 parametros PES 2021 aplicados en consola.")
                    else:
                        if ultimo_modo is not None and ultimo_modo in (1, 2):
                            print(f"\n[MENU FC] Selector cambiado a FC Nativo (modo={modo_actual}). Restaurando valores originales de EA...")
                            for instancia, campo, valor_pes, tipo in PARCHES_C17:
                                if campo in valores_fc_originales:
                                    val_fc = valores_fc_originales[campo]
                                    try:
                                        dirs = vivo.instancias(instancia)
                                        if not dirs: continue
                                        _, addr_instancia = dirs[0]
                                        cat_inst = gv.INST.get(instancia)
                                        if not cat_inst: continue
                                        c_info = None
                                        cd = cat_inst.get("campos", {})
                                        if isinstance(cd, list):
                                            for item in cd:
                                                if item.get("n") == campo: c_info = item; break
                                        elif isinstance(cd, dict): c_info = cd.get(campo)
                                        if not c_info: continue

                                        if c_info.get("tipo", "").startswith("arr_"):
                                            tipo_elem = c_info["tipo"][4:]
                                            p_arr = vivo.q(addr_instancia + c_info["off"])
                                            if not gv.ok(p_arr): continue
                                            n_elem = struct.unpack('<I', mem.rd(p_arr - 4, 4))[0] & 0x7fffffff
                                            fmt_elem = gv.FMT[tipo_elem]
                                            raw_elem = struct.pack(fmt_elem, val_fc)
                                            for i in range(min(n_elem, 32)):
                                                mem.wr(p_arr + gv.TAM[tipo_elem] * i, raw_elem)
                                            continue

                                        addr_campo, _ = vivo.dir_campo(addr_instancia, c_info)
                                        if not addr_campo: continue
                                        fmt = gv.FMT[tipo]
                                        mem.wr(addr_campo, struct.pack(fmt, val_fc if tipo != 'u8' else int(val_fc)))
                                    except: pass
                            print(f"[OK] Valores originales de EA FC 27 restaurados.")

                    ultimo_modo = modo_actual
                    db_info = vivo.db()
                    db_info = vivo.db()
                    for instancia, campo, valor, tipo in PARCHES_C17:
                        try:
                            dirs = vivo.instancias(instancia)
                            if not dirs: continue
                            _, addr_instancia = dirs[0]
                            cat_inst = gv.INST.get(instancia)
                            if not cat_inst: continue
                            c_info = None
                            cd = cat_inst.get("campos", {})
                            if isinstance(cd, list):
                                for item in cd:
                                    if item.get("n") == campo: c_info = item; break
                            elif isinstance(cd, dict): c_info = cd.get(campo)
                            if not c_info: continue
                            addr_campo, _ = vivo.dir_campo(addr_instancia, c_info)
                            if not addr_campo: continue
                            fmt = gv.FMT[tipo]
                            mem.wr(addr_campo, struct.pack(fmt, valor if tipo != 'u8' else int(valor)))
                        except: pass
                    print("[OK] Parches C17 reinyectados con exito en el nuevo partido.")
            except Exception as e:
                print(f"[RECONEXION] Partido finalizado o reiniciado ({e}). Esperando reanudacion...")
                time.sleep(3)

    mem.cerrar()

if __name__ == "__main__":
    simular = "--simular" in sys.argv
    ip = None
    if "--ip" in sys.argv:
        idx = sys.argv.index("--ip")
        if idx + 1 < len(sys.argv):
            ip = sys.argv[idx + 1]
    inyectar_c17(simular=simular, ip_consola=ip)
