+nowarnTFNPC
+nowarnIWFA
+nowarnSVTL
src/timescale.v
src/a25_core_vec_test.v

src/a25_alu.v
src/a25_barrel_shift.v
src/a25_coprocessor.v
src/a25_dcache.v
src/a25_decode.v
src/a25_decompile.v
src/a25_execute.v
src/a25_fetch.v
src/a25_icache.v
src/a25_mem.v
src/a25_multiply.v
src/a25_register_bank.v
src/a25_shifter.v
src/a25_wishbone_buf.v
src/a25_wishbone.v
src/a25_write_back.v
src/a25_core.v
../generic_sram_byte_en/src/generic_sram_byte_en.v
../generic_sram_line_en/src/generic_sram_line_en.v

//
// Add source files above
//
//
// Uncomment for TSMC 180nm
-v /classes/eeee720/maieee/lib/tsmc-0.18/verilog/tsmc18.v
//
// Uncomment for TSMC 65nm
// -v /classes/ee620/maieee/lib/synopsys/TSMC_tcbc65/TSMCHOME/digital/Front_End/verilog/tcbn65lp_200a/tcbn65lp.v
//
// Uncomment for SAED 90nm
// -y /classes/ee620/maieee/lib/synopsys/SAED_EDK90nm/Digital_Standard_Cell_Library/verilog/
//
// Uncomment for SAED 32nm
// -v /classes/ee620/maieee/lib/synopsys/SAED_EDK32-28nm/SAED32_EDK/lib/stdcell_rvt/verilog/saed32nm.v
//
+librescan
