#!/bin/csh -f

# check if any arguments are passed, and at least 4 arguments are passed
#
if (( $#argv == 0 ) || ( $#argv < 1 )) then
  echo "USAGE : $0 [-r] [-s] [-n] [-a] [-help]"
  echo "        -r    : run RTL regression"
  echo "        -s    : run logic synthesis"
  echo "        -n    : run netlist regression"
  echo "        -a    : run all regression steps"
  echo "        -help : print this message"
  exit
endif

set RTL = 0 ;
set SYN = 0 ;
set NET = 0 ;
set HELP = 0 ;

set i = 1 ;

#
# collect arguments
#
while ( $i <= $#argv )
  switch ( ${argv[${i}]} )
    case "-r":
      set RTL = 1 ;
      breaksw
    case "-s":
      set SYN = 1 ;
      breaksw
    case "-n":
      set NET = 1 ;
      breaksw
    case "-a":
      set RTL = 1 ;
      set SYN = 1 ;
      set NET = 1 ;
      breaksw
    case "-help":
      set HELP = 1 ;
      breaksw
    default:
      breaksw
  endsw
  @ i = $i + 1 ;
end

#
# check if module and path was set
#

if (($HELP) || ((! $RTL) && (! $SYN) && (! $NET))) then
  echo "USAGE : $0 [-r] [-s] [-n] [-a] [-help]"
  echo "        -r    : run RTL regression"
  echo "        -s    : run logic synthesis"
  echo "        -n    : run netlist regression"
  echo "        -a    : run all regression steps"
  echo "        -help : print this message"
  exit
endif


set root = `pwd`
set rtllog = ${root}/rtl_sim_all_modules.log
set synlog = ${root}/synthesize_all_modules.log
set netlog = ${root}/netlist_sim_all_modules.log
if (${RTL}) then
	rm -f $rtllog
	touch $rtllog
endif
if (${SYN}) then
	rm -f $synlog
	touch $synlog
endif
if (${NET}) then
	rm -f $netlog
	touch $netlog
endif

set dirs = `grep -v -f etc/FORK_MODULES.txt etc/DIRS.txt`

if (${RTL}) then
echo ""
echo "Logging results in ${rtllog}"
echo ""

foreach dir ($dirs)
	if (-e ${dir}/sim.csh) then
		echo ${dir}
		echo $dir >> $rtllog
		cd ${dir}
		./sim.csh -r -v -ng -run >> $rtllog
		echo "=========================" >> $rtllog
		cd $root
	endif
end
endif

if (${SYN}) then
echo ""
echo "Logging results in ${synlog}"
echo ""

foreach dir ($dirs)
	if (-e ${dir}/sim.csh) then
		echo ${dir}
		echo $dir >> $synlog
		cd ${dir}
		echo "----- RTL Synthesis ---" >> $synlog
		./syn.csh -r >> $synlog
		echo "----- TEST Synthesis ---" >> $synlog
		./syn.csh -t >> $synlog
		echo "=========================" >> $synlog
		cd $root
	endif
end
endif

if (${NET}) then
echo ""
echo "Logging results in ${netlog}"
echo ""

foreach dir ($dirs)
	if (-e ${dir}/sim.csh) then
		echo ${dir}
		echo $dir >> $netlog
		cd ${dir}
		./sim.csh -n -v -ng -run >> $netlog
		echo "=========================" >> $netlog
		cd $root
	endif
end
endif
