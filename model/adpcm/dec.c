/*

           This file will convert ADPCM to uLaw PCM data using formulas 
	   in ANSI spec. T1.301-1987.
*/

/*
 *
 *	@(#) dec.c 3.4@(#)
 *
 */

#define T
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "encode.h"

long decode();

FILE *fpin ;
FILE *fpout ;

long yl;
long dq1,dq2,dq3,dq4,dq5,dq6;
long b1,b2,b3,b4,b5,b6,a1,a2;
long pk1,pk2,sr1,sr2;
long ap,yu;
long dms,dml;
long y,se,al,sez,sr,td;
long tdr,sigpk,tdp;
long a1r,a2r,b1r,b2r,b3r,b4r,b5r,b6r;
long pcm, sd;

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
extern long upa1(),upa2(),upb(),xor(),lima(),subtc(),llog();
extern long filta(),filtb(),filtc();
extern long compress(),synch();
extern long adapt_pred(),inv_adapt_quan(),adapt_quan();
extern long quan_scl_fact(),adapt_speed(),output_conv();
extern long triga(),trigb(),tone(),trans();
extern long getSample();
extern void dumpState();
extern void dumpPCM();

char prog_opts[] = "hatdi:o:";
extern int optind;
extern char *optarg;
void usage();

int debug = 0 ;

main(argc,argv)
int argc;
char *argv[];
{
    long I;
    long dlx,dsx;
    long sp,slx,dx,dlnx;
    long pk0,wb1;
    long u1,u2,u3,u4,u5,u6;
    long b1p,b2p,b3p,b4p,b5p,b6p;
    long wb2,wb3,wb4,wb5,wb6,wa2,wa1;
    long dqln,dqs,dql;
    long dq,wi,yut,fi,ax,a2t,a1t;
    long yup,dmsp,dmlp,app,sr0,dq0,a2p,a1p;
    long ylp,sg,tr,apr;
    fpin = stdin;
    fpout = stdout;
    int i,len,cnt,tst;
	int val;
	int aLaw;
	int inFlag = 0;
	int outFlag = 0;

	char *outfile;
	char *infile;
	char c;

	tst = 0;
	aLaw = 0;
	pcm = 0; /* not used in decoder */

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

/* initialize values */
    I = dms = dml = 0;
    ap = pk1 = pk2 = 0;
    b1 = b2 = b3 = b4 = b5 = b6 = 0;
    a2 = a1 = 0;
    yl = 34816; yu = 544;
    sr1 = sr2 = 32;
    dq1 = dq2 = dq3 = dq4 = dq5 = dq6 = 32;
    td=0;
	len = 0;
	i = 1;
    /* get adpcm sample */
	val = getSample(fpin);

    while(!feof(fpin))
    {

		if(tst)
			printf("\n################### %d ###############\n", i);
		if(tst)
			dumpState(i, aLaw, MODE_DECODER);

		I = (long)val & 0x0f;

/* block a */
    /* step 1 */
        wb1 = fmult(b1,dq1);
        wb2 = fmult(b2,dq2);
        wb3 = fmult(b3,dq3);
        wb4 = fmult(b4,dq4);
        wb5 = fmult(b5,dq5);
        wb6 = fmult(b6,dq6);
        wa2 = fmult(a2,sr2);
        wa1 = fmult(a1,sr1);
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
    /* step 15 */
        sr0 = floatb(sr);
    /* step 16 */
        dq0 = floata(dq);
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
		sp = compress(sr, aLaw);
		sg = g711(sp, aLaw);
    /* step 35 */
		slx = expand(sg, aLaw);
    /* step 36 */
		dx = subta(slx,se);
    /* step 37 */
		llog(dx,&dlx,&dsx);
    /* step 38 */
		dlnx = subtb(dlx,y);
    /* step 39 */
		sd = synch(I,sp,dlnx,dsx, aLaw);
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

		if (debug) {
			dumpPCM(sd, i, aLaw) ;
		}

		fprintf( fpout, "%02x", sd );
		if (( i != 0 ) && ((i % (FILE_LINE)) == 0 ))
			fprintf( fpout, "\n" );
		i++;

    	/* get adpcm sample */
		val = getSample(fpin);

    }
    fclose(fpin);
    fclose(fpout);

	exit(0);
}

/* print usage message */

void usage()
{

	fprintf(stderr,"\n decode ADPCM to PCM, ANSI spec. T1.301-1987");
	fprintf(stderr,"\n usage: dec -h -a -t -d -o <output_file_name> -i <input_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message");
	fprintf(stderr,"\n \t\t-a process a-law pcm");
	fprintf(stderr,"\n \t\t-t enable sample tracing");
	fprintf(stderr,"\n \t\t-d enable debug messages");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t   (default: standard out)");
	fprintf(stderr,"\n \t\t-i <input_file_name>\t(input file to process)");
	fprintf(stderr,"\n \t\t   (default: standard out)\n");
	exit(1);
}

