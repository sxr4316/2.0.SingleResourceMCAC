#!/bin/csh -f

if (-e /tools/rhel6/env/cadence_env.csh) then
        source /tools/rhel6/env/cadence_env.csh
endif

set DES = "ADDB" ;
set INCA = "./INCA_libs" ;

set RTL = 0 ;
set VEC = 0 ;
set SPI = 0 ;
set NET = 0 ;
set NCSIM = 1 ;
set SV = 0 ;
set GUI = 1 ;
set RUN = 0 ;

set NC = "ncverilog +ncaccess+r +nclinedebug +ncvpicompat+1364v1995 +nowarnTFNPC +nowarnIWFA +nowarnSVTL +nowarnSDFNCAP"
set XL = "verilog +access+r +linedebug +nowarnTFNPC +nowarnIWFA +nowarnSVTL +nowarnSDFA"

set i = 1 ;

#
# collect arguments
#
while ( $i <= $#argv )
  switch ( ${argv[${i}]} )
    case "-r":
      set RTL = 1 ;
      breaksw
    case "-n":
      set NET = 1 ;
      breaksw
    case "-v":
      set VEC = 1 ;
      breaksw
    case "-xl":
      set NCSIM = 0 ;
      breaksw
    case "-sv":
      set SV = 1 ;
      breaksw
    case "-ng":
      set GUI = 0 ;
      breaksw
    case "-run":
      set RUN = 1 ;
      breaksw
    default:
      breaksw
  endsw
  @ i = $i + 1 ;
end

#
# check if any arguments are passed, and only 1 argument is passed
#
if ((( $RTL == 0 ) && ( $NET == 0 )) || (( $RTL == 1 ) && ( $NET == 1 ))) then
  echo "USAGE : $0 [-r] [-n] [-v] [-ng] [-sv] [-run] [-xl]"
  echo "        -r   : simulate RTL";
  echo "        -n   : simulate scan inserted netlist";
  echo " Note: only pass one of -r or -n";
  echo "        -v   : use vector based testbench";
  echo " Note: Default is to use the behavioral testbench";
  echo "        -ng  : do not start gui";
  echo " Note: Default is to use the GUI";
  echo "        -sv : add compilation support for SystemVerilog (NC Verilog only)";
  echo " Note: Default is to not support SystemVerilog";
  echo "        -run : start running simulation immediately";
  echo " Note: Default is for simlator to wait for user input";
  echo "        -xl  : simulate using Verilog-XL";
  echo " Note: Default is to use NC Verilog";
  exit
endif

if ( $NCSIM == 1 ) then
  set SIM = "${NC}"
  if ( $SV == 1 ) then
    set SIM = "${SIM} +sv"
  endif
else
  set SIM = "${XL}"
endif

if ( $GUI == 1 ) then
  set RGUI = "+gui"
else
  set RGUI = ""
endif

if ( $RUN == 1 ) then
  if ( $NCSIM == 1 ) then
    set RSIM = "-run +ncrun"
  endif
else
  set RSIM = "-s"
endif

if ( $VEC == 1 ) then
  set RVEC = ".vec"
else
  set RVEC = ""
endif

if ( $SV == 1 ) then
  set USV = ".sv"
else
  set USV = ""
endif


\rm -rf ${INCA}

if ( $RTL == 1 ) then

  if ( ! -d ./saif ) then
    mkdir ./saif
  endif

  ${SIM} \
	${RGUI} ${RSIM} \
	-f etc/${DES}${RVEC}${USV}.rtl.f \
	-input etc/dumpsaif.tcl \

endif

if ( $NET == 1 ) then

  ${SIM} \
	${RGUI} ${RSIM} \
	-f etc/${DES}${RVEC}${USV}.gate.f \

endif


