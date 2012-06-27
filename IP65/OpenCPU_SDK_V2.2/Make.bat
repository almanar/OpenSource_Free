@echo off
@copy qlcode\scat6223M32.txt  build\scat6223.txt
@echo coreSW\M10_MODEM_08A_PCB01_gprs_MT6223_S00.sym > ./build/~coresymlnk.tmp
if "%1"=="" (echo please input "make help") else tools\make.exe %1 %2 %3 

 




