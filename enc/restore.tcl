
# NC-Sim Command File
# TOOL:	ncsim(64)	14.10-s032
#
#
# You can restore this configuration with:
#
#      ncverilog +ncaccess+r +ncvpicompat+1364v1995 +nowarnTFNPC +nowarnIWFA +nowarnSVTL +nowarnSDFNCAP +gui +nclinedebug -s -f etc/enc.vec.rtl.f -input etc/dumpsaif.tcl +tcl+restore.tcl
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
probe -create -database waves test.top.clk test.top.enc_o_clk test.config_mod.enc_ch test.top.enc_o test.top.enc_s_clk test.top.enc_s test.top.enc_o_fs test.data_out test.top.s_wb_adr[4] test.top.s_wb_dat_r[4] test.top.s_wb_dat_w[4] test.top.enc_s_fs test.top.s_wb_stb[4] test.top.s_wb_we[4] test.top.s_wb_ack[4] test.top.s_wb_dat_r[8] test.top.s_wb_dat_w[8] test.top.s_wb_stb[8] test.top.s_wb_we[8] test.top.s_wb_ack[8] test.program_start test.captured_data test.tdmo_ch_count test.top.s_wb_ack[2] test.top.s_wb_adr[2] test.top.s_wb_cyc[2] test.top.s_wb_dat_r[2] test.top.s_wb_dat_w[2] test.top.s_wb_sel[2] test.top.s_wb_stb[2] test.top.s_wb_we[2] test.tdmo_test_count test.top.s_wb_ack[7] test.top.tdmi_inst0.new_data_int test.config_mod.dec_ch test.config_mod.o_wb_dat_e test.config_mod.o_wb_ack_e test.top.tdmo_inst0.data_reg

simvision -input restore.tcl.svcf
