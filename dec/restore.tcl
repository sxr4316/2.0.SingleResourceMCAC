
# NC-Sim Command File
# TOOL:	ncsim(64)	14.10-s032
#
#
# You can restore this configuration with:
#
#      ncverilog +ncaccess+r +nclinedebug +ncvpicompat+1364v1995 +nowarnTFNPC +nowarnIWFA +nowarnSVTL +nowarnSDFNCAP +gui -s -f etc/dec.vec.rtl.f -input etc/dumpsaif.tcl +tcl+restore.tcl
#

set tcl_prompt1 {puts -nonewline "ncsim> "}
set tcl_prompt2 {puts -nonewline "> "}
set vlog_format %h
set vhdl_format %v
set real_precision 6
set display_unit auto
set time_unit module
set heap_garbage_size -200
set heap_garbage_time 0
set assert_report_level note
set assert_stop_level error
set autoscope yes
set assert_1164_warnings yes
set pack_assert_off {}
set severity_pack_assert_off {note warning}
set assert_output_stop_level failed
set tcl_debug_level 0
set relax_path_name 1
set vhdl_vcdmap XX01ZX01X
set intovf_severity_level ERROR
set probe_screen_format 0
set rangecnst_severity_level ERROR
set textio_severity_level ERROR
set vital_timing_checks_on 1
set vlog_code_show_force 0
set assert_count_attempts 1
set tcl_all64 false
set tcl_runerror_exit false
set assert_report_incompletes 0
set show_force 1
set force_reset_by_reinvoke 0
set tcl_relaxed_literal 0
set probe_exclude_patterns {}
set probe_packed_limit 4k
set probe_unpacked_limit 16k
set assert_internal_msg no
set svseed 1
set assert_reporting_mode 0
alias . run
alias iprof profile
alias quit exit
database -open -shm -into waves.shm waves -default
probe -create -database waves test.dec_s test.dec_s_clk test.dec_s_fs test.dec_o test.dec_o_clk test.dec_o_fs test.clk test.reset test.program_start test.top.s_wb_ack[8] test.top.s_wb_ack[7] test.top.s_wb_ack[4] test.top.s_wb_ack[2] test.top.s_wb_dat_w[8] test.top.s_wb_dat_r[8] test.top.s_wb_dat_r[4] test.top.s_wb_dat_r[2] test.top.s_wb_adr[4] test.top.s_wb_we[8] test.top.s_wb_we[4] test.top.s_wb_we[2] test.top.tdmi_int test.o_wb_ack_d test.data_out test.config_mod.addrs test.config_mod.dec_ch test.config_mod.decoder_configured test.config_mod.dec_ready_for_data test.config_mod.r_data test.config_done test.rs

simvision -input restore.tcl.svcf
