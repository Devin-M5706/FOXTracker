path=C:\Qt\5.14.2\msvc2017_64\bin
mkdir Release
mkdir Release\\assets
copy ..\\build-FlightAgentX-Desktop_Qt_5_14_2_MSVC2017_64bit-Release\\release\\FlightAgentX.exe .\\Release\\FOXTracker.exe
copy  .\\assets .\\Release\assets
copy  .\\config.yaml .\\Release\config.yaml

cd Release
windeployqt FOXTracker.exe
