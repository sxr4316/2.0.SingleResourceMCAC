
/*
 *
 *	@(#) stripInit.c 1.2@(#)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include "audio.h"

#define BUFFSIZE	512
#define FILE_LINE 32
#define DEFAULT_INIT (3496 * 2)

FILE *fpin ;
FILE *fpout ;

char prog_opts[] = "hbnl:i:o:";	/* getopt() flags */
extern int optind;
extern char *optarg;
void usage();

enum FCNTL {
	BEGIN,
	CURR,
	END
};

main(argc,argv)
int argc;
char *argv[];
{
	int inFlag = 0;
	int outFlag = 0;
	int asciiFlag = 1;
	int headerFlag = 0;
	char *header;
	char *infile;
	char *outfile;
	int len = DEFAULT_INIT;
	char buff[BUFFSIZE];
	fpin = stdin;
	fpout = stdout;
	Audio_filehdr audiofile;
	char c;
	int val, i;

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
			/* user dosen't default strip length */
			case 'l':
				sscanf( optarg, "%d", &len );
				break;
			/* process ascii vectors */
			case 'b':
				asciiFlag = 0;
				len /= 2;
				break;
			/* file has audiofile header */
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

    if ( !asciiFlag ) {
		if ( headerFlag ) {
			fread((char *) &audiofile.magic, sizeof(char),
				sizeof(audiofile), fpin);
			rewind( fpin );
			len += audiofile.hdr_size;
		}

		fseek(fpin, (long) len, BEGIN);
		while( !feof(fpin)) {
			len = fread( buff, sizeof(char), BUFFSIZE, fpin);
			fwrite( buff, sizeof(char), len, fpout);
		}
	} else {
		i = 0;
		while ( i < len ) {
			c = (char) fgetc(fpin);
			if ( isxdigit(c) )
				i++;
		}
		i = 1;
		c = (char) fgetc(fpin);

		while(!feof( fpin )) {
			if ( isxdigit(c) ) {
				fprintf( fpout, "%c", c );
				if (( i != 0 ) && ((i % (FILE_LINE * 2)) == 0 ))
					fprintf( fpout, "\n" );
				i++;
			}
			c = (char) fgetc(fpin);
		}
	}
		
	close(fpin);
	close(fpout);

	exit(0);
}	

/* print usage message */

void usage()
{

	fprintf(stderr,"\n strip init sequence from ascii (binary) vectors");
	fprintf(stderr,"\n usage: stipInit -h -b -n -l <strip length> -o <output_file_name> -i <input_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message");
	fprintf(stderr,"\n \t\t-b process binary vectors");
	fprintf(stderr,"\n \t\t-n binary file has audiofile header");
	fprintf(stderr,"\n \t\t-l <strip length>");
	fprintf(stderr,"\n \t\t   (default: 3496)");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t   (default: standard out)");
	fprintf(stderr,"\n \t\t-i <input_file_name>\t(input file to process)");
	fprintf(stderr,"\n \t\t   (default: standard in)\n");
	exit(1);
}
