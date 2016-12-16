//////////////////////////////////////////////////////////////////
//                                                              //
//  Register Addresses                                          //
//                                                              //
//  This file is part of the Amber project                      //
//  http://www.opencores.org/project,amber                      //
//                                                              //
//  Description                                                 //
//  Parameters that define the 16 lower bits of the address     //
//  of every register in the system. The upper 16 bits is       //
//  defined by which module the register is in.                 //
//                                                              //
//  Author(s):                                                  //
//      - Conor Santifort, csantifort.amber@gmail.com           //
//                                                              //
//////////////////////////////////////////////////////////////////
//                                                              //
// Copyright (C) 2010 Authors and OPENCORES.ORG                 //
//                                                              //
// This source file may be used and distributed without         //
// restriction provided that this copyright statement is not    //
// removed from the file and that any derivative work contains  //
// the original copyright notice and the associated disclaimer. //
//                                                              //
// This source file is free software; you can redistribute it   //
// and/or modify it under the terms of the GNU Lesser General   //
// Public License as published by the Free Software Foundation; //
// either version 2.1 of the License, or (at your option) any   //
// later version.                                               //
//                                                              //
// This source is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the implied   //
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      //
// PURPOSE.  See the GNU Lesser General Public License for more //
// details.                                                     //
//                                                              //
// You should have received a copy of the GNU Lesser General    //
// Public License along with this source; if not, download it   //
// from http://www.opencores.org/lgpl.shtml                     //
//                                                              //
//////////////////////////////////////////////////////////////////


// ======================================
// Register Addresses
// ======================================

// APRSC Module
localparam APRSC_TR                 = 16'h0000;
localparam APRSC_DQ                 = 16'h0004;
localparam APRSC_REQ                = 16'h0008;
localparam APRSC_RATE               = 16'h000C;
localparam APRSC_A2P                = 16'h0010;
localparam APRSC_SE                 = 16'h0014;
localparam APRSC_ACK                = 16'h0018;
localparam APRSC_SR                 = 16'h001C;

// TDMI Module
localparam TDMI_DATA		 	= 16'h0000;
localparam TDMI_CHANNEL			= 16'h0004;
localparam TDMI_ALL			= 16'h0008;
localparam TDMI_START                   = 16'h000C;

//<<<<<<< Updated upstream
// TDMO Module
localparam TDMO_CH0			= 16'h0000;
localparam TDMO_CH1			= 16'h0004;
localparam TDMO_CH2			= 16'h0008;
localparam TDMO_CH3			= 16'h000C;
localparam TDMO_CH4			= 16'h0010;
localparam TDMO_CH5			= 16'h0014;
localparam TDMO_CH6			= 16'h0018;
localparam TDMO_CH7			= 16'h001C;
localparam TDMO_CH8			= 16'h0020;
localparam TDMO_CH9			= 16'h0024;
localparam TDMO_CH10			= 16'h0028;
localparam TDMO_CH11			= 16'h002C;
localparam TDMO_CH12			= 16'h0030;
localparam TDMO_CH13			= 16'h0034;
localparam TDMO_CH14			= 16'h0038;
localparam TDMO_CH15			= 16'h003C;
localparam TDMO_CH16			= 16'h0040;
localparam TDMO_CH17			= 16'h0044;
localparam TDMO_CH18			= 16'h0048;
localparam TDMO_CH19			= 16'h004C;
localparam TDMO_CH20			= 16'h0050;
localparam TDMO_CH21			= 16'h0054;
localparam TDMO_CH22			= 16'h0058;
localparam TDMO_CH23			= 16'h005C;
localparam TDMO_CH24			= 16'h0060;
localparam TDMO_CH25			= 16'h0064;
localparam TDMO_CH26			= 16'h0068;
localparam TDMO_CH27			= 16'h006C;
localparam TDMO_CH28			= 16'h0070;
localparam TDMO_CH29			= 16'h0074;
localparam TDMO_CH30			= 16'h0078;
localparam TDMO_CH31			= 16'h007C;

// Config Module
//parameter [15:0] CONFIG_CH_REG [0:31]	=16'h0000;
localparam CONFIG_CH_REG0			=16'h0000;
localparam CONFIG_CH_REG1			=16'h0004;
localparam CONFIG_CH_REG2			=16'h0008;
localparam CONFIG_CH_REG3			=16'h000C;
localparam CONFIG_CH_REG4			=16'h0010;
localparam CONFIG_CH_REG5			=16'h0014;
localparam CONFIG_CH_REG6			=16'h0018;
localparam CONFIG_CH_REG7			=16'h001C;
localparam CONFIG_CH_REG8			=16'h0020;
localparam CONFIG_CH_REG9			=16'h0024;
localparam CONFIG_CH_REG10			=16'h0028;
localparam CONFIG_CH_REG11			=16'h002C;
localparam CONFIG_CH_REG12			=16'h0030;
localparam CONFIG_CH_REG13			=16'h0034;
localparam CONFIG_CH_REG14			=16'h0038;
localparam CONFIG_CH_REG15			=16'h003C;
localparam CONFIG_CH_REG16			=16'h0040;
localparam CONFIG_CH_REG17			=16'h0044;
localparam CONFIG_CH_REG18			=16'h0048;
localparam CONFIG_CH_REG19			=16'h004C;
localparam CONFIG_CH_REG20			=16'h0050;
localparam CONFIG_CH_REG21			=16'h0054;
localparam CONFIG_CH_REG22			=16'h0058;
localparam CONFIG_CH_REG23			=16'h005C;
localparam CONFIG_CH_REG24			=16'h0060;
localparam CONFIG_CH_REG25			=16'h0064;
localparam CONFIG_CH_REG26			=16'h0068;
localparam CONFIG_CH_REG27			=16'h006C;
localparam CONFIG_CH_REG28			=16'h0070;
localparam CONFIG_CH_REG29			=16'h0074;
localparam CONFIG_CH_REG30			=16'h0078;
localparam CONFIG_CH_REG31			=16'h007C;
localparam CONFIG_DONE_REG			=16'h0080;
localparam CONFIG_CORE_READY                    =16'h0084;
//>>>>>>> Stashed changes

// Test Module
localparam AMBER_TEST_STATUS   	    = 16'h0000;
localparam AMBER_TEST_FIRQ_TIMER    = 16'h0004;
localparam AMBER_TEST_IRQ_TIMER     = 16'h0008;
localparam AMBER_TEST_UART_CONTROL  = 16'h0010;
localparam AMBER_TEST_UART_STATUS   = 16'h0014;
localparam AMBER_TEST_UART_TXD      = 16'h0018;
localparam AMBER_TEST_SIM_CTRL      = 16'h001c;
localparam AMBER_TEST_MEM_CTRL      = 16'h0020;
localparam AMBER_TEST_CYCLES        = 16'h0024;
localparam AMBER_TEST_LED           = 16'h0028;
localparam AMBER_TEST_PHY_RST       = 16'h002c;

localparam AMBER_TEST_RANDOM_NUM    = 16'h0100;
localparam AMBER_TEST_RANDOM_NUM00  = 16'h0100;
localparam AMBER_TEST_RANDOM_NUM01  = 16'h0104;
localparam AMBER_TEST_RANDOM_NUM02  = 16'h0108;
localparam AMBER_TEST_RANDOM_NUM03  = 16'h010c;
localparam AMBER_TEST_RANDOM_NUM04  = 16'h0110;
localparam AMBER_TEST_RANDOM_NUM05  = 16'h0114;
localparam AMBER_TEST_RANDOM_NUM06  = 16'h0118;
localparam AMBER_TEST_RANDOM_NUM07  = 16'h011c;
localparam AMBER_TEST_RANDOM_NUM08  = 16'h0120;
localparam AMBER_TEST_RANDOM_NUM09  = 16'h0124;
localparam AMBER_TEST_RANDOM_NUM10  = 16'h0128;
localparam AMBER_TEST_RANDOM_NUM11  = 16'h012c;
localparam AMBER_TEST_RANDOM_NUM12  = 16'h0130;
localparam AMBER_TEST_RANDOM_NUM13  = 16'h0134;
localparam AMBER_TEST_RANDOM_NUM14  = 16'h0138;
localparam AMBER_TEST_RANDOM_NUM15  = 16'h013c;


// Interrupt Controller
localparam AMBER_IC_IRQ0_STATUS     = 16'h0000;
localparam AMBER_IC_IRQ0_RAWSTAT    = 16'h0004;
localparam AMBER_IC_IRQ0_ENABLESET  = 16'h0008;
localparam AMBER_IC_IRQ0_ENABLECLR  = 16'h000c;
localparam AMBER_IC_INT_SOFTSET_0   = 16'h0010;
localparam AMBER_IC_INT_SOFTCLEAR_0 = 16'h0014;
localparam AMBER_IC_FIRQ0_STATUS    = 16'h0020;
localparam AMBER_IC_FIRQ0_RAWSTAT   = 16'h0024;
localparam AMBER_IC_FIRQ0_ENABLESET = 16'h0028;
localparam AMBER_IC_FIRQ0_ENABLECLR = 16'h002c;
localparam AMBER_IC_IRQ1_STATUS     = 16'h0040;
localparam AMBER_IC_IRQ1_RAWSTAT    = 16'h0044;
localparam AMBER_IC_IRQ1_ENABLESET  = 16'h0048;
localparam AMBER_IC_IRQ1_ENABLECLR  = 16'h004c;
localparam AMBER_IC_INT_SOFTSET_1   = 16'h0050;
localparam AMBER_IC_INT_SOFTCLEAR_1 = 16'h0054;
localparam AMBER_IC_FIRQ1_STATUS    = 16'h0060;
localparam AMBER_IC_FIRQ1_RAWSTAT   = 16'h0064;
localparam AMBER_IC_FIRQ1_ENABLESET = 16'h0068;
localparam AMBER_IC_FIRQ1_ENABLECLR = 16'h006c;
localparam AMBER_IC_INT_SOFTSET_2   = 16'h0090;
localparam AMBER_IC_INT_SOFTCLEAR_2 = 16'h0094;
localparam AMBER_IC_INT_SOFTSET_3   = 16'h00d0;
localparam AMBER_IC_INT_SOFTCLEAR_3 = 16'h00d4;


// Timer Module
localparam AMBER_TM_TIMER0_LOAD    =  16'h0000;
localparam AMBER_TM_TIMER0_VALUE   =  16'h0004;
localparam AMBER_TM_TIMER0_CTRL    =  16'h0008;
localparam AMBER_TM_TIMER0_CLR     =  16'h000c;
localparam AMBER_TM_TIMER1_LOAD    =  16'h0100;
localparam AMBER_TM_TIMER1_VALUE   =  16'h0104;
localparam AMBER_TM_TIMER1_CTRL    =  16'h0108;
localparam AMBER_TM_TIMER1_CLR     =  16'h010c;
localparam AMBER_TM_TIMER2_LOAD    =  16'h0200;
localparam AMBER_TM_TIMER2_VALUE   =  16'h0204;
localparam AMBER_TM_TIMER2_CTRL    =  16'h0208;
localparam AMBER_TM_TIMER2_CLR     =  16'h020c;


// UART 0 and 1
localparam AMBER_UART_PID0         =  16'h0fe0;
localparam AMBER_UART_PID1         =  16'h0fe4;
localparam AMBER_UART_PID2         =  16'h0fe8;
localparam AMBER_UART_PID3         =  16'h0fec;
localparam AMBER_UART_CID0         =  16'h0ff0;
localparam AMBER_UART_CID1         =  16'h0ff4;
localparam AMBER_UART_CID2         =  16'h0ff8;
localparam AMBER_UART_CID3         =  16'h0ffc;
localparam AMBER_UART_DR           =  16'h0000;
localparam AMBER_UART_RSR          =  16'h0004;
localparam AMBER_UART_LCRH         =  16'h0008;
localparam AMBER_UART_LCRM         =  16'h000c;
localparam AMBER_UART_LCRL         =  16'h0010;
localparam AMBER_UART_CR           =  16'h0014;
localparam AMBER_UART_FR           =  16'h0018;
localparam AMBER_UART_IIR          =  16'h001c;
localparam AMBER_UART_ICR          =  16'h001c;

