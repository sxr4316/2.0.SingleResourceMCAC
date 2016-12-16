#!/bin/csh -f

# check if any arguments are passed, and at least 2 arguments are passed
#
if (( $#argv == 0 ) || ( $#argv < 2 )) then
  echo "USAGE : $0 -mod <module name> [-help]"
  echo "        -help     : print this message"
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
    case "-help":
      set HELP = 1 ;
      breaksw
    default:
      breaksw
  endsw
  @ i = $i + 1 ;
end

#
# check if path was set
#
if (! $?module) then
  echo "USAGE : $0 -mod <module name> [-help]"
  echo "        -help     : print this message"
  exit
endif

echo "module == $module"

set root = /home/mai/work/rit/ee720/cproj/mcac
set template_dir = ${root}/template/module

set MOD = ${module}

set modules = ( ${module} )

set etcs = (etc/${MOD}.vec.gate.f etc/${MOD}.vec.rtl.f)

foreach module (${modules})
	echo -n ${module}

	cd ${root}

	if (-d ${module}) then
		cd ${module}

		cp -pf src/${MOD}_test.v src/${MOD}_vec_test.v
		cp -pf etc/${MOD}.gate.f etc/${MOD}.vec.gate.f
		cp -pf etc/${MOD}.rtl.f etc/${MOD}.vec.rtl.f
		cp -pf etc/${MOD}.rtl.f etc/${MOD}.vec.rtl.f
		cp -pf ${root}/template/module/sim.csh sim.csh
		perl -p -i -e "s/arb/${MOD}/g" sim.csh

		foreach file (${etcs})
			perl -p -i -e "s/${MOD}_test.v/${MOD}_vec_test.v/g" $file
		end
		echo " done."
	else
		echo " does not exist."
	endif
end
