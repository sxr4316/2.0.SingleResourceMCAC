#include "../include/encode.h"
#include "../include/communication.h"

#define NUM_CHANNELS 32

extern void dec();
void initializeChannel(struct channel*);

volatile int INTERRUPT_FLAG = 0;
volatile int TDMI_VAL = 0;

void irq_interrupt(void)
{
    INTERRUPT_FLAG = 1;
    TDMI_VAL = TDMI_DATA;
}

int main(void)
{
    int config = 0;

    struct channel channels[NUM_CHANNELS] = {0};
    struct channel* singleChannel;

    unsigned char n = 0;

    /* initialize channel values */
    for (n=0; n<NUM_CHANNELS; n++)
    {
        singleChannel = &channels[n];
        initializeChannel(singleChannel);
    }

    // make sure config module has been configured
    while (CONFIG_DONE_REG == 0); 	// wait for configuration module to be configured for encoder
    CONFIG_CORE_READY = 1; 			// write to configuration module that the encoder is ready for data
    TDMI_START = 1;					// Enable TDMI
    IC_IRQ0_ENABLESET = (0x100);    // Enable interrupts

    while(1)
    {
        while (INTERRUPT_FLAG == 0);        			// Waits for the interrupt from TDMI
        INTERRUPT_FLAG = 0;                 			// Reset the interrupt flag
        n = TDMI_CHANNEL;                   			// Grab the channel from TDMI
        singleChannel = &channels[n];       			// Set the pointer to the correct channel
        config = config_read(n);            			// Grab the configuration information
        singleChannel->aLaw = config & 0x1; 			// Grab the least significan bit for Law config
        singleChannel->br_2bit = (config & 0x6) >> 1;

        switch(config & 0x6)                // Switch on the next 2 bits for the bitrate select
        {
            case 0x0:
                singleChannel->br = 40;
                break;

            case 0x2:
                singleChannel->br = 32;
                break;

            case 0x4:
                singleChannel->br = 24;
                break;

            case 0x6:
                singleChannel->br = 16;
                break;

            default:
                singleChannel->br = 0;
                break;
        }

        dec(TDMI_VAL, singleChannel, 0);

        tdmo_send(n, (char)singleChannel->I);
     }

	return 0;
}

void initializeChannel(struct channel* singleChannel)
{
    /* initialize channel values */
    singleChannel->aLaw = 0;
    singleChannel->br = 16;
    singleChannel->smdq = 16;

    singleChannel->I = 0;	//added
    singleChannel->dms = 0;
    singleChannel->dml = 0;
    singleChannel->ap = 0;
    singleChannel->pk1 = 0;
    singleChannel->pk2 = 0;
    singleChannel->b1 = 0;
    singleChannel->b2 = 0;
    singleChannel->b3 = 0;
    singleChannel->b4 = 0;
    singleChannel->b5 = 0;
    singleChannel->b6 = 0;
    singleChannel->a2 = 0;
    singleChannel->a1 = 0;
    singleChannel->yl = 34816;
    singleChannel->yu = 544;
    singleChannel->sr1 = 32; 
    singleChannel->sr2 = 32;
    singleChannel->dq1 = 32;
    singleChannel->dq2 = 32;
    singleChannel->dq3 = 32;
    singleChannel->dq4 = 32;
    singleChannel->dq5 = 32;
    singleChannel->dq6 = 32;
    singleChannel->td = 0;
    singleChannel->len = 0;	//added
}