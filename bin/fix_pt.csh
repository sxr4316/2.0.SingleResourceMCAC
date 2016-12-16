#!/bin/csh -f

# check if any arguments are passed, and at least 4 arguments are passed
#
if (( $#argv == 0 ) || ( $#argv < 2 )) then
  echo "USAGE : $0 -root <root directory> -mod <module name> [-help]"
  echo "        -root <dir> : root directory for module creation"
  echo "        -mod <mod>  : name of module to create"
  echo "        -help       : print this message"
  exit
endif

set HELP = 0 ;

set i = 1 ;

#
# collect arguments
#
while ( $i <= $#argv )
  switch ( ${argv[${i}]} )
    case "-mod":
      @ i = $i + 1 ;
      set module = $argv[${i}] ;
      breaksw
    case "-root":
      @ i = $i + 1 ;
      set root = $argv[${i}] ;
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

if (($HELP) || (! $?module) || (! $?root)) then
  echo "USAGE : $0 -root <root directory> -mod <module name> [-help]"
  echo "        -root <dir> : root directory for module creation"
  echo "        -mod <mod>  : name of module to create"
  echo "        -help     : print this message"
  exit
endif

echo "root == $root"
echo "module == $module"

set template_root = ${root}/template
set template_dir = ${template_root}/hier_module

set MOD = EXPAND

set modules = ( ${module} )
set srcs = (sim.csh)

foreach module (${modules})

	cd ${root}

	echo -n ${module}

	if (-d ${module}) then
		cd ${module}
		cp -pf ${template_dir}/pt/time_scan.tcl ./pt/time_scan.tcl 
		perl -p -i -e "s/${MOD}/${module}/g" ./pt/time_scan.tcl

		echo " done."
	else
		echo " does not exist."
	endif
end
