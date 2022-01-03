@echo off

del *.so

set path=%path%;..\..\arm\bin
set CC_PREFIX=arm-none-linux-gnueabi-

rem set path=%path%;..\..\cygwin\bin
rem set path=%path%;..\..\cygwin\staging_dir\arm-linux-uclibc\bin

set CFLAGS=-O2 -fPIC
%CC_PREFIX%gcc.exe -c COMx.c %CFLAGS% -o COMx.o
%CC_PREFIX%gcc.exe -Wl -shared %CFLAGS%,-soname,Customized_UART_protocol.so -o Customized_UART_protocol.so COMx.o libserail.a -lc

REN "Customized_UART_protocol.so" "Customized UART protocol1.so"
COPY  /y  "Customized UART protocol1.so"  "..\..\lib\plc"
COPY  /y  "Customized UART protocol1.dll"  "..\..\lib\plc"
COPY  /y  "Customized UART protocol1.ge"  "..\..\lib\plc"

rename "Customized UART protocol1.so" drv_p0.so
DEL "COMx.o"
pause