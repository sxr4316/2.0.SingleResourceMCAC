/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  CALC-SNR.C
  ~~~~~~~~~~
  
  Description:
  ~~~~~~~~~~~~
  Program for calculating the SNR of two files.
  Print the results on the screen, or in a BINARY file. Supposes
  `short' data format for input.
  
  Usage:
  ~~~~~~
  $ SNR [-options] file1 file2 [BlkSiz [1stBlock [NoOfBlocks [output]]]]
  where:
  file1       is the first file name;
  file2       is the second file name;
  BlkSiz      is block size, in samples;
  1stBlock    is the starting block;
  NoOfBlocks  the number of blocks to be displayed;
  output      if specified, is the name of output
              file, with the difference values.
  Options:
  ~~~~~~~~
  -blk len  .. is the block size in number of samples;
               this parameter is optional, and the default is block size
	       of 256 samples;
  -start sb .. define `sb' as the first block to be measured 
               [default: first block of the file] 
  -n nb ...... define `nb' as the number of blocks to be measured 
               [default: whole file]
  -out of .... binary output file name with binary float SEGSNR
  -q ......... quiet operation; don't print progress flag, results are
               printed all in one line.

  Author: Simao Ferraz de Campos Neto -- CPqD/Telebras
  ~~~~~~~
  
  History:
  ~~~~~~~~
  27/Jan/1991 1.0 1st release
  05/Jun/1995 2.0 Updated/new interface added <simao@ctd.comsat.com>
  02/Feb/2010 2.1 Modified maximum string length (y.hiwasaki)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#include "ugstdemo.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(VMS)
#include <stat.h>
#else 			/* Unix */
#include <sys/stat.h>
#endif

#define PRINT_RULE { for (K=0;K<4;K++) printf("-------------------");}
#define PRINT_RULE2 { for (K=0;K<4;K++) printf("===================");}
#define CR	   printf("\n");

#define YES 1
#define NO  0


#include "snr.c"

/* 
  ============================================================================

        void display_usage (void)
        ~~~~~~~~~~~~~~~~~~

        Display usage information and quit program.

        Parameter:
        ~~~~~~~~~~
        None.

        Returns
        ~~~~~~~
        To calling program, nothing; to calling environment, returns OK
        (1 in VAX/VMS or 0, otherwise).

        Original author
        ~~~~~~~~~~~~~~~
        simao@ctd.comsat.com

        Log of changes
        ~~~~~~~~~~~~~~
        01.Feb.94	v1.0	Creation.

  ============================================================================
*/
#define P(x) printf x
void display_usage()
{
  P(("CALC-SNR.C - Version 2.1 of 02.Feb.2010 \n\n"));
  P(("  Program for calculating the SNR of two files.\n"));
  P(("  Print the results on the screen, or in a BINARY file. Supposes\n"));
  P(("  `short' data format for input.\n"));
  P(("  \n"));
  P(("  Usage:\n"));
  P(("  ~~~~~~\n"));
  P(("  $ SNR [-options] file1 file2 [BlkSiz [1stBlock [NoOfBlocks [output]]]]\n"));
  P(("  where:\n"));
  P(("  file1       is the first file name;\n"));
  P(("  file2       is the second file name;\n"));
  P(("  BlkSiz      is block size, in samples;\n"));
  P(("  1stBlock    is the starting block;\n"));
  P(("  NoOfBlocks  the number of blocks to be displayed;\n"));
  P(("  output      if specified, is the name of output\n"));
  P(("              file, with the difference values.\n"));
  P(("  Options:\n"));
  P(("  ~~~~~~~~\n"));
  P(("  -blk len  .. is the block size in number of samples;\n"));
  P(("               this parameter is optional, and the default is block size\n"));
  P(("	       of 256 samples;\n"));
  P(("  -start sb .. define `sb' as the first block to be measured \n"));
  P(("               [default: first block of the file] \n"));
  P(("  -n nb ...... define `nb' as the number of blocks to be measured \n"));
  P(("               [default: whole file]\n"));
  P(("  -out of .... define `of' as the binary output filename with \n"));
  P(("               binary float SEGSNR [default: log only onto screen]\n"));
  P(("  -q ......... quiet operation: results printed all in one line.\n"));

  /* Quit program */
  exit(-128);
}
#undef P
/* ....................... End of display_usage() .......................... */


/* ==================================== */
int main(argc, argv)
  int             argc;
  char           *argv[];
/* ==================================== */
{
  char            out_is_file = NO, oper;
  int             i, k, l, K;
  char            File1[MAX_STRLEN], File2[MAX_STRLEN];
  FILE           *F1, *F2, *Fo = stdout;
#ifdef VMS
  char            mrs[15] = "mrs=";
#endif

  long            N=256, N1=1, N2=0;
  short           a[4096], b[4096];
  float          *snr_vector, *sqr_vector;
  double          total_snr_dB;
  SNR_state       state;

  /* General stuff */
  char quiet=0;


  /* ......... GET PARAMETERS ......... */
 
  /* Getting options */
  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-blk") == 0)
      {
	/* Change default sampling frequency */
	N = atoi(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-start") == 0)
      {
	/* Change default sampling frequency */
	N1 = atoi(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-n") == 0)
      {
	/* Change default sampling frequency */
	N2 = atoi(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-out") == 0)
      {
	/* Change default sampling frequency */
	if ((Fo = fopen(argv[2], WB)) == NULL)
	  KILL(argv[2], 4);
	out_is_file = YES;

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
      else if (strcmp(argv[1],"-")==0)
      {
	/* Get from stdin */
	break;
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

  /* Get parameters */
  GET_PAR_S(1,  "First file? .......................... ", File1);
  GET_PAR_S(2,  "Second file? ......................... ", File2);
  FIND_PAR_L(3, "Record Length? ....................... ", N,N);
  FIND_PAR_L(4, "Starting Record? ..................... ", N1,N1);
  FIND_PAR_L(5, "Number of Records? ................... ", N2,N2);

  /* Parse if to output a binary log file with the SEGSNR values */ 
  if (argc > 6)
  {
#ifdef VMS
    sprintf(&mrs[4], "%d", 4 * N);
#endif
    if ((Fo = fopen(argv[6], WB)) == NULL)
      KILL(argv[6], 4);
    out_is_file = YES;
  }

  /* Check if is to process the whole file */
  if (N2 == 0)
  {
    struct stat st;
    stat(File1, &st);
    N2 = st.st_size / (N * sizeof(short));
  }

  /* Open input files */
  if ((F1 = fopen(File1, RB)) == NULL)
    KILL(File1, 2);
  if ((F2 = fopen(File2, RB)) == NULL)
    KILL(File2, 3);

  /* Positions file to the starting of block N1 */
  N1--;				/* for the 1st block is not 1 but 0! */
  if (fseek(F1, N1 * N * sizeof(short), 0) != 0l)
    KILL(File1, 5);
  if (fseek(F2, N1 * N * sizeof(short), 0) != 0l)
    KILL(File2, 6);

  /* Allocate memory for SNR vector */
  if ((snr_vector = (float *) calloc(N2, sizeof(float))) == NULL)
  {
    fprintf(stderr, "Error allocating memory for SNR vector\n");
    exit((int) 10);
  }

  /* Allocate memory for squared samples' vector */
  if ((sqr_vector = (float *) calloc(N2, sizeof(float))) == NULL)
  {
    fprintf(stderr, "Error allocating memory for sqr vector\n");
    exit((int) 10);
  }

  /* Print dump information */
  if (out_is_file == NO && !quiet)
  {
    CR;
    PRINT_RULE;
    printf("\n SNR for %s and %s\n", File1, File2);
    PRINT_RULE;
  }

  /* Down to work */
  for (i = 0; i < N2; i++)
  {
     if ((l = fread(a, sizeof(short), N, F1)) > 0 && 
	 (k = fread(b, sizeof(short), N, F2)) > 0)
    {
      oper = i == 0 ? SNR_RESET : (i == N2 - 1 ? SNR_STOP : SNR_MEASURE);
      total_snr_dB = get_SNR(oper, a, b, N, N2, &state);
    }
    else
    {
      if (l < 0)
	KILL(File1, 7);
      if (k < 0)
	KILL(File2, 8);
      break;
    }
  }

  /* Output of results */
  if (out_is_file)
  {
    if ((l = fwrite(state.snr_vector, 4, N2, Fo)) != N2)
      KILL(argv[6], 9);
  }

  /* Release SNR and sqr buffers */
  total_snr_dB = get_SNR(SNR_DEALLOC, a, b, N, N2, &state);

  /* Statistics */
  if(quiet)
  {
      printf("Samples: %5ld ",   state.tot_smpno);
      printf("(Used: %.2f%%) ", (state.count*N/(double)state.tot_smpno)*100);
      printf("TotPwrdB: %6.2f ", state.total_sqr_dB);
      printf("AvgPwrdB: %6.2f ", state.avg_sqr);
      printf("MinPwrdB: %6.2f ", state.min_sqr);
      printf("MaxPwrdB: %6.2f ", state.max_sqr);
      printf("TotSNRdB: %6.2f ", state.total_snr_dB);
      printf("AvgSNRdB: %6.2f ", state.avg_snr);
      printf("MinSNRdB: %6.2f ", state.min_snr);
      printf("MaxSNRdB: %6.2f ", state.max_snr);
      printf("\t%s/%s\n", File1,File2);
  }
  else
  {
    printf("\n--------------------------------------------");
    printf("\n->Total power is   \t%f [dB]", state.total_sqr_dB);
    printf("\n->Average power is \t%f +/- %f [dB]", 
	   state.avg_sqr, state.var_sqr);
    printf("\n->Max/Min power:   \t%f and %f [dB]", 
	   state.max_sqr, state.min_sqr);
    printf("\n->Total SNR is     \t%f [dB]", state.total_snr_dB);
    printf("\n->Average SNR is   \t%f +/- %f [dB]", 
	   state.avg_snr, state.var_snr);
    printf("\n->Max/Min seg.SNR: \t%f and %f [dB]", 
	   state.max_snr, state.min_snr);
    printf("\n--------------------------------------------");
    printf("\n  Total of samples \t%ld", (long) state.tot_smpno);
    printf("\n  %% Used for averages \t%5.2f",
	   (state.count * N / (double) state.tot_smpno) * 100);
    printf("\n--------------------------------------------\n");
  }

  /* Closing... */
  fclose(F1);
  fclose(F2);
  if (out_is_file)
    fclose(Fo);
#ifndef VMS
  return(0);
#endif
}
