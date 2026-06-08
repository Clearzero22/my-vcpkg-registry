@echo off
REM Windows 配置脚本 - 设置 vcpkg 私有仓库

set VCPKG_REGISTRY=C:\Users\admin\my-vcpkg-registry

echo ========================================
echo   VCPKG 私有仓库配置 (Windows)
echo ========================================
echo.
echo 注册表路径: %VCPKG_REGISTRY%
echo.

REM 设置临时环境变量（当前会话）
set VCPKG_OVERLAY_PORTS=%VCPKG_REGISTRY%\ports
echo ✅ 已设置 VCPKG_OVERLAY_PORTS
echo    值: %VCPKG_OVERLAY_PORTS%
echo.

REM 检查 vcpkg 是否可用
where vcpkg >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误: 找不到 vcpkg 命令
    echo    请将 vcpkg 添加到 PATH 环境变量
    echo    vcpkg 位置: D:\code_tools\c_plus_plus_tools\vcpkg\vcpkg
    echo.
) else (
    echo ✅ vcpkg 命令已找到
    vcpkg version | findstr "version"
    echo.
)

echo ========================================
echo   可用命令:
echo ========================================
echo.
echo 1. 搜索包:
echo    vcpkg search example-package
echo.
echo 2. 安装包:
echo    vcpkg install example-package:x64-windows
echo.
echo 3. 查看包信息:
echo    vcpkg x-package-info example-package
echo.
echo 4. 列出已安装包:
echo    vcpkg list
echo.
echo 5. 卸载包:
echo    vcpkg remove example-package --recurse
echo.
echo ========================================
echo.
echo 💡 要永久生效，请手动添加到系统环境变量:
echo    VCPKG_OVERLAY_PORTS = C:\Users\admin\my-vcpkg-registry\ports
echo.

pause