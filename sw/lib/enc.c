#include "../include/encode.h"
#include "../include/communication.h"

extern void accum();
extern long adda();
extern long addb();
extern long addc();
extern long antilog();
extern long expand();
extern long filta();
extern long filtb();
extern long filtc();
extern long filtd();
extern long filte();
extern long floata();
extern long floatb();
extern long fmult();
extern long functf();
extern long functw();
extern long g711();
extern long lima();
extern long limb();
extern long limc();
extern long limd();
extern void llog();
extern long mix();
extern long quan();
extern void reconst();
extern long subta();
extern long subtb();
extern long subtc();
extern long tone();
extern long trans();
extern long triga();
extern long trigb();
extern long upa1();
extern long upa2();
extern long upb();
extern long xor();

volatile int ACK = 0;

void enc(int val, struct channel* channel, int printVectors)
{
        channel->pcm = (long)val & 0xff;
        channel->ss = g711(channel->pcm, channel->aLaw);       /* expand 8 bits to 14/13 bit */
        channel->sl = expand(channel->ss, channel->aLaw);      /* convert into linear */
    /* block a */
    /* step 1 */
#ifndef CO_P
        channel->wb1 = fmult(channel->b1,channel->dq1);
        channel->wb2 = fmult(channel->b2,channel->dq2);
        channel->wb3 = fmult(channel->b3,channel->dq3);
        channel->wb4 = fmult(channel->b4,channel->dq4);
        channel->wb5 = fmult(channel->b5,channel->dq5);
        channel->wb6 = fmult(channel->b6,channel->dq6);
        channel->wa2 = fmult(channel->a2,channel->sr2);
        channel->wa1 = fmult(channel->a1,channel->sr1);
    /* step 2 */
        accum(channel->wa1,channel->wa2,channel->wb1,channel->wb2,channel->wb3,channel->wb4,channel->wb5,channel->wb6, &(channel->sez), &(channel->se));
#else
	APRSC_REQ = 0x01;
#endif
    /* step 3 */
        channel->al = lima(channel->ap);      /* limit speed control */
    /* step 4 */
        channel->y = mix(channel->al,channel->yu,channel->yl);  /* form linear combination of fast and slow */
/* encode block */

#ifdef CO_P
        while (APRSC_ACK == 0);
        APRSC_REQ = 0;
        channel->se = APRSC_SE;
#endif

    /* step 5 */
        channel->d = subta(channel->sl,channel->se);   /* compute difference  */
    /* step 6 */        /* adaptive quantizer */
        llog(channel->d,&(channel->dl),&(channel->ds));        /* convert linear to log domain */
    /* step 7 */
        channel->dln = subtb(channel->dl,channel->y);
    /* step 8 */
        channel->I = quan(channel->dln,channel->ds,channel->br);   /* quantize difference signal */
/* block b */
    /* step 9 */        /* inverse adaptive quantizer */
        reconst(channel->I,channel->br,&(channel->dqln),&(channel->dqs));  /* reconstruct quantized signal */
    /* step 10 */
        channel->dql = adda(channel->dqln,channel->y); /* scale log of quantized signal */
    /* step 11 */
        channel->dq = antilog(channel->dql,channel->dqs);  /* convert quantized signal to linear */
    /* step 12 */
        channel->tr = trans(channel->td,channel->yl,channel->dq);
    /* step 13 */
#ifdef CO_P
        // Write to APRSC dq, tr, br
        APRSC_TR = channel->tr;
        APRSC_RATE = channel->br_2bit;
        APRSC_DQ = channel->dq;
#else
        channel->pk0 = addc(channel->dq,channel->sez,&(channel->sigpk));
    /* step 14 */
        channel->a2t = upa2(channel->pk0,channel->pk1,channel->pk2,channel->a1,channel->a2,channel->sigpk);
    /* step 15 */
        channel->a2p = limc(channel->a2t);
    /* step 18 */
        channel->sr = addb(channel->dq,channel->se);
    /* step 19 */
        channel->sr0 = floatb(channel->sr);
    /* step 20 */
        channel->dq0 = floata(channel->dq);
    /* step 21 */
        channel->u1 = xor(channel->dq1,channel->dq);
        channel->u2 = xor(channel->dq2,channel->dq);
        channel->u3 = xor(channel->dq3,channel->dq);
        channel->u4 = xor(channel->dq4,channel->dq);
        channel->u5 = xor(channel->dq5,channel->dq);
        channel->u6 = xor(channel->dq6,channel->dq);
    /* step 22 */
        channel->b1p = upb(channel->u1,channel->b1,channel->dq,channel->br);
        channel->b2p = upb(channel->u2,channel->b2,channel->dq,channel->br);
        channel->b3p = upb(channel->u3,channel->b3,channel->dq,channel->br);
        channel->b4p = upb(channel->u4,channel->b4,channel->dq,channel->br);
        channel->b5p = upb(channel->u5,channel->b5,channel->dq,channel->br);
        channel->b6p = upb(channel->u6,channel->b6,channel->dq,channel->br);
    /* step 23 */
        channel->b1r = trigb(channel->tr,channel->b1p);
        channel->b2r = trigb(channel->tr,channel->b2p);
        channel->b3r = trigb(channel->tr,channel->b3p);
        channel->b4r = trigb(channel->tr,channel->b4p);
        channel->b5r = trigb(channel->tr,channel->b5p);
        channel->b6r = trigb(channel->tr,channel->b6p);
    /* step 24 */
        channel->a2r = trigb(channel->tr,channel->a2p);
    /* step 25 */
        channel->a1t = upa1(channel->pk0,channel->pk1,channel->a1,channel->sigpk);
    /* step 26 */
        channel->a1p = limd(channel->a1t,channel->a2p);
    /* step 27 */
        channel->a1r = trigb(channel->tr,channel->a1p);
#endif
    /* step 28 */
        channel->wi = functw(channel->I, channel->br);     /* map quantizer into logarithmic version */
    /* step 29 */
        channel->yut = filtd(channel->wi,channel->y);  /* update fast quantizer */
    /* step 30 */
        channel->yup = limb(channel->yut);    /* limit quantizer */
    /* step 31 */
        channel->ylp = filte(channel->yup,channel->yl);    /* update slow quantizer */
    /* step 32 */
        channel->fi = functf(channel->I, channel->br);     /* map quantizer output into fi function */
    /* step 33 */
        channel->dmsp = filta(channel->fi,channel->dms);   /* update short term average */
    /* step 34 */
        channel->dmlp = filtb(channel->fi,channel->dml);   /* update long term average */ 
    /* step 16 */
#ifdef CO_P
        // Read from APRSC a2p
        channel->a2p = APRSC_A2P;
#endif
        channel->tdp = tone(channel->a2p);
    /* step 17 */
        channel->tdr = trigb(channel->tr,channel->tdp);
    /* step 35 */     
        channel->ax = subtc(channel->dmsp,channel->dmlp,channel->y,channel->tdp); /* compute difference of shrt/lng term*/
    /* step 36 */
        channel->app = filtc(channel->ax,channel->ap); /* low pass filter */
    /* step 37 */
        channel->apr = triga(channel->tr,channel->app);

#ifdef MODEL
        if (printVectors)
        {
            printVars(channel);
        }
#endif

     /* now perform the delays, steps 38-59 */

#ifndef CO_P
        channel->b1 = channel->b1r;
        channel->b2 = channel->b2r;
        channel->b3 = channel->b3r;
        channel->b4 = channel->b4r;
        channel->b5 = channel->b5r;
        channel->b6 = channel->b6r;
        channel->sr2 = channel->sr1;
        channel->sr1 = channel->sr0;
        channel->dq6 = channel->dq5;
        channel->dq5 = channel->dq4;
        channel->dq4 = channel->dq3;
        channel->dq3 = channel->dq2;
        channel->dq2 = channel->dq1;
        channel->dq1 = channel->dq0;
        channel->pk2 = channel->pk1;
        channel->pk1 = channel->pk0;
        channel->a2 = channel->a2r;
        channel->a1 = channel->a1r;
#endif
        channel->yu = channel->yup;
        channel->yl = channel->ylp;
        channel->dms = channel->dmsp;
        channel->dml = channel->dmlp;
        channel->ap = channel->apr;   
        channel->td = channel->tdr;
}
