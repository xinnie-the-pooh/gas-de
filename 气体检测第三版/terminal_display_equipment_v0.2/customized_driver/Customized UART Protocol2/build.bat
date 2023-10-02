@echo off
set path=%path%;..\..\arm\bin
set CC_PREFIX=arm-none-linux-gnueabi-

set CFLAGS=-O2 -fPIC
%CC_PREFIX%gcc.exe -c COMx.c %CFLAGS% -o COMx.o
%CC_PREFIX%gcc.exe -Wl -shared %CFLAGS%,-soname,Customized_UART_protocol.so -o Customized_UART_protocol.so COMx.o libserail.a -lc


REN "Customized_UART_protocol.so" "Customized UART protocol2.so"
COPY  /y  "Customized UART protocol2.so"  "..\..\lib\plc"
COPY  /y  "Customized UART protocol2.dll"  "..\..\lib\plc"
COPY  /y  "Customized UART protocol2.ge"  "..\..\lib\plc"

DEL "Customized UART protocol2.so"
DEL "COMx.o"
pause