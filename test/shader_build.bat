setlocal
pushd %~dp0

set DXC="..\external\dxc\dxc.exe"

call :BUILD main vs_6_0 TestVS
call :BUILD main ps_6_0 TestPS

goto :END

:BUILD
%DXC% -E %1 -T %2 -Vn %3 -Fh %3.inc -Fo %3.cso %3.hlsl
exit /b

:END
popd

