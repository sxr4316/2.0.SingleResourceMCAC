/*                                                            02.Feb.2010 v2.1
  ============================================================================

        PCMDEMO.C
        ~~~~~~~~~

        Description:
        ~~~~~~~~~~~~

        Example program for testing the correct implementation of the
        standard PCM filtering and up/down-sampling module.

        The test procedure of 3 filtering in series run on the user-
        specified speech file. After each filtering, the input for the
        next stage is taken as the output of the previous filter. For
        each stage, it may be chosen 4 options: no filtering ("short
        cut"), G712 filtering with no rate change (factor 1:1), or with
        decimation (factor 2:1), or with interpolation (factor 1:2).


        Usage:
        ~~~~~~
          $ PCMDEMO ! ---> HELP text is printed to screen
         or
          $ PCMDEMO [-options] ifile [ ofile typ1 typ2 typ3 [lseg]]
            ! (---> filtering is carried out)
         where:

        ifile: .. INPUT  FILE with short data (binary files)

        ofile: .. OUTPUT FILE with short data (binary files)

        typn: ... type of filtering for filtering stage n, n = 1..3:
                  1_1: input is at 16 kHz, output at 16 kHz
                  1_2: input is at  8 kHz, output at 16 kHz
                  2_1: input is at 16 kHz, output at  8 kHz
                  0  : short cut (no filtering at all!).

        lseg: ... number of samples per processing block
                  (default is LSEG0=256)

	Options:
	~~~~~~~~
	-skip no ... skips saving to file the first `no' processed samples 
	-lseg l .... defines as `l' the number of samples per processing block

        Compilation:
        ~~~~~~~~~~~

        The test program is implemented for segment-wise filtering. To
        test the  dependency of the segment length, the user must enter
        a value for the  segment length (from 1 ... LSEGMAX).

        VAX/VMS:
                 $ CC PCMDEMO
                 $ link PCMDEMO
                 $ PCMDEMO :== $pcmdemo_Disk:[pcmdemo_Dir]PCMDEMO
                 $ PCMDEMO ifile ofile 0 1_2 2_1 133

                   The data from file "ifile" are processed in the
                 following way, being equivalent to the asynchronous filtering
                 needed to simulating the return from digital to analogue
                 domains, and the way back:

                 1) segments with 133 samples are read from file "ifile"
                 2) the first filtering just keep the input data as is;
                 3) the second stage will take the data of the previous stage
                    as sampled at 8 kHz, filter by the std.PCM and
                    upsample to 16 kHz;
                 4) the output of the second stage is taken as 16 kHz data,
                    which is filtered by the std.PCM filter and down-sampled
                    back to 8 kHz.
                 5) the output of the third stage is rounded and saved to
                    file `ofile'.


        Turbo-C, Turbo-C++:
                 > tcc pcmdemo
                 > pcmdemo ifile ofile 1_1 1_1 2_1 1024

                   The data from file "ifile" are processed in the
                 following way:
                 1) segments with 1024 samples are read from file "ifile"
                 2) PCM filtering for 1st stage keeping the rate at 16 kHz;
                 3) the same for the second;
                 4) the output of the second step is then filtered and
                    down-sampled (decimated) to 8 kHz.
                 5) the output of the third filter is then
                    saved to file using rounding.


        HighC (MetaWare, version R2.32):
                 > hc386 -stack 16384 pcmdemo.c
                 > Run386 pcmdemo ifile ofile 2_1 1_2 0 133

        In our test some C-implementations have shown errors, which
        could be  eliminated by increasing the stack size (at compile
        time).


        SunC (BSD Unix)
                 # cc -o pcmdemo pcmdemo.c
                 # pcmdemo


        Original author:
        ~~~~~~~~~~~~~~~~

                Rudolf Hofmann
                Advanced Development Digital Signal Processing
                PHILIPS KOMMUNIKATIONS INDUSTRIE AG
                Kommunikationssysteme
                Thurn-und-Taxis-Strasse 14
                D-8500 Nuernberg 10 (Germany)

                Phone : +49 911 526-2603
                FAX   : +49 911 526-3385
                EMail : hf@pkinbg.uucp

        History:
        ~~~~~~~~
        16.Oct.91 v0.0 Release of beta version to UGST. <hf@pkinbg.uucp>
        26.Feb.92 v1.0 (ILS-files removed). <hf@pkinbg.uucp>
        18.May.92 v1.1 Use of sh2fl_16bit w/ file normalization.
                       <tdsimao@cpqd.ansp.br>
        30.Oct.94 v2.0 Change to encompass splitting of module into subunits
        02.Feb.10 v2.1 Modified maximum string length (y.hiwasaki)
  ============================================================================
*/

/*
 * ......... INCLUDES .........
 */

#include <stdio.h>		  /* UNIX Standard I/O Definitions */
#include <stdlib.h>   /* For allocation routine */
#include "ugstdemo.h"		  /* private defines for user interface */
#include "ugst-utl.h"		  /* conversion from float -> short */
#include "iirflt.h"		  /* Standard PCM filtering functions */

/*
 * ......... Definitions for this test program .........
 */

#define LSEG0    256	/* default segment length for segment-wise filtering */
#define LSEGMAX 2048	/* max. number of samples to be proc. */


/*
  ============================================================================

        void display_usage (void);
        ~~~~~~~~~~~~~~~~~~

        Description:
        ~~~~~~~~~~~~

        Display usage of this demo program and exit;

        Return value:
        ~~~~~~~~~~~~~
        Returns the number of longs read.

        Author: <hf@pkinbg.uucp>
        ~~~~~~~

        History:
        ~~~~~~~~
        26.Feb.92 v1.0 Release of 1st version <hf@pkinbg.uucp>
        30.Oct.94 v2.0 Revamped <simao@ctd.comsat.com>

 ============================================================================
*/
#define P(x) printf x
void display_usage()
{
  P(("PCMDEMO.C - Version 2.1 of 02.Feb.2010 \n\n"));
  P((" Run 3 filter in cascade with the G.712 (standard PCM) mask.\n"));
  P(("For each filter one can select between: \n"));
  P(("  - input signal with sf=16 kHz, output signal with 16 kHz\n"));
  P(("  - input signal with sf= 8 kHz, output signal with 16 kHz\n"));
  P(("  - input signal with sf=16 kHz, output signal with  8 kHz\n"));
  P(("  - short cut \n\n"));

  P(("Usage:\n"));
  P(("~~~~~~\n"));
  P((" $ PCMDEMO ! ---> HELP text is printed to screen\n"));
  P(("  or\n"));
  P((" $ PCMDEMO [-options] ifile [ ofile typ1 typ2 typ3 [lseg]]\n"));
  P((" where:\n"));
  P((" ifile: .. INPUT  FILE with short data (binary files)\n"));
  P((" ofile: .. OUTPUT FILE with short data (binary files)\n"));
  P((" typn: ... type of filtering for filtering stage n, n = 1..3:\n"));
  P(("           1_1: input is at 16 kHz, output at 16 kHz\n"));
  P(("           1_2: input is at  8 kHz, output at 16 kHz\n"));
  P(("           2_1: input is at 16 kHz, output at  8 kHz\n"));
  P(("           0  : short cut (no filtering at all!).\n"));
  P((" lseg: ... number of samples per processing block "));
  P(("(default is LSEG0=%d)\n", LSEG0));
  P(("\n"));
  P(("Options:\n"));
  P(("~~~~~~~~\n"));
  P((" -skip no ... skips saving to file the first `no' processed samples\n"));
  P((" -lseg l .... set `l' as the number of samples per processing block\n"));

  /* Quit program */
  exit(-128);
}
#undef P
/* ...................... End of display_usage() ........................... */



/* ......................... Begin of main() .............................. */
/*
 **************************************************************************
 **************************************************************************
 ***                                                                    ***
 ***        Test-Program for testing the correct implementation         ***
 ***               and to show how to use the programs                  ***
 ***                                                                    ***
 **************************************************************************
 **************************************************************************
*/
int main(argc, argv)
  int             argc;
  char           *argv[];
{
/*
 * ......... Define symbols of type  SCD_IIR for each filter .........
 */
  SCD_IIR        *typ1_ptr;
  SCD_IIR        *typ2_ptr;
  SCD_IIR        *typ3_ptr;

  /* ......... signal arrays ......... */
  short           sh_buff[8 * LSEGMAX];	/* 16-bit buffer */
  float           fl_buff[LSEGMAX];	/* float buffer */
#ifdef STATIC_ALLOCATION
  float           buff1[2 * LSEGMAX];	/* output of 1. filter */
  float           buff2[4 * LSEGMAX];	/* output of 2. filter */
  float           buff3[8 * LSEGMAX];	/* output of 3. filter */
#else
  float           *buff1;	/* output of 1. filter */
  float           *buff2;	/* output of 2. filter */
  float           *buff3;	/* output of 3. filter */
#endif

  /* ......... File related variables ......... */
  char            inpfil[MAX_STRLEN], outfil[MAX_STRLEN];
  FILE           *inpfilptr, *outfilptr;
#if defined(VMS)
  static char     mrs[15] = "mrs=512";
#endif

  /* ......... other auxiliary variables ......... */
  clock_t         t1, t2;	  /* aux. for CPU-time measurement */
  char            typ1[MAX_STRLEN], typ2[MAX_STRLEN], typ3[MAX_STRLEN];
  long            lseg=LSEG0, lsegx, lseg1, lseg2, lseg3;
  long            noverflows1 = 0, noverflows2 = 0, noverflows3 = 0;
  long            nsam = 0;
  long            k;
  long skip=0;

  /* ......... PRINT INFOS ......... */

  printf("%s%s", "*** V1.1 DEMO-Program: Standard G.712 Up/Down Sampling ",
                 "Filter  30-Oct-1994 ***\n");


/*
 * ......... PARAMETERS FOR PROCESSING .........
 */

  /* GETTING OPTIONS */

  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-skip") == 0)
      {
	/* No reset */
	skip = atoi(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-len") == 0)
      {
	lseg = atoi(argv[2]);
	/* If max.seg.length is exceeded, display warning */
	if (lseg > LSEGMAX)
	{
	  lseg = LSEGMAX;
	  fprintf(stderr, "Warning! lseg limited to max of %ld\n", lseg);
	}

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
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


  /* ......... GETTING PARAMETERS ......... */

  /* ......... GETTING PARAMETERS ......... */
  GET_PAR_S(1, "_BIN-File to be processed: ............... ", inpfil);
  if ((inpfilptr = fopen(inpfil, RB)) == NULL)
    HARAKIRI("\n   Error opening input file", 1);

  GET_PAR_S(2, "_BIN-Output File: ........................ ", outfil);
  if ((outfilptr = fopen(outfil, WB)) == NULL)
    HARAKIRI("\n   Error opening output file", 1);

  GET_PAR_S(3, "_type of filter 1: ....................... ", typ1);
  if ((strcmp(typ1, "0") != 0) && (strcmp(typ1, "1_1") != 0) &&
      (strcmp(typ1, "1_2") != 0) && (strcmp(typ1, "2_1") != 0))
    HARAKIRI("\n   undefined filter type (use 0, 1_1, 1_2 or 2_1)", 1);

  GET_PAR_S(4, "_type of filter 2: ....................... ", typ2);
  if ((strcmp(typ2, "0") != 0) && (strcmp(typ2, "1_1") != 0) &&
      (strcmp(typ2, "1_2") != 0) && (strcmp(typ2, "2_1") != 0))
    HARAKIRI("\n   undefined filter type (use 0, 1_1, 1_2 or 2_1)", 1);

  GET_PAR_S(5, "_type of filter 3: ....................... ", typ3);
  if ((strcmp(typ3, "0") != 0) && (strcmp(typ3, "1_1") != 0) &&
      (strcmp(typ3, "1_2") != 0) && (strcmp(typ3, "2_1") != 0))
    HARAKIRI("\n   undefined filter type (use 0, 1_1, 1_2 or 2_1)", 1);

  FIND_PAR_L(6, "_Segment Length for Filtering: ........... ", lseg, lseg);
  if (lseg > LSEGMAX)
  {
    /* If max.seg.length is exceeded, display warning */
    lseg = LSEGMAX;
    printf("\t\t\t\t(limited to %ld)\n", lseg);
  }

/*
   * ... ALLOCATE MEMORY FOR INTERMEDIATE AND FINAL DATA ...
   */
   if ((buff1 = (float *) calloc((long)(2*lseg), sizeof(float)))==NULL)
     HARAKIRI("Error allocating memory for 1st filtering step\n", 3);
   if ((buff2 = (float *) calloc((long)(4*lseg), sizeof(float)))==NULL)
     HARAKIRI("Error allocating memory for 2nd filtering step\n", 3);
   if ((buff3 = (float *) calloc((long)(8*lseg), sizeof(float)))==NULL)
     HARAKIRI("Error allocating memory for 3rd filtering step\n", 3);

/*
   * ... INITIALIZE SELECTED IIR-STRUCTURES FOR UP-/DOWNSAMPLING ...
   *       the types are as follows:
   *          1_1 : standard PCM with 16 kHz, no rate change (1:1)
   *          1_2 : standard PCM with 16 kHz, upsampling factor of 1:2
   *          2_1 : standard PCM with 16 kHz, downsampling factor of 2:1
   *       the return value is a pointer to filter coefficients and
   *       cleared state variables
   */

  /* ... for Filter #1 */
  if (strcmp(typ1, "1_1") == 0)
  {
    if ((typ1_ptr = stdpcm_16khz_init()) == 0)
      HARAKIRI("Filter 1: initialization failure stdpcm_16khz_init()", 1);
  }
  else if (strcmp(typ1, "1_2") == 0)
  {
    if ((typ1_ptr = stdpcm_1_to_2_init()) == 0)
      HARAKIRI("Filter 1: initialization failure stdpcm_1_to_2_init()", 1);
  }
  else if (strcmp(typ1, "2_1") == 0)
  {
    if ((typ1_ptr = stdpcm_2_to_1_init()) == 0)
      HARAKIRI("Filter 1: initialization failure stdpcm_2_to_1_init()", 1);
  }
  else
    typ1_ptr = (SCD_IIR *) NULL;  /* Init.the pointer to NULL */


  /* ... for Filter #2: */
  if (strcmp(typ2, "1_1") == 0)
  {
    if ((typ2_ptr = stdpcm_16khz_init()) == 0)
      HARAKIRI("Filter 2: initialization failure stdpcm_16khz_init()", 1);
  }
  else if (strcmp(typ2, "1_2") == 0)
  {
    if ((typ2_ptr = stdpcm_1_to_2_init()) == 0)
      HARAKIRI("Filter 2: initialization failure stdpcm_1_to_2_init()", 1);
  }
  else if (strcmp(typ2, "2_1") == 0)
  {
    if ((typ2_ptr = stdpcm_2_to_1_init()) == 0)
      HARAKIRI("Filter 2: initialization failure stdpcm_2_to_1_init()", 1);
  }
  else
    typ2_ptr = (SCD_IIR *) NULL;


  /* ... for Filter #3: */
  if (strcmp(typ3, "1_1") == 0)
  {
    if ((typ3_ptr = stdpcm_16khz_init()) == 0)
      HARAKIRI("Filter 3: initialization failure stdpcm_16khz_init()", 1);
  }
  else if (strcmp(typ3, "1_2") == 0)
  {
    if ((typ3_ptr = stdpcm_1_to_2_init()) == 0)
      HARAKIRI("Filter 3: initialization failure stdpcm_1_to_2_init()", 1);
  }
  else if (strcmp(typ3, "2_1") == 0)
  {
    if ((typ3_ptr = stdpcm_2_to_1_init()) == 0)
      HARAKIRI("Filter 3: initialization failure stdpcm_2_to_1_init()", 1);
  }
  else
    typ3_ptr = (SCD_IIR *) NULL;


/*
   * ......... CARRY OUT FILTERING .........
   */

  /* measure CPU-time */
  t1 = clock();

  lsegx = lseg;
  while (lsegx == lseg)
  {
    /* Read input buffer */
    lsegx = fread(sh_buff, sizeof(short), lseg, inpfilptr);

    /* convert short data to float in normalized range */
    sh2fl_16bit(lsegx, sh_buff, fl_buff, 1);


/*
   * WHAT TO DO IN FILTERING STAGE 1?
   */
    if (strcmp(typ1, "0") == 0)	  /* No filtering; so, ... */
    {
      /* ...copy input buffer to second step's buffer */
      for (k = 0; k < lsegx; k++)
	buff1[k] = fl_buff[k];
      /* ... and return no. of output samples */
      lseg1 = lsegx;
    }
    else
    {
      /* Standard PCM filtering */
      lseg1 =			  /* Returned: number of output samples */
	stdpcm_kernel(		  /* standard PCM filter */
		      lsegx,	  /* In   : number of input samples */
		      fl_buff,	  /* In   : array with input samples */
		      typ1_ptr,	  /* InOut: pointer to IIR struct */
		      buff1	  /* Out  : array with output samples */
	);

      /* Convert to integer for testing overflows -- do not save! */
      noverflows1 += fl2sh_16bit(lseg1, buff1, sh_buff, (int) 1);
    }

/*
   * WHAT TO DO IN FILTERING STAGE 2?
   */
    if (strcmp(typ2, "0") == 0)	  /* No filtering; so, ... */
    {
      /* ... copy input buffer to third step's buffer */
      for (k = 0; k < lseg1; k++)
	buff2[k] = buff1[k];
      /* ... and return number of output samples */
      lseg2 = lseg1;
    }
    else
    {
      /* Standard PCM filtering */
      lseg2 =			  /* Returned: number of output samples */
	stdpcm_kernel(		  /* standard PCM filter */
		      lseg1,	  /* In   : number of input samples */
		      buff1,	  /* In   : array with input samples */
		      typ2_ptr,	  /* InOut: pointer to IIR struct */
		      buff2	  /* Out  : array with output samples */
	);

      /* Convert to Integer for testing overflows -- do not save! */
      noverflows2 += fl2sh_16bit(lseg2, buff2, sh_buff, (int) 1);
    }

/*
   * WHAT TO DO IN FILTERING STAGE 3?
   */
    if (strcmp(typ3, "0") == 0)	  /* No filtering; so ... */
    {
      /* ... copy input buffer to output buffer */
      for (k = 0; k < lseg2; k++)
	buff3[k] = buff2[k];
      /* ... and return number of output samples */
      lseg3 = lseg2;
    }
    else
    {
      /* Standard PCM filtering */
      lseg3 =			  /* Returned: number of output samples */
	stdpcm_kernel(		  /* standard PCM filter */
		      lseg2,	  /* In   : number of input samples */
		      buff2,	  /* In   : array with input samples */
		      typ3_ptr,	  /* InOut: pointer to IIR struct */
		      buff3	  /* Out  : array with output samples */
	);
    }


/*
  * ......... CONVERTION FROM FLOAT TO SHORT with rounding .........
  *                      (now saves the data!)
  */
    noverflows3 += fl2sh_16bit(lseg3, buff3, sh_buff, (int) 1);

/*
  * ......... WRITE SAMPLES TO OUTPUT FILE .........
  */
    /* Skip samples if requested */
    if (lseg1 > skip)
    {
      /* Write samples to output file */
      nsam += fwrite(&sh_buff[skip], sizeof(short), (lseg3-skip), outfilptr);
      skip = 0;
    }
    else
      skip -= lseg1;
/*
    nsam += fwrite(sh_buff, sizeof(short), lseg3, outfilptr);
*/
  }


/*
   * ......... FINALIZATIONS .........
   */

  /* Print time statistics - Include file I/O! */
  t2 = clock();
  printf("\nDONE: %f sec CPU-time for %ld generated samples\n",
	 (t2 - t1) / (double) CLOCKS_PER_SEC, nsam);

  /* Print overflow statistics */
  if (noverflows1 == 0 && noverflows2 == 0 && noverflows3 == 0)
    printf("      no overflows occurred\n");
  else
  {
    printf("\t Overflows - filter 1: %ld\n", noverflows1);
    printf("\t           - filter 2: %ld\n", noverflows2);
    printf("\t           - filter 3: %ld\n", noverflows3);
  }

  /* Release memory allocated to IIR structures */
  if (typ3_ptr != (SCD_IIR *) NULL)
    stdpcm_free(typ3_ptr);
  if (typ2_ptr != (SCD_IIR *) NULL)
    stdpcm_free(typ2_ptr);
  if (typ1_ptr != (SCD_IIR *) NULL)
    stdpcm_free(typ1_ptr);

#ifndef STATIC_ALLOCATION
  /* Free memory of data vectors */
  free(buff3);
  free(buff2);
  free(buff1);
#endif

  /* Close files */
  fclose(outfilptr);
  fclose(inpfilptr);

#ifndef VMS
  return 0;
#endif
}
/* ......................... End of main() ......................... */


