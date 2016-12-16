
#
# set design name
#
set design_name LIMD

set my_type "_scan"

set max_path_count 10

# Turn on auto wire load selection
# (library must support this feature)
set auto_wire_load_selection true

# To account for clock network latency in external delays, you can set the
# estimate_io_latency variable to true. When this variable is set to true, the tool assigns
# clock network delay to every input or output port. The tool assumes that clock tree synthesis
# balances network latency across different clocks. Based on this assumption, the tool
# estimates off-chip clock latency with reference to the synthesized clock tree in the current
# design.
set estimate_io_latency true

file mkdir "./netlist" "./sdf" "./spf" "./report" "./report/dc" "./report/pt"
set report_dir "./report/pt/"

#/* set technology library */
source "dc/tech_config.tcl"

#
# read netlist
#
read_file -format verilog [format "%s%s"  [format "%s%s"  [format "%s%s"  "./netlist/" $design_name] $tech_lib] "${my_type}.vs"]

current_design $design_name

read_sdf [format "%s%s"  [format "%s%s"  [format "%s%s"  "./sdf/" $design_name] $tech_lib] "${my_type}.sdf"]

current_design $design_name

#
# apply constraints
#
source "cons/${design_name}_cons_defaults.tcl"
source "cons/${design_name}_clocks_all_cons.tcl"
source "cons/${design_name}_cons.tcl"


##################################################################
#    Clock Tree Synthesis Section                                #
##################################################################

set_propagated_clock [all_clocks]


##################################################################
#    Update_timing and check_timing Section                      #
##################################################################

set_case_analysis 0 [get_ports reset]
set_case_analysis 0 [get_ports scan_enable]
set_case_analysis 0 [get_ports test_mode]

update_timing -full

set_case_analysis rising [get_ports reset]

update_timing

set_case_analysis falling [get_ports reset]

update_timing

set_case_analysis rising [get_ports scan_enable]

update_timing

set_case_analysis falling [get_ports scan_enable]

update_timing

set_case_analysis rising [get_ports test_mode]

update_timing

set_case_analysis falling [get_ports test_mode]

update_timing


# Ensure design is properly constrained
redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_ct.rpt"] { check_timing -verbose }

##################################################################
#    Report_timing Section                                       #
##################################################################
# Report Timing
redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -pba_mode exhaustive -path_type full -delay_type max -sort_by slack -max_paths $max_path_count }
redirect -tee -append -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -pba_mode exhaustive -path_type full -delay_type max -sort_by slack -from [all_registers -clock_pins] -to [all_registers -data_pins] -max_paths $max_path_count }
redirect -tee -append -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -path_type full -delay_type max -sort_by slack -cover_design }
redirect -tee -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -true -slack_greater_than 0.0}
redirect -tee -append -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -max_paths 10 -from [all_registers -clock_pins] -to [all_registers -data_pins] -slack_greater_than 0.0}
redirect -tee -append -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -max_paths 10 -slack_greater_than 0.0}
redirect -tee -append [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -true -slack_lesser_than 0.0}
redirect -tee -append -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -max_paths 10 -from [all_registers -clock_pins] -to [all_registers -data_pins] -slack_lesser_than 0.0}
redirect -tee -append -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rt.rpt"] { report_timing -sign 4 -max_paths 10 -slack_lesser_than 0.0}

redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rcv.rpt"] { report_constraints -all_violators }

redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rclk.rpt"] { report_clock -skew -attribute }

redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rac.rpt"] { report_analysis_coverage -status_details { untested violated } }

redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rca.rpt"] { report_case_analysis -all }

redirect -tee -file [format "%s%s"  [format "%s%s"  [format "%s%s"  $report_dir $design_name] $tech_lib] "${my_type}_pt_rdt.rpt"] { report_disable_timing }
