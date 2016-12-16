#!/bin/csh -f

if (-e /tools/rhel6/env/synopsys_env.csh) then
        source /tools/rhel6/env/synopsys_env.csh
endif 

set SYN = 0 ;
set TEST = 0 ;

set i = 1 ;

#
# collect arguments
#
while ( $i <= $#argv )
  switch ( ${argv[${i}]} )
    case "-r":
      set SYN = 1 ;
      breaksw
    case "-t":
      set TEST = 1 ;
      breaksw
    case "-b":
      set SYN = 1 ;
      set TEST = 1 ;
      breaksw
    default:
      breaksw
  endsw
  @ i = $i + 1 ;
end

#
# check if any arguments are passed, and at least 2 arguments are passed
#
if (( $SYN == 0 ) && ( $TEST == 0 )) then
  echo "USAGE : $0 [-r] [-t] [-b]"
  echo "        -r   : rtl logic synthesis";
  echo "        -t   : netlist insert scan";
  echo "        -b   : run rtl logic synthesis & netlist insert scan";
  exit
endif


if ( $SYN == 1 ) then
  set scr = cmp 
  dc_shell < dc/${scr}.tcl |& tee dc_shell_${scr}.log
endif

if ( $TEST == 1 ) then
  set scr = cmp_scan 
  dc_shell < dc/${scr}.tcl |& tee dc_shell_${scr}.log
endif
