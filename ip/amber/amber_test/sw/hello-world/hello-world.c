#include "stdio.h"
#include "communication.h"

void aprsc_send(char tr, char rate, short int dq);
char aprsc_read(char enc_dec, short int *a2p, short int *se, short int *sr);

unsigned char tdmi_read_data(void);
unsigned char tdmi_read_channel(void);
unsigned int tdmi_read_all(void);

void timer_start(unsigned char timer_number, short int countdown_value);
void timer_stop(unsigned char timer_number);
void timer_clear(unsigned char timer_number);

void enable_interrupt(char int_number);
void disable_interrupt(char int_number);

void tdmo_send(char channel, char data);
unsigned char tdmo_read(unsigned char channel);

char config_read(char channel);

volatile char data[32] = {0x00};

volatile char dummy;

volatile char flag = 0;
volatile char i = 0;
void main (void) 
{
/*	enable_interrupt(8);
	for (i=0; i<32; i++) {
		while (flag == 0);
		flag = 0;
	}
	disable_interrupt(8);
	for (i=0; i<32; i++) {
		printf("\r\n Data = %d \r\n", data[i]);
	}
	printf("                     ");*/
	dummy = config_read(0);
	printf("\r\n channel 0 config = 0x%02x                \r\n", dummy);
	for (i=0; i<32; i++) {
		dummy = 0xC0 + (i<<2);
		tdmo_send(i, dummy);
	}
	for (i=0; i<32; i++) {
		printf("\r\n %02x \r\n", (0xC0 + (i<<2)));
	}
}

void irq_interrupt(void) {
	dummy = TDMI_CHANNEL;
	if (dummy == 0) {
		flag++;
		data[i] = TDMI_DATA;
	}
}

//---------------------------------------------------------------------------------------------
// APRSC Communication Functions

void aprsc_send(char tr, char rate, short int dq) {
	APRSC_TR = tr;
	APRSC_RATE = rate;
	APRSC_DQ = dq;
	APRSC_REQ = 0x01;
}

char aprsc_read(char enc_dec, short int *a2p, short int *se, short int *sr) {
	char ack = APRSC_ACK;
	if (ack) {
		*a2p = APRSC_A2P;
		*se = APRSC_SE;
		if (enc_dec) {
			*sr = APRSC_SR;
		}
	}
	return ack;
}
//---------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------
// TDMI Communication Functions

unsigned char tdmi_read_data(void) {
	return TDMI_DATA;
}

unsigned char tdmi_read_channel(void) {
	return TDMI_CHANNEL;
}

unsigned int tdmi_read_all(void) {
	return TDMI_ALL; 
}
//---------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------
// Timer Communication Functions

void timer_start(unsigned char timer_number, short int countdown_value) {
	switch (timer_number) {
		case 0:
			TIMER0_LOAD = countdown_value;
			TIMER0_CTRL = 0xC0;
			break;
		case 1:
			TIMER1_LOAD = countdown_value;
			TIMER1_CTRL = 0xC0;
			break;
		case 2:
			TIMER2_LOAD = countdown_value;
			TIMER2_CTRL = 0xC0;
			break;
		default:
			TIMER0_LOAD = countdown_value;
			TIMER0_CTRL = 0xC0;
			break;
	}
}

void timer_stop(unsigned char timer_number) {
	switch (timer_number) {
		case 0:
			TIMER0_CTRL = 0x00;
			TIMER0_CLR = 0x00;
			break;
		case 1:
			TIMER1_CTRL = 0x00;
			TIMER1_CLR = 0x00;
			break;
		case 2:
			TIMER2_CTRL = 0x00;
			TIMER2_CLR = 0x00;
			break;
		default:
			TIMER0_CTRL = 0x00;
			TIMER0_CLR = 0x00;
			break;
	}
}

void timer_clear(unsigned char timer_number) {
	switch (timer_number) {
		case 0:
			TIMER0_CLR = 0x00;
			break;
		case 1:
			TIMER1_CLR = 0x00;
			break;
		case 2:
			TIMER2_CLR = 0x00;
			break;
		default:
			TIMER0_CLR = 0x00;
			break;
	}
}
//---------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------
// Interrupt Control Functions

void enable_interrupt(char int_number) {
	IC_IRQ0_ENABLESET = (1 << int_number);
}

void disable_interrupt(char int_number) {
	IC_IRQ0_ENABLECLR = (1 << int_number);
}

//---------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------
// TDMO Communication Functions

void tdmo_send(char channel, char data) {
	switch (channel) {
		case 0:
			TDMO_CH0 = data;
			break;
		case 1:
			TDMO_CH1 = data;
			break;
		case 2:
			TDMO_CH2 = data;
			break;
		case 3:
			TDMO_CH3 = data;
			break;
		case 4:
			TDMO_CH4 = data;
			break;
		case 5:
			TDMO_CH5 = data;
			break;
		case 6:
			TDMO_CH6 = data;
			break;
		case 7:
			TDMO_CH7 = data;
			break;
		case 8:
			TDMO_CH8 = data;
			break;
		case 9:
			TDMO_CH9 = data;
			break;
		case 10:
			TDMO_CH10 = data;
			break;
		case 11:
			TDMO_CH11 = data;
			break;
		case 12:
			TDMO_CH12 = data;
			break;
		case 13:
			TDMO_CH13 = data;
			break;
		case 14:
			TDMO_CH14 = data;
			break;
		case 15:
			TDMO_CH15 = data;
			break;
		case 16:
			TDMO_CH16 = data;
			break;
		case 17:
			TDMO_CH17 = data;
			break;
		case 18:
			TDMO_CH18 = data;
			break;
		case 19:
			TDMO_CH19 = data;
			break;
		case 20:
			TDMO_CH20 = data;
			break;
		case 21:
			TDMO_CH21 = data;
			break;
		case 22:
			TDMO_CH22 = data;
			break;
		case 23:
			TDMO_CH23 = data;
			break;
		case 24:
			TDMO_CH24 = data;
			break;
		case 25:
			TDMO_CH25 = data;
			break;
		case 26:
			TDMO_CH26 = data;
			break;
		case 27:
			TDMO_CH27 = data;
			break;
		case 28:
			TDMO_CH28 = data;
			break;
		case 29:
			TDMO_CH29 = data;
			break;
		case 30:
			TDMO_CH30 = data;
			break;
		case 31:
			TDMO_CH31 = data;
			break;
		default:
			TDMO_CH0 = data;
			break;
	}
}

unsigned char tdmo_read(unsigned char channel) {
	unsigned char data;
	switch (channel) {
		case 0:
			data = TDMO_CH0;
			break;
		case 1:
			data = TDMO_CH1;
			break;
		case 2:
			data = TDMO_CH2;
			break;
		case 3:
			data = TDMO_CH3;
			break;
		case 4:
			data = TDMO_CH4;
			break;
		case 5:
			data = TDMO_CH5;
			break;
		case 6:
			data = TDMO_CH6;
			break;
		case 7:
			data = TDMO_CH7;
			break;
		case 8:
			data = TDMO_CH8;
			break;
		case 9:
			data = TDMO_CH9;
			break;
		case 10:
			data = TDMO_CH10;
			break;
		case 11:
			data = TDMO_CH11;
			break;
		case 12:
			data = TDMO_CH12;
			break;
		case 13:
			data = TDMO_CH13;
			break;
		case 14:
			data = TDMO_CH14;
			break;
		case 15:
			data = TDMO_CH15;
			break;
		case 16:
			data = TDMO_CH16;
			break;
		case 17:
			data = TDMO_CH17;
			break;
		case 18:
			data = TDMO_CH18;
			break;
		case 19:
			data = TDMO_CH19;
			break;
		case 20:
			data = TDMO_CH20;
			break;
		case 21:
			data = TDMO_CH21;
			break;
		case 22:
			data = TDMO_CH22;
			break;
		case 23:
			data = TDMO_CH23;
			break;
		case 24:
			data = TDMO_CH24;
			break;
		case 25:
			data = TDMO_CH25;
			break;
		case 26:
			data = TDMO_CH26;
			break;
		case 27:
			data = TDMO_CH27;
			break;
		case 28:
			data = TDMO_CH28;
			break;
		case 29:
			data = TDMO_CH29;
			break;
		case 30:
			data = TDMO_CH30;
			break;
		case 31:
			data = TDMO_CH31;
			break;
		default:
			data = TDMO_CH0;
			break;
	}
	return data;
}
//---------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------
// Configuration Module Functions

char config_read(char channel) {
	char data = 0;
	switch (channel) {
		case 0:
			data = CONFIG_CH0;
			break;
		case 1:
			data = CONFIG_CH1;
			break;
		case 2:
			data = CONFIG_CH2;
			break;
		case 3:
			data = CONFIG_CH3;
			break;
		case 4:
			data = CONFIG_CH4;
			break;
		case 5:
			data = CONFIG_CH5;
			break;
		case 6:
			data = CONFIG_CH6;
			break;
		case 7:
			data = CONFIG_CH7;
			break;
		case 8:
			data = CONFIG_CH8;
			break;
		case 9:
			data = CONFIG_CH9;
			break;
		case 10:
			data = CONFIG_CH10;
			break;
		case 11:
			data = CONFIG_CH11;
			break;
		case 12:
			data = CONFIG_CH12;
			break;
		case 13:
			data = CONFIG_CH13;
			break;
		case 14:
			data = CONFIG_CH14;
			break;
		case 15:
			data = CONFIG_CH15;
			break;
		case 16:
			data = CONFIG_CH16;
			break;
		case 17:
			data = CONFIG_CH17;
			break;
		case 18:
			data = CONFIG_CH18;
			break;
		case 19:
			data = CONFIG_CH19;
			break;
		case 20:
			data = CONFIG_CH20;
			break;
		case 21:
			data = CONFIG_CH21;
			break;
		case 22:
			data = CONFIG_CH22;
			break;
		case 23:
			data = CONFIG_CH23;
			break;
		case 24:
			data = CONFIG_CH24;
			break;
		case 25:
			data = CONFIG_CH25;
			break;
		case 26:
			data = CONFIG_CH26;
			break;
		case 27:
			data = CONFIG_CH27;
			break;
		case 28:
			data = CONFIG_CH28;
			break;
		case 29:
			data = CONFIG_CH29;
			break;
		case 30:
			data = CONFIG_CH30;
			break;
		case 31:
			data = CONFIG_CH31;
			break;
		default:
			data = CONFIG_CH0;
			break;
	}
	return data;
}
//---------------------------------------------------------------------------------------------

