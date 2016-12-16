 
/* 

           This file will convert ADPCM to uLaw PCM data using formulas 
	   in ANSI spec. T1.301-1987.
*/

#include "stdio.h"
#include <stdlib.h>
#include "encode.h"
#define PRNT(x) if(tst)printf("x = %d\t",x)

long decode();

FILE		*fclose(),*fopen(),*fpout,*fpin;

long  yl;
long dq1,dq2,dq3,dq4,dq5,dq6;
long b1,b2,b3,b4,b5,b6,a1,a2;
long pk1,pk2,sr1,sr2;
long ap,yu;
long dms,dml;
long y,se,al,sez;
long td,tdr,sigpk,tdp;
long a1r,a2r,b1r,b2r,b3r,b4r,b5r,b6r;

extern struct tbl quan_tbl[];
extern long recon_tbl[];
extern long mult_tbl[];

extern long inv_g711(),g711(),expand(),subta(),subtb();
extern long quan(),adda(),antilog();
extern long filtd(),functw();
extern long  filte();
extern long mix(),functf(),addb(),addc();
extern long floata(),floatb(),fmult();
extern long limc(),limb(),limd(),upa1(),calcbn();
extern long upa1(),upa2(),upb(),xor(),lima(),subtc();
extern long filta(),filtb(),filtc();
extern long compress(),synch();
extern long adapt_pred(),inv_adapt_quan(),adapt_quan();
extern long quan_scl_fact(),adapt_speed(),output_conv();
extern long triga(),trigb(),tone(),trans();

char inbuff[512];
char outbuff[1024];

main(argc,argv)
int argc;
char *argv[];
{
    long I;
    long dlx,dsx,sd;
    long sp,slx,dx,dlnx;
    long pk0,wb1;
    long u1,u2,u3,u4,u5,u6;
    long b1p,b2p,b3p,b4p,b5p,b6p;
    long wb2,wb3,wb4,wb5,wb6,wa2,wa1;
    long dqln,dqs,dql;
    long dq,wi,yut,fi,ax,a2t,a1t;
    long yup,dmsp,dmlp,app,sr0,dq0,a2p,a1p;
    long ylp,sg,tr,apr;
    long sr;
    int i,len,cnt,tst,test;
test = 0;
tst = 0;
if(*argv[1] == 't') tst = 1;
    if ( ( fpin = fopen(argv[1],"r" )) == NULL )
    {
	printf(" can't open file %s \n",argv[1]);
	exit(0);
    }
    if ( ( fpout = fopen(argv[2],"w" )) == NULL )
    {
	printf(" can't open file %s \n",argv[2]);
	exit(0);
    }

/* initialize values */
    I = dms = dml = 0;
    ap = pk1 = pk2 = 0;
    b1 = b2 = b3 = b4 = b5 = b6 = 0;
    a2 = a1 = 0;
    yl = 34816; yu = 544;
    sr1 = sr2 = 32;
    dq1 = dq2 = dq3 = dq4 = dq5 = dq6 = 32;
    td=0;

    while((len = fread(inbuff,sizeof(*inbuff),512,fpin)) > 0)
    {
      for(i = 0; i < len*2; i++)
      {
if(tst)printf("\n################### %d ###############\n",i);
    /* get adpcm sample */
        if(!(i & 1)) I = (inbuff[i/2] & 0xf0) >> 4;
        else I = inbuff[i/2] &0x0f;
	if ( I != 0xf ) test = 1;
	if ( test )
	printf("I = %x\n",I);
/* block a */
    /* step 1 */
	
        wb1 = fmult(b1,dq1,test);
	if(test)
	printf("BX= %x	DQX = %x	WBX = %x\n",b1,dq1,wb1);
        wb2 = fmult(b2,dq2,test);
        wb3 = fmult(b3,dq3,test);
        wb4 = fmult(b4,dq4,test);
        wb5 = fmult(b5,dq5,test);
        wb6 = fmult(b6,dq6,test);
        wa2 = fmult(a2,sr2,test);
        wa1 = fmult(a1,sr1,test);
    /* step 2 */
        accum(wa1,wa2,wb1,wb2,wb3,wb4,wb5,wb6);
    /* step 3 */
        al = lima(ap);		/* limit speed control */
    /* step 4 */
        y = mix(al,yu,yl);	/* form linear combination of fast and slow */
    /* step 5 */		/* inverse adaptive quantizer */
        reconst(I,&dqln,&dqs);	/* reconstruct quantized signal */
    /* step 6 */
        dql = adda(dqln,y);	/* scale log of quantized signal */
    /* step 7 */
        dq = antilog(dql,dqs);	/* convert quantized signal to linear */
	if ( test )
	printf("dq = %x   \n",dq);
    /* step 8 */
	tr = trans(td,yl,dq);
    /* step 9 */
        pk0 = addc(dq,sez,&sigpk);
    /* step 10 */
        a2t = upa2(pk0,pk1,pk2,a1,a2,sigpk);
    /* step 11 */
        a2p = limc(a2t);
    /* step 12 */
	tdp = tone(a2p);
    /* step 13 */
	tdr = trigb(tr,tdp);
    /* step 14 */
        sr = addb(dq,se);
	if ( test )
	printf("dq = %x   se = %x  \n",dq,se);
    /* step 15 */
        sr0 = floatb(sr);
	if ( test )
	printf("sr = %x   sr0 = %x  \n",sr,sr0);
    /* step 16 */
        dq0 = floata(dq);
	if ( test )
	printf("dq0 = %x   \n",dq0);
    /* step 17 */
        u1 = xor(dq1,dq);
        u2 = xor(dq2,dq);
        u3 = xor(dq3,dq);
        u4 = xor(dq4,dq);
        u5 = xor(dq5,dq);
        u6 = xor(dq6,dq);
    /* step 18 */
        b1p = upb(u1,b1,dq);
        b2p = upb(u2,b2,dq);
        b3p = upb(u3,b3,dq);
        b4p = upb(u4,b4,dq);
        b5p = upb(u5,b5,dq);
        b6p = upb(u6,b6,dq);
    /* step 19 */
	b1r = trigb(tr,b1p);
	b2r = trigb(tr,b2p);
	b3r = trigb(tr,b3p);
	b4r = trigb(tr,b4p);
	b5r = trigb(tr,b5p);
	b6r = trigb(tr,b6p);
    /* step 20 */
	a2r = trigb(tr,a2p);
    /* step 21 */
        a1t = upa1(pk0,pk1,a1,sigpk);
    /* step 22 */
        a1p = limd(a1t,a2p);
    /* step 23 */
	a1r = trigb(tr,a1p);
    /* step 24 */
        wi = functw(I);		/* map quantizer into logarithmic version */
    /* step 25 */
        yut = filtd(wi,y);	/* update fast quantizer */
    /* step 26 */
        yup = limb(yut);	/* limit quantizer */
    /* step 27 */
        ylp = filte(yup,yl);	/* update slow quantizer */
    /* step 28 */
        fi = functf(I);		/* map quantizer output into fi function */
    /* step 29 */
        dmsp = filta(fi,dms);	/* update short term average */
    /* step 30 */
        dmlp = filtb(fi,dml);	/* update long term average */
    /* step 31 */
        ax = subtc(dmsp,dmlp,y,tdp); /* compute difference of shrt/lng term*/
    /* step 32 */
        app = filtc(ax,ap);	/* low pass filter */
    /* step 33 */
	apr = triga(tr,app);
    /* step 34 */
	sp = compress(sr);
	sg = g711(sp);
	if ( test )
	printf("sp = %x   sr =  %x   sg = %x  \n",sp,sr,sg);
    /* step 35 */
	slx = expand(sg);
	if ( test )
	printf("slx = %x\n",slx);
    /* step 36 */
	dx = subta(slx,se);
	if ( test ) {
	printf("wb6-1  wa2-1  %04x %04x %04x %04x %04x %04x %04x %04x\n",wb6,wb5,wb4,wb3,wb2,wb1,wa2,wa1);
	printf("dx = %x    se = %x \n",dx,se);
	}
    /* step 37 */
	log(dx,&dlx,&dsx);
	if ( test )
	printf("dlx = %x    dsx = %x\n",dlx,dsx);
    /* step 38 */
	dlnx = subtb(dlx,y);
	if ( test )
	printf("dlnx = %x    y = %x\n",dlnx,y);
    /* step 39 */
	sd = synch(I,sp,dlnx,dsx);
	if ( test )
	printf("sd = %x   I = %x   sp = %x   dlnx = %x   dsx = %x\n",sd,I,sp,dlnx,dsx);
	if ( test )
	printf("\n************************************\n");
/* block c */
   /* now perform the delays, steps 40-63 */
	yu = yup;
	yl = ylp;
	dms = dmsp;
	dml = dmlp;
	ap = apr;
	pk2 = pk1;
	pk1 = pk0;
	sr2 = sr1;
	sr1 = sr0;
	b1 = b1r;
	b2 = b2r;
	b3 = b3r;
	b4 = b4r;
	b5 = b5r;
	b6 = b6r;
	dq6 = dq5;
	dq5 = dq4;
	dq4 = dq3;
	dq3 = dq2;
	dq2 = dq1;
	dq1 = dq0;
	a2 = a2r;
	a1 = a1r;
	td = tdr;


/***********************************************/

        outbuff[i] = sd;
      }
      fwrite(outbuff,sizeof(*outbuff),len*2,fpout);
    }
    fclose(fpin);
    fclose(fpout);
}

