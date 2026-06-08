# PowerShell 配置脚本 - vcpkg 私有仓库

$VCPKG_REGISTRY = "C:\Users\admin\my-vcpkg-registry"
$VCPKG_OVERLAY = "$VCPKG_REGISTRY\ports"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  vcpkg 私有仓库配置 (PowerShell)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "注册表路径: $VCPKG_REGISTRY" -ForegroundColor Yellow
Write-Host "Ports 路径: $VCPKG_OVERLAY" -ForegroundColor Yellow
Write-Host ""

# 设置当前会话的环境变量
$env:VCPKG_OVERLAY_PORTS = $VCPKG_OVERLAY
Write-Host "✅ 已设置 VCPKG_OVERLAY_PORTS (当前会话)" -ForegroundColor Green
Write-Host "   值: $env:VCPKG_OVERLAY_PORTS" -ForegroundColor Green
Write-Host ""

# 检查 vcpkg
$VCPKG_PATH = Get-Command vcpkg -ErrorAction SilentlyContinue
if ($VCPKG_PATH) {
    Write-Host "✅ vcpkg 命令已找到: $($VCPKG_PATH.Source)" -ForegroundColor Green
    & vcpkg version | Select-String "version"
    Write-Host ""
} else {
    Write-Host "❌ 错误: 找不到 vcpkg 命令" -ForegroundColor Red
    Write-Host "   请将 vcpkg 添加到 PATH 环境变量" -ForegroundColor Red
    Write-Host "   vcpkg 位置: D:\code_tools\c_plus_plus_tools\vcpkg\vcpkg" -ForegroundColor Red
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  测试搜索私有包" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

try {
    vcpkg search example-package
} catch {
    Write-Host "❌ 搜索失败: $_" -ForegroundColor Red
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  常用命令" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "搜索包:" -ForegroundColor White
Write-Host "  vcpkg search example-package" -ForegroundColor Gray
Write-Host ""
Write-Host "安装包:" -ForegroundColor White
Write-Host "  vcpkg install example-package:x64-windows" -ForegroundColor Gray
Write-Host ""
Write-Host "列出已安装:" -ForegroundColor White
Write-Host "  vcpkg list" -ForegroundColor Gray
Write-Host ""
Write-Host "查看依赖:" -ForegroundColor White
Write-Host "  vcpkg depend-info example-package" -ForegroundColor Gray
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  永久配置" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "💡 要永久生效，添加到 PowerShell 配置文件:" -ForegroundColor Yellow
Write-Host ""
Write-Host "1. 编辑配置文件:" -ForegroundColor White
Write-Host "   notepad $PROFILE" -ForegroundColor Gray
Write-Host ""
Write-Host "2. 添加以下内容:" -ForegroundColor White
Write-Host "   `$env:VCPKG_OVERLAY_PORTS = 'C:\Users\admin\my-vcpkg-registry\ports'" -ForegroundColor Cyan
Write-Host ""
Write-Host "3. 保存后重启 PowerShell 或运行:" -ForegroundColor White
Write-Host "   . `$PROFILE" -ForegroundColor Gray
Write-Host ""