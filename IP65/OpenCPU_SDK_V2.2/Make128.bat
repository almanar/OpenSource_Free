@echo off
@copy qlcode\scat6223M128.txt  build\scat6223.txt
if "%1"=="" (echo please input "make help") else tools\make.exe %1 %2 %3 

 




