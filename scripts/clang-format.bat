@echo off

cd %~dp0..\Components

call :processDir
goto :finish

:processDir
for %%f in ("*.cpp", "*.h") do (
	clang-format --style=file --verbose -i %%f
)

for /D %%d in (*) do (
	cd %%d
	if not %%d==kissfft if not %%d==Mongoose if not %%d==RtAudio call :processDir
	cd ..
)

exit /b

:finish
