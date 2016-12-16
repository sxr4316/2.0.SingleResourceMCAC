/*
Description		: Definitions for all peripherals that the amber core communicates with. Everything is memory mapped,
			: so every register is a simple pointer that is dereferenced to return the actualy value.

Author		: David Malanga

Revision History	:
//----------------------------------------------------------------------------------
3/23/16	-	by Dave Malanga	-	Initial creation
//----------------------------------------------------------------------------------
*/


//----------------------------------------------------------------------------------
// APRSC Functions
void aprsc_send(char tr, char rate, short int dq);
char aprsc_read(char enc_dec, short int *a2p, short int *se, short int *sr);
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// TDMI Functions
unsigned char tdmi_read_data(void);
unsigned char tdmi_read_channel(void);
unsigned int tdmi_read_all(void);
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Timer Functions
void timer_start(unsigned char timer_number, short int countdown_value);
void timer_stop(unsigned char timer_number);
void timer_clear(unsigned char timer_number);
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Interrupt Control Functions
void enable_interrupt(char int_number);
void disable_interrupt(char int_number);
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// TDMO Functions
void tdmo_send(char channel, char data);
unsigned char tdmo_read(unsigned char channel);
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Configuration Module Functions
char config_read(char channel);
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// Interrupt service routine prototype
void irq_interrupt(void);

//----------------------------------------------------------------------------------
// Base Addresses For Each Peripheral
#define	TIMER_BASE			(0x13000000)
#define	IC_BASE			(0x14000000)
#define	UART_BASE			(0x16000000)
#define	TDMI_BASE			(0x17000000)
#define	TDMO_BASE			(0x18000000)
#define	APRSC_BASE			(0x19000000)
#define	CONFIG_BASE			(0x1A000000)
#define	TEST_BASE			(0xF0000000)
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Register Declarations
#define	TDMI_DATA		(*(volatile unsigned long int *)(TDMI_BASE + 0x0000))
#define	TDMI_CHANNEL	(*(volatile unsigned long int *)(TDMI_BASE + 0x0004))
#define	TDMI_ALL		(*(volatile unsigned long int *)(TDMI_BASE + 0x0008))
#define	TDMI_START		(*(volatile unsigned long int *)(TDMI_BASE + 0x000C))
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// TDMO Register Declarations
#define	TDMO_CH0		(*(volatile unsigned long int *)(TDMO_BASE + 0x0000))
#define	TDMO_CH1		(*(volatile unsigned long int *)(TDMO_BASE + 0x0004))
#define	TDMO_CH2		(*(volatile unsigned long int *)(TDMO_BASE + 0x0008))
#define	TDMO_CH3		(*(volatile unsigned long int *)(TDMO_BASE + 0x000C))
#define	TDMO_CH4		(*(volatile unsigned long int *)(TDMO_BASE + 0x0010))
#define	TDMO_CH5		(*(volatile unsigned long int *)(TDMO_BASE + 0x0014))
#define	TDMO_CH6		(*(volatile unsigned long int *)(TDMO_BASE + 0x0018))
#define	TDMO_CH7		(*(volatile unsigned long int *)(TDMO_BASE + 0x001C))
#define	TDMO_CH8		(*(volatile unsigned long int *)(TDMO_BASE + 0x0020))
#define	TDMO_CH9		(*(volatile unsigned long int *)(TDMO_BASE + 0x0024))
#define	TDMO_CH10		(*(volatile unsigned long int *)(TDMO_BASE + 0x0028))
#define	TDMO_CH11		(*(volatile unsigned long int *)(TDMO_BASE + 0x002C))
#define	TDMO_CH12		(*(volatile unsigned long int *)(TDMO_BASE + 0x0030))
#define	TDMO_CH13		(*(volatile unsigned long int *)(TDMO_BASE + 0x0034))
#define	TDMO_CH14		(*(volatile unsigned long int *)(TDMO_BASE + 0x0038))
#define	TDMO_CH15		(*(volatile unsigned long int *)(TDMO_BASE + 0x003C))
#define	TDMO_CH16		(*(volatile unsigned long int *)(TDMO_BASE + 0x0040))
#define	TDMO_CH17		(*(volatile unsigned long int *)(TDMO_BASE + 0x0044))
#define	TDMO_CH18		(*(volatile unsigned long int *)(TDMO_BASE + 0x0048))
#define	TDMO_CH19		(*(volatile unsigned long int *)(TDMO_BASE + 0x004C))
#define	TDMO_CH20		(*(volatile unsigned long int *)(TDMO_BASE + 0x0050))
#define	TDMO_CH21		(*(volatile unsigned long int *)(TDMO_BASE + 0x0054))
#define	TDMO_CH22		(*(volatile unsigned long int *)(TDMO_BASE + 0x0058))
#define	TDMO_CH23		(*(volatile unsigned long int *)(TDMO_BASE + 0x005C))
#define	TDMO_CH24		(*(volatile unsigned long int *)(TDMO_BASE + 0x0060))
#define	TDMO_CH25		(*(volatile unsigned long int *)(TDMO_BASE + 0x0064))
#define	TDMO_CH26		(*(volatile unsigned long int *)(TDMO_BASE + 0x0068))
#define	TDMO_CH27		(*(volatile unsigned long int *)(TDMO_BASE + 0x006C))
#define	TDMO_CH28		(*(volatile unsigned long int *)(TDMO_BASE + 0x0070))
#define	TDMO_CH29		(*(volatile unsigned long int *)(TDMO_BASE + 0x0074))
#define	TDMO_CH30		(*(volatile unsigned long int *)(TDMO_BASE + 0x0078))
#define	TDMO_CH31		(*(volatile unsigned long int *)(TDMO_BASE + 0x007C))
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// APRSC Register Declarations
#define	APRSC_TR 		(*(volatile unsigned long int *)(APRSC_BASE + 0x0000))
#define	APRSC_DQ 		(*(volatile unsigned long int *)(APRSC_BASE + 0x0004))
#define	APRSC_REQ 		(*(volatile unsigned long int *)(APRSC_BASE + 0x0008))
#define	APRSC_RATE 		(*(volatile unsigned long int *)(APRSC_BASE + 0x000C))
#define	APRSC_A2P 		(*(volatile unsigned long int *)(APRSC_BASE + 0x0010))
#define	APRSC_SE 		(*(volatile unsigned long int *)(APRSC_BASE + 0x0014))
#define	APRSC_ACK 		(*(volatile unsigned long int *)(APRSC_BASE + 0x0018))
#define	APRSC_SR 		(*(volatile unsigned long int *)(APRSC_BASE + 0x001C))
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Configuration Module Declarations
#define	CONFIG_CH0		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0000))
#define	CONFIG_CH1		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0004))
#define	CONFIG_CH2		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0008))
#define	CONFIG_CH3		(*(volatile unsigned long int *)(CONFIG_BASE + 0x000C))
#define	CONFIG_CH4		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0010))
#define	CONFIG_CH5		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0014))
#define	CONFIG_CH6		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0018))
#define	CONFIG_CH7		(*(volatile unsigned long int *)(CONFIG_BASE + 0x001C))
#define	CONFIG_CH8		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0020))
#define	CONFIG_CH9		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0024))
#define	CONFIG_CH10		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0028))
#define	CONFIG_CH11		(*(volatile unsigned long int *)(CONFIG_BASE + 0x002C))
#define	CONFIG_CH12		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0030))
#define	CONFIG_CH13		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0034))
#define	CONFIG_CH14		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0038))
#define	CONFIG_CH15		(*(volatile unsigned long int *)(CONFIG_BASE + 0x003C))
#define	CONFIG_CH16		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0040))
#define	CONFIG_CH17		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0044))
#define	CONFIG_CH18		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0048))
#define	CONFIG_CH19		(*(volatile unsigned long int *)(CONFIG_BASE + 0x004C))
#define	CONFIG_CH20		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0050))
#define	CONFIG_CH21		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0054))
#define	CONFIG_CH22		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0058))
#define	CONFIG_CH23		(*(volatile unsigned long int *)(CONFIG_BASE + 0x005C))
#define	CONFIG_CH24		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0060))
#define	CONFIG_CH25		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0064))
#define	CONFIG_CH26		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0068))
#define	CONFIG_CH27		(*(volatile unsigned long int *)(CONFIG_BASE + 0x006C))
#define	CONFIG_CH28		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0070))
#define	CONFIG_CH29		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0074))
#define	CONFIG_CH30		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0078))
#define	CONFIG_CH31		(*(volatile unsigned long int *)(CONFIG_BASE + 0x007C))
#define CONFIG_DONE_REG		(*(volatile unsigned long int *)(CONFIG_BASE + 0x0080))
#define CONFIG_CORE_READY	(*(volatile unsigned long int *)(CONFIG_BASE + 0x0084))
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Test Module Declarations
#define	TEST_STATUS		(*(volatile unsigned long int *)(TEST_BASE + 0x0000))
#define	TEST_FIRQ_TIMER	(*(volatile unsigned long int *)(TEST_BASE + 0x0004))
#define	TEST_IRQ_TIMER	(*(volatile unsigned long int *)(TEST_BASE + 0x0008))
#define	TEST_UART_CONTROL	(*(volatile unsigned long int *)(TEST_BASE + 0x0010))
#define	TEST_UART_STATUS	(*(volatile unsigned long int *)(TEST_BASE + 0x0014))
#define	TEST_UART_TXD	(*(volatile unsigned long int *)(TEST_BASE + 0x0018))
#define	TEST_SIM_CTRL	(*(volatile unsigned long int *)(TEST_BASE + 0x001C))
#define	TEST_MEM_CTRL	(*(volatile unsigned long int *)(TEST_BASE + 0x0020))
#define	TEST_CYCLES		(*(volatile unsigned long int *)(TEST_BASE + 0x0024))
#define	TEST_LED		(*(volatile unsigned long int *)(TEST_BASE + 0x0028))
#define	TEST_PHY_RST	(*(volatile unsigned long int *)(TEST_BASE + 0x002C))
#define	TEST_RANDOM_NUM	(*(volatile unsigned long int *)(TEST_BASE + 0x0100))
#define	TEST_RANDOM_NUM00	(*(volatile unsigned long int *)(TEST_BASE + 0x0100))
#define	TEST_RANDOM_NUM01	(*(volatile unsigned long int *)(TEST_BASE + 0x0104))
#define	TEST_RANDOM_NUM02	(*(volatile unsigned long int *)(TEST_BASE + 0x0108))
#define	TEST_RANDOM_NUM03	(*(volatile unsigned long int *)(TEST_BASE + 0x010C))
#define	TEST_RANDOM_NUM04	(*(volatile unsigned long int *)(TEST_BASE + 0x0110))
#define	TEST_RANDOM_NUM05	(*(volatile unsigned long int *)(TEST_BASE + 0x0114))
#define	TEST_RANDOM_NUM06	(*(volatile unsigned long int *)(TEST_BASE + 0x0118))
#define	TEST_RANDOM_NUM07	(*(volatile unsigned long int *)(TEST_BASE + 0x011C))
#define	TEST_RANDOM_NUM08	(*(volatile unsigned long int *)(TEST_BASE + 0x0120))
#define	TEST_RANDOM_NUM09	(*(volatile unsigned long int *)(TEST_BASE + 0x0124))
#define	TEST_RANDOM_NUM10	(*(volatile unsigned long int *)(TEST_BASE + 0x0128))
#define	TEST_RANDOM_NUM11	(*(volatile unsigned long int *)(TEST_BASE + 0x012C))
#define	TEST_RANDOM_NUM12	(*(volatile unsigned long int *)(TEST_BASE + 0x0130))
#define	TEST_RANDOM_NUM13	(*(volatile unsigned long int *)(TEST_BASE + 0x0134))
#define	TEST_RANDOM_NUM14	(*(volatile unsigned long int *)(TEST_BASE + 0x0138))
#define	TEST_RANDOM_NUM15	(*(volatile unsigned long int *)(TEST_BASE + 0x013C))
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Interrupt Controller Declarations
#define	IC_IRQ0_STATUS		(*(volatile unsigned long int *)(IC_BASE + 0x0000))
#define	IC_IRQ0_RAWSTAT		(*(volatile unsigned long int *)(IC_BASE + 0x0004))
#define	IC_IRQ0_ENABLESET		(*(volatile unsigned long int *)(IC_BASE + 0x0008))
#define	IC_IRQ0_ENABLECLR		(*(volatile unsigned long int *)(IC_BASE + 0x000C))
#define	IC_INT_SOFTSET_0		(*(volatile unsigned long int *)(IC_BASE + 0x0010))
#define	IC_INT_SOFTCLEAR_0	(*(volatile unsigned long int *)(IC_BASE + 0x0014))
#define	IC_FIRQ0_STATUS		(*(volatile unsigned long int *)(IC_BASE + 0x0020))
#define	IC_FIRQ0_RAWSTAT		(*(volatile unsigned long int *)(IC_BASE + 0x0024))
#define	IC_FIRQ0_ENABLESET	(*(volatile unsigned long int *)(IC_BASE + 0x0028))
#define	IC_FIRQ0_ENABLECLR	(*(volatile unsigned long int *)(IC_BASE + 0x002C))
#define	IC_IRQ1_STATUS		(*(volatile unsigned long int *)(IC_BASE + 0x0040))
#define	IC_IRQ1_RAWSTAT		(*(volatile unsigned long int *)(IC_BASE + 0x0044))
#define	IC_IRQ1_ENABLESET		(*(volatile unsigned long int *)(IC_BASE + 0x0048))
#define	IC_IRQ1_ENABLECLR		(*(volatile unsigned long int *)(IC_BASE + 0x004C))
#define	IC_INT_SOFTSET_1		(*(volatile unsigned long int *)(IC_BASE + 0x0050))
#define	IC_INT_SOFTCLEAR_1	(*(volatile unsigned long int *)(IC_BASE + 0x0054))
#define	IC_FIRQ1_STATUS		(*(volatile unsigned long int *)(IC_BASE + 0x0060))
#define	IC_FIRQ1_RAWSTAT		(*(volatile unsigned long int *)(IC_BASE + 0x0064))
#define	IC_FIRQ1_ENABLESET	(*(volatile unsigned long int *)(IC_BASE + 0x0068))
#define	IC_FIRQ1_ENABLECLR	(*(volatile unsigned long int *)(IC_BASE + 0x006C))
#define	IC_INT_SOFTSET_2		(*(volatile unsigned long int *)(IC_BASE + 0x0090))
#define	IC_INT_SOFTCLEAR_2	(*(volatile unsigned long int *)(IC_BASE + 0x0094))
#define	IC_INT_SOFTSET_3		(*(volatile unsigned long int *)(IC_BASE + 0x00D0))
#define	IC_INT_SOFTCLEAR_3	(*(volatile unsigned long int *)(IC_BASE + 0x00D4))
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Timer Module Declarations
#define	TIMER0_LOAD			(*(volatile unsigned long int *)(TIMER_BASE + 0x0000))
#define	TIMER0_VALUE		(*(volatile unsigned long int *)(TIMER_BASE + 0x0004))
#define	TIMER0_CTRL			(*(volatile unsigned long int *)(TIMER_BASE + 0x0008))
#define	TIMER0_CLR			(*(volatile unsigned long int *)(TIMER_BASE + 0x000C))
#define	TIMER1_LOAD			(*(volatile unsigned long int *)(TIMER_BASE + 0x0100))
#define	TIMER1_VALUE		(*(volatile unsigned long int *)(TIMER_BASE + 0x0104))
#define	TIMER1_CTRL			(*(volatile unsigned long int *)(TIMER_BASE + 0x0108))
#define	TIMER1_CLR			(*(volatile unsigned long int *)(TIMER_BASE + 0x010C))
#define	TIMER2_LOAD			(*(volatile unsigned long int *)(TIMER_BASE + 0x0200))
#define	TIMER2_VALUE		(*(volatile unsigned long int *)(TIMER_BASE + 0x0204))
#define	TIMER2_CTRL			(*(volatile unsigned long int *)(TIMER_BASE + 0x0208))
#define	TIMER2_CLR			(*(volatile unsigned long int *)(TIMER_BASE + 0x020C))
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// UART Module Declarations
#define	UART_PID0			(*(volatile unsigned long int *)(UART_BASE + 0x0FE0))
#define	UART_PID1			(*(volatile unsigned long int *)(UART_BASE + 0x0FE4))
#define	UART_PID2			(*(volatile unsigned long int *)(UART_BASE + 0x0FE8))
#define	UART_PID3			(*(volatile unsigned long int *)(UART_BASE + 0x0FEC))
#define	UART_CID0			(*(volatile unsigned long int *)(UART_BASE + 0x0FF0))
#define	UART_CID1			(*(volatile unsigned long int *)(UART_BASE + 0x0FF4))
#define	UART_CID2			(*(volatile unsigned long int *)(UART_BASE + 0x0FF8))
#define	UART_CID3			(*(volatile unsigned long int *)(UART_BASE + 0x0FFC))
#define	UART_DR			(*(volatile unsigned long int *)(UART_BASE + 0x0000))
#define	UART_RSR			(*(volatile unsigned long int *)(UART_BASE + 0x0004))
#define	UART_LCRH			(*(volatile unsigned long int *)(UART_BASE + 0x0008))
#define	UART_LCRM			(*(volatile unsigned long int *)(UART_BASE + 0x000C))
#define	UART_LCRL			(*(volatile unsigned long int *)(UART_BASE + 0x0010))
#define	UART_CR			(*(volatile unsigned long int *)(UART_BASE + 0x0014))
#define	UART_FR			(*(volatile unsigned long int *)(UART_BASE + 0x0018))
#define	UART_IIR			(*(volatile unsigned long int *)(UART_BASE + 0x001C))
#define	UART_ICR			(*(volatile unsigned long int *)(UART_BASE + 0x001C))
//----------------------------------------------------------------------------------


