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
    case "-d":
    case "-debug":
      set DEBUG = " -d " ;
      breaksw
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
  echo "USAGE : $0 [-ulaw] [-alaw] [-debug] [-test] [-help]"
  echo "        -help  			: print help message";
  echo "        -ulaw  			: run only ulaw tests";
  echo "        -alaw  			: run only alaw tests";
  echo "		-rate <bitrate>	: run the specified bitrate. Can specify multiple. Default is all bitrates";
  echo "        -debug 			: enable debug printing";
  echo "        -test  			: enable codec state printing";
  echo "		-vector			: enable creation of test vectors";
  exit
endif

set DIFF = "diff -I RE -i -b -B -E -w";
set STRIPINIT = "./stripInit";
set VEC_TMP_DIR = "/tmp/${USER}_sr_vec_out"
set BASE_VEC_OUT_DIR = "./vector_out"

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

# Initialize files
if ( -f CHK_OUT.O ) then
	printf "" > CHK_OUT.O
else
	touch CHK_OUT.O
endif

if ( -f XOVER_OUT.O ) then
	printf "" > XOVER_OUT.O
else
	touch XOVER_OUT.O
endif

#set ENC = "./enc"
#set ENC = "../../sw/sc_model/sc_enc"
set ENC = "../../sw/sc_model/sc_enc_flat"

#set DEC = "./dec"
set DEC = "../../sw/sc_model/sc_dec_flat"

foreach law ( $LAWS )
	if ( $law == "M" ) then
		set dir = "./test_vectors/itu_ulaw";
		set otherDir = "./test_vectors/itu_alaw";
		set ENC = "${ENC} ${DEBUG} ${TEST} ${VFLAG}";
		set DEC = "${DEC} ${DEBUG} ${TEST} ${VFLAG}";
		set XDEC = "${DEC} -a";
		set crossLaw = "C";
		set otherLaw = "A";
	else if ( $law == "A" ) then
		set dir = "./test_vectors/itu_alaw";
		set otherDir = "./test_vectors/itu_ulaw";
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
		set inputVector = "$dir/INPUT/NRM.$law";
		set outputVector = "${dir}/RESET/${rate}/RN${rate}F${law}.I";
		set COMMAND = "${ENC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Encoder Reset Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/enc/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif
		
		# Reset Normal: Decoder
		# Decode the intermediate ADPCM vector into the output pcm vector
		set inputVector = "${dir}/RESET/${rate}/RN${rate}F${law}.I";
		set outputVector = "${dir}/RESET/${rate}/RN${rate}F${law}.O";
		set COMMAND = "${DEC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Normal: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif

		# Reset I: Decoder
		# Decodes an ADPCM input into a PCM output
		set inputVector = "$dir/INPUT/I${rate}"
		set outputVector = "${dir}/RESET/${rate}/RI${rate}F${law}.O";
		set COMMAND = "${DEC} -i $inputVector -o CHK_OUT.O -r $rate";
		${COMMAND};
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Input: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/reset/i/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif

		# Reset Overload: Encoder
		set inputVector = "$dir/INPUT/OVR.$law";
		set outputVector = "${dir}/RESET/${rate}/RV${rate}F${law}.I";
		set COMMAND = "${ENC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Reset Overload Encoder: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/enc/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif

		# Reset Overload: Decoder
		set inputVector = "${dir}/RESET/${rate}/RV${rate}F${law}.I";
		set outputVector = "${dir}/RESET/${rate}/RV${rate}F${law}.O";
		set COMMAND = "${DEC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Decoder Reset Overload: Law=$law, Rate=$rate";
			echo "Command: ${COMMAND}";
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${law}/dec/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif

		# Reset Crossover: Normal
		set inputVector = "$dir/INPUT/NRM.$law";
		set outputVector = "${dir}/RESET/${rate}/RN${rate}F${crossLaw}.O";
		set CMD1 = "${ENC} -i $inputVector -o XOVER_OUT.O -r $rate";
		set CMD2 = "${XDEC} -i XOVER_OUT.O -o CHK_OUT.O -r $rate";
		${CMD1};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/enc/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif
		${CMD2};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/reset/nrm/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Reset Crossover Normal: Starting Law = $law to $crossLaw, Rate=$rate";
			echo "Command: ${CMD1}; ${CMD2}";
			echo "Diff command: ${DIFF} CHK_OUT.O $outputVector"
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif

		# Reset Crossover: Overload
		set inputVector = "$dir/INPUT/OVR.$law";
		set outputVector = "${dir}/RESET/${rate}/RV${rate}F${crossLaw}.O";
		set CMD1 = "${ENC} -i $inputVector -o XOVER_OUT.O -r $rate";
		set CMD2 = "${XDEC} -i XOVER_OUT.O -o CHK_OUT.O -r $rate";
		${CMD1};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/enc/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif
		${CMD2};
		if ($VECTOR) then
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/reset/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif
		if (`${DIFF} CHK_OUT.O $outputVector` != "") then
			echo "FAIL: Reset Crossover Normal: Starting Law = $law to $crossLaw, Rate=$rate";
			echo "Command: ${CMD1}; ${CMD2}";
			echo "Diff command: ${DIFF} CHK_OUT.O $outputVector"
			echo "Input Vector: $inputVector";
			echo "Output Vector: $outputVector";
			echo "Output result:";
			${HEAD} CHK_OUT.O;
			echo "";
			echo "Expected result:";
			${HEAD} $outputVector;
			echo "-------------------------------------";
		endif

		# --------------------------------------Homing Sequence--------------------------------------

		# Homing Normal: Encoder
		# Converts the input vector into ADPCM and compares with the expected output of the encoder
		cat ${dir}/PCM_INIT.${law} > TMP.O;		
		cat ${dir}/INPUT/NRM.${law} >> TMP.O;				
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HN${rate}F${law}.I";
		set COMMAND = "${ENC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
		endif

		# Homing Normal: Decoder
		# Decode the intermediate ADPCM vector into the output pcm vector
		cat ${dir}/HOMING/${rate}/I_INI_${rate}.${law} > TMP.O;
		cat ${dir}/HOMING/${rate}/HN${rate}F${law}.I >> TMP.O;
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HN${rate}F${law}.O";
		set COMMAND = "${DEC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
		endif

		#I
		cat ${dir}/HOMING/${rate}/I_INI_${rate}.${law} > TMP.O;
		cat ${dir}/INPUT/I${rate} >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HI${rate}F${law}.O";
		set COMMAND = "${DEC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
		endif

		# Homing Overload: Encoder
		cat ${dir}/PCM_INIT.${law} > TMP.O;
		cat ${dir}/INPUT/OVR.${law} >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HV${rate}F${law}.I";
		set COMMAND = "${ENC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
		endif

		# Homing Overload: Decoder
		cat ${dir}/HOMING/${rate}/I_INI_${rate}.${law} > TMP.O;
		cat ${dir}/HOMING/${rate}/HV${rate}F${law}.I >> TMP.O;
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HV${rate}F${law}.O";
		set COMMAND = "${DEC} -i $inputVector -o CHK_OUT.O -r $rate"
		${COMMAND};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
		endif

		# ---------------Crossover---------------

		# Homing Crossover: Normal Decoder
		cat ${otherDir}/HOMING/${rate}/I_INI_${rate}.${otherLaw} > TMP.O;
		cat $dir/HOMING/${rate}/HN${rate}F${law}.I >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HN${rate}F${crossLaw}.O";
		set CMD1 = "${XDEC} -i $inputVector -o CHK_OUT.O -r $rate";
		${CMD1};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
		endif

		# Homing Crossover: Overload Decoder
		cat ${otherDir}/HOMING/${rate}/I_INI_${rate}.${otherLaw} > TMP.O;
		cat $dir/HOMING/${rate}/HV${rate}F${law}.I >> TMP.O;	
		set inputVector = "TMP.O";
		set outputVector = "${dir}/HOMING/${rate}/HV${rate}F${crossLaw}.O";
		set CMD1 = "${XDEC} -i $inputVector -o CHK_OUT.O -r $rate";
		${CMD1};
		${STRIPINIT} -i CHK_OUT.O -o CHK_OUT_STRIP.O;
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
			set vecDir = "${BASE_VEC_OUT_DIR}/${law}${otherLaw}/dec/homing/ovr/$rate/"
			mkdir -p $vecDir
			mv ${VEC_TMP_DIR}/* $vecDir
		endif

		echo "Completed tests for ${law}-law at ${rate}kbps"

	end # foreach rate
end # foreach law

# Clean up
rm -f CHK_OUT.O
rm -f CHK_OUT_STRIP.O
rm -f XOVER_OUT.O
rm -f TMP.O
rm -rf $VEC_TMP_DIR

exit 0
