# KeyboardToGamepad

Convierte multiples teclados USB en gamepads XInput independientes para juegos local multiplayer.

> **Estado**: Milestone 1 — Raw Input detectando teclados, UI con lista de dispositivos, auto-asignacion de Players, y logging estructurado.

## Caracteristicas (Roadmap)

- [x] Scaffold C++20 + CMake + Dear ImGui
- [x] Ventana con DirectX 11 y docking UI
- [x] Icono en bandeja del sistema (minimizar a tray)
- [x] **Raw Input: detectar multiples teclados fisicos con handles distintos**
- [x] **DeviceRouter con auto-asignacion de Player 1-4**
- [x] **Logging estructurado con niveles (TRACE/DEBUG/INFO/WARN/ERROR/FATAL)**
- [x] **UI en tiempo real: lista de teclados conectados, VID/PID, botones de asignacion**
- [ ] Gamepads virtuales XInput via ViGEmBus *(Milestone 2)*
- [ ] Mapeo visual de teclas por jugador *(Milestone 3)*
- [ ] Instalador con ViGEmBus integrado *(Milestone 4)*

## Uso rapido (Desarrollo)

### 1. Clonar repo

```powershell
git clone --recursive https://github.com/tu-usuario/keyboard-to-gamepad.git
cd keyboard-to-gamepad
```

### 2. Setup automatico (PowerShell)

```powershell
.\setup.ps1
```

Esto clona automaticamente:
- Dear ImGui (branch `docking`)
- nlohmann/json
- spdlog
- **ViGEmClient** (desde GitHub, compilado como parte del build)

> Si da error de permisos de ejecucion: `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`

### 3. Build (PowerShell)

**Importante**: Abrí **"Developer PowerShell for VS 2022"** desde el menu Inicio (necesita MSVC en el PATH):

```powershell
cd C:\ruta\al\repo
.\build.ps1
```

El ejecutable queda en `build/bin/Release/KeyboardToGamepad.exe`.

### 4. Ejecutar y probar

```powershell
.\build\bin\Release\KeyboardToGamepad.exe
```

**Para probar el Milestone 1:**
1. Conecta 2 o mas teclados USB.
2. En la ventana, vas a ver aparecer cada teclado en el panel "Dispositivos Conectados".
3. Cada teclado se asigna automaticamente como Player 1, 2, 3 o 4.
4. Apreta una tecla en cualquier teclado: el log registra el evento con el Player correspondiente.
5. Los logs se guardan en `%LOCALAPPDATA%\KeyboardToGamepad\logs\app.log`.

### 5. Build limpio

```powershell
.\build.ps1 -Clean
```

### 6. Generar instalador (futuro, Milestone 4)

```powershell
.\build.ps1 -Installer
```

Requiere [Inno Setup 6](https://jrsoftware.org/isdl.php) instalado.

## Estructura del proyecto

```
keyboard-to-gamepad/
├── src/
│   ├── app/          # Application singleton, Constants, Version
│   ├── core/         # RawInputManager, DeviceRouter, GamepadManager (ViGEm)
│   ├── ui/           # Dear ImGui + DX11 renderer + TrayIcon
│   └── utils/        # Logger (spdlog), Win32 helpers
├── deps/             # Dependencias (git clones)
│   ├── imgui/        # Dear ImGui (docking branch)
│   ├── nlohmann_json/ # JSON library
│   ├── spdlog/       # Logging
│   └── vigem-client/ # ViGEmClient SDK (compilado con el proyecto)
├── installer/        # Script Inno Setup
├── config/           # Perfiles JSON por defecto
├── setup.ps1         # Setup automatico del entorno
├── build.ps1         # Build automatico
└── CMakeLists.txt
```

## Prerrequisitos

| Herramienta | Version | Descarga |
|-------------|---------|----------|
| Visual Studio 2022 | Community o superior | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/vs/) |
| CMake | 3.25+ | [cmake.org/download](https://cmake.org/download/) |
| Git | Cualquiera | [git-scm.com](https://git-scm.com/download/win) |
| Inno Setup | 6.2+ *(solo para installer)* | [jrsoftware.org](https://jrsoftware.org/isdl.php) |

**Workload requerido en VS2022**: *Desktop development with C++* (incluye Windows SDK, MSVC, CMake).

## Logging

El sistema de logging guarda todo en `%LOCALAPPDATA%\KeyboardToGamepad\logs\app.log`:

```
================================================================================
  KeyboardToGamepad Session Started: 2026-08-08 18:30:15
================================================================================
18:30:15.123 [INFO ] ========================================
18:30:15.124 [INFO ]   KeyboardToGamepad v1.0.0
18:30:15.125 [INFO ] ========================================
18:30:15.200 [INFO ] [Application] Main window created: 1024x768
18:30:15.250 [INFO ] [Application] Core subsystems ready
18:30:15.251 [INFO ] [RawInputManager] Registered for keyboard Raw Input
18:30:20.001 [INFO ] [DeviceRouter] Keyboard found: HID#VID_046D&PID_C31C | VID_046D&PID_C31C | Player 1
18:30:25.500 [DEBUG] [DeviceRouter] Input from Player 1 (HID#VID_046D&PID_C31C): VK=0x0057 DOWN
```

Niveles: `TRACE < DEBUG < INFO < WARN < ERROR < FATAL`

## Licencia

MIT License — ver [LICENSE](LICENSE).

Todas las dependencias son compatibles con MIT (Dear ImGui, nlohmann/json, spdlog, ViGEmClient).

## Contribuir

1. Fork del repo
2. Branch: `git checkout -b feature/mi-feature`
3. Commit: `git commit -m "feat: descripcion"`
4. Push: `git push origin feature/mi-feature`
5. Abri un Pull Request

---

**Hecho para revivir el local multiplayer.**
