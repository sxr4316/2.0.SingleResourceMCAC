
/*
 *
 *	@(#) check_vec.c 1.2@(#)
 *
 *	check vector length in characters and bytes
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

char prog_opts[] = "i:";	/* getopt() flags */
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
	char *header;
	char *infile;
	char *outfile;
	int len = DEFAULT_INIT;
	char buff[BUFFSIZE];
	fpin = stdin;
	char c;
	int val, i;

	/* get command line options */
	while ((c = getopt(argc, argv, prog_opts)) != EOF)
		switch (c) {
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
		if (( fpin = fopen(infile,"r" )) == NULL ) {
			printf(" can't open file %s \n", infile);
			exit(0);
    			}

		i = 0;
		while( !feof(fpin)) {
			c = (char) fgetc(fpin);
			if ( isxdigit(c) )
				i++;
			}
		fprintf(stderr,"\n check vector length: chars: %6d bytes: %6d\n", i, i/2);
	} else {
		usage();
	}
		
	close(fpin);

	exit(0);
}	

/* print usage message */

void usage()
{

	fprintf(stderr,"\n check vector length");
	fprintf(stderr,"\n usage: check_vec -i <input_file_name>");
	fprintf(stderr,"\n  where:");
	fprintf(stderr,"\n \t\t-h print this help message");
	fprintf(stderr,"\n \t\t-i <input_file_name>\t(input file to process)");
	fprintf(stderr,"\n \t\t   (default: standard in)\n");
	exit(1);
}
