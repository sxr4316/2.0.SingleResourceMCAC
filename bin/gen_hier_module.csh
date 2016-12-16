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
set srcs = (src/${MOD}_test.v src/${MOD}_test.sv src/${MOD}_vec_test.v src/${MOD}.v)
set dcs = (dc/${MOD}_config.tcl)
set cons = ( \
			cons/${MOD}_clocks_all_cons.tcl \
			cons/${MOD}_clocks_cons.tcl \
			cons/${MOD}_cons_defaults.tcl \
			cons/${MOD}_cons.tcl \
)
set etcs = (etc/${MOD}.gate.f etc/${MOD}.rtl.f etc/${MOD}.vec.gate.f etc/${MOD}.vec.rtl.f etc/${MOD}.sv.gate.f etc/${MOD}.sv.rtl.f)

foreach module (${modules})

	cd ${root}

	echo -n ${module}

	if (! -d ${module}) then
		mkdir ${module}
		cd ${module}
		(cd ${template_dir} ; tar cpf - .) | tar xpf -

		foreach src (${srcs})
			set file = `echo ${src} | sed -e "s/${MOD}/${module}/"`
			mv -f ${src} ${file}
		end
		foreach dc (${dcs})
			set file = `echo ${dc} | sed -e "s/${MOD}/${module}/"`
			mv -f ${dc} ${file}
		end
		foreach con (${cons})
			set file = `echo ${con} | sed -e "s/${MOD}/${module}/"`
			mv -f ${con} ${file}
		end
		foreach etc (${etcs})
			set file = `echo ${etc} | sed -e "s/${MOD}/${module}/"`
			mv -f ${etc} ${file}
		end
		foreach file (`find . -type f -print`)
			perl -p -i -e "s/${MOD}/${module}/g" $file
		end
		echo " done."
	else
		echo " already exists."
	endif
end
