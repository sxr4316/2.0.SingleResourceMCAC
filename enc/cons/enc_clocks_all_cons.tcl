
current_design $design_name

create_clock -name clk -period $CLK_PERIOD -waveform [list 0 [expr $CLK_PERIOD / 2.0]] [get_ports clk]
set_clock_uncertainty $CLK_SKEW clk

#create_clock -name enc_s_clk -period $CLK_PERIOD -waveform [list 0 [expr #$CLK_PERIOD / 2.0]] [get_ports enc_s_clk]
#set_clock_uncertainty $CLK_SKEW enc_s_clk
create_generated_clock -divide_by $SER_DIVIDE_FACTOR -source clk [get_ports enc_s_clk]

set_clock_gating_check -setup [expr $CLK_PERIOD * 0.05] -hold [expr $CLK_PERIOD * 0.15] [all_clocks]

current_design $design_name

#/*********************************************************/
#/*                                                       */
