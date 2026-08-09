# Guia de Build Paso a Paso

Documentacion detallada para compilar KeyboardToGamepad desde cero en Windows.

## Requisitos previos

1. **Windows 10/11 64-bit**
2. **Visual Studio 2022** con workload *"Desktop development with C++"*
3. **CMake 3.25+** (instalalo con el installer de Windows x64)
4. **Git** (incluido en VS2022 o instalado por separado)

## Paso 1: Obtener el codigo

Descarga el repo (o clonalo con Git):

```powershell
# Opcion A: Clonar con Git
git clone --recursive https://github.com/tu-usuario/keyboard-to-gamepad.git
cd keyboard-to-gamepad

# Opcion B: Descargar ZIP y extraer
# Descomprimi el ZIP en C:\Dev\keyboard-to-gamepad
```

## Paso 2: Setup automatico (PowerShell)

Desde la raiz del proyecto, ejecuta:

```powershell
.\setup.ps1
```

### Que hace este script?

1. Verifica que tengas Git y CMake accesibles.
2. Crea el directorio `deps/`.
3. Clona las dependencias desde GitHub:
   - `imgui` (branch `docking`)
   - `nlohmann_json` (branch `develop`)
   - `spdlog` (branch `v1.x`)
   - `vigem-client` (repo oficial de Nefarius)
4. Verifica que todos los archivos necesarios existan.

> **Nota sobre MSBuild**: El script advierte si no encuentra MSBuild. Esto es normal si no estas en *Developer PowerShell for VS 2022*. Las dependencias se descargan igual; MSBuild solo se necesita para compilar.

> Si el script falla por permisos de ejecucion:
> ```powershell
> Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
> ```

## Paso 3: Compilar

**IMPORTANTE**: Abrí **Developer PowerShell for VS 2022** (buscalo en el menu Inicio).

Navega al repo:

```powershell
cd C:\Dev\keyboard-to-gamepad
```

Ejecuta el build:

```powershell
.\build.ps1
```

### Que hace este script?

1. Ejecuta `cmake -B build -S . -A x64` para generar la solucion.
2. Compila con `cmake --build build --config Release`.
3. Copia el `.exe` resultante a tu carpeta `Downloads` para conveniencia.

### Build limpio

Si queres borrar todo y recompilar desde cero:

```powershell
.\build.ps1 -Clean
```

### Generar instalador

```powershell
.\build.ps1 -Installer
```

Esto requiere que tengas Inno Setup 6 instalado en `C:\Program Files (x86)\Inno Setup 6\`.

## Paso 4: Ejecutar

```powershell
.\build\bin\Release\KeyboardToGamepad.exe
```

Deberias ver:
- Una ventana con interfaz Dear ImGui.
- Un icono en la bandeja del sistema.
- Al minimizar, la ventana se oculta (recuperala haciendo doble click en el icono de tray).
- El panel "Dispositivos Conectados" muestra teclados USB detectados.

## Paso 5: Instalar ViGEmBus (requerido para Milestone 2+)

El driver ViGEmBus es necesario para crear gamepads virtuales.

1. Descarga el ultimo release desde: https://github.com/nefarius/ViGEmBus/releases
2. Ejecuta `ViGEmBusSetup_x64.msi` con privilegios de administrador.
3. Reinicia si te lo pide.

> El instalador final del Milestone 4 hara esto automaticamente.

## Solucion de problemas

### "MSBuild no encontrado"

Abrí **Developer PowerShell for VS 2022**, no PowerShell común. Esto configura las variables de entorno de MSVC.

### "CMake no encontrado"

Agrega CMake al PATH durante la instalacion, o reinicia la terminal despues de instalarlo.

### "Cannot open include file: 'ViGEm/Client.h'"

Ejecuta `setup.ps1` nuevamente. Verifica que exista `deps/vigem-client/include/ViGEm/Client.h`.

### "imgui.cpp no encontrado"

Los clones no se completaron. Ejecuta:

```powershell
.\setup.ps1
```

O manualmente:

```powershell
git clone --branch docking --depth 1 https://github.com/ocornut/imgui.git deps/imgui
git clone --branch develop --depth 1 https://github.com/nlohmann/json.git deps/nlohmann_json
git clone --branch v1.x --depth 1 https://github.com/gabime/spdlog.git deps/spdlog
git clone --depth 1 https://github.com/nefarius/ViGEmClient.git deps/vigem-client
```

## Build manual (sin scripts)

Si preferis no usar los scripts de PowerShell:

```powershell
# 1. Clonar dependencias manualmente
git clone --branch docking --depth 1 https://github.com/ocornut/imgui.git deps/imgui
git clone --branch develop --depth 1 https://github.com/nlohmann/json.git deps/nlohmann_json
git clone --branch v1.x --depth 1 https://github.com/gabime/spdlog.git deps/spdlog
git clone --depth 1 https://github.com/nefarius/ViGEmClient.git deps/vigem-client

# 2. Configurar y compilar
cmake -B build -S . -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
