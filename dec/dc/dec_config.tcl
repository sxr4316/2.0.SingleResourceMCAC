#
# read verilog source files for mcac
#
set all_modules [list \
"dec" \
"../CLK_GEN/src/CLK_GEN" \
"../wishbone_arbiter/src/wishbone_arbiter" \
"../boot_mem128/src/boot_mem128" \
"../uart/src/uart" \
"../timer_module/src/timer_module" \
"../test_module/src/test_module" \
"../ACCUM/src/ACCUM" \
"../ADDB/src/ADDB" \
"../ADDC/src/ADDC" \
"../DELAY/src/DELAY" \
"../FLOATA/src/FLOATA" \
"../FLOATB/src/FLOATB" \
"../FMULT/src/FMULT" \
"../FMULT_ACCUM/src/FMULT_ACCUM" \
"../LIMC/src/LIMC" \
"../LIMD/src/LIMD" \
"../TRIGB/src/TRIGB" \
"../UPA1/src/UPA1" \
"../UPA2/src/UPA2" \
"../UPB/src/UPB" \
"../XOR/src/XOR" \
"../APRSC/src/APRSC" \
"../interrupt_controller/src/interrupt_controller" \
"../TDMI/src/TDMI" \
"../TDMO/src/TDMO" \
"../a25_core/src/a25_alu" \
"../a25_core/src/a25_barrel_shift" \
"../a25_core/src/a25_coprocessor" \
"../a25_core/src/a25_dcache" \
"../a25_core/src/a25_decode" \
"../a25_core/src/a25_decompile" \
"../a25_core/src/a25_execute" \
"../a25_core/src/a25_fetch" \
"../a25_core/src/a25_icache" \
"../a25_core/src/a25_mem" \
"../a25_core/src/a25_multiply" \
"../a25_core/src/a25_register_bank" \
"../a25_core/src/a25_shifter" \
"../a25_core/src/a25_wishbone" \
"../a25_core/src/a25_wishbone_buf" \
"../a25_core/src/a25_write_back" \
"../a25_core/src/a25_core" \
]

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}
