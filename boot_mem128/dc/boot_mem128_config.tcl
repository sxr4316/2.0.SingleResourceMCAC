#
# read verilog source files for mcac
#
set all_modules [list \
"boot_mem128" \
]

set vs_modules [list \
"../generic_sram_byte_en/netlist/generic_sram_byte_en_tsmc18_scan/generic_sram_byte_en" \
"../generic_sram_line_en/netlist/generic_sram_line_en_tsmc18_scan/generic_sram_line_en" \
]

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}

foreach this_module $vs_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".vs"]]
}
