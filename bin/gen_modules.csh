#!/bin/csh -f

# check if any arguments are passed, and at least 4 arguments are passed
#
if (( $#argv == 0 ) || ( $#argv < 2 )) then
  echo "USAGE : $0 -root <root directory> [-help]"
  echo "        -root <dir> : root directory for module creation"
  echo "        -fork       : fork module creation"
  echo "        -help       : print this message"
  exit
endif

set HELP = 0 ;
set FORK = 0 ;

set i = 1 ;

#
# collect arguments
#
while ( $i <= $#argv )
  switch ( ${argv[${i}]} )
    case "-root":
      @ i = $i + 1 ;
      set root = $argv[${i}] ;
      breaksw
    case "-fork":
      set FORK = 1 ;
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

if (($HELP) || (! $?root) || (! -d ${root})) then
  echo "USAGE : $0 -root <root directory> [-help]"
  echo "        -root <dir> : root directory for module creation"
  echo "        -fork       : fork module creation"
  echo "        -help     : print this message"
  exit
endif

set template_dir = ${root}/template/module
set mod_list = ${root}/etc/MODULES.txt 
set gen_mod = ${root}/bin/gen_hier_module.csh

if ($FORK) then
  set mod_list = ${root}/etc/FORK_MODULES.txt 
endif

echo "========="
echo ${root}
echo "========="

cd ${root}

set modules = (`cat ${mod_list}`)

foreach module (${modules})
	echo "========="
	echo ${module}
	echo "========="

	cd ${root}
	${gen_mod} -root ${root} -mod ${module}
end
