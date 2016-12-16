#
# read verilog source files for mcac
#
set all_modules [list \
"mcac" \
"../CLK_GEN/src/CLK_GEN" \
"../wishbone_arbiter/src/wishbone_arbiter" \
"../boot_mem128/src/boot_mem128" \
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
"../uart/src/uart" \
"../TDMI/src/TDMI" \
"../test_module/src/test_module" \
"../timer_module/src/timer_module" \
"../interrupt_controller/src/interrupt_controller" \
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
"../enc/src/enc" \
"../dec/src/dec" \
"../CFG_INT/src/CFG_INT" \
]

set vs_modules [list \
"../a25_core/src/generic_sram_byte_en_tsmc18_scan" \ # amber cache memory file
"../a25_core/src/generic_sram_line_en_tsmc18_scan" \ # amber cache memory file
]
# will need to add references to boot memory files as well

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}

echo ""
echo "Reading Verilog Netlist Files"
echo ""

foreach this_module $vs_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".vs"]]
}
