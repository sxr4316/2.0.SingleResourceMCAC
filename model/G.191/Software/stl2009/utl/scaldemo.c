/*                                                          02.Feb.2010 v1.4
  ----------------------------------------------------------------------------
  scaldemo.c
  ~~~~~~~~~~
  Demo program to scale an input file by a factor A, scalar or in dB. 
  Input data is supposed to be aligned in 16-bit, 2-complement words and 
  containing left-adjusted samples. Default resolution is 16 bits per 
  sample, and rounding is used as default when converting from float to 
  short.

  When resolutions different from 16 bits are used with rounding,
  versions 1.2 and earlier of the program might not produce the
  "expected" results. The program used to limit the resolution of the
  samples (by masking 16-resolution LSbs) when converting from short
  to float. Additional rounding is applied after scaling when
  converting from float to short. If the desired operation is,
  actually, scale & then reduce the resolution with rounding, masking
  before the scaling operation should be disabled. In version 1.3 and
  later, the default behavior is NOT to apply such mask, (same as the
  option -nopremask) for backward compatible behavior, the option
  -premask should be explicitly used.

  Usage:
  ~~~~~~
  scaldemo [-options] filein fileout [blklen [1stblk [blkno [gain]]]]
  where:
  filein     input filename
  fileout    output (scales) filename
  blklen     block length, in samples [default: 256 samples]
  1stblk     first block to process [default: first]
  blkno      number of blocks to process [default: till end of file]
  gain       gain to be applied to the input file, dB or linear (default)

  Options:
  -dB        gain is in dB
  -lin       gain is linear gain [default]
  -gain #    gain value (same as parameter gain above)
  -bits #    define a different bit resolution (16-bit default)
  -round     round samples after scaling (default)
  -trunc     truncate samples after scaling
  -premask   enables bit masking before scaling
  -nopremask disables bit masking before scaling (default)
  -blk #     sample block size
  -n #       number of blocks to process
  -start #   first block to process
  -end #     last block to process (n-start+1)
  -q         quiet mode operation

  Modules:  ugst-utl.c
  ~~~~~~~~

  Original author:
  ~~~~~~~~~~~~~~~~
  Simao Ferraz de Campos Neto          *All comments are strictly my own*
  Comsat Laboratories                  Tel:    +1-301-428-4516
  22300 Comsat Drive                   Fax:    +1-301-428-9287
  Clarksburg MD 20871 - USA            E-mail: simao@ctd.comsat.com

  History
  ~~~~~~~
  05.Sep.95  v1.0  Created.
  18.Jan.98  v1.1  Inform user of % of clipped samples in file <simao>
  06.Apr.98  v1.2  Solved small bug that occurred when the file size was
                   not a multiple of the frame size. The program was
                   truncating the output file size to a multiple of
                   the current block size. <simao>
  10.Aug.99  v1.3  Corrected a bug in the initialization of the mask[] 
                   array; the value for 14 bit masking was
                   incorrect. Correct from 0xFFFB to 0xFFFC. Added
                   option to enable or disable that samples be
                   truncated (masked) before the scaling operation is
                   performed. Truncation was the previous behavior,
                   which is disabled by default. For backward
                   compatibility, masking is enabled by option
                   -premask). <simao>

  02.Feb.10  v1.4  Modified maximum string length to avoid buffer
                   overruns (y.hiwasaki)

  ---------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ugstdemo.h"
#include "ugst-utl.h"

#ifdef VMS
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#define ABSURD_VALUE 1E+100

/*
 -----------------------------------------------------------------------------
 void display_usage(void);
 ~~~~~~~~~~~~~~~~~~~~~~~~~
 Display program usage

 Author: Simao

 History:
 05.Sep.95 v1.0 Created
 -----------------------------------------------------------------------------
 */
#define P(x) printf x
void display_usage()
{
  P(("Scaldemo: Version 1.4 of 02.Feb.2010 \n"));
 
  P(("  Demo program to scale an input file by a factor A, scalar or dB.\n"));
  P(("  Input data is supposed to be aligned in 16-bit,2-complement words\n"));
  P(("  and containing left-adjusted samples. Default res. is 16 bits per\n"));
  P(("  sample, and rounding is used as default when converting from\n"));
  P(("  float to short.\n"));
  P(("\n"));
  P(("  Usage:\n"));
  P(("  scaldemo [-options] filein fileout [blklen [1stblk [blkno [gain]]]]\n"));
  P(("  where:\n"));
  P(("  filein     input filename\n"));
  P(("  fileout    output (scales) filename\n"));
  P(("  blklen     block length, in samples [default: 256 samples]\n"));
  P(("  1stblk     first block to process [default: first]\n"));
  P(("  blkno      number of blocks to process [default: till end of file]\n"));
  P(("  gain       gain to be applied to the input file, dB or linear (default)\n"));
  P(("\n"));
  P(("  Options:\n"));
  P(("  -dB        gain is in dB\n"));
  P(("  -lin       gain is linear gain [default]\n"));
  P(("  -gain #    gain value (same as parameter gain above)\n"));
  P(("  -bits #    define a different bit resolution (16-bit default)\n"));
  P(("  -round     round samples after scaling (default)\n"));
  P(("  -trunc     truncate samples after scaling\n"));
  P(("  -premask   enables bit masking before scaling\n"));
  P(("  -nopremask disables bit masking before scaling (default)\n"));
  P(("  -blk #     sample block size\n"));
  P(("  -n #       number of blocks to process\n"));
  P(("  -start #   first block to process\n"));
  P(("  -end #     last block to process (n-start+1)\n"));
  P(("  -q         quiet mode operation\n"));

  /* Quit program */
  exit(-128);
}
#undef P
/* ....................... End of display_usage() ......................... */


/* ************************************************************************ */
/* *********************** Demo program *********************************** */
/* ************************************************************************ */
int main(argc, argv)
  int             argc;
  char           *argv[];
{
  /* Parameters for operation */
  long            N=256, N1=1, N2=0;
  long            blk_count, nsam, NrSat = 0, NrTot = 0;
  long            bitno=16, start_byte, round=1;
  short           *s_buf;
  float           *f_buf;
  double          factor = ABSURD_VALUE, h;
  char            use_dB=0, quiet=0, pre_mask=0;

  /* File variables */
  FILE *Fi, *Fo;
  char FileIn[MAX_STRLEN], FileOut[MAX_STRLEN];
#ifdef VMS
  char            mrs[15];
#endif

  /* Miscelaneous */
  static char     funny[5] = {'/', '-', '\\', '|', '-'};
  static unsigned mask[5] = {0xFFFF,0xFFFE,0xFFFC,0xFFF8,0xFFF0};


  /* ......... GET PARAMETERS ......... */
 
  /* Getting options */
  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-") == 0)
      {
	/* Input file is stdin, skip while loop */
	break;
      }
      else if (strcmp(argv[1], "-bits") == 0)
      {
	/* Change default number of bits per sample */
	bitno = atol(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-round") == 0)
      {
	/* Round samples when converting from float to short */
	round = 1;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-trunc") == 0)
      {
	/* Truncate samples when converting from float to short */
	round = 0;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-premask") == 0)
      {
	/* Enables masking before scaling */
	pre_mask = 1;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-nopremask") == 0)
      {
	/* Disables masking before scaling -- Default */
	pre_mask = 0;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-dB") == 0)
      {
	/* Use the dB gain to normalize instead of linear */
	use_dB = 1;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strncmp(argv[1], "-lin", 3) == 0)
      {
	/* Use the linear gain to normalize instead of dB */
	use_dB = 0;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-gain") == 0)
      {
	/* Choose a gain value */
	factor = (double)atof(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-blk") == 0)
      {
	/* Change default block size */
	N = atol(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-start") == 0)
      {
	/* Change default starting block */
	N1 = atol(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-end") == 0)
      {
	/* Change number of blocks based on the last block */
	N2 = atol(argv[2]) - N1 + 1;

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-n") == 0)
      {
	/* Change default number of blocks */
	N2 = atol(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-q") == 0)
      {
	/* Don't print progress indicator */
	quiet = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-help") == 0)
      {
	/* Print help */
	display_usage();
      }
      else
      {
	fprintf(stderr, "ERROR! Invalid option \"%s\" in command line\n\n",
		argv[1]);
	display_usage();
      }
  }

  /* Read parameters for processing */
  GET_PAR_S(1, "_Input File: ........................... ", FileIn);
  GET_PAR_S(2, "_Output File: .......................... ", FileOut);
  FIND_PAR_L(3, "_Block Length: ......................... ", N, N);
  FIND_PAR_L(4, "_Start Block: .......................... ", N1, N1);
  FIND_PAR_L(5, "_No. of Blocks: ........................ ", N2, N2);
  FIND_PAR_D(6, "_Gain: ................................. ", factor, factor);

  /* ......... SOME INITIALIZATIONS ......... */
  if (factor==ABSURD_VALUE)
    HARAKIRI("Gain not specified; aborting\n",3);

  /* Convert dB to a linear factor, if it is the case */
  if (use_dB)
    factor = pow((double)10.0, (double)factor/(double)20.0);

  /* Print info */
  fprintf(stderr, "> Using %s gain\n", use_dB? "dB" : "linear" );

  /* Position for start of processing */
  start_byte = --N1;
  start_byte *= N * sizeof(short);

  /* Check if is to process the whole file */
  if (N2 == 0)
  {
    struct stat     st;

    /* ... find the input file size ... */
    stat(FileIn, &st);
    N2 = ceil((st.st_size - start_byte) / (double)(N * sizeof(short)));
  }

  /* Allocate memory for data buffers */
  if ((s_buf=(short *) calloc(sizeof(short), N))==NULL)
    HARAKIRI("Unable to allocate short buffer\n",5);
  if ((f_buf=(float *) calloc(sizeof(float), N))==NULL)
    HARAKIRI("Unable to allocate float buffer\n",5);

  /* Choose rounding number; it will be 0 when truncating */
  h = 0.5 * (round << (16 - bitno));


/*
 * ......... FILE PREPARATION .........
 */

  /* Opening input file; abort if there's any problem */
#ifdef VMS
  sprintf(mrs, "mrs=%d", 2 * N);
#endif
  if ((Fi = fopen(FileIn, RB)) == NULL)
    KILL(FileIn, 2);

  /* Creates output file */
  if ((Fo = fopen(FileOut, WB)) == NULL)
    KILL(FileOut, 3);

  /* Move pointer to 1st desired block */
  if (fseek(Fi, start_byte, 0) < 0l)
    KILL(FileIn, 4);

  /* Get data of interest, equalize and de-normalize */
  for (blk_count = 0; blk_count < N2; blk_count++)
  {
    /* Print some preliminary information */
    if (!quiet) 
      printf("%c\r", funny[blk_count % 5]);

    /* Read block of data */
    if ((nsam = fread(s_buf, sizeof(short), N, Fi)) > 0)
    {
      /* convert samples to float */
      sh2fl((long) nsam, s_buf, f_buf, pre_mask?bitno:16, 1);

      /* equalizes vector */
      scale(f_buf, (long) nsam, (double) factor);

      /* Convert from float to short */
      NrSat += fl2sh((long) nsam, f_buf, s_buf, h, mask[16-bitno]);

      /* write equalized, de-normalized and hard-clipped samples to file */
      if ((nsam = fwrite(s_buf, sizeof(short), nsam, Fo)) < 0)
	KILL(FileOut, 6);

      /* Update total number of samples in file */
      NrTot += nsam;
    }
    else
    {
      KILL(FileIn, 5);
    }
  }
  if (NrSat != 0)
    printf("Number of clippings: .......... %7ld (Tot: %7ld) [] (%6.2f%%)\n",
	   NrSat, NrTot, 100.0 * NrSat/(float)NrTot);


  /* FINALIZATIONS */

  if (!quiet)
    printf("---> DONE    \n");

  /* Close files, free memory */
  fclose(Fi);
  fclose(Fo);
  free(f_buf);
  free(s_buf);

  /* Return status: OK */
#ifndef VMS
  return (0);
#endif
}
