param(
    [string]$Stage = "$env:TEMP\codex-body-core-build-20261015",
    [string]$IdfRoot = 'D:\.espressif\v6.1\esp-idf',
    [string]$ToolsRoot = 'D:\.espressif\v6.1',
    [string]$PythonEnv = 'C:\Users\indeg\.espressif\python_env\idf6.1_py3.14_env',
    [switch]$Flash,
    [string]$Port = 'COM5'
)
$ErrorActionPreference = 'Stop'
$sourceRoot = Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force (Join-Path $Stage 'main') | Out-Null
Copy-Item -Path "$sourceRoot\main\*" -Destination "$Stage\main" -Force
foreach ($name in @('CMakeLists.txt','sdkconfig.defaults','dependencies.lock')) {
    Copy-Item -LiteralPath (Join-Path $sourceRoot $name) -Destination (Join-Path $Stage $name) -Force
}
$env:IDF_PATH = $IdfRoot
$env:IDF_TOOLS_PATH = $ToolsRoot
$env:IDF_PYTHON_ENV_PATH = $PythonEnv
$env:ESP_IDF_VERSION = '6.1.0'
$env:PYTHONUTF8 = '1'
$env:PATH = "$PythonEnv\Scripts;$ToolsRoot\tools\cmake\4.0.3\bin;$ToolsRoot\tools\ninja\1.12.1;$ToolsRoot\tools\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin;" + $env:PATH
Push-Location $Stage
try {
    & "$PythonEnv\Scripts\python.exe" "$IdfRoot\tools\idf.py" build
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
    if ($Flash) {
        & "$PythonEnv\Scripts\python.exe" "$IdfRoot\tools\idf.py" -p $Port flash
        if ($LASTEXITCODE -ne 0) { throw "Flash failed: $LASTEXITCODE" }
    }
} finally { Pop-Location }
