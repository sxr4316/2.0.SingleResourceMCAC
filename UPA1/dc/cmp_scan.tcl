
#
# set design name
#
set design_name UPA1

set my_type "_scan"
set my_input_type "_rtl"

set my_max_area 600
#
# compile effort can be: low, medium, high
#
set my_compile_effort "high"

set hdlin_enable_presto false
set hdlin_keep_signal_name all

set bus_naming_style {%s[%d]}
set bus_inference_style {%s[%d]}

#/* connect to all ports in the design, even if driven by the same net */
#/* compile_fix_multiple_port_nets = true */
set_fix_multiple_port_nets -all -buffer_constants

#/* do not allow wire type tri in the netlist */
set verilogout_no_tri true

set verilogout_equation false

#/* to fix those pesky escaped names */
#/* to be used with 'change_names -hierarchy' */
#/* after a compile - should only be needed in */
#/* extreme cases when 'bus_naming_style' isn't fully working */
define_name_rules Verilog -allowed {a-z A-Z 0-9 _} -first_restricted {0-9 _} -replacement_char "__" -type cell
define_name_rules Verilog -allowed {a-z A-Z 0-9 _ []} -first_restricted {0-9 _} -replacement_char "__" -type port
define_name_rules Verilog -allowed {a-z A-Z 0-9 _} -first_restricted {0-9 _} -replacement_char "__" -type net
set default_name_rules Verilog

#
# for SAIF file generation
#
set power_preserve_rtl_hier_names true

remove_design -all
file mkdir "./netlist" "./sdf" "./spf" "./report" "./report/dc" "./report/pt" "./saif"
set report_dir "./report/dc/"
set saif_dir "./saif/"

set hdlin_use_cin true
set synlib_model_map_effort "high"
set hdlout_uses_internal_busses true
# Turn on auto wire load selection
# (library must support this feature)
set auto_wire_load_selection true

set synlib_wait_for_design_license "DesignWare"

#/* set technology library */
source "dc/tech_config.tcl"

set link_library [concat  $link_library $synthetic_library]

#
# read netlist
#
read_file -format verilog [list [format "%s%s%s%s" "netlist/" $design_name $tech_lib "${my_input_type}.vs"]]

current_design $design_name

#
# apply constraints
#
source "cons/${design_name}_cons_defaults.tcl"
source "cons/${design_name}_clocks_cons.tcl"

redirect [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { echo [concat [format "%s%s"  {Compile effort is } $my_compile_effort]] }

link

redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { echo [concat {Pre-compile check_design}] }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { check_design }

current_design $design_name

#/*********************************************************/
#/*                                                       */
#/* Set up default constraints for scan                   */
#/* insertion.  This includes setting all scan input,     */
#/* scan_output, and scan enable port constraints.  This  */
#/* script will only work if the scan ports are named     */
#/* using the standard naming conventions.                */
#/*                                                       */
#/*********************************************************/

#/* Set scan defaults                                     */

set test_scan_in_port_naming_style scan_in%s%s
set test_scan_out_port_naming_style scan_out%s%s

set t_reset [find port "reset"]

if { [get_object_name $t_reset] != "reset" } {
   echo [concat {DFT WARNING : NO RESET SIGNAL FOUND IN CURRENT DESIGN}]
} else {

set_dft_signal -view existing_dft -port $t_reset -type Reset -active_state 1
}

set t_test_mode [find port "test_mode"]

if { [get_object_name $t_test_mode] != "test_mode" } {
   echo [concat {DFT WARNING : NO TEST_MODE SIGNAL FOUND IN CURRENT DESIGN}]
} else {

set_dft_signal -view existing_dft -port $t_test_mode -type TestMode -active_state 1
}

set t_scan_en [find port "scan_enable"]

if { [get_object_name $t_scan_en] != "scan_enable" } {
   echo [concat {DFT WARNING : NO SCAN_ENABLE SIGNAL FOUND IN CURRENT DESIGN}]
} else {

set_dft_signal -view existing_dft -port $t_scan_en -type ScanEnable -active_state 1
}

set scan_inputs [find port scan_in*]

if {  $scan_inputs == [list] } {
   echo [concat {ERROR: NO SCAN INPUTS DEFINED TEST INSERTION NOT PERFORMED}]
  exit 0
}

set scan_outputs [find port scan_out*]

if {  $scan_outputs == [list] } {
   echo [concat {ERROR: NO SCAN OUTPUTS DEFINED TEST INSERTION NOT PERFORMED}]
  exit 0
}

set input_length 0

foreach_in_collection scan_input $scan_inputs {
  set t_scan_in [find port $scan_input]
  echo [format "%s %s %s" "FOUND" [get_attribute -quiet $t_scan_in full_name] "IN CURRENT DESIGN"]
  set_dft_signal -view existing_dft -port $t_scan_in -type ScanDataIn
  set input_length [expr $input_length + 1]
}

set output_length 0

foreach_in_collection scan_output $scan_outputs {
  set t_scan_out [find port $scan_output]
  echo [format "%s %s %s" "FOUND" [get_attribute -quiet $t_scan_out full_name] "IN CURRENT DESIGN"]
  set_dft_signal -view existing_dft -port $t_scan_out -type ScanDataOut
  set output_length [expr $output_length + 1]
}

set length 0

if {  $input_length == $output_length } {
   set length $input_length
} else {
   echo [concat {ERROR: NUMBER OF SCAN INPUTS (} $input_length {) DOES NOT MATCH THE NUMBER OF SCAN OUTPUTS (} $output_length ")"]
  echo [concat {       SCAN INSERTION NOT PEFORMED.}]
  exit 0
}

set_scan_configuration -chain_count $length
echo [concat {INFO: NUMBER OF CHAINS IS } $length]

#/*********************************************************/
#/*                                                       */
#/* Perform scan test insertion                           */                   
#/*                                                       */
#/*********************************************************/
#   

#/* Insert full scan                                      */ 
echo [concat "############################################"]
echo [concat { INFO: CHECKING DFT RULES}]
echo [concat "############################################"]
create_test_protocol -infer_asynch -infer_clock -capture_procedure single_clock
preview_dft
write_test_protocol -out [format "%s%s"  [format "%s%s"  [format "%s%s"  "./spf/" $design_name] $tech_lib] "${my_type}.spf"]

echo [concat "############################################"]
echo [concat { INFO: INSERTING TEST LOGIC}]
echo [concat "############################################"]
insert_dft

echo [concat "############################################"]
echo [concat { INFO: CHECKING TEST LOGIC}]
echo [concat "############################################"]
dft_drc -verbose
redirect -append -tee [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { dft_drc -coverage_estimate }
# 
current_design $design_name

#/* 10k gates in tsmc 0.18 */

#
# apply constraints
#
source "cons/${design_name}_cons.tcl"

set_max_area $my_max_area
set_critical_range 1 [current_design]

#
# quick logic synthesis
#
compile -map_effort $my_compile_effort -incremental -scan

#/* connect to all ports in the design, even if driven by the same net */
#/* compile_fix_multiple_port_nets = true */
set_fix_multiple_port_nets -all -buffer_constants

#/* do not allow wire type tri in the netlist */
set verilogout_no_tri true

set verilogout_equation false

change_names -hierarchy

#
# Write reports
#
current_design $design_name
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { echo [concat {Post-compile check_design}] }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { check_design }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_area }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_area -hierarchy }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_reference }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_reference -hierarchy }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_dft }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_scan_path -chain all }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s" $report_dir $design_name] $tech_lib] "_dc.rpt"] { report_net_fanout -high_fanout }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_clocks }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_timing -sign 4 -max_paths 10 }

echo [concat {Power Analysis}]
current_design $design_name

if {[file exists [format "%s%s" $saif_dir "${design_name}_bw.saif"]]} {
    reset_switching_activity
    echo [concat {Reading Backwards SAIF File}]
    read_saif -verbose -map_names -input [format "%s%s" $saif_dir "${design_name}_bw.saif"] -instance_name test/top
}

redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_power }
redirect -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_dc.rpt"] { report_power -hier }

#
# Write Netlist and SDF
#
current_design $design_name
write -hierarchy -format verilog $design_name -output [format "%s%s"  [format "%s%s"  [format "%s%s"  "./netlist/" $design_name] $tech_lib] "${my_type}.vs"]
write_sdf -context verilog [format "%s%s"  [format "%s%s"  [format "%s%s"  "./sdf/" $design_name] $tech_lib] "${my_type}.sdf"]

current_design $design_name
set hier_dir [format "%s%s%s%s%s%s" "./netlist" "/" $design_name $tech_lib ${my_type} "/"]
file mkdir $hier_dir

current_design $design_name
write -format verilog -output [format "%s%s" [format "%s%s" $hier_dir $design_name] ".vs"] $design_name

foreach_in_collection this_cell [find design -hierarchy] {
  current_design $this_cell
  set current_cell [get_attribute [get_designs $this_cell] full_name]
  write -format verilog -output [format "%s%s" [format "%s%s" $hier_dir $current_cell] ".vs"] $current_cell
}


current_design $design_name
