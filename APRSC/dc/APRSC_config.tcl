#
# read verilog source files for mcac
#
set all_modules [list \
"APRSC" \
"../ADDB/src/ADDB" \
"../ADDC/src/ADDC" \
"../DELAY/src/DELAY" \
"../FMULT_ACCUM/src/FMULT_ACCUM" \
"../FLOATA/src/FLOATA" \
"../FLOATB/src/FLOATB" \
"../LIMC/src/LIMC" \
"../LIMD/src/LIMD" \
"../TRIGB/src/TRIGB" \
"../UPA1/src/UPA1" \
"../UPA2/src/UPA2" \
"../UPB/src/UPB" \
"../XOR/src/XOR" \
"../FMULT/src/FMULT" \
"../ACCUM/src/ACCUM" \
]

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}
