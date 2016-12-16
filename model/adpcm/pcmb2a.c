
/*
 *
 *	@(#) pcmb2a.c 1.3@(#)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

enum FCNTL {
	BEGIN,
	CURR,
	END
};

#define FILE_LINE 32

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

#define SAMPLE	8000		/* sample rate, Hz */
#define AUDIO	".au"		/* audio file extension */

/* audio file header */
typedef struct {
	u_32		magic;		/* magic number */
	u_32		hdr_size;	/* size of this header */
	u_32		data_size;	/* length of data (optional) */
	u_32		encoding;	/* data encoding format */
	u_32		sample_rate;	/* samples per second */
	u_32		channels;	/* number of interleaved channels */
} Audio_filehdr;

FILE *fpin ;
FILE *fpout ;

char prog_opts[] = "hni:o:";	/* getopt() flags */
extern int optind;
extern char *optarg;
void usage();

main(argc,argv)
int argc;
char *argv[];
{
	int inFlag = 0;
	int outFlag = 0;
	int headerFlag = 0;
	char *header;
	char *infile;
	char *outfile;
	char c;
	int val, i;
	fpin = stdin;
	fpout = stdout;
	Audio_filehdr audiofile;

	/* get command line options */
	while ((c = getopt(argc, argv, prog_opts)) != EOF)
		switch (c) {
			/* user passed output filename */
			case 'o':
				outfile = optarg;
				outFlag++;
				break;
			/* user passed input filename */
			case 'i':
				infile = optarg;
				inFlag++;
				break;
			/* file has audiofile header? */
			case 'n':
				headerFlag++;
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


	if ( headerFlag ) {
		fread((char *) &audiofile.magic, sizeof(char),
			sizeof(audiofile), fpin);
		rewind( fpin );
		fseek(fpin, (long) audiofile.hdr_size, BEGIN);
	}

	i = 1;
	val = fgetc(fpin);

	while(!feof( fpin )) {
		fprintf( fpout, "%02x", val );
		if (( i != 0 ) && ((i % (FILE_LINE)) == 0 ))
			fprintf( fpout, "\n" );
		i++;
		val = fgetc(fpin);
	}

	close(fpin);
	close(fpout);

	exit(0);
}	

/* print usage message */

void usage()
{

	fprintf(stderr,"\n convert binary audiofile to an ascii pcm file");
	fprintf(stderr,"\n usage: pcmb2a -h -n -o <output_file_name> -i <input_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message");
	fprintf(stderr,"\n \t\t-n binary file has audiofile header");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t   (default: standard out)");
	fprintf(stderr,"\n \t\t-i <input_file_name>\t(input file to process)");
	fprintf(stderr,"\n \t\t   (default: standard in)\n");
	exit(1);
}
