#include "../include/encode.h"
#include "../include/communication.h"

extern void dec();

volatile int INTERRUPT_FLAG = 0;
volatile int TDMI_VAL = 0;

void irq_interrupt(void)
{
    INTERRUPT_FLAG = 1;
    TDMI_VAL = TDMI_DATA;   // Read data from TDMI
}

int main(void)
{
    int config = 0;

    struct channel singleChannelObject = {0};
    struct channel* singleChannel = &singleChannelObject;

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

    IC_IRQ0_ENABLESET = (0x100);                // Enable interrupts
    while(1)
    {
        while (INTERRUPT_FLAG == 0);            // Waits for the interrupt from TDMI
        INTERRUPT_FLAG = 0;                     // Reset the interrupt flag
        config = CONFIG_CH0;                    // Grab the configuration information
        singleChannel->aLaw = config & 0x1;     // Grab the least significan bit for Law config

        switch(config & 0x6)                    // Switch on the next 2 bits for the bitrate select
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
        TDMO_CH0 = (char)singleChannel->sr;
    }

    return 0;
}
