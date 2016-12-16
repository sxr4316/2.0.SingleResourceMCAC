/*
 *	This program will eventually generate signal data files
 *	in linear and u-law PCM hex formats for use with the
 *	56k dsp simulator.
 *	The data samples will be built from a bunch of sinusoids
 *	added together. Twist, skew, and amplitude relative to 0 dBm
 *	will be definable. Noise (uniform or gaussian) can be added
 *	to the signal if required. Noise level relative to 0 dBm will
 *	be definable by the user. Of course the sample rate and total
 *	set size will be definable by the user.
 *	If I can find the time, funcitons will be included to mimic near
 *	and far end echo, harmonic and intermodulation distortion.
 *      mai
 */

/*
 *
 *	@(#) genpcm.c 1.1@(#)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


#define DEBUG
#undef DEBUG
#define DEBUG2
#undef DEBUG2

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef u_32
typedef unsigned        u_32;
#endif

/* Define the audio magic number */
#define	AUDIO_FILE_MAGIC		((u_32)0x2e736e64)

/* Define the encoding fields */
#define	AUDIO_FILE_ENCODING_MULAW_8	(1)	/* 8-bit u-law pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_8	(2)	/* 8-bit linear pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_16	(3)	/* 16-bit linear pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_24	(4)	/* 24-bit linear pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_32	(5)	/* 32-bit linear pcm */
#define	AUDIO_FILE_ENCODING_FLOAT	(6)	/* 32-bit IEEE floating point */
#define	AUDIO_FILE_ENCODING_DOUBLE	(7)	/* 64-bit IEEE floating point */

#define MAXPCM	0x2000		/* maximum PCM value */
#define FSAMPLE	8000.0		/* sample rate, Hz */
#define SAMPLE	8000		/* sample rate, Hz */
#define RANDMAX	32767		/* maximum random value */
#define MAXDBM	3.17		/* maximum dBm value */
#define ZERODBM	0.7746		/* zero dBm voltage value */
#define MYPI	3.14159265358979323846		/* pi */
#define TWOPI	2.0 * MYPI			/* 2.0 * pi */
#define THETA	((0.0) * (TWOPI))		/* initial phase */
#define BASE	10.0		/* numeric base */
#define FLTMIN	(1.0/FSAMPLE)	/* floating point minimum for time input */
#define AUDIO	".au"		/* audio file extension */
#define SINMAX	10		/* maximum number of sinusoids per signal */
#define MAXPACKETS	50	/* maximum number of waveform packets */
#define SINAPPROXPTS	10	/* # of pts. in sine function approximation */
#define CHARMAX	80		/* maximum characters allowed on input */
#define BUFMAX	512		/* buffer size */
#define USERMODE	0644	/* buffer size */

enum sigtypes {
	QUIET,			/* quiet flag */
	SIGNAL			/* signal flag */
	};

enum noisetypes {
	UNIFORM,			/* uniform noise */
	GAUSSIAN,			/* gaussian noise */
	NONOISE				/* no noise */
	};

/* audio file header */
typedef struct {
	u_32		magic;		/* magic number */
	u_32		hdr_size;	/* size of this header */
	u_32		data_size;	/* length of data (optional) */
	u_32		encoding;	/* data encoding format */
	u_32		sample_rate;	/* samples per second */
	u_32		channels;	/* number of interleaved channels */
} Audio_filehdr;

/* audible signal */
typedef struct {
	int		length;			/* lenght of audible signal */
	int		qlength;		/* lenght of quiet signal */
	int		playptr;		/* signal playback pointer */
	int		reps;			/* number of times signal repeats */
	float		freqs[SINMAX];		/* frequency array */
	double		ohmf[SINMAX];		/* normalized radian frequency */
	double		level[SINMAX];		/* frequency level array */
	int		intdelay[SINMAX];	/* initial tone delay */
	int		clipoff[SINMAX];	/* early off time */
	int		number;			/* frequency count */
	int		type;			/* noise type flag */
	double		noise_level;		/* noise level */
} Audio_signal;

/* functions */
double gain(),gaussian(),uniform(),wavept(),quietpt(),noisept();
long conflt();
unsigned char lin_to_u();
int testcnt(),setup(),buflength();
void usage(),version(),timen(),filename();
void sigdef();
extern double sin(),sqrt(),log(),pow(),fabs();
extern int rand();

char prog_opts[] = "dshvo:";           /* getopt() flags */
extern int optind;
extern char *optarg;
int ready = 0;       /* flag to indicated stored value, needed by gaussian noise generator */
double gstore;       /* place to store other value, needed by gaussian noise generator */

main(argc,argv)
int argc;
char *argv[];
{
	short pcm,linear;
	short l,p;
	int debug,err,i,j,k,filenlen,packets;
	int fdout,wrcnt;
	u_32 *ptr;
	char *header;
	char *outfile = "pcm.au";	/* default filename */
	unsigned char buffer[BUFMAX];
	Audio_filehdr audiofile;
	Audio_signal audiosignal[MAXPACKETS];
	Audio_signal *audsigptr[MAXPACKETS];

	err = FALSE;
	debug = FALSE;
	for (i = 0 ; i < MAXPACKETS ; i++) 
		audsigptr[i] = &audiosignal[i];

	/* get command line options */
	while ((i = getopt(argc, argv, prog_opts)) != EOF)
		switch (i) {
			/* user passed output filename */
			case 'o':
				outfile = optarg;
				filename(outfile);
				break;
			/* print version info */
			case 'v':
				version();
				break;
			/* print signal definition */
			case 's':
				sigdef();
				break;
			/* build debug 1kHz, 0dBm signal */
			case 'd':
				debug = TRUE;
				err = TRUE;
				audiosignal[0].length = 8000;
				audiosignal[0].qlength = 0;
				audiosignal[0].freqs[0] = 1000.0;
				audiosignal[0].ohmf[0] = (double) (TWOPI * audiosignal[0].freqs[0]/FSAMPLE);
				audiosignal[0].level[0] = gain(0.0);
				audiosignal[0].intdelay[0] = 0;
				audiosignal[0].clipoff[0] = audiosignal[0].length;
				audiosignal[0].number = 1;
				audiosignal[0].type = 2;
				packets = 1;
				break;
			/* print usage (help) message */
			case 'h':
			default:
				usage();
				break;
			}

	if (!strcmp(outfile,"pcm.au"))
		fprintf(stderr," Generating default file: pcm.au\n");

        if (( fdout = open(outfile,(O_CREAT | O_TRUNC | O_WRONLY),USERMODE)) < 0 ) {
		printf("\n can't create output file -> %s \n\n",outfile);
		exit(0);
		}

	if (!debug) {
		/* get user input, grab multiple packets */
		for (packets = 0 ; packets < MAXPACKETS ; packets++) {
			if(!setup(audsigptr[packets],packets)) {
				packets++;
				err = TRUE;
				break;
				}
			}
		}
	if (!err)
		fprintf(stderr," Sorry, only %d packets allowed for each file.\n",MAXPACKETS);
	if (packets == 1)
		fprintf(stderr,"\n Creating 1 Tone Packet\n");
	else
		fprintf(stderr,"\n Creating %d Tone Packets\n",packets);

	/* build audio file header */
	audiofile.magic = AUDIO_FILE_MAGIC;
	filenlen = strlen(outfile) + 1;
	audiofile.hdr_size = sizeof(audiofile) + filenlen;
	audiofile.data_size = audsigptr[0]->length;
	audiofile.encoding = AUDIO_FILE_ENCODING_MULAW_8;
	audiofile.sample_rate = SAMPLE;
	audiofile.channels = 1;

	wrcnt = write(fdout,&audiofile.magic,sizeof(audiofile));
	wrcnt = write(fdout,outfile,filenlen);

	/* create required audio signal and write data to file */
	for(k = 0 ; k < packets ; k++) {
		if (audsigptr[k]->qlength != 0) {
			for(j = 0 ; j < audsigptr[k]->reps ; j++) {
				/* quiet before signal */
				audsigptr[k]->playptr = 0;
				do {
					i = buflength(audsigptr[k]->qlength,audsigptr[k]->playptr);
					if (i != 0) {
						for (l = 0 ; l < i ; l++) {
							*(buffer + l) = lin_to_u(conflt(noisept(audsigptr[k],quietpt())));
							audsigptr[k]->playptr++;
							}
						wrcnt = write(fdout,buffer,i);
						}
					} while (i != 0);
				audsigptr[k]->playptr = 0;
				do {
					i = buflength(audsigptr[k]->length,audsigptr[k]->playptr);
					if (i != 0) {
						for (l = 0 ; l < i ; l++) {
							*(buffer + l) = lin_to_u(conflt(noisept(audsigptr[k],wavept(audsigptr[k]))));
							audsigptr[k]->playptr++;
							}
						wrcnt = write(fdout,buffer,i);
						}
					} while (i != 0);
				}
			}
		else {
			audsigptr[k]->playptr = 0;
			do {
				i = buflength(audsigptr[k]->length,audsigptr[k]->playptr);
				if (i != 0) {
					for (l = 0 ; l < i ; l++) {
						*(buffer + l) = lin_to_u(conflt(noisept(audsigptr[k],wavept(audsigptr[k]))));
						audsigptr[k]->playptr++;
						}
					wrcnt = write(fdout,buffer,i);
					}
				} while (i != 0);
			}
		}
	close(fdout);

	exit(1);
}	

/* append '.au' identifer to filename */

void filename(ptr)
register char *ptr;
{

	register int len;

	len = strlen(ptr);
	if(strcmp((ptr + len - strlen(AUDIO)),AUDIO) != 0)
		strcat(ptr,AUDIO);
	return;
}

/* print usage message */

void usage()
{

	fprintf(stderr,"\n usage: gpcm -s -h -v -d -o <output_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t-v\t\t\t(print version message)");
	fprintf(stderr,"\n \t\t-h\t\t\t(print usage message)");
	fprintf(stderr,"\n \t\t-d\t\t\t(build debug 1kHz, 0dBm signal)");
	fprintf(stderr,"\n \t\t-s\t\t\t(print signal definition message) \n\n");
	exit(1);
}

/* print signal definition message */

void sigdef()
{

	fprintf(stderr,"\n 'gpcm' signal definition:\n");
	fprintf(stderr,"\n <-----Signal Packet #1----->...<-----Signal Packet #N----->");
	fprintf(stderr,"\n QQQQQQQQQQQQQQSSSSSSSSSSSSSS...QQQQQQQQQQQQQQSSSSSSSSSSSSSS");
	fprintf(stderr,"\n qqqqqqqqqqqqqqiiiittttttcccc...qqqqqqqqqqqqqqiiiittttttcccc");
	fprintf(stderr,"\n\n  where:");
	fprintf(stderr,"\n\tQ = q = Quiet Portion of Packet == Off-Time");
	fprintf(stderr,"\n\tS = Signal Portion of Packet == On-Time == Duration");
	fprintf(stderr,"\n\tt = Tone Portion of Packet");
	fprintf(stderr,"\n\ti = Initial Tone Delay (Quiet) Portion of Packet");
	fprintf(stderr,"\n\tS = Clip Time (Early Off Quiet) Portion of Packet\n\n");
	exit(1);
}

/* print version message */

void version()
{
	char *string = " \"gpcm\" SCCS info: @(#) gpcm.c 1.8@(#)\t10/23/92  13:42:23";

	fprintf(stderr,"\n%s\n\n",string);
	exit(1);
}

/* get parameters from user */

int setup(sigptr,id)
register Audio_signal *sigptr;
register int id;
{

	char s[CHARMAX];
	register int cnt;
	int flag;
	float len,olen;

	flag = 0;

	printf("\n Signal Packet #%d",(id + 1));
	for (sigptr->number = 0 ; sigptr->number < SINMAX && flag != 1 ; (sigptr->number)++) {
		do {
			printf("\n Signal Frequency #%d, Hz (0 terminates list) -> ",(sigptr->number +1));
			cnt = scanf("%f",(sigptr->freqs + sigptr->number));
			if ((*(sigptr->freqs + sigptr->number) == 0.0) && (sigptr->number == 0)) {
				printf("\n ** At least one frequency should be non-zero!");
				cnt = 0;
				}
			} while (testcnt(cnt));
		*(sigptr->freqs + sigptr->number) = (float) fabs(*(sigptr->freqs + sigptr->number));
		if (*(sigptr->freqs + sigptr->number) == 0.0) flag = 1;
		else {
        		*(sigptr->ohmf + sigptr->number) = (double) (TWOPI * *(sigptr->freqs + sigptr->number)/FSAMPLE);
			do {
				printf("\n  Level of Tone #%d, dBm -> ",(sigptr->number +1));
				cnt = scanf("%f",&len);
				} while (testcnt(cnt));
			*(sigptr->level + sigptr->number) = gain(len);
			do {
				printf("\n  Initial Tone Delay, Seconds -> ");
				cnt = scanf("%f",&len);
				} while (testcnt(cnt));
			timen(len,(sigptr->intdelay + sigptr->number),FALSE);
			do {
				printf("\n  Clip Time (Early Off), Seconds -> ");
				cnt = scanf("%f",&len);
				} while (testcnt(cnt));
			timen(len,(sigptr->clipoff + sigptr->number),FALSE);
		}
	}
	if (flag == 1) (sigptr->number)--;
	if (sigptr->number == 1)
		fprintf(stderr,"  1 Frequency entered.\n");
	else
		fprintf(stderr,"  %d Frequencies entered.\n",sigptr->number);
	do {
		printf("\n Repeat Signal, y/n -> ");
		cnt = scanf("%s",s);
		} while (testcnt(cnt));
	if ((strcmp(s,"y") == 0) || (strcmp(s,"Y") == 0)) {
		do {
			printf("\n On-Time, Seconds -> ");
			cnt = scanf("%f",&len);
			} while (testcnt(cnt));
		timen(len,&sigptr->length,FALSE);
		do {
			printf("\n Off-Time, Seconds -> ");
			cnt = scanf("%f",&olen);
			} while (testcnt(cnt));
		timen(olen,&sigptr->qlength,FALSE);
		do {
			printf("\n Number of Repeats -> ");
			cnt = scanf("%d",&sigptr->reps);
			} while (testcnt(cnt));
		if (sigptr->reps <= 0) sigptr->reps = 1;
		len = (len + olen) * (float) sigptr->reps;
		timen(len,&flag,TRUE);
	}
	else {
		do {
			printf("\n Signal Length, Seconds -> ");
			cnt = scanf("%f",&len);
			} while (testcnt(cnt));
		timen(len,&sigptr->length,TRUE);
		sigptr->qlength = 0;
	}
	do {
		printf("\n Noise type, %d <uniform>, %d <gaussian>, %d <none> -> ",UNIFORM,GAUSSIAN,NONOISE);
		cnt = scanf("%d",&sigptr->type);
		} while (testcnt(cnt));

	switch (sigptr->type) {
		case UNIFORM:
		case GAUSSIAN:
			do {
				printf("\n  Noise Level, dBm -> ");
				cnt = scanf("%f",&len);
				} while(testcnt(cnt));
			sigptr->noise_level = gain(len);
			break;
		case NONOISE:
		default:
			sigptr->type = 2;
			break;
	}

	/* adjust clipoff to reflect correct position in signal */
	for (cnt = 0 ; cnt < sigptr->number ; cnt++) {
		if ((*(sigptr->clipoff + cnt) + *(sigptr->intdelay + cnt)) >= sigptr->length) {
			*(sigptr->clipoff + cnt) = 0;
			*(sigptr->intdelay + cnt) = 0;
			}
		else {
			*(sigptr->clipoff + cnt) = sigptr->length - *(sigptr->clipoff + cnt);
			}
		}

	do {
		printf("\n Do you want to build another packet, y/n -> ");
		cnt = scanf("%s",s);
		} while(testcnt(cnt));
	if ((strcmp(s,"y") == 0) || (strcmp(s,"Y") == 0)) 
		return(TRUE);
	else
		return(FALSE);
}

/* quick function to check scanf return count */

int testcnt(cnt)
register int cnt;
{
	if (!cnt) {
		fprintf(stderr,"Opps, Lets try that again...\n");
		rewind(stdin);
		return(TRUE);
		}
	else
		return(FALSE);
}

/* convert dBm gain to a raw multiplier */

double gain(val)
register float val;
{

	register double x;

	if (val > MAXDBM)
		x = (double) MAXDBM/10.0;
	else
		x = (double) (val/10.0);
	if (x > 0.0) {
		return(pow((double)BASE,x) * (double) ZERODBM);
	}
	else if (x < 0.0) {
		return(((double) 1.0/pow((double)BASE, fabs(x))) * (double) ZERODBM);
	}
	else return((double) ZERODBM);
}

/* convert length in time to length in samples */

void timen(duration,length,printvar)
register int *length,printvar;
register float duration;
{

	*length = (int) (SAMPLE * duration);
	if(printvar) fprintf(stderr,"  Generating %d samples.\n",*length);
	return;
}

/* convert floating point value to integer */

long conflt(val)
register float val;
{
	return(MAXPCM * val);
}

/* convert integer to u-law compressed pcm */

unsigned char lin_to_u(val)
register long val;
{
	register long sign,mag,i,s,q,ymag;
	register unsigned char inv;

	if(val < 0) {		/* get sign of sample */
    		mag = abs(val) + 33;
		sign = 1;
		}
	else {
    		mag = val + 33;
		sign = 0;
		}

	if(mag >= MAXPCM)
		ymag = 0x7f;
	else {
		for(i = 12; i >= 5; i--) {	/* normalize magnitude */
		if(mag & (1 << i)) break;
		}

		s = i - 5;
		q = (mag & ( 0x0f << (i - 4))) >> (i - 4);
		ymag = (s << 4) + q;
		}
	inv = sign ? ymag | 0x80 : ymag & 0x7f;
	inv = inv ^ 0xff;
	if (inv == 0) inv = 2;		/* zero is not allowed */
	return(inv);
}

/* generate single signal point */

double wavept(sigptr)
register Audio_signal *sigptr;
{
	register int j;
	register double samp;	/* computed sample */

	samp = 0.0;
	for (j = 0 ; j < sigptr->number ; j++) {
		if ((sigptr->playptr >= sigptr->intdelay[j]) && (sigptr->playptr < sigptr->clipoff[j])) {
			samp += sigptr->level[j]*sin(((double) sigptr->playptr * sigptr->ohmf[j]) + (double) THETA);
			}
    	}
	return(samp);
}

/* generate single quiet tone point */

double quietpt()
{
    return((double) 0.0);
}

/* add noise signal point*/

double noisept(sigptr,samp)
register Audio_signal *sigptr;
register double samp;
{
	switch(sigptr->type) {
		case UNIFORM:	
			return(samp + sigptr->noise_level*uniform());
			break;
		case GAUSSIAN:
			return(samp + sigptr->noise_level*gaussian());
			break;
		case NONOISE:
		default:
			return(samp);
			break;
	}
}


/* gaussian distribution noise generator */

double gaussian()
{
	register double v1,v2,r,fac,gaus;

	/* make two numbers if none stored */
	if(ready == 0) {
		do {
			v1 = 2.*uniform();
			v2 = 2.*uniform();
			r = v1*v1 + v2*v2;
			} while(r > 1.0);	   /* make radius less than 1 */

	/* remap v1 and v2 to two Gaussian numbers */
		fac = sqrt(-2.*log(r)/r);
		gstore = v1*fac;	/* store one */
		gaus = v2*fac;		/* return one */
		ready = 1;		/* set ready flag */
		}

	else {
		ready = 0;	  /* reset ready flag for next pair */
		gaus = gstore;  /* return the stored one */
		}

	return(gaus);
}


/* uniform distribution noise generator */

double uniform()
{
	return((double)(rand() & RANDMAX) / RANDMAX - 0.5);
}

/* compute buffer length */

int buflength(length,ptr)
register int length,ptr;
{
	register int remain;

	remain = length % BUFMAX;
	if(remain == 0)
		if((ptr + BUFMAX) > length)
			return((int)NULL);
		else
			return(BUFMAX);
	else if (remain > length)
		if(remain == ptr)
			return((int)NULL);
		else
			return(remain);
	else
		if((ptr + remain) > length)
			return((int)NULL);
		else if ((length - ptr) == remain)
			return(remain);
		else
			return(BUFMAX);
}
