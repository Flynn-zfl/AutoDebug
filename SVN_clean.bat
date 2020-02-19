@echo off 
echo clean SVN start
del /f /s *.tmp 
del /f /s *.sdf
del /f /s *.ncb
del /f /s *.log
del /f /s *.pdb
del /f /s *.pch
del /f /s *.idb
del /f /s *.aps
::del /f /s *.htm
del /f /s *.obj
del /f /s *.tlog
del /f /s *.cache
del /f /s *.unsuccessfulbuild
del /f /s *.ilk
del /f /s *.res
del /f /s *.manifest
del /f /s *.lastbuildstate
del /f /s *.ipch
del /f /s *UpgradeReport*
echo clean SVN finish
echo. & pause