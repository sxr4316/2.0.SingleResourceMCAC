
/*
 *
 *	@(#) vec_strip.c 1.2@(#)
 *
 * Strip ITU vectors:
 *  remove cr-lf, check checksum, then remove checksum
 *
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

char prog_opts[] = "hi:o:";	/* getopt() flags */
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
	char *infile;
	char *outfile;
	int len = DEFAULT_INIT;
	char buff[BUFFSIZE];
	fpin = stdin;
	fpout = stdout;
	char c;
	unsigned char p;
	int val, i, j, k;
	unsigned int cksum = 0;
	unsigned int cksum1 = 0;

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
		fprintf(stdout, "src vector: %s\n", infile);
	}

    if ( outFlag ) {
		if (( fpout = fopen(outfile,"w" )) == NULL )
   			{
			printf(" can't open file %s \n", outfile);
			exit(0);
   			}
	}

	// find checksum
	i = 1;
	j = 0;
	c = (char) fgetc(fpin);

	while(!feof( fpin )) {
		if ( isxdigit(c) ) {
			i++;
			j++;
			if (j == 2) {
				j = 0;
				cksum1 = cksum ;
				fseek(fpin, (ftell(fpin) - (long) 2), SEEK_SET);
				fscanf(fpin, "%2x", &p);
				cksum += p;
			}
		}
		c = (char) fgetc(fpin);
	}

	if (p == (cksum1 % 0x0ff)) {
		fprintf(stdout, "cksum: [%02x] == (%08x)\n", p, cksum1 % 0x0ff);
	} else {
		fprintf(stdout, "cksum: [%02x] != (%08x)\n", p, cksum1 % 0x0ff);
	}
	rewind(fpin);

	// write out vector minus checksum
	i -= 2;
	k = 1;
	j = 0;
	c = (char) fgetc(fpin);

	while((!feof( fpin )) && (k < i)) {
		if ( isxdigit(c) ) {
			fprintf( fpout, "%c", c );
			if (( k != 0 ) && ((k % (FILE_LINE * 2)) == 0 ))
				fprintf( fpout, "\n" );
			k++;
		}
		c = (char) fgetc(fpin);
	}

	//fprintf( fpout, "\n" );
	close(fpin);
	close(fpout);

	exit(0);
}	

/* print usage message */

void usage()
{

	fprintf(stderr,"\n strip ITU factory vectors");
	fprintf(stderr,"\n usage: vec_strip -h -o <output_file_name> -i <input_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message");
	fprintf(stderr,"\n \t\t-o <output_file_name>\t(output file created)");
	fprintf(stderr,"\n \t\t   (default: standard out)");
	fprintf(stderr,"\n \t\t-i <input_file_name>\t(input file to process)");
	fprintf(stderr,"\n \t\t   (default: standard in)\n");
	exit(1);
}
