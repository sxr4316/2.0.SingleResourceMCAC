#
# read verilog source files for mcac
#
set all_modules [list \
"wb_xs6_ddr3_bridge" \
]

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}
