:: delete build folder
rm -rf .\build\

:: cmake build
cmake -AWin32 -Bbuild .\

:: makefile build
cmake --build .\build\

:: run application Win32
.\build\Debug\csv2bf.exe .\V_NR.csv V_NR

:: run clang format code
.\clang-format.exe -style=File -i .\V_NR.h