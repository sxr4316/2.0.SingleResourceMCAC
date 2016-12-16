/*
           This file will convert uLaw PCM data to ADPCM using formulas 
	   in ANSI spec. T1.301-1987.
*/

/*
 *
 *  @(#) enc.c 3.4@(#)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/encode.h"
#include "../include/quan_lut.h"
#include "../include/g711_lut.h"
#include "../include/expand_lut.h"
#include "../include/llog_exp_lut.h"

long encode();
long getSample();

FILE *fpin ;
FILE *fpout ;

char prog_opts[] = "hatdvi:o:r:";
extern int optind;
extern char *optarg;
void usage();

extern void openFiles();
extern void closeFiles();
extern void printVars();
extern void printChannel();

extern void encFlat();

extern struct tbl quanbr32_tbl[];
extern struct tbl quanbr40_tbl[];
extern struct tbl quanbr24_tbl[];
extern struct tbl quanbr16_tbl[];

extern long     reconbr32_tbl[];
extern long     reconbr40_tbl[];
extern long     reconbr24_tbl[];
extern long     reconbr16_tbl[];

extern long multbr40_tbl[];
extern long multbr32_tbl[];
extern long multbr24_tbl[];
extern long multbr16_tbl[];

int main(argc,argv)
int argc;
char *argv[];
{
    fpin = stdin;
    fpout = stdout;
    int init_cnt;
	int inFlag = 0;
	int outFlag = 0;
    int printVectors = 0;
	int i, val;

	char *outfile;
	char *infile;
	char c;
    int debug = 0;
	int tst = 0;
    long br = 16;
    long smdq = 16;
	long aLaw = 0;

	/* get command line options */
	while ((c = getopt(argc, argv, prog_opts)) != EOF)
		switch (c) {
			/* user passed input filename */
			case 'i':
				infile = optarg;
				inFlag++;
				break;
			/* user passed output filename */
			case 'o':
				outfile = optarg;
				outFlag++;
				break;
			/* print test sequence */
			case 't':
				tst = 1;
				break;
			/* print debug messages */
			case 'd':
				debug = 1;
				break;
			/* process a-law pcm */
			case 'a':
				aLaw = 1;
				break;
            /* print variable vectors */
            case 'v':
                printVectors = 1;
                break; 
            /* set bit rate */
            case 'r':
                br = atoi(optarg);
                break;
			/* print usage (help) message */
			case 'h':
			default:
				usage();
				break;
			}

    if ( inFlag ) {
		if (( fpin = fopen(infile,"r" )) == NULL )
    		{
			printf(" can't open file %s \n", infile);
			exit(0);
    		}
	}

    if ( outFlag ) {
		if (( fpout = fopen(outfile,"w" )) == NULL )
    		{
			printf(" can't open file %s \n", outfile);
			exit(0);
    		}
	}

    if (printVectors)
    {
        openFiles();
    }

    struct channel singleChannelObject = {0};
    struct channel* singleChannel = &singleChannelObject;

    /* initialize channel values */
    singleChannel->aLaw = aLaw;
    singleChannel->br = br;
    singleChannel->smdq = smdq;

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

    switch (br) {
        case 40:
            singleChannel->br_2bit = 0;
            break;

        case 32:
            singleChannel->br_2bit = 1;
            break;
            
        case 24:
            singleChannel->br_2bit = 2;
            break;
            
        case 16:
            singleChannel->br_2bit = 3;
            break;    
    }

    /* initialize program values*/
    i = 1;

    /* get pcm sample */
	val = getSample(fpin);
    long lIn = 0;
    long lOut = 0;

    while(!feof(fpin))
    {
		if(tst)
        {
			printf("#################### %d #################\n", i);
        }
		
        lIn = (long)val;
        if (printVectors)
        {
           printinput(lIn);
        }

        encFlat(val, singleChannel, printVectors);

		/* output adpcm */
		fprintf( fpout, "%02x", singleChannel->I );
		if (( i != 0 ) && ((i % FILE_LINE) == 0 ))
			fprintf( fpout, "\n" );
		i++;

        lOut = singleChannel->I;
        if (printVectors)
        {
          printoutput(lOut);
        }

    	/* get pcm sample */
		val = getSample(fpin);

    }

    fclose(fpin);
    fclose(fpout);

    if (printVectors)
    {
        closeFiles();
    }

	exit(0);
}


void encFlat(int val, struct channel* singleChannel, int printVectors)
{
    int i;

        singleChannel->pcm = (long)val & 0xff;
        //singleChannel->ss = g711(singleChannel->pcm, singleChannel->aLaw);
        int aLaw_pcm;
        aLaw_pcm = (singleChannel->aLaw << BITS_PCM) | (singleChannel->pcm);
        singleChannel->ss = g711_lut[aLaw_pcm];
	   
        //singleChannel->sl = expand(singleChannel->ss, singleChannel->aLaw);
        int aLaw_ss;
        aLaw_ss = (singleChannel->aLaw << BITS_SS) | (singleChannel->ss);
        singleChannel->sl = expand_lut[aLaw_ss];
    /* block a */
    /* step 1 */
        singleChannel->wb1 = fmult(singleChannel->b1,singleChannel->dq1);
        singleChannel->wb2 = fmult(singleChannel->b2,singleChannel->dq2);
        singleChannel->wb3 = fmult(singleChannel->b3,singleChannel->dq3);
        singleChannel->wb4 = fmult(singleChannel->b4,singleChannel->dq4);
        singleChannel->wb5 = fmult(singleChannel->b5,singleChannel->dq5);
        singleChannel->wb6 = fmult(singleChannel->b6,singleChannel->dq6);
        singleChannel->wa2 = fmult(singleChannel->a2,singleChannel->sr2);
        singleChannel->wa1 = fmult(singleChannel->a1,singleChannel->sr1);
    /* step 2 */
        accum(singleChannel->wa1,singleChannel->wa2,singleChannel->wb1,singleChannel->wb2,singleChannel->wb3,singleChannel->wb4,singleChannel->wb5,singleChannel->wb6, &(singleChannel->sez), (&singleChannel->se));
   /* step 3 */
        singleChannel->al = (singleChannel->ap >= 256) ? 64 : singleChannel->ap >> 2;
    /* step 4 */ // channel->y = mix(channel->al,channel->yu,channel->yl); 
        long    mix_dif, mix_difs, mix_difm, mix_prodm, mix_prod;

        mix_dif = (singleChannel->yu + 16384 - (singleChannel->yl >> 6)) & 16383;
        mix_difs = mix_dif >> 13;

        mix_difm = mix_difs ? (16384 - mix_dif) & 8191 : mix_dif;
        mix_prodm = (mix_difm * singleChannel->al) >> 6;
        mix_prod = mix_difs ? (16384 - mix_prodm) & 16383 : mix_prodm;
        singleChannel->y = ((singleChannel->yl >> 6) + mix_prod) & 8191;
/* encode block */
    /* step 5 */ //channel->d = subta(channel->sl,channel->se);
        long    subta_sls, subta_sli, subta_ses, subta_sei;

        subta_sls = singleChannel->sl >> 13;
        subta_sli = subta_sls ? 49152 + singleChannel->sl : singleChannel->sl;

        subta_ses = singleChannel->se >> 14;
        subta_sei = subta_ses ? 32768 + singleChannel->se : singleChannel->se;

        singleChannel->d = (subta_sli + 65536 - subta_sei) & 65535;
    /* step 6 */        /* adaptive quantizer */ // llog(channel->d,&(channel->dl),&(channel->ds)); 
            long    llog_dqm, llog_exp, llog_mant;

            singleChannel->ds = ( singleChannel->d >> 15) ;
            llog_dqm =  singleChannel->ds ? (65536 -  singleChannel->d) & 32767 :  singleChannel->d;

            llog_exp = llog_exp_lut[singleChannel->d];

            llog_mant = ((llog_dqm << 7) >> llog_exp) & 127;
            singleChannel->dl = ((llog_exp << 7) + llog_mant);
    /* step 7 */ //channel->dln = subtb(channel->dl,channel->y);
         singleChannel->dln = (singleChannel->dl + 4096 - (singleChannel->y >> 2)) & 4095;
    /* step 8 */ //channel->I = quan(channel->dln,channel->ds,channel->br);
         int dln_ds_br;
         dln_ds_br = (singleChannel->dln << BITS_BR+BITS_DS) | (singleChannel->ds << BITS_BR) | (singleChannel->br_2bit);
         singleChannel->I = quan_lut[dln_ds_br];
/* block b */
    /* step 9 */        /* inverse adaptive quantizer */ //reconst(channel->I,channel->br,&(channel->dqln),&(channel->dqs));
        if(singleChannel->br_2bit==0)
        {
           singleChannel->dqln = reconbr40_tbl[singleChannel->I];
           singleChannel->dqs  = singleChannel->I >> 4;
        }
        else if (singleChannel->br_2bit==1)
        {
           singleChannel->dqln = reconbr32_tbl[singleChannel->I];
           singleChannel->dqs  = singleChannel->I >> 3;
        }
        else if(singleChannel->br_2bit==2)
        {
          singleChannel->dqln = reconbr24_tbl[singleChannel->I];
          singleChannel->dqs  = singleChannel->I >> 2;
        }
        else if(singleChannel->br_2bit==3)
        {
          singleChannel->dqln = reconbr16_tbl[singleChannel->I];
          singleChannel->dqs  = singleChannel->I >> 1;
        }
    /* step 10 */ //channel->dql = adda(channel->dqln,channel->y);
        singleChannel->dql = (singleChannel->dqln + (singleChannel->y >> 2)) & 4095; /* scale log of quantized signal */
    /* step 11 */ //channel->dq = antilog(channel->dql,channel->dqs);
        long   /* antilog_dq,*/ antilog_dex, antilog_dmn, antilog_dqt, antilog_dqmag, antilog_ds;

        antilog_ds = (singleChannel->dql >> 11) ;
        antilog_dex = (singleChannel->dql >> 7) & 15;
        antilog_dmn = singleChannel->dql & 127;
        antilog_dqt = (1 << 7) + antilog_dmn;
        antilog_dqmag = antilog_ds ? 0 : (antilog_dqt << 7) >> ( 14 - antilog_dex);

        singleChannel->dq = ((singleChannel->dqs << 15) + antilog_dqmag);
    
        //singleChannel->dq = antilog_dq;  /* convert quantized signal to linear */
    /* step 12 */ //channel->tr = trans(channel->td,channel->yl,channel->dq);
        long    /*trans_tr,*/ trans_dqmag, trans_ylint, trans_ylfrac, trans_thr1, trans_thr2, trans_dqthr;
        trans_dqmag = singleChannel->dq & 32767;
        trans_ylint = (singleChannel->yl >> 15) ;  
        trans_ylfrac = (singleChannel->yl >> 10) & 31;
        trans_thr1 = (32 + trans_ylfrac) << trans_ylint;

        trans_thr2 = trans_ylint > 9 ? 31 << 10 : trans_thr1;
        

        trans_dqthr = (trans_thr2 + (trans_thr2 >> 1)) >> 1;
        singleChannel->tr = (trans_dqmag > trans_dqthr) && (singleChannel->td == 1) ? 1 : 0;
        //flattened_tr =trans_tr;
    /* step 13 */
        singleChannel->pk0 = addc(singleChannel->dq,singleChannel->sez,&(singleChannel->sigpk));
    /* step 14 */
        singleChannel->a2t = upa2(singleChannel->pk0,singleChannel->pk1,singleChannel->pk2,singleChannel->a1,singleChannel->a2,singleChannel->sigpk);
    /* step 15 */
        singleChannel->a2p = limc(singleChannel->a2t);
    /* step 18 */
        singleChannel->sr = addb(singleChannel->dq,singleChannel->se);
    /* step 19 */
        singleChannel->sr0 = floatb(singleChannel->sr);
    /* step 20 */
        singleChannel->dq0 = floata(singleChannel->dq);
    /* step 21 */
        singleChannel->u1 = xor(singleChannel->dq1,singleChannel->dq);
        singleChannel->u2 = xor(singleChannel->dq2,singleChannel->dq);
        singleChannel->u3 = xor(singleChannel->dq3,singleChannel->dq);
        singleChannel->u4 = xor(singleChannel->dq4,singleChannel->dq);
        singleChannel->u5 = xor(singleChannel->dq5,singleChannel->dq);
        singleChannel->u6 = xor(singleChannel->dq6,singleChannel->dq);
    /* step 22 */
        singleChannel->b1p = upb(singleChannel->u1,singleChannel->b1,singleChannel->dq,singleChannel->br);
        singleChannel->b2p = upb(singleChannel->u2,singleChannel->b2,singleChannel->dq,singleChannel->br);
        singleChannel->b3p = upb(singleChannel->u3,singleChannel->b3,singleChannel->dq,singleChannel->br);
        singleChannel->b4p = upb(singleChannel->u4,singleChannel->b4,singleChannel->dq,singleChannel->br);
        singleChannel->b5p = upb(singleChannel->u5,singleChannel->b5,singleChannel->dq,singleChannel->br);
        singleChannel->b6p = upb(singleChannel->u6,singleChannel->b6,singleChannel->dq,singleChannel->br);
    /* step 23 */
        singleChannel->b1r = trigb(singleChannel->tr,singleChannel->b1p);
        singleChannel->b2r = trigb(singleChannel->tr,singleChannel->b2p);
        singleChannel->b3r = trigb(singleChannel->tr,singleChannel->b3p);
        singleChannel->b4r = trigb(singleChannel->tr,singleChannel->b4p);
        singleChannel->b5r = trigb(singleChannel->tr,singleChannel->b5p);
        singleChannel->b6r = trigb(singleChannel->tr,singleChannel->b6p);
    /* step 24 */
        singleChannel->a2r = trigb(singleChannel->tr,singleChannel->a2p);
    /* step 25 */
        singleChannel->a1t = upa1(singleChannel->pk0,singleChannel->pk1,singleChannel->a1,singleChannel->sigpk);
    /* step 26 */
        singleChannel->a1p = limd(singleChannel->a1t,singleChannel->a2p);
    /* step 27 */
        singleChannel->a1r = trigb(singleChannel->tr,singleChannel->a1p);
    /* step 28 */ //channel->wi = functw(channel->I, channel->br);
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
    /* step 29 */ //channel->yut = filtd(channel->wi,channel->y); 
        long    filtd_dif, filtd_difs, filtd_difsx;

        filtd_dif = ((singleChannel->wi << 5) + 131072 - singleChannel->y) & 131071;
        filtd_difs = filtd_dif >> 16;

        filtd_difsx = filtd_difs ? (filtd_dif >> 5) + 4096 : filtd_dif >> 5;
        singleChannel->yut = (singleChannel->y + filtd_difsx) & 8191;
    /* step 30 */ //  channel->yup = limb(channel->yut); 
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
    /* step 31 */ // channel->ylp = filte(channel->yup,channel->yl); 
        long    filte_dif, filte_difs, filte_difsx;

        filte_dif = ((singleChannel->yup + ((1048576 - singleChannel->yl) >> 6)) & 16383);
        filte_difs = filte_dif >> 13;

        filte_difsx = filte_difs ? filte_dif + 507904 : filte_dif;
        singleChannel->ylp = (singleChannel->yl + filte_difsx) & 524287;
    /* step 32 */ //channel->fi = functf(channel->I, channel->br);  
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
    /* step 33 */ //channel->dmsp = filta(channel->fi,channel->dms); 
        long    filta_dif, filta_difs, filta_difsx;

        filta_dif = ((singleChannel->fi << 9) + 8192 - singleChannel->dms) & 8191;
        filta_difs = filta_dif >> 12;

        filta_difsx = filta_difs ? (filta_dif >> 5) + 3840 : filta_dif >> 5;
        singleChannel->dmsp = (filta_difsx + singleChannel->dms) & 4095;
    /* step 34 */ //channel->dmlp = filtb(channel->fi,channel->dml); 
        long    filtb_dif, filtb_difs, filtb_difsx;

        filtb_dif = ((singleChannel->fi << 11) + 32768 - singleChannel->dml) & 32767;
        filtb_difs = filtb_dif >> 14;

        filtb_difsx = filtb_difs ? (filtb_dif >> 7) + 16128 : filtb_dif >> 7;
        singleChannel->dmlp = (filtb_difsx + singleChannel->dml) & 16383;
    /* step 16 */ // channel->tdp = tone(channel->a2p);
        singleChannel->tdp = 0;
        if ((32768 <= singleChannel->a2p) && (singleChannel->a2p < 53760)) 
            singleChannel->tdp = 1;
    /* step 17 */ //channel->tdr = trigb(channel->tr,channel->tdp);
       singleChannel->tdr = (singleChannel->tr ? 0 : singleChannel->tdp);
    /* step 35 */ //channel->ax = subtc(channel->dmsp,channel->dmlp,channel->y,channel->tdp); 
       long    subtc_dif, subtc_difs, subtc_difm, subtc_dthr;

        subtc_dif = ((singleChannel->dmsp << 2) + 32768 - singleChannel->dmlp) & 32767;
        subtc_difs = subtc_dif >> 14;

        subtc_difm = subtc_difs ? (32768 - subtc_dif) & 16383 : subtc_dif;
        subtc_dthr = singleChannel->dmlp >> 3;
        singleChannel->ax = ((singleChannel->y >= 1536) && (subtc_difm < subtc_dthr)) && (singleChannel->tdp == 0) ? 0 : 1;
    /* step 36 */ //channel->app = filtc(channel->ax,channel->ap);
        long    filtc_dif, filtc_difs, filtc_difsx;

        filtc_dif = ((singleChannel->ax << 9) + 2048 - singleChannel->ap) & 2047;
        filtc_difs = filtc_dif >> 10;

        filtc_difsx = filtc_difs ? (filtc_dif >> 4) + 896 : filtc_dif >> 4;
        singleChannel->app = (filtc_difsx + singleChannel->ap) & 1023;
    /* step 37 */ //channel->apr = triga(channel->tr,channel->app);
        singleChannel->apr = (singleChannel->tr ? 256 : singleChannel->app);

        if (printVectors)
        {
            printVars(singleChannel);
        }

     /* now perform the delays, steps 38-59 */

        singleChannel->b1 = singleChannel->b1r;
        singleChannel->b2 = singleChannel->b2r;
        singleChannel->b3 = singleChannel->b3r;
        singleChannel->b4 = singleChannel->b4r;
        singleChannel->b5 = singleChannel->b5r;
        singleChannel->b6 = singleChannel->b6r;
        singleChannel->sr2 = singleChannel->sr1;
        singleChannel->sr1 = singleChannel->sr0;
        singleChannel->dq6 = singleChannel->dq5;
        singleChannel->dq5 = singleChannel->dq4;
        singleChannel->dq4 = singleChannel->dq3;
        singleChannel->dq3 = singleChannel->dq2;
        singleChannel->dq2 = singleChannel->dq1;
        singleChannel->dq1 = singleChannel->dq0;
        singleChannel->pk2 = singleChannel->pk1;
        singleChannel->pk1 = singleChannel->pk0;
        singleChannel->a2 = singleChannel->a2r;
        singleChannel->a1 = singleChannel->a1r;

        singleChannel->yu = singleChannel->yup;
        singleChannel->yl = singleChannel->ylp;
        singleChannel->dms = singleChannel->dmsp;
        singleChannel->dml = singleChannel->dmlp;
        singleChannel->ap = singleChannel->apr;   
        singleChannel->td = singleChannel->tdr;
}


long
getSample( fpin )
FILE *fpin;
{
    char    buff[3];
    long    val;

    buff[2] = (char)NULL;
    buff[0] = fgetc(fpin);
    if ( !isxdigit(buff[0]))
        buff[0] = fgetc(fpin);
    buff[1] = fgetc(fpin);
    if ( !isxdigit(buff[1]))
        buff[1] = fgetc(fpin);
    sscanf( buff, "%x", &val );

    return( val );
}

atoh(val)
char val;
{
    if(val <= 0x39) return(val & 0x0f);
    else return((val & 0x0f) + 9);
}

/* print usage message */
void usage()
{

	fprintf(stderr,"\n encode PCM to ADPCM, ANSI spec. T1.301-1987");
	fprintf(stderr,"\n usage: enc -h -a -t -d -o <output_file_name> -i <input_file_name> -r <bitrate>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message)");
	fprintf(stderr,"\n \t\t-a process a-law pcm");
	fprintf(stderr,"\n \t\t-t enable sample tracing");
	fprintf(stderr,"\n \t\t-d enable debug messages");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t   (default: standard out)");
	fprintf(stderr,"\n \t\t-i <input_file_name>\t(input file to process)");
	fprintf(stderr,"\n \t\t   (default: standard in)");
    fprintf(stderr,"\n \t\t-r <bitrate>\t(16, 24, 32, or 40)");
    fprintf(stderr,"\n \t\t   (default: 16)");
    fprintf(stderr,"\n \t\t-v enable test vector generation");
    fprintf(stderr,"\n");
	exit(1);
}
