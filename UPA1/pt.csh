#!/bin/csh -f

if (-e /tools/rhel6/env/synopsys_env.csh) then
        source /tools/rhel6/env/synopsys_env.csh
endif 

set scr = time_scan 
pt_shell < pt/${scr}.tcl |& tee pt_shell_${scr}.log
