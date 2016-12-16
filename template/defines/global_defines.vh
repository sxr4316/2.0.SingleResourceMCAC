
`ifdef ENC
  `define A 1
  `define B 1
`else
  `ifdef DEC
    `define A 2
    `define B 2
  `else
    `define A 3
    `define B 3
  `endif
`endif
