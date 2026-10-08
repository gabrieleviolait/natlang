# Run in Visual Studio Developer PowerShell (2022 or newer), with CMake installed.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
cmake -S $root -B "$root\build"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
cmake --build "$root\build" --config Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$compiler = "$root\build\Release\natc.exe"
if (-not (Test-Path $compiler)) { $compiler = "$root\build\natc.exe" }
Write-Host "NatLang compiler: $compiler"
& $compiler "$root\examples\countdown.nat" -o "$root\countdown.exe" --compiler cl
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host 'Demo:'
& "$root\countdown.exe"
