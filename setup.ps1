#Requires -Version 5.1
<#
.SYNOPSIS
    Setup inicial del entorno de desarrollo para KeyboardToGamepad.
.DESCRIPTION
    Clona dependencias via git submodule o clone directo.
    Ejecutar desde la raiz del repo.
#>

$ErrorActionPreference = "Stop"
$ProgressPreference = "Continue"

$depsDir = Join-Path $PSScriptRoot "deps"

function Ensure-Dir {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Host "[+] Directorio creado: $Path" -ForegroundColor Green
    }
}

function Clone-Repo {
    param(
        [string]$Url,
        [string]$Dest,
        [string]$Branch = ""
    )
    if (Test-Path $Dest) {
        Write-Host "[~] Ya existe: $Dest, actualizando..." -ForegroundColor Yellow
        Push-Location $Dest
        git fetch origin --depth 1
        if ($Branch) {
            git checkout $Branch 2>$null
            git reset --hard origin/$Branch 2>$null
        } else {
            git pull origin $(git branch --show-current) 2>$null
        }
        Pop-Location
    } else {
        if ($Branch) {
            git clone --branch $Branch --depth 1 $Url $Dest
        } else {
            git clone --depth 1 $Url $Dest
        }
        Write-Host "[+] Clonado: $Url -> $Dest" -ForegroundColor Green
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  KeyboardToGamepad - Setup" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# 1. Verificar prerequisitos
Write-Host "[1/5] Verificando prerequisitos..." -ForegroundColor Cyan

$git = Get-Command git -ErrorAction SilentlyContinue
if (-not $git) {
    Write-Error "Git no encontrado. Instalalo desde https://git-scm.com/download/win"
}

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Error "CMake no encontrado. Instalalo desde https://cmake.org/download/ (Windows x64 Installer)"
}

Write-Host "    Git: $($git.Source)" -ForegroundColor Gray
Write-Host "    CMake: $($cmake.Source)" -ForegroundColor Gray

# Nota sobre MSBuild
$msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
if (-not $msbuild) {
    Write-Warning "MSBuild no encontrado. Para compilar, abri 'Developer PowerShell for VS 2022'."
    Write-Warning "El setup de dependencias puede continuar sin MSBuild."
}

# 2. Crear directorio deps
Ensure-Dir $depsDir

# 3. Clonar dependencias
Write-Host "`n[2/4] Descargando dependencias..." -ForegroundColor Cyan

Clone-Repo "https://github.com/ocornut/imgui.git" (Join-Path $depsDir "imgui") "docking"
Clone-Repo "https://github.com/nlohmann/json.git" (Join-Path $depsDir "nlohmann_json") "develop"
Clone-Repo "https://github.com/gabime/spdlog.git" (Join-Path $depsDir "spdlog") "v1.x"
Clone-Repo "https://github.com/nefarius/ViGEmClient.git" (Join-Path $depsDir "vigem-client")

# 4. Verificar estructura
Write-Host "`n[3/4] Verificando estructura..." -ForegroundColor Cyan

$checks = @(
    (Join-Path $depsDir "imgui\imgui.cpp"),
    (Join-Path $depsDir "nlohmann_json\include\nlohmann\json.hpp"),
    (Join-Path $depsDir "spdlog\include\spdlog\spdlog.h"),
    (Join-Path $depsDir "vigem-client\include\ViGEm\Client.h"),
    (Join-Path $depsDir "vigem-client\CMakeLists.txt")
)

$allOk = $true
foreach ($c in $checks) {
    if (Test-Path $c) {
        Write-Host "    [OK] $c" -ForegroundColor Green
    } else {
        Write-Host "    [FAIL] $c" -ForegroundColor Red
        $allOk = $false
    }
}

if (-not $allOk) {
    Write-Error "Faltan archivos de dependencias. Revisa la salida anterior."
}

# 5. Resumen
Write-Host "`n[4/4] Setup completo!" -ForegroundColor Cyan
Write-Host "`nProximos pasos:" -ForegroundColor White
Write-Host "  1. Abrí 'Developer PowerShell for VS 2022'" -ForegroundColor Gray
Write-Host "  2. cd '$PSScriptRoot'" -ForegroundColor Gray
Write-Host "  3. .\build.ps1" -ForegroundColor Gray
Write-Host "`nPara instalar ViGEmBus (requerido en runtime):" -ForegroundColor White
Write-Host "  https://github.com/nefarius/ViGEmBus/releases" -ForegroundColor Blue

Write-Host "`n========================================" -ForegroundColor Cyan
