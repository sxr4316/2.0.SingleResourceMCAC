#!/bin/csh -f
#
set DEBUG = "" ;
set TEST = "" ;
set HELP = 0 ;
set ULAW = 0 ;
set ALAW = 0 ;
set RATES = ( );
set LAWS = ( );
set HEAD = "head --lines=4 " ;
set i = 1 ;
set VECTOR = 0;
set VFLAG = "";
set USER_CHANNEL = 0;
set CHANNEL = 0;
set EMAIL = 0;
set NO_COMPILE = 0;
set FORCE_ALL_VECS = 0;
#
# collect arguments
#
while ( $i <= $#argv )
  switch ( ${argv[${i}]} )
    case "-u":
    case "-ulaw":
      set ULAW = 1 ;
      breaksw
    case "-a":
    case "-alaw":
      set ALAW = 1 ;
      breaksw
    case "-c":
    case "-channel":
    	@ i = $i + 1;
    	set USER_CHANNEL = 1;
    	set CHANNEL = ${argv[${i}]};
    	breaksw;
    case "-d":
    case "-debug":
      set DEBUG = " -d " ;
      breaksw
    case "-e":
    case "-email":
    	@ i = $i + 1;
    	set EMAIL = 1;
    	set EMAIL_ADDRESS = ${argv[${i}]};
    	breaksw;
    case "-n":
    case "-no-compile":
    	set NO_COMPILE = 1
    	breaksw;
    case "-t":
    case "-test":
      set TEST = " -t " ;
      breaksw
    case "-h":
    case "-help":
      set HELP = 1 ;
      breaksw
    case "-r":
    case "-rate":
        @ i = $i + 1;
        set RATES = ($RATES ${argv[${i}]});
		breaksw;
	case "-v":
	case "-vectors":
		set VECTOR = 1;
		set VFLAG = " -v ";
		breaksw;
	case "-f":
	case "-force"
		set FORCE_ALL_VECS = 1;
		breaksw;
    default:
      echo "ERROR: Unknown argument: ${argv[$i]}";
      exit -1
  endsw
  @ i = $i + 1 ;
end

#
# print usage
#
if ($HELP) then
  echo "USAGE : $0 [-ulaw] [-alaw] [-channel] [-rate] [-vector] [-test] [-debug] [-help]"
  echo "        -help  			: print help message";
  echo "        -ulaw  			: run only ulaw tests";
  echo "        -alaw  			: run only alaw tests";
  echo "		-rate <bitrate>	: run the specified bitrate. Can specify multiple. Default is all bitrates";
  echo "        -channel <0-31> : use the specified channel. A random one will be chosen otherwise"
  echo "        -debug 			: enable debug printing";
  echo "        -test  			: enable codec state printing";
  echo "		-vector			: enable creation of test vectors";
  exit
endif

set pwd = `pwd`
set dn = `dirname $0`
set bn = `basename ${pwd}`

if ($bn != "util") then
	cd ${dn}
endif

set pwd = `pwd`
set bn = `basename ${pwd}`

if ($bn != "util") then
	echo "ERROR: Could not change directory. Please manually run:"
	echo "cd ${dn}"
	exit 1
endif

set DIFF = "diff -I RE -i -b -B -E -w";
set STRIPINIT = "../../model/adpcm-itu/stripInit";
set VEC_TMP_DIR = "/tmp/${USER}_sr_vec_out"
set BASE_VEC_OUT_DIR = "../mc_model/vector_out"
set BASE_TEST_VEC_DIR = "../../model/adpcm-itu/test_vectors"
set BASE_NULL_VEC_DIR = "../sc_model/null_vectors/output"

if ((! $ULAW) && (! $ALAW)) then
	set ULAW = 1 ;
	set ALAW = 1 ;
endif

if ( $ULAW ) then
	set LAWS = ( $LAWS "M" )
endif

if ( $ALAW ) then
	set LAWS = ( $LAWS "A" )
endif

if ( $CHANNEL < 0 || $CHANNEL > 31) then
	echo "ERROR: Channel must be in range [0,31]. Input: ${CHANNEL}"
	exit 1
endif

if ( "$RATES" == "" ) then 
	set RATES = ( 16 24 32 40 ); # Set default rate if no arguments specified
else
	foreach rate ($RATES)
		if ( "$rate" != "16" && "$rate" != "24" && "$rate" != "32" && "$rate" != "40" ) then
			echo "ERROR: Unknown rate: $rate"
			exit -1
		endif
	end
	set RATES = `echo $RATES | fmt -1 | sort -n`; # Sort rate list
endif

if ( $VECTOR ) then
	rm -rf $VEC_TMP_DIR;
	mkdir -p $VEC_TMP_DIR;
	rm -rf $BASE_VEC_OUT_DIR;
	mkdir $BASE_VEC_OUT_DIR;
endif

set ENC = "../mc_model/mc_enc"
set DEC = "../mc_model/mc_dec"

if ( ${FORCE_ALL_VECS} ) then
	set ENC = "${ENC} -f"
	set DEC = "${DEC} -f"
endif

if ( ! ${NO_COMPILE} ) then
	make -C .. clean
endif

if ( ! -f ${ENC} ) then
	echo "WARNING: Could not find ${ENC}";
	echo "Attempting to compile:"
	make -C .. mc
	if ( ! -f ${ENC} ) then
		echo "ERROR: Failed to compile ${ENC}"
		exit 1
	endif
else if ( ! -f ${DEC} ) then
	echo "WARNING: Could not find ${DEC}";
	echo "Attempting to compile:"
	make -C .. mc
	if ( ! -f ${DEC} ) then
		echo "ERROR: Failed to compile ${DEC}"
		exit 1
	endif
else if ( ! -f ${STRIPINIT} ) then
	echo "WARNING: Could not find ${STRIPINIT}";
	echo "Attempting to compile:";
	make -C ../../model/adpcm-itu
	if ( ! -f ${STRIPINIT} ) then
		echo "ERROR: Failed to compile ${STRIPINIT}";
		exit 1
	endif
endif

set CHANNELIFY = "./channelify.py"

foreach law ( $LAWS )
	if ( $law == "M" ) then
		set dir = "${BASE_TEST_VEC_DIR}/itu_ulaw";
		set otherDir = "${BASE_TEST_VEC_DIR}/itu_alaw";
		set ENC = "${ENC} ${DEBUG} ${TEST} ${VFLAG}";
		set DEC = "${DEC} ${DEBUG} ${TEST} ${VFLAG}";
		set XDEC = "${DEC} -a";
		set crossLaw = "C";
		set otherLaw = "A";
	else if ( $law == "A" ) then
		set dir = "${BASE_TEST_VEC_DIR}/itu_alaw";
		set otherDir = "${BASE_TEST_VEC_DIR}/itu_ulaw";
		set ENC = "${ENC} ${DEBUG} ${TEST} ${VFLAG} -a";
		set XDEC = "${DEC} ${DEBUG} ${TEST} ${VFLAG}";
		set DEC = "${XDEC} -a";
		set crossLaw = "X";
		set otherLaw = "M";
	else
		echo "ERROR: Unknown law: $law";
		exit;
	endif

	foreach rate ($RATES)

		# Reset Normal: Encoder
		# Converts the input vector into ADPCM and compares with the expected output of the encoder
		set inputVector = "${dir}/INPUT/NRM.$law";
		set outputVector = "${dir}/RESET/${rate}/RN${rate}F${law}.I";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${ENC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set NULL_VEC = "${BASE_NULL_VEC_DIR}/${law}/reset/nrm/${rate}/NULL.O"
		set CHK_CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${outputVector} -o CHK_MC.O -n ${NULL_VEC}"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHK_CHANNEL_ENCODE};	
		if (`${DIFF} CHK_OUT.O CHK_MC.O` != "") then
			echo "FAIL: Encoder Reset Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} CHK_MC.O;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/enc/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif

		# Reset Normal: Decoder
		# Decode the intermediate ADPCM vector into the output pcm vector
		set inputVector = "${dir}/RESET/${rate}/RN${rate}F${law}.I";
		set outputVector = "${dir}/RESET/${rate}/RN${rate}F${law}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${DEC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		if (`${DIFF} CTEMP.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CTEMP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Reset I: Decoder
		# Decodes an ADPCM input into a PCM output
		set inputVector = "$dir/INPUT/I${rate}"
		set outputVector = "${dir}/RESET/${rate}/RI${rate}F${law}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${DEC} -i CTEMP.I -o CHK_OUT.O -r $rate";
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		if (`${DIFF} CTEMP.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Input: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CTEMP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/reset/i/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Reset Overload: Encoder
		set inputVector = "$dir/INPUT/OVR.$law";
		set outputVector = "${dir}/RESET/${rate}/RV${rate}F${law}.I";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${ENC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		if (`${DIFF} CTEMP.O $outputVector` != "") then
			echo "FAIL: Reset Overload Encoder: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CTEMP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/enc/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Reset Overload: Decoder
		set inputVector = "${dir}/RESET/${rate}/RV${rate}F${law}.I";
		set outputVector = "${dir}/RESET/${rate}/RV${rate}F${law}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${DEC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		if (`${DIFF} CTEMP.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Overload: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CTEMP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Reset Crossover: Normal
		set inputVector = "$dir/INPUT/NRM.$law";
		set outputVector = "${dir}/RESET/${rate}/RN${rate}F${crossLaw}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set CMD1 = "${ENC} -i CTEMP.I -o XOVER_OUT.O -r $rate";
		set CMD2 = "${XDEC} -i XOVER_OUT.O -o CHK_OUT.O -r $rate";
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${CMD1};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/enc/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		${CMD2};
		${CHANNEL_DECODE};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if (`${DIFF} CTEMP.O $outputVector` != "") then
			echo "FAIL: Reset Crossover Normal: Starting Law = $law to $crossLaw, Rate=$rate";
			echo "Command: ${CMD1}; ${CMD2}";
			echo "Diff command: ${DIFF} CHK_OUT.O $outputVector"
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CTEMP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Reset Crossover: Overload
		set inputVector = "$dir/INPUT/OVR.$law";
		set outputVector = "${dir}/RESET/${rate}/RV${rate}F${crossLaw}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set CMD1 = "${ENC} -i CTEMP.I -o XOVER_OUT.O -r $rate";
		set CMD2 = "${XDEC} -i XOVER_OUT.O -o CHK_OUT.O -r $rate";
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${CMD1};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/enc/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		${CMD2};
		${CHANNEL_DECODE};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if (`${DIFF} CTEMP.O $outputVector` != "") then
			echo "FAIL: Reset Crossover Normal: Starting Law = $law to $crossLaw, Rate=$rate";
			echo "Command: ${CMD1}; ${CMD2}";
			echo "Diff command: ${DIFF} CHK_OUT.O $outputVector"
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CTEMP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# --------------------------------------Homing Sequence--------------------------------------

		# Homing Normal: Encoder
		# Converts the input vector into ADPCM and compares with the expected output of the encoder
		cat ${dir}/PCM_INIT.${law} > TMP.O;		
		cat ${dir}/INPUT/NRM.${law} >> TMP.O;				
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HN${rate}F${law}.I";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${ENC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Encoder HOMING Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		rm -f TMP.O;
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/enc/homing/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Homing Normal: Decoder
		# Decode the intermediate ADPCM vector into the output pcm vector
		cat ${dir}/HOMING/${rate}/I_INI_${rate}.${law} > TMP.O;
		cat ${dir}/HOMING/${rate}/HN${rate}F${law}.I >> TMP.O;
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HN${rate}F${law}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${DEC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Decoder HOMING Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
			echo "DONE: ${rate} dec";
		endif
		rm -f TMP.O;
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/homing/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		#I
		cat ${dir}/HOMING/${rate}/I_INI_${rate}.${law} > TMP.O;
		cat ${dir}/INPUT/I${rate} >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HI${rate}F${law}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${DEC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Encoder HOMING Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		rm -f TMP.O;
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/homing/i/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Homing Overload: Encoder
		cat ${dir}/PCM_INIT.${law} > TMP.O;
		cat ${dir}/INPUT/OVR.${law} >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HV${rate}F${law}.I";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${ENC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Reset Overload Encoder: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		rm -f TMP.O;
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/enc/homing/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Homing Overload: Decoder
		cat ${dir}/HOMING/${rate}/I_INI_${rate}.${law} > TMP.O;
		cat ${dir}/HOMING/${rate}/HV${rate}F${law}.I >> TMP.O;
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HV${rate}F${law}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set COMMAND = "${DEC} -i CTEMP.I -o CHK_OUT.O -r $rate"
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${COMMAND};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Overload: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		rm -f TMP.O;
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/homing/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# ---------------Crossover---------------

		# Homing Crossover: Normal Decoder
		cat ${otherDir}/HOMING/${rate}/I_INI_${rate}.${otherLaw} > TMP.O;
		cat $dir/HOMING/${rate}/HN${rate}F${law}.I >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HN${rate}F${crossLaw}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set CMD1 = "${XDEC} -i CTEMP.I -o CHK_OUT.O -r $rate";
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${CMD1};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Reset Crossover Normal: Starting Law = $law to $crossLaw, Rate=$rate";
			echo "Command: ${CMD1}";
			echo "Diff command: ${DIFF} CHK_OUT_STRIP.O $outputVector"
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		rm -f TMP.O;
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/homing/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		# Homing Crossover: Overload Decoder
		cat ${otherDir}/HOMING/${rate}/I_INI_${rate}.${otherLaw} > TMP.O;
		cat $dir/HOMING/${rate}/HV${rate}F${law}.I >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HV${rate}F${crossLaw}.O";
		set CHANNEL_ENCODE = "${CHANNELIFY} -c ${CHANNEL} -i ${inputVector} -o CTEMP.I"
		set CMD1 = "${XDEC} -i CTEMP.I -o CHK_OUT.O -r $rate";
		set CHANNEL_DECODE = "${CHANNELIFY} -c ${CHANNEL} -i CHK_OUT.O -o CTEMP.O"
		${CHANNEL_ENCODE};
		${CMD1};
		${CHANNEL_DECODE};
		${STRIPINIT} -i CTEMP.O -o CHK_OUT_STRIP.O;
		if (`${DIFF} CHK_OUT_STRIP.O $outputVector` != "") then
			echo "FAIL: Reset Crossover Normal: Starting Law = $law to $crossLaw, Rate=$rate";
			echo "Command: ${CMD1}";
			echo "Diff command: ${DIFF} CHK_OUT_STRIP.O $outputVector"
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT_STRIP.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/homing/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
			printf "@0000\t%06X\n" ${CHANNEL} > $vecDir/channel.t
		endif
		if ( ! $USER_CHANNEL ) then
			@ CHANNEL = $CHANNEL + 1;
			if ($CHANNEL == 32) then
				set CHANNEL = 0;
			endif
		endif
		
		rm -f CTEMP.I
		rm -f CTEMP.O
		rm -f CHK_OUT.O
		rm -f CHK_OUT_STRIP.O
		rm -f XOVER_OUT.O
		rm -f TMP.O
		rm -rf $VEC_TMP_DIR
		echo "Completed tests for ${law}-law at ${rate}kbps"

	end # foreach rate
end # foreach law

# Clean up
set EMAIL_SCRIPT = "../../scripts/SABRE.py"
set EMAIL_SUBJECT = "SABRE - chk_itu_mc.csh Completed"
if ($EMAIL) then
	if ( ! -f $EMAIL_SCRIPT) then
		echo "ERROR: Could not find email script: ${EMAIL_SCRIPT}"
		exit 1
	endif
	set EMAIL_COMMAND = "${EMAIL_SCRIPT} -a ${EMAIL_ADDRESS} -s ${EMAIL_SUBJECT} -f";
	${EMAIL_COMMAND};
endif

exit 0
