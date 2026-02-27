pushd %0¥..

@rem 共通で使うやつを最初にコンバート.
"..\..\..\external\flatbuffers\bin\flatc.exe" --cpp -o "..\include" "..\..\..\res\format\ResTypes.fbs" --no-warnings
xcopy "..\include\ResTypes_generated.h" "..\..\..\src\res" /y /c

@rem 各コンバーター用のバイナリ.
call :gen TextureBinary
call :gen FontBinary
call :gen MapChipBinary
call :gen ModelBinary
call :gen MotionBinary

@rem ----- 終了.
popd
exit

@rem 変換サブルーチン
:gen
"..\..\..\external\flatbuffers\bin\flatc.exe" --cpp -o "..\include" "..\format\%1.fbs" --no-warnings
xcopy "..\include\%1_generated.h" "..\..\..\src\res" /y /c
exit /b

