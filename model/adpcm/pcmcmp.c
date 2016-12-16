
/*
 *
 *	@(#) pcmcmp.c 1.2@(#)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int tst = 0;
long g711();
long getSample();
void usage();

char prog_opts[] = "htu:a:";
extern int optind;
extern char *optarg;

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fpin_u, *fpin_a;
	int uFlag = 0;
	int aFlag = 0;
	long ulaw, alaw, ulin, alin;

	char *infile_u;
	char *infile_a;
	char c;

	/* get command line options */
	while ((c = getopt(argc, argv, prog_opts)) != EOF)
		switch (c) {
			/* user passed u-law filename */
			case 'u':
				infile_u = optarg;
				uFlag++;
				break;
			/* user passed a-law filename */
			case 'a':
				infile_a = optarg;
				aFlag++;
				break;
			/* turn on debug */
			case 't':
				tst++;
				break;
			/* print usage (help) message */
			case 'h':
			default:
				usage();
				break;
			}

    if ( uFlag ) {
		if (( fpin_u = fopen(infile_u,"r" )) == NULL )
    		{
			printf(" can't open file %s \n", infile_u);
			exit(0);
    		}
	}

    if ( aFlag ) {
		if (( fpin_a = fopen(infile_a,"r" )) == NULL )
    		{
			printf(" can't open file %s \n", infile_a);
			exit(0);
    		}
	}

	if ( !uFlag || !aFlag ) {
		usage();
		exit(1);
	}

    /* get pcm sample */
	ulaw = getSample(fpin_u);
	alaw = getSample(fpin_a);

    while(!feof(fpin_u) && !feof(fpin_a))
    {
        ulaw &= 0xff;
        alaw &= 0xff;
        ulin = g711(ulaw, 0);	/* expand 8 bits to 14/13 bit */
        alin = g711(alaw, 1);	/* expand 8 bits to 14/13 bit */

		printf( "ulaw: %02x -> %8d  alaw: %02x -> %8d\n",
			ulaw, ulin, alaw, alin );

    	/* get pcm sample */
		ulaw = getSample(fpin_u);
		alaw = getSample(fpin_a);

    }
    fclose(fpin_u);
    fclose(fpin_a);

	exit(0);
}

/* print usage message */

void usage()
{

	fprintf(stderr,"\n print linear versions of a/u -law pcm");
	fprintf(stderr,"\n usage: pcmcmp -h -t -u <u-law file> -a <a-law file>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message)");
	fprintf(stderr,"\n \t\t-t turn on debug printing)");
	fprintf(stderr,"\n \t\t-u <u-law input file>");
	fprintf(stderr,"\n \t\t-a <a-law input file>");
	fprintf(stderr,"\n");
	exit(1);
}

long
getSample( fpin )
FILE *fpin;
{
	char	buff[3];
	long	val;

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

long
g711(pcm, aLaw)
long	pcm;
int	aLaw;
{
	long	ipcm, q, s, sgn, mag;

	if ( !aLaw ) {
		/* process u-law pcm */
		ipcm = (pcm ^ 0xff);
		q = 0x0f & ipcm;
		s = (0x70 & ipcm) >> 4;
		sgn = 0x80 & ipcm;
		if ( tst ) printf(" ulaw: pcm %02x ipcm %02x sgn %02x s %1x q %1x\n",
			pcm, ipcm, sgn, s, q );

		mag = (((2 * q) + 33) << s) - 33;
		mag = (sgn ? mag | (1 << 13) : mag) & 16383;
	} else {
		/* process a-law pcm */
		ipcm = (pcm ^ 0x55);
		q = 0x0f & ipcm;
		s = (0x70 & ipcm) >> 4;
		sgn = (0x80 & ipcm);
		if ( tst ) printf(" alaw: pcm %02x ipcm %02x sgn %02x s %1x q %1x\n",
			pcm, ipcm, sgn, s, q );

		mag = (s > 0) ? (((2 * q) + 33) << s) : ((2 * q) + 1);
		mag = (sgn ? mag : mag | (1 << 12)) & 8191;
	}
	return(mag);
}
