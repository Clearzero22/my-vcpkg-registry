$ErrorActionPreference = "Stop"
$VcpkgRoot = & where.exe vcpkg | Select-Object -First 1

Write-Host "=== Installing dependencies ==="
& $VcpkgRoot install --overlay-ports="$PSScriptRoot\..\ports" --x-manifest-root="$PSScriptRoot"

Write-Host "=== Configuring ==="
& cmake -B "$PSScriptRoot\build" -S "$PSScriptRoot" `
    -DCMAKE_TOOLCHAIN_FILE="$((Get-Item $VcpkgRoot).Directory.FullName)\scripts\buildsystems\vcpkg.cmake"

Write-Host "=== Building ==="
& cmake --build "$PSScriptRoot\build" --config Release

Write-Host "=== Running tests ==="
& "$PSScriptRoot\build\Release\test_all.exe"
