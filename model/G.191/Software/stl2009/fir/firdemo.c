/*                                                            02.Feb.2010 v2.5
  ============================================================================

        FIRDEMO.C
        ~~~~~~~~~

        Description:
        ~~~~~~~~~~~~

        This test program carries out a series of 5 filterings
        by feeding ("piping") the output of a filtering stage to the input
        of the next one. This stages are, in order of implementation:

          1st. filter:  IRS Weighting Filter (none,8k,16k,16kMod,48kMod,16kHT)
          Intermediate: Delta-SM filtering (yes/no)
          2nd. filter:  Up-Sampling Filter   (factor 1, 2, 2b or 3)
          3rd. filter:  Up-Sampling Filter   (factor 1, 2, 2b, or 3)
          4th. filter:  Down-Sampling Filter (factor 1, 2, 2b, or 3)
          5th. filter:  Down-Sampling Filter (factor 1, 2, 2b, or 3)

          (NOTE: factor 1 means that no filtering is carried out)


        Usage:
        ~~~~~~
        $ firdemo !(---> HELP text is printed to screen)
        or
        $ firdemo [-options] ifile 
                      [ ofile irs delta_sm up1 up2 down1 down2 [lseg]]
                 (---> filtering is carried out, as specified)

        Where:
        ifile: .. input file with short data (binary files)
        ofile: .. output file with short data (binary files)

        irs: .... IRS filtering option (filtering #1):
                   0: no IRS filter
                   8: IRS-8kHz filter
                  16: IRS-16kHz (original|modified|half-tilt) filter
                  48: IRS-48kHz (modified) characteristic filter

        delta_sm: Delta-SM filtering option (complement to IRS):
                   0: skip Delta-SM filtering
                   1: cascade Delta-SM filtering
                   
        up1: .... upsampling option for filtering #2
                  1: no upsampling
                  2: upsampling by 2 
                 -2: upsampling by 2 using band-pass filter
                  3: upsampling by 3

        up2: .... upsampling option for filtering #3
                  1: no upsampling
                  2: upsampling by 2
                 -2: upsampling by 2 using band-pass filter
                  3: upsampling by 3

        down1: .. down-sampling option for filtering #4
                  1: no downsampling
                  2: downsampling by 2
                 -2: downsampling by 2 using band-pass filter
                  3: downsampling by 3

        down2: .. down-sampling option for filtering #5
                  1: no downsampling
                  2: downsampling by 2
                 -2: downsampling by 2 using band-pass filter
                  3: downsampling by 3

        lseg: ... number of samples per processing block
	          (default is 65)

	Options:
	~~~~~~~~
        -mod .... uses the modified IRS characteristic instead of the
                  "regular" one (if IRS was selected).
        -ht ..... uses the half-tilt IRS (if IRS was selected)
        -lseg ... changes the segment (block) length (default:LSEG0=256)
        -q ...... quiet processing (no progress flag)


        Compilation:
        ~~~~~~~~~~~~

        The test program is implemented for segment-wise filtering. To
        test the  dependency of the segment length, the user must enter
        a value for the  segment length (from 1 ... LSEGMAX). Examples
        follows:

        VAX/VMS:
                 $ CC firdemo
                 $ link firdemo
                 $ firdemo :== $firdemo_Disk:[firdemo_Dir]firdemo

        Turbo-C, Turbo-C++:
                 > tcc firdemo


        HighC (MetaWare, version R2.32):
                 > hc386 -stack 16384 firdemo.c

                 In our test some C-implementations have shown errors,
                 which can be eliminated by increasing the stack size
                 (at compile time).

        SunC (BSD Unix)
                 # cc -o firdemo firdemo.c
                 # firdemo


        Applications:
        ~~~~~~~~~~~~~

        1. Converting a file sampled at 16 kHz with a high-quality
        filtering and A/D system, to 8 KHz bandwidth, but making IRS
        weighting before downsampling: (eg, on the VAX)

                $ firdemo ifile ofile 16 1 1 1 2 133

                   The data from file "ifile" are processed in the
                 following way:
                 1) segments with 133 samples are read from file "ifile"
                 2) each segment is at first filtered with IRS send part
                    filter (coefficients for 16 kHz sampling frequency)
                 3) the output of the IRS-filter is passed through the
                    first up-sampling filter without filtering.
                 4) the output of the first up-sampling filter is passed
                    through the second up-sampling filter without filtering
                 5) the output of the second up-sampling filter is passed
                    through the first down-sampling filter without
                    filtering.
                 6) the output of the first down-sampling filter is fed
                    into the second down-sampling filter, which carries
                    out the down-sampling by a factor of 2.

        2. Dummy processing: upsamples by a factor of 6 times
        (8->48kHz), and downsamples back to 8 kHz: (eg, tcc)

                > firdemo ifile ofile 0 0 2 3 2 3 1024

                   The data from file "ifile" are processed in the
                 following way:
                 1) segments with 1024 samples are read from file "ifile"
                 2) the IRS send-part filter and Delta-SM are switched off.
                 3) the signal is then up-sampled by a factor of 2 in the
                    first up-sampling filter.
                 4) the output of the first up-sampling filter then up-
                    sampled by a factor of 3 in the second up-sampl. filter
                    -> total up-sampling factor = 6
                 5) the output of the second up-sampling filter is then
                    down-sampled by factor 2 in the first down-sampling
                    filter.
                 6) the output of the first down-sampling filter is fed
                    into the second down-sampling filter, which carries
                    out a down-sampling by a factor of 3.

        3. Upsampling a processed file from 8 kHz to 16 kHz, for
        listening purposes: (eg, using HighC)
                > run386 firdemo ifile ofile 0 0 2 0 0 0 256

        4. Upsampling a processed file from 8 kHz to 48 kHz, for
        recording in a DAT: (eg, using SunC)
                # firdemo ifile ofile 0 0 2 3 0 0 256

        5. Digitizing speech material recorded in a DAT, at 48 kHz,
        passing by the IRS at 16 kHz, and downsamplig to 8kHz:
                $ firdemo ifile tmpfile  0 0 0 0 0 3 256 ! tmpfile is at 16kHz
                $ firdemo tmpfile ofile 16 0 0 0 0 2 256 ! ofile is at 8kHz


        Original author:
        ~~~~~~~~~~~~~~~~

                Rudolf Hofmann
                Advanced Development Digital Signal Processing
                PHILIPS KOMMUNIKATIONS INDUSTRIE AG
                Kommunikationssysteme

        History:
        ~~~~~~~~
        16.Oct.91 v0.0 Release of beta version to UGST. <hf@pkinbg.uucp>
        26.Feb.92 v1.0 (ILS-files removed). <hf@pkinbg.uucp>
        18.May.92 v1.1 Use of sh2fl_16bit w/ file normalization.
                       <simao@cpqd.ansp.br>
        20.Apr.94 v1.2 Added new filtering options: modified IRS at 16kHz and 
                       48kHz, Delta-SM filtering, and band-pass (G.712-like)
                       2:1 and 1:2 filtering. <simao@cpqd.ansp.br>
        30.Sep.94 v2.0 Changes in the code/documentation to encompass changes
                       and spliting into several files of the old-name hq 
		       module, now FIRFLT. <simao@ctd.comsat.com>
        30.Oct.94 v2.1 Incorporated half-tilt IRS. <simao@ctd.comsat.com>
        06.Feb.96 v2.2 Corrected help message, included header string.h
        06.Jul.99 v2.3 Inserted conditional compilation for CYGWIN and 
                       MS Visual C compiler.
        02.Feb.10 v2.5 Modified maximum filename length (y.hiwasaki)
  ============================================================================
*/

/*
 * ......... INCLUDES .........
 */

/* Generic Includes */
#include <stdio.h>		  /* UNIX Standard I/O Definitions */
#include <stdlib.h>		  /* for atoi(), atol() */
#include <string.h>		  /* for str...() */
#include "ugstdemo.h"		  /* private defines for user interface */
#include "ugst-utl.h"		  /* conversion from float -> short */
#include "firflt.h"		  /* definitions for high quality filter */


/* Specific Includes */
#if defined(unix)
#include <malloc.h>		  /* For calloc() */
#elif defined(VMS) || defined (__STDC__)
#include <stdlib.h>
#elif defined(MSDOS) && ! defined(_MSC_VER)
#include <alloc.h>
#endif

/*
 * ......... LOCAL DEFINITIONS .........
 */

#define LSEG0    256		  /* default block length for segment-wise
				   * filtering */
#define LSEGMAX 2048		  /* max. number of samples to be proc. */


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

 ============================================================================
*/
#define P(x) printf x
void            display_usage()
{
  P(("\n Six filters are running in cascade: \n"));
  P(("\t1st. Filter:  IRS Send Part Filter (none, 8 kHz, 16, or 48 kHz)\n"));
  P(("\t2nd. Filter:  Delta-SM (16kHz only)\n"));
  P(("\t3rd. Filter:  Up-Sampling Filter   (factor 1, 2 or 3)\n"));
  P(("\t4th. Filter:  Up-Sampling Filter   (factor 1, 2 or 3)\n"));
  P(("\t5th. Filter:  Down-Sampling Filter (factor 1, 2 or 3)\n"));
  P(("\t6th. Filter:  Down-Sampling Filter (factor 1, 2 or 3)\n\n"));

  P((" Usage: $ FIRDEMO [-options] ifile [ofile irs dsm up1 up2 down1 down2 [lseg]]\n"));

  P(("   ifile:  INPUT  FILE with short data (binary files)\n"));
  P(("   ofile:  OUTPUT FILE with short data (binary files)\n\n"));

  P(("%s%s", "   irs  :  0: short cut\t 8: IRS-8kHz filter  \t ",
	 "16: IRS-16kHz filter\n          48: IRS-48kHz\n"));
  P(("%s",   "   dsm  :  0: short cut\t 1: use 16kHz Delta-SM filter\n"));
  P(("%s%s", "   up1  :  0: short cut\t 2: upsampling   by 2\t ",
	 "3:  upsampling by 3 \n"));
  P(("%s%s", "   up2  :  0: short cut\t 2: upsampling   by 2\t ",
	 "3:  upsampling by 3 \n"));
  P(("%s%s", "   down1:  0: short cut\t 2: downsampling by 2\t ",
	 "3:  downsampling by 3 \n"));
  P(("%s%s", "   down2:  0: short cut\t 2: downsampling by 2\t ",
	 "3:  downsampling by 3 \n"));
  P(("   lseg:   number of samples per processing block (default is %d)\n",
	 LSEG0));
  P(("   NOTE: if up?/down? above is -2, the filter used is the bandpass \n"));
  P(("         FIR filter (instead of the \"default\" lowpass FIR)\n"));

  P((" Options:\n"));
  P(("  -mod ....... uses the modified IRS characteristic instead of the\n"));
  P(("               \"regular\" one. For 16 and 48kHz.\n"));
  P(("  -ht ........ uses the half-tilt for 16 kHz IRS, if IRS filtering is selected\n"));
  P(("  -q ......... quiet processing (no progress flag)\n"));
  P(("  -lseg ...... changes the segment (block) length (default:%d)\n",LSEG0));

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
 * ......... Define symbols of type SCD_FIR for each filter .........
 */
  SCD_FIR        *irs_ptr;
  SCD_FIR	 *delta_sm_ptr;
  SCD_FIR        *up1_ptr;
  SCD_FIR        *up2_ptr;
  SCD_FIR        *down1_ptr;
  SCD_FIR        *down2_ptr;

  /* ......... signal arrays ......... */
#ifdef STATIC_ALLOCATION
  short           sh_buff[9 * LSEGMAX];		/* 16-bit buffer */
  float           fl_buff[LSEGMAX];		/* float buffer */
  float           irs_buff[LSEGMAX];		/* output of irs filter */
  float           up1_buff[3 * LSEGMAX];	/* output of first upsampling
						 * buffer */
  float           up2_buff[9 * LSEGMAX];	/* output of second
						 * upsampling buffer */
  float           down1_buff[9 * LSEGMAX];	/* output of first
						 * downsampling buf. */
  float           down2_buff[9 * LSEGMAX];	/* output of second
						 * downsampling buf. */
#else
  short          *sh_buff;	/* 16-bit buffer */
  float          *fl_buff;	/* float buffer */
  float          *irs_buff;	/* output of irs filter */
  float          *up1_buff;	/* output of first upsampling buffer */
  float          *up2_buff;	/* output of second upsampling buffer */
  float          *down1_buff;	/* output of first downsampling buf. */
  float          *down2_buff;	/* output of second downsampling buf. */
#endif

  /* ......... File related variables ......... */
  char            inpfil[MAX_STRLEN], outfil[MAX_STRLEN];
  FILE           *inpfilptr, *outfilptr;
  int             inp, out;
#if defined(VMS)
  char            mrs[15];
#endif

  /* ......... other auxiliary variables ......... */
  clock_t         t1, t2;	  /* aux. for CPU-time measurement */
  long            irs;
  long            delta_sm;
  long            up_1, up_2;
  long            down_1, down_2;
  long            lsegdown1, lsegdown2;
  long            lsegup1, lsegup2;
  long            lsegx, lsegirs, lseg=LSEG0;
  static long     noverflows0 = 0, noverflows1 = 0, noverflows2 = 0, 
                  noverflows3 = 0, noverflows4 = 0, noverflows5 = 0;
  long            nsam = 0;
  long            k;
  int		  quiet=0, modified_irs=0, half_tilt=0;


/*
  * ......... PRINT INFOS .........
  */

  printf("%s%s", "*** v2.5 FIR-Up/Down-Sampling ",
	 "and IRS Send Part Filter -  02.Feb.2010 ***\n");

/*
 * ......... PARAMETERS FOR PROCESSING .........
 */

  /* GETTING OPTIONS */

  /* If no pars. are specified, display usage and quit; else, parse args */
  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-ht") == 0)
      {
	/* Use half-tilt IRS */
	half_tilt = 1;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-mod") == 0)
      {
	/* Use modified IRS, when applicable */
	modified_irs = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-q") == 0)
      {
	/* Don't print progress indicator */
	quiet = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-lseg") == 0)
      {
	/* Set new segment length */
	lseg = atoi(argv[2]);

	/* Move argv over the option to the next argument */
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


/*
   * ......... GETTING PARAMETERS .........
   */
#ifdef VMS
  sprintf(mrs, "mrs=%d", 2 * 256);/* mrs definition for VMS */
#endif

  GET_PAR_S(1,  "_BIN-File to be filtered: ................ ", inpfil);
  if ((inpfilptr = fopen(inpfil, RB)) == NULL)
    HARAKIRI("Error opening input file\n", 1);
  inp = fileno(inpfilptr);

  GET_PAR_S(2,  "_BIN-Output File: ........................ ", outfil);
  if ((outfilptr = fopen(outfil, WB)) == NULL)
    HARAKIRI("Error opening output file\n", 1);
  out = fileno(outfilptr);

  GET_PAR_L(3,  "_IRS Filter       (0, 8, 16, 48): ........ ", irs);
  if ((irs != 0) && (irs != 8) && (irs != 16) && (irs != 48))
    HARAKIRI("wrong IRS filter\n", 1);

  GET_PAR_L(4,  "_Delta-SM filtering (0:skip, 1:apply): ... ", delta_sm);

  GET_PAR_L(5,  "_First  Up-Sampling   (0, 2, -2, 3): ..... ", up_1);
  if ((up_1 != 0) && (up_1 != 2) && (up_1 != -2) && (up_1 != 3))
    HARAKIRI("wrong upsampling factor\n", 1);

  GET_PAR_L(6,  "_Second Up-Sampling   (0, 2, -2,  3): .... ", up_2);
  if ((up_2 != 0) && (up_2 != 2) && (up_2 != -2) && (up_2 != 3))
    HARAKIRI("wrong upsampling factor\n", 1);

  GET_PAR_L(7,  "_First  Down-Sampling (0, 2, -2,  3): .... ", down_1);
  if ((down_1 != 0) && (down_1 != 2) && (down_1 != -2) && (down_1 != 3))
    HARAKIRI("wrong downsampling factor\n", 1);

  GET_PAR_L(8,  "_Second Down-Sampling (0, 2, -2,  3): .... ", down_2);
  if ((down_2 != 0) && (down_2 != 2) && (down_2 != -2) && (down_2 != 3))
    HARAKIRI("wrong downsampling factor\n", 1);

  FIND_PAR_L(9, "_Segment Length for Filtering: ........... ", lseg, lseg);
  if (lseg > LSEGMAX)
  {
    lseg = LSEGMAX;
    printf("\t\t\t\t(limited to %ld)\n", lseg);
  }


/*
   * ... Allocate memory ...
   */
  sh_buff = (short *)calloc(9l * lseg, sizeof(short));	
  fl_buff = (float *)calloc(lseg, sizeof(float));	
  irs_buff = (float *)calloc(lseg, sizeof(float));	
  up1_buff = (float *)calloc(3l * lseg, sizeof(float));	
  up2_buff = (float *)calloc(9l * lseg, sizeof(float));	
  down1_buff = (float *)calloc(9l * lseg, sizeof(float));
  down2_buff = (float *)calloc(9l * lseg, sizeof(float));

  if(!(sh_buff && fl_buff && irs_buff && up1_buff && up2_buff && 
       down1_buff && down2_buff))
    HARAKIRI("Error allocating memory for sample buffers\n",3);
   
/*
   * ... Initialize selected FIR-structures for up-/downsampling ...
   *       the types are as follows:
   *     irs:    8 -> irs filter for  8 kHz sampled data
   *            16 -> (original|modified|half-tilt) irs for 16 kHz sampled data
   *            48 -> modified irs filter for 48 kHz sampled data
   *     up_1:   2 -> up-sampling filter with factor 1:2
   *            -2 -> up-sampling filter with factor 1:2 for lin.-ph. band-pass
   *             3 -> up-sampling filter with factor 1:3
   *     up_2:   2 -> up-sampling filter with factor 1:2
   *            -2 -> up-sampling filter with factor 1:2 for lin.-ph. band-pass
   *             3 -> up-sampling filter with factor 1:3
   *     down_1: 2 -> down-sampling filter with factor 1:2
   *            -2 -> down-sampling filter with factor 1:2 for lin.-ph.bnd-pass
   *             3 -> down-sampling filter with factor 1:3
   *     down_2: 2 -> down-sampling filter with factor 1:2
   *            -2 -> down-sampling filter with factor 1:2 for lin.-ph.bnd-pass
   *             3 -> down-sampling filter with factor 1:3
   */

  /* Option for IRS-Filter selected? */
  if (irs == 8)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a 8 kHz IRS */
    if ((irs_ptr = irs_8khz_init()) == 0)
      HARAKIRI("irs_8khz initialization failure!\n", 1);

    /* Inhibit delta-sm: not available for 8kHz */
    delta_sm=0;
  }
  else if (irs == 16)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a modified, half-tilt or original 16 kHz IRS */
    if (modified_irs)
      irs_ptr = mod_irs_16khz_init();
    else if (half_tilt)
      irs_ptr = ht_irs_16khz_init();
    else
      irs_ptr = irs_16khz_init();
    if (irs_ptr  == 0)
      HARAKIRI("irs16_khz initialization failure!\n", 1);
  }
  else if (irs == 48)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a modified 16 kHz IRS */
    if ((irs_ptr = mod_irs_48khz_init()) == 0)
      HARAKIRI("mod_irs48_khz initialization failure!\n", 1);

    /* Inhibit delta-sm: not available for 48kHz */
    delta_sm=0;
  }
  else
  {
    irs_ptr = NULL;
  }


  /* Check whether to use the Delta-SM filtering */
  if (delta_sm)
  {
   if (( delta_sm_ptr = delta_sm_16khz_init()) == 0)
      HARAKIRI("delta_sm initialization failure!\n", 1);
  }
  else
    delta_sm_ptr = NULL;
  
  /* First Upsampling Procedure */
  if (up_1 == 2)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a upsampling factor of 2 */
    if ((up1_ptr = hq_up_1_to_2_init()) == 0)
      HARAKIRI("hq_up_1_to_2 initialization failure!\n", 1);
  }
  else if (up_1 == -2)
  {
    /* get pointer to a struct which contains bpf coefficients and cleared
     * state variables for a upsampling factor of 2 */
    if ((up1_ptr = linear_phase_pb_1_to_2_init()) == 0)
      HARAKIRI("linear-phase band-pass 1:2 initialization failure!\n", 1);
  }
  else if (up_1 == 3)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a upsampling factor of 3 */
    if ((up1_ptr = hq_up_1_to_3_init()) == 0)
      HARAKIRI("hq_up_1_to_3 initialization failure!\n", 1);
  }
  else
    up1_ptr = NULL;


  /* Second Upsampling Procedure */
  if (up_2 == 2)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a upsampling factor of 2 */
    if ((up2_ptr = hq_up_1_to_2_init()) == 0)
      HARAKIRI("hq_up_1_to_2 initialization failure!\n", 1);
  }
  else if (up_2 == -2)
  {
    /* get pointer to a struct which contains bpf coefficients and cleared
     * state variables for a upsampling factor of 2 */
    if ((up2_ptr = linear_phase_pb_1_to_2_init()) == 0)
      HARAKIRI("linear-phase band-pass 1:2 initialization failure!\n", 1);
  }
  else if (up_2 == 3)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a upsampling factor of 3 */
    if ((up2_ptr = hq_up_1_to_3_init()) == 0)
      HARAKIRI("hq_up_1_to_3 initialization failure!\n", 1);
  }
  else
    up2_ptr = NULL;


  /* First Downsampling Procedure */
  if (down_1 == 2)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a downsampling factor of 2 */
    if ((down1_ptr = hq_down_2_to_1_init()) == 0)
      HARAKIRI("hq_down_2_to_1 initialization failure!\n", 1);
  }
  else if (down_1 == -2)
  {
    /* get pointer to a struct which contains bpf coefficients and cleared
     * state variables for a downsampling factor of 2 */
    if ((down1_ptr = linear_phase_pb_2_to_1_init()) == 0)
      HARAKIRI("linear-phase band-pass 2:1 initialization failure!\n", 1);
  }
  else if (down_1 == 3)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a downsampling factor of 3 */
    if ((down1_ptr = hq_down_3_to_1_init()) == 0)
      HARAKIRI("hq_down_3_to_1 initialization failure!\n", 1);
  }
  else
    down1_ptr = NULL;


  /* Second Downsampling Procedure */
  if (down_2 == 2)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a downsampling factor of 2 */
    if ((down2_ptr = hq_down_2_to_1_init()) == 0)
      HARAKIRI("hq_down_2_to_1 initialization failure!\n", 1);
  }
  else if (down_2 == -2)
  {
    /* get pointer to a struct which contains bpf coefficients and cleared
     * state variables for a downsampling factor of 2 */
    if ((down2_ptr = linear_phase_pb_2_to_1_init()) == 0)
      HARAKIRI("linear-phase band-pass 2:1 initialization failure!\n", 1);
  }
  else if (down_2 == 3)
  {
    /* get pointer to a struct which contains filter coefficients and cleared
     * state variables for a downsampling factor of 3 */
    if ((down2_ptr = hq_down_3_to_1_init()) == 0)
      HARAKIRI("hq_down_3_to_1 initialization failure!\n", 1);
  }
  else
    down2_ptr = NULL;


/*
   * ... Print some infos ...
   */
   if (modified_irs) 
     fprintf(stderr, "Running modified IRS\n");
   if (half_tilt) 
     fprintf(stderr, "Running half-tilt IRS\n");
   if (down_1<0 || down_2<0 || up_1<0 || up_2 <0)
     fprintf(stderr, "Running flat FIR band-pass filter\n");

/*
  * ......... CARRY OUT FILTERING .........
  */

  /* Start measuring CPU-time -- includes file I/O! */
  t1 = clock();

  lsegx = lseg;
  while (lsegx == lseg)
  {
    /* Read data from file in a short array ... */
    lsegx = fread(sh_buff, sizeof(short), lseg, inpfilptr);

    /* ... and convert short to float, normalizing */
    sh2fl_16bit(lsegx, sh_buff, fl_buff, 1);

    /* 1ST STAGE: which kind of IRS filtering? (0=none / 8=8kHz / 16=16kHz) */
    if (irs == 0)
    {
      /* copy input buffer to 1st stage output (irs buffer) */
      for (k = 0; k <= lsegx - 1; k++)
	irs_buff[k] = fl_buff[k];
      lsegirs = lsegx;		  /* Returned: number of output samples */
    }
    else
    {
      /* Filter using the IRS coeffs. initialized before (8 or 16 kHz) */
      lsegirs =			  /* Returned: number of output samples */
	hq_kernel(		  /* IRS send part filter */
		  lsegx,	  /* In   : number of input samples */
		  fl_buff,	  /* In   : array with input samples */
		  irs_ptr,	  /* InOut: pointer to FIR struct */
		  irs_buff	  /* Out  : array with output samples */
	);

      /* Convert to integer for testing overflows -- do not save! */
      noverflows0 += fl2sh_16bit(lsegirs, irs_buff, sh_buff, (int) 1);
    }

    /* INTERMEDIATE STAGE: Apply Delta-SM? (0=no / 1=yes) */
    if (delta_sm)
    {
      /* Copy IRS-processed buffer back onto the original data buffer */
      for (k = 0; k < lsegirs; k++)
	fl_buff[k] = irs_buff[k];
	
      /* Apply filtering */
      lsegirs =			  /* Returned: number of output samples */
	hq_kernel(		  /* IRS send part filter */
		  lsegx,	  /* In   : number of input samples */
		  fl_buff,	  /* In   : array with input samples */
		  delta_sm_ptr,	  /* InOut: pointer to FIR struct */
		  irs_buff	  /* Out  : array with output samples */
	);

      /* Convert to integer for testing overflows -- do not save! */
      noverflows1 += fl2sh_16bit(lsegirs, irs_buff, sh_buff, (int) 1);
    }


    /* 2ND STAGE: First upsampling filter (0=none / +-2=1:2 / 3=1:3) */
    if (up_1 == 0)
    {
      /* copy irs buffer to 2nd stage output (up-sample buffer 1) */
      for (k = 0; k <= lsegirs - 1; k++)
	up1_buff[k] = irs_buff[k];
      lsegup1 = lsegirs;	  /* Returned: number of output samples */
    }
    else
    {
      lsegup1 =			  /* Returned: number of output samples */
	hq_kernel(		  /* (up-sampling filter) */
		  lsegirs,	  /* In   : number of input samples */
		  irs_buff,	  /* In   : array with input samples */
		  up1_ptr,	  /* InOut: pointer to FIR struct */
		  up1_buff	  /* Out  : array with output samples */
	);

      /* Convert to integer for testing overflows -- do not save! */
      noverflows2 += fl2sh_16bit(lsegup1, up1_buff, sh_buff, (int) 1);
    }


    /* THIRD STAGE: Second upsampling filter: (0=none / +-2=1:2 / 3=1:3) */
    if (up_2 == 0)
    {
      /* copy buffer up-sample buf.1 to 3rd stage output (up-sample buf.2) */
      for (k = 0; k <= lsegup1 - 1; k++)
	up2_buff[k] = up1_buff[k];
      lsegup2 = lsegup1;	  /* Returned: number of output samples */
    }
    else
    {
      lsegup2 =			  /* Returned: number of output samples */
	hq_kernel(		  /* (up-sampling filter) */
		  lsegup1,	  /* In   : number of input samples */
		  up1_buff,	  /* In   : array with input samples */
		  up2_ptr,	  /* InOut: pointer to FIR struct */
		  up2_buff	  /* Out  : array with output samples */
	);

      /* Convert to integer for testing overflows -- do not save! */
      noverflows3 += fl2sh_16bit(lsegup2, up2_buff, sh_buff, (int) 1);
    }


    /* FOURTH STAGE: First downsampling filter: (0=none / +-2=2:1 / 3=3:1) */
    if (down_1 == 0)
    {
      /* copy buffer up-sample buf.2 to 4th stage output (down-sample buf.1) */
      for (k = 0; k <= lsegup2 - 1; k++)
	down1_buff[k] = up2_buff[k];
      lsegdown1 = lsegup2;	  /* Returned: number of output samples */
    }
    else
    {
      lsegdown1 =		  /* Returned: number of output samples */
	hq_kernel(		  /* (down-sampling filter) */
		  lsegup2,	  /* In   : number of input samples */
		  up2_buff,	  /* In   : array with input samples */
		  down1_ptr,	  /* InOut: pointer to FIR struct */
		  down1_buff	  /* Out  : array with output samples */
	);

      /* Convert to integer for testing overflows -- do not save! */
      noverflows4 += fl2sh_16bit(lsegdown1, down1_buff, sh_buff, (int) 1);
    }


    /* FIFTH STAGE: Second downsampling filter: (0=none / +-2=2:1 / 3=3:1) */
    if (down_2 == 0)
    {
      /* copy buffer down-sample buf.1 to 5th stage output (down-sample
       * buf.2) */
      for (k = 0; k <= lsegdown1 - 1; k++)
	down2_buff[k] = down1_buff[k];
      lsegdown2 = lsegdown1;	  /* Returned: number of output samples */
    }
    else
    {
      lsegdown2 =		  /* Returned: number of output samples */
	hq_kernel(		  /* (down-sampling filter) */
		  lsegdown1,	  /* In   : number of input samples */
		  down1_buff,	  /* In   : array with input samples */
		  down2_ptr,	  /* InOut: pointer to FIR struct */
		  down2_buff	  /* Out  : array with output samples */
	);
    }


/*
  * ......... CONVERTION FROM FLOAT TO SHORT with rounding .........
  *                      (now saves the data!)
  */
    noverflows5 += fl2sh_16bit(lsegdown2, down2_buff, sh_buff, (int) 1);

/*
  * ......... WRITE SAMPLES TO OUTPUT FILE .........
  */
    nsam += fwrite(sh_buff, sizeof(short), lsegdown2, outfilptr);
  }


/*
   * ......... FINALIZATIONS .........
   */

  /* Print time statistics - Include file I/O! */
  t2 = clock();
  printf("\nDONE: %f sec CPU-time for %ld generated samples\n",
	 (t2 - t1) / (double) CLOCKS_PER_SEC, nsam);

  /* Print overflow statistics */
  if (noverflows0 == 0 && noverflows1 == 0 && noverflows2 == 0 && 
      noverflows3 == 0 && noverflows4 == 0 && noverflows5 == 0)
    printf("      no overflows occurred\n");
  else
  {
    printf("\tOverflows  - IRS filter ................: %ld\n", noverflows0);
    printf("\t           - Delta-SM filter ...........: %ld\n", noverflows1);
    printf("\t           - up-sampling filter 1 ......: %ld\n", noverflows2);
    printf("\t           - up-sampling filter 2 ......: %ld\n", noverflows3);
    printf("\t           - down-sampling filter 1 ....: %ld\n", noverflows4);
    printf("\t           - down-sampling filter 2 ....: %ld\n", noverflows5);
  }

  /* Release allocated FIR structures */
  if (down2_ptr != (SCD_FIR *) NULL)
    hq_free(down2_ptr);
  if (down1_ptr != (SCD_FIR *) NULL)
    hq_free(down1_ptr);
  if (up2_ptr != (SCD_FIR *) NULL)
    hq_free(up2_ptr);
  if (up1_ptr != (SCD_FIR *) NULL)
    hq_free(up1_ptr);
  if (irs_ptr != (SCD_FIR *) NULL)
    hq_free(irs_ptr);

  /* Release memory */
  free(down2_buff);
  free(down1_buff);
  free(up2_buff);	
  free(up1_buff);	
  free(irs_buff);	
  free(fl_buff);	
  free(sh_buff);	
  
  /* Close files */
  fclose(outfilptr);
  fclose(inpfilptr);

#ifndef VMS
  return (0);
#endif
}
/* ......................... End of main() ......................... */
