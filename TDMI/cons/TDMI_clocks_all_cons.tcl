
current_design $design_name

create_clock -name clk -period $CLK_PERIOD -waveform [list 0 [expr $CLK_PERIOD / 2.0]] [get_ports clk]
set_clock_uncertainty $CLK_SKEW clk

set_clock_gating_check -setup [expr $CLK_PERIOD * 0.05] -hold [expr $CLK_PERIOD * 0.15] [all_clocks]

create_clock -name ser_clk -period $SER_CLK_PERIOD -waveform [list 0 [expr $SER_CLK_PERIOD / 2.0]] [get_ports ser_clk]
set_clock_uncertainty $CLK_SKEW ser_clk

set_clock_gating_check -setup [expr $SER_CLK_PERIOD * 0.05] -hold [expr $SER_CLK_PERIOD * 0.15] [all_clocks]

current_design $design_name

#/*********************************************************/
#/*                                                       */
