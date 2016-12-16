/*
       This file will convert linear PCM data to compress PCM using formulas 
	   in G.711.
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "encode.h"

FILE *fpin ;
FILE *fpout ;

long yl;
long dq1,dq2,dq3,dq4,dq5,dq6;
long b1,b2,b3,b4,b5,b6,a1,a2;
long pk1,pk2,sr1,sr2;
long ap,yu;
long dms,dml;
long y,se,al,sez,sr,td;
long sd,pcm;

extern long inv_g711(),g711();
extern long getSample();

char prog_opts[] = "haDdti:o:";
extern int optind;
extern char *optarg;
void usage();

int debug = 0 ;

main(argc,argv)
int argc;
char *argv[];
{
    long npcm,I;
    long tdr,sigpk,tdp;
    long pk0,ss,nss,ssr,sl,wb1,b1p,b2p,b3p,b4p,b5p,b6p;
    long u1,u2,u3,u4,u5,u6;
    long wb2,wb3,wb4,wb5,wb6,wa2,wa1;
    long d,dl,ds,dln,dqln,dqs,dql;
    long dq,wi,yut,fi,ax,a2t,a1t;
    long yup,dmsp,dmlp,app,sr0,dq0,a2p,a1p;
    long a1r,a2r,b1r,b2r,b3r,b4r,b5r,b6r;
    long ylp,is,imag;
    fpin = stdin;
    fpout = stdout;
    int len,tst,init_cnt;
	int inFlag = 0;
	int outFlag = 0;
	int i, j, l;
	int aLaw, decode;
	unsigned short val;
	short tc,tc2;
	short is2,imag2;

	char *outfile;
	char *infile;
	char c;

	tst = 0;
	aLaw = 0;
	decode = 0;

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
			/* test mode */
			case 't':
				tst = 1;
				break;
			/* debug mode */
			case 'd':
				debug = 1;
				break;
			/* process a-law pcm */
			case 'a':
				aLaw = 1;
				break;
			/* user passed output filename */
			case 'D':
				decode = 1;
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

	l = (decode) ? 0x00ff : (aLaw ? 0x01fff : 0x03fff) ;
	val = 0 ;

	for( j = 0; j <= l; j++)
	{
		/* initialize values */
   		if (tst) printf("#################### %d #################\n", i);

    	if (decode) { /* decode pcm to linear */
        	pcm = (long)val & 0xff;
        	ss = g711(pcm, aLaw);	/* expand 8 bits to 14/13 bit */
        	is = aLaw ? ss >> 12 : ss >> 13 ;
        	imag = aLaw ? (ss & 4095) : (ss & 8191) ;
        	npcm = inv_g711(is, imag, aLaw);	/* compress 14/13 bit to 8 bits */
			tc = is ? (~imag + 1) : imag ;
			if (aLaw) {
				fprintf( fpout, "i:%02x(^%02x)  do:%04x(%5d)  ro:%02x(^%02x)\n",
					pcm, pcm ^ 0x55, ss, tc, npcm, npcm ^ 0x55 );
			} else {
				fprintf( fpout, "i:%02x  do:%04x(%5d)  ro:%02x\n", pcm, ss, tc, npcm );
			}
    	} else { /* encode linear to pcm */
        	is = aLaw ? val >> 12 : val >> 13 ;
        	imag = aLaw ? (val & 4095) : (val & 8191) ;
        	pcm = inv_g711(is, imag, aLaw);	/* compress 14/13 bit to 8 bits */
			ss =  aLaw ? (is << 12) | (imag & 4095) : (is << 13) | (imag & 8191) ;
			tc = is ? (~imag + 1) : imag ;
			nss = g711(pcm, aLaw);	/* expand 8 bits to 14/13 bit */
        	is2 = aLaw ? nss >> 12 : nss >> 13 ;
        	imag2 = aLaw ? (nss & 4095) : (nss & 8191) ;
			tc2 = is2 ? (~imag2 + 1) : imag2 ;
			if (aLaw) {
				fprintf( fpout, "i:%04x(%5d)  eo:%02x(^%02x)  ro:%04x(%5d)\n", ss, tc,
pcm, pcm ^ 0x55, nss, tc2 );
			} else {
				fprintf( fpout, "i:%04x(%5d)  eo:%02x  ro:%04x(%5d)\n", ss, tc, pcm, nss, tc2 );
			}
    	}
		i++;
		val++;

    }
    fclose(fpin);
    fclose(fpout);

	exit(0);
}


/* print usage message */

void usage()
{

	fprintf(stderr,"\n encode / decode linear PCM to G.711");
	fprintf(stderr,"\n usage: enc -h -a -D -t -d -o <output_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message)");
	fprintf(stderr,"\n \t\t-a process a-law pcm");
	fprintf(stderr,"\n \t\t-D decode to g.711 (default is to encode)");
	fprintf(stderr,"\n \t\t-t enable sample tracing");
	fprintf(stderr,"\n \t\t-d enable debug messages");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t   (default: standard in)\n");
	exit(1);
}
