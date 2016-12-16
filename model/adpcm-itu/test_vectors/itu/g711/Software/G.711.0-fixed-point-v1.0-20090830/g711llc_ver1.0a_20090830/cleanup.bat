@echo ----- delvc6.cmd -----
@echo Removing VC++6 intermediate files.
@pause
del /S *.ncb *.opt *.aps *.plg *.pch *.obj *.idb *.exp *.pdb *.ilk *.res *.sbr *.bak

@echo ----- delvc7.cmd -----
@echo Removing VC++7 intermediate files.
@pause
del /S *.ncb BuildLog.htm *.obj *.pch *.pdb *.idb *.ilk *.sbr *.res *.exp *.aps *.bak
del /AH /S *.suo

@echo ----- delvc8.cmd -----
@echo Removing VC++8 intermediate files.
@pause
del /S *.user

@echo ----- linux.cmd -----
@echo Removing linux (emacs) intermediate files.
@pause
del /S *~ *.*~
