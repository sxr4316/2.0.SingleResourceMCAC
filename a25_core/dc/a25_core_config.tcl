#
# read verilog source files for mcac
#
set all_modules [list \
"a25_core" \
"a25_alu" \
"a25_barrel_shift" \
"a25_coprocessor" \
"a25_dcache" \
"a25_decode" \
"a25_decompile" \
"a25_execute" \
"a25_fetch" \
"a25_icache" \
"a25_mem" \
"a25_multiply" \
"a25_register_bank" \
"a25_shifter" \
"a25_wishbone_buf" \
"a25_wishbone" \
"a25_write_back" \
]

#set vs_modules [list \
#"generic_sram_byte_en_tsmc18_scan" \
#"generic_sram_line_en_tsmc18_scan" \
#]

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}

#foreach this_module $vs_modules {
#	read_file -format verilog [list [format "%s%s" $this_module ".vs"]]
#}
