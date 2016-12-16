
current_design $design_name

#/* 10k gates in tsmc 0.18 */

#
# apply constraints
#

#
# clock constraints must already be defined
#

set hfo_nets [list "dec_i_clk" "clk" "reset" "scan_enable" "test_mode"]

foreach_in_collection this_cell [find design -hierarchy] {
  current_design $this_cell
  foreach hfo_net $hfo_nets {
    foreach_in_collection this_port [find net $hfo_net] {
      set_ideal_network [list $this_port]
    }
    foreach_in_collection this_port [find port $hfo_net] {
      set_dont_touch_network [list $this_port]
    }
  }
}

current_design $design_name
set_false_path -from [list reset scan_enable]

if {$tech_lib == $tsmc18_tech_lib} {
set_driving_cell  -lib_cell DFFX1  -pin Q -library $library_name [list [remove_from_collection [all_inputs] [list dec_i_clk clk reset scan_enable]]]
}

if {$tech_lib == $tsmc065_tech_lib} {
set_driving_cell  -lib_cell DFQD1  -pin Q -library $library_name [list [remove_from_collection [all_inputs] [list dec_i_clk clk reset scan_enable]]]
}


if {$tech_lib == $saed90nm_tech_lib} {
set_driving_cell  -lib_cell DFFARX1  -pin Q -library $library_name [list [remove_from_collection [all_inputs] [list dec_i_clk clk reset scan_enable]]]
}

if {$tech_lib == $saed32nm_tech_lib} {
set_driving_cell  -lib_cell DFFARX1_RVT  -pin Q -library $library_name [list [remove_from_collection [all_inputs] [list dec_i_clk clk reset scan_enable]]]
}

set_input_delay $INPUT_DELAY -clock clk [list [remove_from_collection [all_inputs] [list dec_i_clk clk reset scan_enable]]]
set_input_delay 0 -clock clk [list dec_i_clk clk reset scan_enable]
set_output_delay $OUTPUT_DELAY -clock clk [list [all_outputs]]
set_max_delay -from clk -to [list [all_outputs]] $OUTPUT_DELAY

set_input_delay $INPUT_DELAY -clock dec_i_clk [list [remove_from_collection [all_inputs] [list dec_i_clk clk reset scan_enable]]]
set_input_delay 0 -clock dec_i_clk [list dec_i_clk clk reset scan_enable]
set_output_delay $OUTPUT_DELAY -clock dec_i_clk [list [all_outputs]]
set_max_delay -from dec_i_clk -to [list [all_outputs]] $OUTPUT_DELAY

if {$tech_lib == $tsmc18_tech_lib} {
set my_load [load_of $library_name/DFFX1/D]
}

if {$tech_lib == $tsmc065_tech_lib} {
set my_load [load_of $library_name/DFQD1/D]
}

if {$tech_lib == $saed90nm_tech_lib} {
set my_load [load_of $library_name/DFFARX1/D]
}

if {$tech_lib == $saed32nm_tech_lib} {
set my_load [load_of $library_name/DFFARX1_RVT/D]
}

set my_load [expr $my_load * $LOAD_MULT]
set_load $my_load [list [all_outputs]]

current_design $design_name

