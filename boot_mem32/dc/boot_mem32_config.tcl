#
# read verilog source files for mcac
#
set all_modules [list \
"boot_mem32" \
]

echo ""
echo "Reading Verilog Source Files"
echo ""

foreach this_module $all_modules {
	read_file -format verilog [list [format "%s%s" $this_module ".v"]]
}
