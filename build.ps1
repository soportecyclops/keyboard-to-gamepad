#Requires -Version 5.1
<#
.SYNOPSIS
    Build del proyecto KeyboardToGamepad.
.DESCRIPTION
    Configura con CMake y compila en modo Release.
    Ejecutar desde la raiz del repo, preferentemente en Developer PowerShell for VS 2022+.
.PARAMETER Clean
    Limpia el directorio de build antes de compilar.
.PARAMETER Installer
    Genera el instalador con Inno Setup al finalizar.
#>

param(
    [switch]$Clean,
    [switch]$Installer
)

$ErrorActionPreference = "Stop"

$repoRoot = $PSScriptRoot
$buildDir = Join-Path $repoRoot "build"
$binDir = Join-Path $buildDir "bin\Release"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  KeyboardToGamepad - Build" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Verificar CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    $cmakePaths = @(
        "C:\Program Files\CMake\bin\cmake.exe",
        "C:\Program Files (x86)\CMake\bin\cmake.exe"
    )
    foreach ($path in $cmakePaths) {
        if (Test-Path $path) {
            $env:Path = "$(Split-Path $path -Parent);$env:Path"
            $cmake = Get-Command cmake -ErrorAction SilentlyContinue
            break
        }
    }
    if (-not $cmake) {
        Write-Error "CMake no encontrado. Instalalo desde https://cmake.org/download/"
    }
}

Write-Host "CMake: $($cmake.Source)" -ForegroundColor Gray

# Verificar si estamos en Developer PowerShell
$isDevShell = $env:VisualStudioVersion -ne $null
if ($isDevShell) {
    Write-Host "Developer PowerShell detectado (VS $($env:VisualStudioVersion))" -ForegroundColor Green
}

# Verificar que el compilador C++ este disponible
$cl = Get-Command cl -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Warning "Compilador C++ (cl.exe) no encontrado. Intentando configurar vcvarsall..."

    $vcvarsPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
    )

    $vcvarsFound = $false
    foreach ($path in $vcvarsPaths) {
        if (Test-Path $path) {
            Write-Host "Configurando entorno desde: $path" -ForegroundColor Cyan
            $envDump = cmd /c "`"$path`" x64 && set"
            $envDump | ForEach-Object {
                if ($_ -match '^(.*?)=(.*)$') {
                    [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
                }
            }
            $vcvarsFound = $true
            break
        }
    }

    if (-not $vcvarsFound) {
        Write-Error "No se encontro vcvarsall.bat. Instala Visual Studio con 'Desktop development with C++'"
    }

    # Verificar de nuevo
    $cl = Get-Command cl -ErrorAction SilentlyContinue
    if (-not $cl) {
        Write-Error "cl.exe sigue sin estar disponible despues de configurar vcvarsall"
    }
    Write-Host "Compilador C++ OK: $($cl.Source)" -ForegroundColor Green
}

# Limpiar si se pidio
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "[1/4] Limpiando build anterior..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force $buildDir
    Write-Host "    Build anterior eliminado." -ForegroundColor Green
}

# Configurar CMake
Write-Host "`n[2/4] Configurando CMake..." -ForegroundColor Cyan

# Detectar el generador apropiado o dejar que CMake auto-detecte
$cmakeArgs = @("-B", $buildDir, "-S", $repoRoot)

# Si NO estamos en Developer PowerShell, forzar generador Visual Studio
if (-not $isDevShell) {
    $cmakeArgs += @("-G", "Visual Studio 17 2022", "-A", "x64")
    Write-Host "    Usando generador: Visual Studio 17 2022" -ForegroundColor Gray
} else {
    # En Developer PowerShell, dejar que CMake auto-detecte el generador
    # pero especificar la plataforma x64
    $cmakeArgs += "-A"
    $cmakeArgs += "x64"
    Write-Host "    Dejando que CMake auto-detecte el generador (Developer Shell)" -ForegroundColor Gray
}

if ($Installer) {
    $cmakeArgs += "-DKTG_BUILD_INSTALLER=ON"
}

& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
}
Write-Host "    CMake OK" -ForegroundColor Green

# Compilar
Write-Host "`n[3/4] Compilando..." -ForegroundColor Cyan
& cmake --build $buildDir --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
}
Write-Host "    Build OK" -ForegroundColor Green

# Verificar output
$exePath = Join-Path $binDir "KeyboardToGamepad.exe"
if (Test-Path $exePath) {
    $fileInfo = Get-Item $exePath
    Write-Host "`n[4/4] Ejecutable generado:" -ForegroundColor Cyan
    Write-Host "    $exePath" -ForegroundColor Green
    Write-Host "    Tamano: $([math]::Round($fileInfo.Length / 1KB, 2)) KB" -ForegroundColor Gray
} else {
    # Buscar en otras rutas posibles
    $found = Get-ChildItem -Path $buildDir -Recurse -Filter "KeyboardToGamepad.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        Write-Host "`n[4/4] Ejecutable encontrado:" -ForegroundColor Cyan
        Write-Host "    $($found.FullName)" -ForegroundColor Green
        $exePath = $found.FullName
    } else {
        Write-Error "No se encontro el ejecutable"
    }
}

# Instalador
if ($Installer) {
    Write-Host "`n[5/4] Generando instalador..." -ForegroundColor Cyan
    & cmake --build $buildDir --config Release --target package_installer
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Fallo la generacion del instalador. Tenes Inno Setup instalado?"
    } else {
        $setupFiles = Get-ChildItem "$buildDir\Setup_*.exe" -ErrorAction SilentlyContinue
        if ($setupFiles) {
            Write-Host "    Instalador generado:" -ForegroundColor Green
            foreach ($f in $setupFiles) {
                Write-Host "    $($f.FullName)" -ForegroundColor Green
            }
        }
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Build completo!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Copiar a Downloads
if ($exePath -and (Test-Path $exePath)) {
    $downloadsExe = Join-Path $env:USERPROFILE "Downloads\KeyboardToGamepad.exe"
    Copy-Item $exePath $downloadsExe -Force
    Write-Host "`nCopiado a Downloads: $downloadsExe" -ForegroundColor Blue
}
