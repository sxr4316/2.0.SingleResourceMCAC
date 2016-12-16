#include "../include/encode.h"
#include "../include/communication.h"
#include "../include/compress_lut.h"
#include "../include/expand_lut.h"
#include "../include/llog_exp_lut.h"
#include "../include/g711_lut.h"
#include "../include/synch_quan_lut.h"

#define NUM_CHANNELS 32

void dec(int val, volatile unsigned char ch, struct channel* singleChannel);

volatile int INTERRUPT_FLAG = 0;
volatile int TDMI_VAL = 0;
volatile unsigned char n = 0;

extern long     reconbr32_tbl[];
extern long     reconbr40_tbl[];
extern long     reconbr24_tbl[];
extern long     reconbr16_tbl[];

extern long multbr40_tbl[];
extern long multbr32_tbl[];
extern long multbr24_tbl[];
extern long multbr16_tbl[];

void irq_interrupt(void)
{
	INTERRUPT_FLAG = 1;
	TDMI_VAL = TDMI_DATA;
	n = TDMI_CHANNEL;
}

int main(void)
{
	int val = 0;
	int config = 0;

	struct channel channels[NUM_CHANNELS] = {0};
	struct channel* singleChannel;

	/* initialize channel values */
	for (n=0; n<NUM_CHANNELS; n++)
	{
		singleChannel = &channels[n];
		singleChannel->yl = 34816;
		singleChannel->yu = 544;
	}

	// make sure config module has been configured
	while (CONFIG_DONE_REG == 0); // wait for configuration module to be configured for encoder
	CONFIG_CORE_READY = 1; // write to configuration module that the decoder is ready for data

	TDMI_START = 1;
	IC_IRQ0_ENABLESET = (0x100);            // Enable interrupts
	while(1)
	{
		while (INTERRUPT_FLAG == 0);        // Waits for the interrupt from TDMI
		INTERRUPT_FLAG = 0;                 // Reset the interrupt flag
		singleChannel = &channels[n];       // Set the pointer to the correct channel
		config = config_read(n);            // Grab the configuration information
		singleChannel->aLaw = config & 0x1; // Grab the least significan bit for Law config
		singleChannel->br_2bit = (config & 0x06) >> 1;
		dec(TDMI_VAL, n, singleChannel);
	}
	return 0;
}

void dec(int val, unsigned char ch, struct channel* singleChannel) {

	APRSC_REQ = 0x01;

	//-----------------------------------------------------------------------------------------------------------------
	/* Quantizer Scale Factor Adaptation (mix) */
	singleChannel->al = (singleChannel->ap >= 256) ? 64 : singleChannel->ap >> 2;
	long    mix_dif, mix_difs, mix_difm, mix_prodm, mix_prod;
	mix_dif = (singleChannel->yu + 16384 - (singleChannel->yl >> 6)) & 16383;
	mix_difs = mix_dif >> 13;
	mix_difm = mix_difs ? (16384 - mix_dif) & 8191 : mix_dif;
	mix_prodm = (mix_difm * singleChannel->al) >> 6;
	mix_prod = mix_difs ? (16384 - mix_prodm) & 16383 : mix_prodm;
	singleChannel->y = ((singleChannel->yl >> 6) + mix_prod) & 8191;
	//-----------------------------------------------------------------------------------------------------------------


	//-----------------------------------------------------------------------------------------------------------------
	/* Inverse Adaptive Quantization */
	switch(singleChannel->br_2bit) {
	case 0:
		singleChannel->I = (long)val & 0x1f;
		singleChannel->dqln = reconbr40_tbl[singleChannel->I];
		singleChannel->dqs  = singleChannel->I >> 4;
		break;
	case 1:
		singleChannel->I = (long)val & 0x0f;
		singleChannel->dqln = reconbr32_tbl[singleChannel->I];
		singleChannel->dqs  = singleChannel->I >> 3;
		break;
	case 2:
		singleChannel->I = (long)val & 0x7;
		singleChannel->dqln = reconbr24_tbl[singleChannel->I];
		singleChannel->dqs  = singleChannel->I >> 2;
		break;
	case 3:
		singleChannel-> I = (long)val & 0x3;
		singleChannel->dqln = reconbr16_tbl[singleChannel->I];
		singleChannel->dqs  = singleChannel->I >> 1;
		break;
	default:
		break;
	}
	singleChannel->dql = (singleChannel->dqln + (singleChannel->y >> 2)) & 4095; /* scale log of quantized signal */

	long antilog_dex, antilog_dmn, antilog_dqt, antilog_dqmag, antilog_ds;
	antilog_ds = (singleChannel->dql >> 11) ;
	antilog_dex = (singleChannel->dql >> 7) & 15;
	antilog_dmn = singleChannel->dql & 127;
	antilog_dqt = (1 << 7) + antilog_dmn;
	antilog_dqmag = antilog_ds ? 0 : (antilog_dqt << 7) >> ( 14 - antilog_dex);
	singleChannel->dq = ((singleChannel->dqs << 15) + antilog_dqmag);	
	//-----------------------------------------------------------------------------------------------------------------


	//-----------------------------------------------------------------------------------------------------------------
	/* Transition Detector */

	/* TRANS */
	long trans_dqmag, trans_ylint, trans_ylfrac, trans_thr1, trans_thr2, trans_dqthr;
	trans_dqmag = singleChannel->dq & 32767;
	trans_ylint = (singleChannel->yl >> 15) ;  
	trans_ylfrac = (singleChannel->yl >> 10) & 31;
	trans_thr1 = (32 + trans_ylfrac) << trans_ylint;
	trans_thr2 = trans_ylint > 9 ? 31 << 10 : trans_thr1;
	trans_dqthr = (trans_thr2 + (trans_thr2 >> 1)) >> 1;
	singleChannel->tr = (trans_dqmag > trans_dqthr) && (singleChannel->td == 1) ? 1 : 0;
	//-----------------------------------------------------------------------------------------------------------------

	
	//-----------------------------------------------------------------------------------------------------------------
	/* Adaptive Prediction Result from APRSC Co-processor */
	while (APRSC_ACK == 0);
	APRSC_REQ = 0;
	singleChannel->se = APRSC_SE;
	//-----------------------------------------------------------------------------------------------------------------


	//-----------------------------------------------------------------------------------------------------------------
	// Send Data to APRSC Co-processor for next prediction
	APRSC_TR = singleChannel->tr;
	APRSC_RATE = singleChannel->br_2bit;
	APRSC_DQ = singleChannel->dq;

	//-----------------------------------------------------------------------------------------------------------------


	//-----------------------------------------------------------------------------------------------------------------
	// Quantizer Scale Factor Adaptation

	/* FUNCTW */
	long functw_is, functw_im;
	if(singleChannel->br_2bit==0)
	{
		functw_is = singleChannel->I >> 4;
		functw_im = functw_is ? (31 - singleChannel->I) & 15 : singleChannel->I & 15;
		singleChannel->wi = multbr40_tbl[functw_im];
	}
	else if(singleChannel->br_2bit==1)
	{
		functw_is = singleChannel->I >> 3;
		functw_im = functw_is ? (15 - singleChannel->I) & 7 : singleChannel->I & 7;
		singleChannel->wi = multbr32_tbl[functw_im];
	}
	else if(singleChannel->br_2bit==2)
	{
		functw_is = singleChannel->I >> 2;
		functw_im = functw_is ? (7 - singleChannel->I) & 3 : singleChannel->I & 3;
		singleChannel->wi = multbr24_tbl[functw_im];
	}
	else if(singleChannel->br_2bit==3)
	{
		functw_is = singleChannel->I >> 1;
		functw_im = functw_is ? (3 - singleChannel->I) & 1 : singleChannel->I & 1;
		singleChannel->wi = multbr16_tbl[functw_im];
	}

	/* FILTD */
	long    filtd_dif, filtd_difs, filtd_difsx;
	filtd_dif = ((singleChannel->wi << 5) + 131072 - singleChannel->y) & 131071;
	filtd_difs = filtd_dif >> 16;
	filtd_difsx = filtd_difs ? (filtd_dif >> 5) + 4096 : filtd_dif >> 5;
	singleChannel->yut = (singleChannel->y + filtd_difsx) & 8191;

	/* LIMB */
	long    limb_geul, limb_gell;
	limb_geul = (((singleChannel->yut + 11264) & 16383) >> 13) ;
	limb_gell = ((singleChannel->yut + 15840) & 16383) >> 13;
	if (limb_gell == 1) {
		singleChannel->yup = 544;
	}
	else if (limb_geul == 0) {
		singleChannel->yup = 5120;
	}
	else {
		singleChannel->yup = singleChannel->yut;
	}

	/* FILTE */ 
	long    filte_dif, filte_difs, filte_difsx;
	filte_dif = ((singleChannel->yup + ((1048576 - singleChannel->yl) >> 6)) & 16383);
	filte_difs = filte_dif >> 13;
	filte_difsx = filte_difs ? filte_dif + 507904 : filte_dif;
	singleChannel->ylp = (singleChannel->yl + filte_difsx) & 524287;
	//-----------------------------------------------------------------------------------------------------------------


	//-----------------------------------------------------------------------------------------------------------------
	// Tone Detector

	singleChannel->sr = APRSC_SR;
	singleChannel->a2p = APRSC_A2P;

	/* TONE */
	singleChannel->tdp = 0;
	if ((32768 <= singleChannel->a2p) && (singleChannel->a2p < 53760)) {
		singleChannel->tdp = 1;
	}

	/* TRIGB */
	singleChannel->tdr = (singleChannel->tr ? 0 : singleChannel->tdp);
	//-----------------------------------------------------------------------------------------------------------------

	
	//-----------------------------------------------------------------------------------------------------------------
	// Adaptation Speed Control

	/* FUNCTF */
	long    functf_is, functf_im;
	if(singleChannel->br_2bit==0)
	{
		functf_is = singleChannel->I >> 4;
		functf_im = functf_is ? ((31 - singleChannel->I) & 15) : (singleChannel->I & 15);

		switch ((int)functf_im) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				singleChannel->fi = 0;
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				singleChannel->fi = 0x1;
				break;
			case 10:
				singleChannel->fi = 0x2;
				break;
			case 11:
				singleChannel->fi = 0x3;
				break;
			case 12:
				singleChannel->fi = 0x4;
				break;
			case 13:
				singleChannel->fi = 0x5;
				break;
			case 14:
			case 15:
				singleChannel->fi = 0x6;
				break;
			default:
				break;
		}
	}
	else if(singleChannel->br_2bit==1)
	{
		functf_is = singleChannel->I >> 3;
		functf_im = functf_is ? ((15 - singleChannel->I) & 7) : (singleChannel->I & 7);
		switch ((int)functf_im) {
			case 0:
			case 1:
			case 2:
				singleChannel->fi = 0;	
				break;
			case 3:
			case 4:
			case 5:
				singleChannel->fi = 0x1;
				break;
			case 6:
				singleChannel->fi = 0x3;
				break;
			case 7:
				singleChannel->fi = 0x7;
				break;
			default:
				break;
		}
	}
	else if(singleChannel->br_2bit==2)
	{
		functf_is = singleChannel->I >> 2;
		functf_im = functf_is ? ((7 - singleChannel->I) & 3) : (singleChannel->I & 3);
		switch ((int)functf_im) {
		case 0:
			singleChannel->fi = 0;
			break;
		case 1:
			singleChannel->fi = 0x1;
			break;
		case 2:
			singleChannel->fi = 0x2;
			break;
		case 3:
			singleChannel->fi = 0x7;
			break;
		default:
			break;
		}
	}
	else if(singleChannel->br_2bit==3)
	{
		functf_is = singleChannel->I >> 1;
		functf_im = functf_is ? ((3 - singleChannel->I) & 1) : (singleChannel->I & 1);

		switch ((int)functf_im) {
			case 0:
				singleChannel->fi = 0;
				break;
			case 1:
				singleChannel->fi = 0x7;
				break;
			default:
				break;
		}
	}

	/* FILTA */
	long    filta_dif, filta_difs, filta_difsx;
	filta_dif = ((singleChannel->fi << 9) + 8192 - singleChannel->dms) & 8191;
	filta_difs = filta_dif >> 12;
	filta_difsx = filta_difs ? (filta_dif >> 5) + 3840 : filta_dif >> 5;
	singleChannel->dmsp = (filta_difsx + singleChannel->dms) & 4095;

	/* FILTB */
	long    filtb_dif, filtb_difs, filtb_difsx;
	filtb_dif = ((singleChannel->fi << 11) + 32768 - singleChannel->dml) & 32767;
	filtb_difs = filtb_dif >> 14;
	filtb_difsx = filtb_difs ? (filtb_dif >> 7) + 16128 : filtb_dif >> 7;
	singleChannel->dmlp = (filtb_difsx + singleChannel->dml) & 16383;
	
	/* SUBTC */
	long    subtc_dif, subtc_difs, subtc_difm, subtc_dthr;
	subtc_dif = ((singleChannel->dmsp << 2) + 32768 - singleChannel->dmlp) & 32767;
	subtc_difs = subtc_dif >> 14;
	subtc_difm = subtc_difs ? (32768 - subtc_dif) & 16383 : subtc_dif;
	subtc_dthr = singleChannel->dmlp >> 3;
	singleChannel->ax = ((singleChannel->y >= 1536) && (subtc_difm < subtc_dthr)) && (singleChannel->tdp == 0) ? 0 : 1;

	/* FILTC */
	long    filtc_dif, filtc_difs, filtc_difsx;
	filtc_dif = ((singleChannel->ax << 9) + 2048 - singleChannel->ap) & 2047;
	filtc_difs = filtc_dif >> 10;
	filtc_difsx = filtc_difs ? (filtc_dif >> 4) + 896 : filtc_dif >> 4;
	singleChannel->app = (filtc_difsx + singleChannel->ap) & 1023;

	/* TRIGA */
	singleChannel->apr = (singleChannel->tr ? 256 : singleChannel->app);
	//-----------------------------------------------------------------------------------------------------------------
 

	//-----------------------------------------------------------------------------------------------------------------
	// Output PCM Formatting

	/* COMPRESS */
	//singleChannel->sp = compress(singleChannel->sr,singleChannel->aLaw, singleChannel->br); 
	long aLaw_br_sri;
    aLaw_br_sri = (singleChannel->aLaw << BITS_SRI+BITS_BR) | (singleChannel->br_2bit << BITS_SRI) | (singleChannel->sr);
    
    if (aLaw_br_sri >= 0x1CA06 && aLaw_br_sri <= 0x1CA17){
    	singleChannel->sp = 0;
    } else {
    	singleChannel->sp = compress_lut[aLaw_br_sri];
	}

	/* EXPAND */
	int aLaw_pcm;
	aLaw_pcm = (singleChannel->aLaw << BITS_PCM) | (singleChannel->sp);
	singleChannel->sg = g711_lut[aLaw_pcm];
	int aLaw_sg;
	aLaw_sg = (singleChannel->aLaw << BITS_SS) | (singleChannel->sg);
	singleChannel->slx = expand_lut[aLaw_sg];
	
	/* SUBTA */
	long    subta_sls, subta_sli, subta_ses, subta_sei;
	subta_sls = singleChannel->slx >> 13;
	subta_sli = subta_sls ? 49152 + singleChannel->slx : singleChannel->slx;
	subta_ses = singleChannel->se >> 14;
	subta_sei = subta_ses ? 32768 + singleChannel->se : singleChannel->se;
	singleChannel->dx = (subta_sli + 65536 - subta_sei) & 65535;

	/* LOG */
	long    llog_dqm, llog_exp, llog_mant;
	singleChannel->dsx = ( singleChannel->dx >> 15) ;
	llog_dqm =  singleChannel->dsx ? (65536 -  singleChannel->dx) & 32767 :  singleChannel->dx;
	llog_exp = llog_exp_lut[singleChannel->dx];
	llog_mant = ((llog_dqm << 7) >> llog_exp) & 127;
	singleChannel->dlx = ((llog_exp << 7) + llog_mant);	

	/* SUBTB */
	singleChannel->dlnx = (singleChannel->dlx + 4096 - (singleChannel->y >> 2)) & 4095;

	/* SYNCH */
	//singleChannel->sd = synch(singleChannel->I,singleChannel->sp, singleChannel->dlnx, singleChannel->dsx, singleChannel->aLaw, singleChannel->br);
	long	synch_is, synch_im, synch_id, synch_sd;
	long dlnx_dsx_br;
	dlnx_dsx_br = (singleChannel->dlnx << BITS_BR+BITS_DSX) | (singleChannel->dsx << BITS_BR) | (singleChannel->br_2bit);
	synch_id = synch_quan_lut[dlnx_dsx_br];

	switch(singleChannel->br_2bit) {
		case 0:
			synch_is = singleChannel->I >> 4;
			synch_im = synch_is ? (singleChannel->I & 15) : (singleChannel->I + 16);
			break;
		case 1:
			synch_is = singleChannel->I >> 3;
			synch_im = synch_is ? (singleChannel->I & 7) : (singleChannel->I + 8);
			break;
		case 2:
			synch_is = singleChannel->I >> 2;
			synch_im = synch_is ? (singleChannel->I & 3) : (singleChannel->I + 4);
			break;
		case 3:
			synch_is = singleChannel->I >> 1;
			synch_im = synch_is ? (singleChannel->I & 1) : (singleChannel->I + 2);
			break;
	}
	
	if ( !singleChannel->aLaw ) {
		/* u-law pcm */
		if (synch_id == synch_im) 
			synch_sd = singleChannel->sp;
		else
		{
			switch ((int)singleChannel->sp) {
			case 0x80:
				if (synch_id > synch_im) 
					synch_sd = singleChannel->sp + 1;
				else 
					synch_sd = singleChannel->sp;
				break;
			case 0xff:
				if (synch_id > synch_im) 
					synch_sd = 0x7e;
				else 
					synch_sd = singleChannel->sp - 1;
				break;
			case 0x00:
				if (synch_id > synch_im) 
					synch_sd = singleChannel->sp;
				else 
					synch_sd = singleChannel->sp + 1;
				break;
			case 0x7f:
				if (synch_id > synch_im) 
					synch_sd = singleChannel->sp - 1;
				else 
					synch_sd = 0xfe;
				break;
			default:
				if (singleChannel->sp & 0x80) {
					if (synch_id > synch_im) 
						synch_sd = singleChannel->sp + 1;
					else 
						synch_sd = singleChannel->sp - 1;
				} else
				 {
					if (synch_id > synch_im) 
						synch_sd = singleChannel->sp - 1;
					else 
						synch_sd = singleChannel->sp + 1;
				}
				break;
			}
		}
	} else {
		/* a-law pcm */
		singleChannel->sp ^= 0x55 ;
		if (synch_id == synch_im) {
			synch_sd = singleChannel->sp;
		} else {
			switch (singleChannel->sp) {
			case 0xff:	/* + max */
				if (synch_id > synch_im) 
					synch_sd = 0xFE;
				else 
					synch_sd = singleChannel->sp;
				break;
			case 0x80:	/* + 0 */
				if (synch_id > synch_im) 
					synch_sd = 0x00;
				else 
				//	synch_sd = singleChannel->sp - 1;
					synch_sd = 0x81;
				break;
			case 0x00:	/* - 0 */
				if (synch_id > synch_im)
					synch_sd = 0x01;
				else
					synch_sd = 0x80;
				break;
			case 0x7F:	/* - max */
				if (synch_id > synch_im) 
					synch_sd = singleChannel->sp;
				else 
					synch_sd =0x7E ;
				break;
			default:
				if (singleChannel->sp & 0x80) {
					if (synch_id > synch_im) 
						synch_sd = singleChannel->sp - 1;
					else 
						synch_sd = singleChannel->sp + 1;
				} else {
					if (synch_id > synch_im) 
						synch_sd = singleChannel->sp + 1;
					else 
						synch_sd = singleChannel->sp - 1;
				}
				break;
			}
		}
		synch_sd ^= 0x55 ;
	}
	singleChannel->sd = synch_sd & 0xFF;

	//-----------------------------------------------------------------------------------------------------------------

	char data = (char)singleChannel->sd;
	switch (ch) {
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

	/* now perform the delays, steps 40-63 */
	singleChannel->yu = singleChannel->yup;
	singleChannel->yl = singleChannel->ylp;
	singleChannel->dms = singleChannel->dmsp;
	singleChannel->dml = singleChannel->dmlp;
	singleChannel->ap = singleChannel->apr;   
	singleChannel->td = singleChannel->tdr;
}
