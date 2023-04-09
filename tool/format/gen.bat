@echo off
setlocal

@rem --- 以下に追加してください --
call :CompileFormat base
call :CompileFormat mesh
call :CompileFormat material


@rem --- おしまい ---
endlocal
exit /b

@rem --- コンパイルコマンド ---
:CompileFormat
"../../external/flatbuffers-2.0.0/bin/flatc.exe" -c %1.fbs --natural-utf8 --filename-suffix _format
move "%1_format.h" "..\..\include\generated\%1_format.h"
exit /b
