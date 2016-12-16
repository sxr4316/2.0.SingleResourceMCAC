+nowarnTFNPC
+nowarnIWFA
+nowarnSVTL
src/timescale.v
src/dec_vec_test.v
src/tb_uart.v
src/dumpvcd.v
src/main_mem.v
src/dec.v

-y ../a25_core/src/

../generic_sram_byte_en/src/generic_sram_byte_en.v
../generic_sram_line_en/src/generic_sram_line_en.v
../wishbone_arbiter/src/wishbone_arbiter.v
../boot_mem128/src/boot_mem128.v
../APRSC/src/APRSC.v
../uart/src/uart.v
../TDMI/src/TDMI.v
../test_module/src/test_module.v
../timer_module/src/timer_module.v
../interrupt_controller/src/interrupt_controller.v
../TDMO/src/TDMO.v
../CFG_INT/src/CFG_INT.v
../CLK_GEN/src/CLK_GEN.v

../ADDB/src/ADDB.v
../ADDC/src/ADDC.v
../DELAY/src/DELAY.v
../FMULT_ACCUM/src/FMULT_ACCUM.v
../FLOATA/src/FLOATA.v
../FLOATB/src/FLOATB.v
../LIMC/src/LIMC.v
../LIMD/src/LIMD.v
../TRIGB/src/TRIGB.v
../UPA1/src/UPA1.v
../UPA2/src/UPA2.v
../UPB/src/UPB.v
../XOR/src/XOR.v
../FMULT/src/FMULT.v
../ACCUM/src/ACCUM.v

//
// Add source files above
//
//
// Uncomment for TSMC 180nm
//-v /classes/eeee720/maieee/lib/tsmc-0.18/verilog/tsmc18.v
//
// Uncomment for TSMC 65nm
 -v /classes/ee620/maieee/lib/synopsys/TSMC_tcbc65/TSMCHOME/digital/Front_End/verilog/tcbn65lp_200a/tcbn65lp.v
//
// Uncomment for SAED 90nm
// -y /classes/ee620/maieee/lib/synopsys/SAED_EDK90nm/Digital_Standard_Cell_Library/verilog/
//
// Uncomment for SAED 32nm
// -v /classes/ee620/maieee/lib/synopsys/SAED_EDK32-28nm/SAED32_EDK/lib/stdcell_rvt/verilog/saed32nm.v
//
+libext+.v
+librescan
