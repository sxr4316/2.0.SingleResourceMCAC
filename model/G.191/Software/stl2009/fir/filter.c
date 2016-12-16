/*                                                           02.Feb.2010 v3.5
  ===========================================================================

  FILTER.C
  ~~~~~~~~

  Description:
  ~~~~~~~~~~~~

  Test program to process a given fil by one of the possible filter
  characteristic available in the STL. Multiple filterings (as
  available in firdemo.c and pcmdemo.c) can be obtained by piping
  (cascading) several runs of this program. Asynchronous tandeming
  simulation is available for some types of filter; delay of the input
  file or skipping samples from the input file is also available with
  the async operation.

  Usage:
  ~~~~~~
  $ filter [-options] Flt_type InpFile OutFile 
           [BlockSize [1stBlock [NoOfBlocks]]]

  where:
  flt_type: 	is the filter type (see list below)
  InpFile       is the name of the file to be processed;
  OutFile       is the name with the processed data;
  BlockSize     is the block size, in number of samples
  1stBlock      is the number of the first block of the input file
                to be processed;
  NoOfBlocks    is the number of blocks to be processed, starting on
                block "1stBlock"
        
  Options:
  -mod .......... uses the modified IRS characteristic instead of the
                  "regular" one.
  -down ......... filtering is downsample (for HQ2, HQ3, FLAT, IFLAT, and PCM)
  -up ........... filtering is upsample (for HQ2, HQ3, FLAT, IFLAT, and PCM)
  -async ........ asynchronization operation (disables -down) \n"))  
  -delay d ...... number of samples to delay the input signal for 
                  asynchronous tandeming simulation. For d>0, null
                  samples are inserted in the begining of the file,
                  d<0 causes samples to be dropped. Default is d=0.
  -q ............ quiet processing (no progress flag)

  Valid filter specifications:
  Flt_type Description
   IRS8     (regular) IRS weighting with factor 1:1 at 8kHz
   IRS16    (regular or modified) IRS weighting with factor 1:1 at 16kHz
   IRS48    (modified) IRS weighting with factor 1:1 at 48kHz
   HIRS16   Half-tilt IRS weighting for data sampled at 16kHz, factor 1:1
   TIRS     IRS weighting with TIA coefficients, sf=8kHz, factor 1:1
   DSM      Delta-SM filtering characteristic, 1:1
   PSO      Psophometric wheighting filter, 1:1
   HQ2      FIR (High quality) low-pass with factor 1:2 (up) or 2:1 (down)
   HQ3      FIR (High quality) low-pass with factor 1:3 (up) or 3:1 (down)
   FLAT     Linear-phase pass-band with factor 1:2 (up) or 2:1 (down)
   FLAT1    Linear-phase pass-band with factor 1:1 (no rate change)
   PCM      Standard IIR PCM quality factor 1:2 (up) or 2:1 (down)
   PCM1     Standard PCM quality with factor 1:1 at 16 kHz
   GSM1     GSM Mobile station input response w/ factor 1:1 at 16 kHz
   P341     Send-part weighting of ITU-T Rec.P.341 (Wideband telephones)
   DC       Direct-form DC-removal IIR filter (factor 1:1)
   IFLAT    Flat IIR low-pass with factor 1:3 (up) or 3:1 (down) using a 
            cascade structure
   5KBP     50-5000 Hz Flat bandpass FIR filter for fs=16kHz, 1:1
   100_5KBP	100-5000 Hz Flat bandpass FIR filter for fs=16kHz, 1:1
   14KBP	50-14000 Hz Flat bandpass FIR filter for fs=32kHz, 1:1
   20KBP	20-20000 Hz Flat bandpass FIR filter for fs=48kHz, 1:1 (non-linear phase in HP-design)
   LP1p5	low-pass filter with cut-off frequency 1.5kHz for fs=48kHz, 1:1
   LP35		low-pass filter with cut-off frequency 3.5kHz for fs=48kHz, 1:1
   LP7		low-pass filter with cut-off frequency 7kHz for fs=48kHz, 1:1
   LP10		low-pass filter with cut-off frequency 10kHz for fs=48kHz, 1:1
 // FILTER_12k48k_HW
   LP12		low-pass filter with cut-off frequency 12kHz for fs=48kHz, 1:1
 //  FILTER_12k48k_HW
   LP14		low-pass filter with cut-off frequency 14kHz for fs=48kHz, 1:1
   LP20		low-pass filter with cut-off frequency 20kHz for fs=48kHz, 1:1
   RXIRS8   Receive-side Modified IRS weighting with factor 1:1 at 8kHz
   RXIRS16  Receive-side Modified IRS weighting with factor 1:1 at 16kHz


  Testing:
  ~~~~~~~~
  HPUX 10.20 gcc 2.95.2 (master)
  Dec/Ultrix 4.3 cc
  MSDOS Borland tcc++ 1.0, gcc 2.6.3/DJGPP
  Win95 gcc/Cygnus egcs.2.91.57
  SunOs 4.1 cc
   
  Original author:
  ~~~~~~~~~~~~~~~~
  Simao Ferraz de Campos Neto
  CPqD - Telebras                       Tel:    +55-192-39-6637
  Rod. Mogi Mirim - Campinas Km. 118,5  Fax:    +55-192-39-2179
  13.088-061 - Campinas - SP - Brazil   E-mail: simao@cpqd.ansp.br

  History:
  ~~~~~~~~
  30.Apr.1994 v1.0  Release of 1st version <tdsimao@venus.cpqd.ansp.br>
  23.Sep.1994 v2.0  Updated to accomodate changes in the name of the name and
                    splitting of module in several files, for ease of expansion.
  12.Oct.1994 v2.1  Added US-TIA IRS coefficients @ 8kHz and half-tilt IRS 
                    @16kHz. <simao>
  01.Jul.1995 v2.2  Added ITU-T P.341 send mask characteristic, cascade-form 
                    IIR 3:1/1:3 filter, direct-form DC-removal filter, and
                    asynchronization operation <simao@ctd.comsat.com>
  16.May.1997 v2.3  Added 50-5000 Hz band pass filter for signals sampled at
                    16 kHz, necessary for the wideband qualification tests
                    and added extension of the HQ band-pass filter to have
                    a 1:1 rate operation. Also changed the way some filter
                    type options are printed (rate change, mod.IRS,
                    FIR/IIR/...).
  02.Jan.1999 v3.0  - Added option to delay or skip samples in output file  
                      when the -async option is used. This is to
                      improve the asynchronization between input and
                      output files, in particular for the wideband case
                      when the IFLAT filter type is used. <simao>
                    - Corrected a bug to flag an error when modified
                      IRS is selected for 8 kHz sampling rate (filter
                      not available!). <simao>
                    - Found a memory leakage problem which happens for
                      downsampling operation by a factor of 3 when the
                      block size is not a multiple of 3, and fixed
                      using ceil() in the 4 places where the output
                      buffer size is calculated <simao>
  02.Jul.1999 v3.1  - Added GSM mobile station 16 kHz input high-pass
                      FIR filter 16 1:1 characteristic, after Kyrill
                      Fisher (DT/Berkom) (used in the ETSI AMR tests).
                      <simao>
  12.Jul.2000 v3.2  - Updated display_usage() for missing option. 
                    - Added hook to prevent selection of an FIR filter
                      with downsampling *and* block size N=1. In this
                      case, there is a bug in fir_downsampling_kernel() 
		      [in fir-lib.c] that causes the output file to be
                      one sample long. Until the function is
                      corrected, this operation is disabled in this
                      program <simao>.

   10.Feb.2005 v3.3  - The Bug in fir_downsampling_kernel() [in fir_lib.c]
					  has been corrected. The hook, to prevent selection of
					  an FIR filter with downsampling and block size N=1,
					  was removed.
					- Added 50-14000Hz bandpass filter
					- Added LP filters (3.5kHz, 7kHz, 10kHz) for fs=48kHz
					- Added 100-5000 Hz bandpass filter
						<Cyril Guillaume & Stephane Ragot - stephane.ragot@francetelecom.com>
   15.May.2007 v3.3++  Added 20Hz-20kHz bandpass filter
					- Added LP filters (1.5kHz,14kHz, 20kHz) for fs=48kHz 
				    < Ingemar Johansson & Jonas Svedberg , Ericsson>
   31.Dec.2008 v3.4 - Added LP filters (12kHz) for fs=48kHz 
				    < huawei >

   02.Feb.2010 v3.5 - Modified maximum string length for filenames to avoid
                      buffer overruns (y.hiwasaki)
  ===========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strncmp() */
#include <math.h>

#if defined(VMS)
#include <stat.h>
#else				/* Unix, DOS, etc */
#include <sys/stat.h>
#endif

/* UGST MODULES */
#include "ugstdemo.h"
#include "iirflt.h"
#include "firflt.h"
#include "ugst-utl.h"

/* LOCAL DEFINITIONS */
#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

/*
 * Check if the specified filter is a valid one!
 *   (potentially: does not check consistency of fs and F_type
 * By: Simao in 30.Apr.1992
 * Return: 1 -> OK
 *         0 -> invalid choice!
 * Last update: 11.May.2007 
 */
int             valid_filter(F_type, modified_IRS)
  char           *F_type, modified_IRS;
{
  int valid=0;

  if (strncmp(F_type, "irs", 3) == 0     || strncmp(F_type, "IRS", 3) == 0   
      || strncmp(F_type, "hirs", 4) == 0 || strncmp(F_type, "HIRS", 4) == 0
      || strncmp(F_type, "tirs", 4) == 0 || strncmp(F_type, "TIRS", 4) == 0
      || strncmp(F_type, "rxirs", 5) ==0 || strncmp(F_type, "RXIRS", 5) == 0 
      || strncmp(F_type, "dsm", 3) == 0  || strncmp(F_type, "DSM", 3) == 0
      || strncmp(F_type, "pso", 3) == 0  || strncmp(F_type, "PSO", 3) == 0
      || strncmp(F_type, "hq", 2) == 0   || strncmp(F_type, "HQ", 2) == 0
      || strncmp(F_type, "flat", 4) == 0 || strncmp(F_type, "FLAT",4) == 0
      || strncmp(F_type, "gsm1", 4) == 0 || strncmp(F_type, "GSM1",4) == 0
      || strncmp(F_type, "msin", 4) == 0 || strncmp(F_type, "MSIN",4) == 0
      || strncmp(F_type, "pcm", 3) == 0  || strncmp(F_type, "PCM",3) == 0
      || strncmp(F_type, "p341", 4) == 0 || strncmp(F_type, "P341",4) == 0
      || strncmp(F_type, "dc", 2) == 0   || strncmp(F_type, "DC",2) == 0
      || strncmp(F_type, "iflat", 5) ==0 || strncmp(F_type, "IFLAT",5) == 0
      || strncmp(F_type, "5kbp", 4) == 0 || strncmp(F_type, "5KBP",4) == 0   || strncmp(F_type, "5kBP",4) == 0 || strncmp(F_type, "5Kbp",4) == 0
      || strncmp(F_type, "100_5kbp", 8) == 0 || strncmp(F_type, "100_5KBP",8) == 0
	  || strncmp(F_type, "14kbp", 5) == 0 || strncmp(F_type, "14KBP",5) == 0 || strncmp(F_type, "14kBP",5) == 0 || strncmp(F_type, "14Kbp",5) == 0
	  || strncmp(F_type, "20kbp", 5) == 0 || strncmp(F_type, "20KBP",5) == 0 || strncmp(F_type, "20kBP",5) == 0 || strncmp(F_type, "20Kbp",5) == 0
	  || strncmp(F_type, "LP1P5", 5) == 0 || strncmp(F_type, "lp1p5",5) == 0 || strncmp(F_type, "LP1p5", 5) == 0 
      || strncmp(F_type, "LP35", 4) == 0 || strncmp(F_type, "lp35",4) == 0
	  || strncmp(F_type, "LP7", 3) == 0  || strncmp(F_type, "lp7",3) == 0
	  || strncmp(F_type, "LP10", 4) == 0 || strncmp(F_type, "lp10",4) == 0
// FILTER_12k48k_HW
	  || strncmp(F_type, "LP12", 4) == 0 || strncmp(F_type, "lp12",4) == 0
// FILTER_12k48k_HW
	  || strncmp(F_type, "LP14", 4) == 0 || strncmp(F_type, "lp14",4) == 0
	  || strncmp(F_type, "LP20", 4) == 0 || strncmp(F_type, "lp20",4) == 0
	  )
    valid = 1;
  
  /* No MOD-IRS filter at 8 kHz */
  if ((strncmp(F_type, "irs8", 4) == 0 || strncmp(F_type, "IRS8", 4) == 0) &&
      modified_IRS)
    valid=0;

  return(valid);
}


/*
 * Function to display usage
 * By: Simao in 1.May.1994
 * Last update: 15.May.2007 <>
 */
#define P(x) printf x
void display_usage()
{
  P(("FILTER.C - Version 3.5 of 02.Feb.2010 \n\n"));
 
  P((" Test program to process a given file by one of the possible filter\n"));
  P((" characteristics of the STL. Multiple filterings (as available\n"));
  P((" in firdemo.c and pcmdemo.c) can be obtained by piping (cascading) \n"));
  P((" several runs of this program. Asynchronous tandeming simulation \n"));
  P((" is available for some types of filter; delay of the input file \n"));
  P((" or skipping samples from the input file is also available with \n"));
  P((" the async operation.\n"));
  P(("\n"));
  P((" Usage:\n"));
  P((" $ filter   [-options] Flt_type InpFile OutFile \n"));
  P(("            [BlockSize [1stBlock [NoOfBlocks]]]\n"));
  P((" where:\n"));
  P(("  Flt_type:    is the filter type (see list below)\n"));
  P(("  InpFile      is the name of the file to be processed;\n"));
  P(("  OutFile      is the name with the processed data;\n"));
  P(("  BlockSize    is the block size, in number of samples\n"));
  P(("  1stBlock     is the number of the first block of the input file\n"));
  P(("               to be processed;\n"));
  P(("  NoOfBlocks   is the number of blocks to be processed, starting on\n"));
  P(("               block \"1stBlock\"\n"));
  P(("\n"));
  P((" Options:\n"));
  P(("  -mod ....... uses the modified IRS characteristic instead of the\n"));
  P(("               \"regular\" one for 16 & 48 kHz sampling.\n"));
  P(("  -up ........ upsampling filtering (for HQ2, HQ3, FLAT, IFLAT, and PCM)\n"));
  P(("  -down ...... downsampling filtering (HQ2, HQ3, FLAT, IFLAT and PCM)[default]\n"));
  P(("  -async ..... asynchronization operation (disables -down) \n"));
  P(("  -delay d ... number of samples to delay the input signal for \n"));
  P(("               asynchronous tandeming simulation. For d>0, null\n"));
  P(("               samples are inserted in the begining of the file,\n"));
  P(("               d<0 causes samples to be dropped. Default is d=0.\n"));
  P(("  -q ......... quiet processing (no progress flag)\n"));
  P(("\n"));
  P((" Valid filter specifications:\n"));
  P(("  Flt_type Description\n"));
  P(("   IRS8    (regular) IRS weighting with factor 1:1 at 8kHz\n"));
  P(("   IRS16   (regular or modified) IRS weighting with factor 1:1 at 16kHz\n"));
  P(("   IRS48   (modified) IRS weighting with factor 1:1 at 48kHz\n"));
  P(("   RXIRS8  Receive-side Modified IRS weighting with factor 1:1 at 8kHz\n"));
  P(("   RXIRS16 Receive-side Modified IRS weighting with factor 1:1 at 16kHz\n"));
  P(("   HIRS16  Half-tilt IRS weighting, sf=16kHz, factor 1:1.\n"));
  P(("   TIRS    IRS weighting w/ TIA coefficients, sf=8kHz, factor 1:1.\n"));
  P(("   DSM     Delta-SM filtering characteristic, 1:1\n"));
  P(("   PSO     Psophometric wheighting filter, 1:1\n"));
  P(("   HQ2     FIR (High quality) low-pass for factor 1:2 (up) or 2:1(down)\n"));
  P(("   HQ3     FIR (High quality) low-pass for factor 1:3 (up) or 3:1(down)\n"));
  P(("   FLAT    Linear-phase passband w/ factor 1:2 (up) or 2:1 (down)\n"));
  P(("   FLAT1   Linear-phase pass-band FIR w/ factor 1:1 (no rate change)\n"));
  P(("   PCM     Standard IIR PCM quality factor 1:2 (up) or 2:1 (down)\n"));
  P(("   PCM1    Standard PCM quality with factor 1:1 at 16 kHz\n"));
  P(("   GSM1    GSM Mobile station input FIR w/ factor 1:1 at 16 kHz\n"));
  P(("   MSIN    Same as GSM1\n"));
  P(("   P341    P.341 send-mask FIR with factor 1:1 at 16 kHz\n"));
  P(("   DC      Direct-form DC-removal IIR filter (factor 1:1)\n"));
  P(("   IFLAT   Cascade-form IIR flat low-pass with factor 1:3 (up) or 3:1 (down)\n"));
  P(("   5KBP    50-5k Hz Flat bandpass FIR filter w/ factor 1:1 at sf=16kHz\n"));
  P(("   100_5KBP 100-5k Hz Flat bandpass FIR filter w/ factor 1:1 at sf=16kHz\n"));
  P(("   14KBP   50-14k Hz Flat bandpass FIR filter w/ factor 1:1 at sf=32kHz\n"));
  P(("   20KBP   20-20k Hz Flat bandpass FIR filter w/ factor 1:1 at sf=48kHz\n"));
  P(("   LP1p5   1.5kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));
  P(("   LP35    3.5kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));
  P(("   LP7     7kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));
  P(("   LP10    10kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));
  P(("   LP12    12kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));
  P(("   LP14    14kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));
  P(("   LP20    20kHz low-pass filter for fs=48kHz, w/ factor 1:1\n"));

  P(("\n"));

  /* Quit program */
  exit(-128);
}
#undef P


/* Define filter type identifiers */
enum filter_kernel_type {FIR,IIR_PARALLEL, IIR_CASCADE, IIR_DIRECT};
char *filter_type_str[] = {"FIR", "Parallel-form IIR",
			   "Cascade-form IIR", "Direct-form IIR"};

/*============================== */
int main(argc, argv)
  int             argc;
  char           *argv[];
/*============================== */
{
  /* DECLARATIONS */

  /* Algorithm variables */
  SCD_FIR        *fir_state;
  SCD_IIR        *parallel_iir_state;
  CASCADE_IIR    *cascade_iir_state;
  DIRECT_IIR     *direct_iir_state;

  float          *InpBuff, *OutBuff;
  short			 *TmpBuff;
  char            F_type[MAX_STRLEN], async = 0, upsample = 0;
  long            cur_blk, satur = 0, total = 0, k, N, N1, N2;
  char            modified_IRS = 0, quiet = 0;
  long            inp_size, out_size, factor, smpno;
  double          fs=8000;
  char            kernel_type = 0;
  static char     funny[9] = "|/-\\|/-\\";

  /* For asynchronous tandem simulation */
  long            delay=0, skip=0;
  short          *zero;

  /* File variables */
  char            FileIn[MAX_STRLEN], FileOut[MAX_STRLEN];
  FILE           *Fi, *Fo;
  long            start_byte;
#ifdef VMS
  char            mrs[15];
#endif


  /* ......... GET PARAMETERS ......... */

  /* Check options */
  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1],"-mod")==0)
      {
	/* Set modified IRS flag */
	modified_IRS = 1;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp(argv[1], "-fs") == 0)
      {
	/* Change sampling frequency */
	fs = atof(argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp(argv[1], "-q") == 0)
      {
	/* Change sampling frequency */
	quiet = 1;

	/* Move arg{c,v} over the option to the next argument */
	argc --;
	argv ++;
      }
      else if (strcmp(argv[1], "-down") == 0)
      {
	/* Filtering is for downsampling */
	upsample = async = 0;

	/* Move arg{c,v} over the option to the next argument */
	argc --;
	argv ++;
      }
      else if (strcmp(argv[1], "-up") == 0)
      {
	/* Filtering is for upsampling */
	upsample = 1;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp(argv[1], "-async") == 0)
      {
	/* Filtering is an asyncronization process */
	async = upsample = 1;

	/* Move arg{c,v} over the option to the next argument */
	argc --;
	argv ++;
      }
      else if (strcmp(argv[1], "-delay") == 0)
      {
	/* Filtering is an asyncronization process */
	delay = atoi(argv[2]);
	if (delay<0)
	  skip = -delay;

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0)
      {
	/* Display help message */
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
  GET_PAR_S(1, "_Filter type: ................. ", F_type);
  GET_PAR_S(2, "_Input File: .................. ", FileIn);
  GET_PAR_S(3, "_Output File: ................. ", FileOut);
  FIND_PAR_L(4, "_Block Size: .................. ", N, 256);
  FIND_PAR_L(5, "_Starting Block: .............. ", N1, 1);
  FIND_PAR_L(6, "_No. of Blocks: ............... ", N2, 0);


  /* ......... CHECK CONSISTENCY ......... */

  /* Verify that a valid filter was selected */
  if (!valid_filter(F_type, modified_IRS))
  {
    if (modified_IRS &&
	(strcmp(F_type, "irs8")==0 || strcmp(F_type, "IRS8")==0))
      fprintf(stderr, "\nModified IRS is NOT available at 8 kHz! Aborted.\n");
    else
      fprintf(stderr, "\nInvalid filter chosen! Aborted.\n");
    exit(2);
  }

  /* The delay option is only available with asynchronous filtering */
  if (delay!=0 && !async)
    HARAKIRI("\nDelay option only available for ASYNC filtering! Aborted.\n",5);


  /* ......... STARTING ......... */

  /* Find starting byte in file */
  start_byte = sizeof(short) * (long) (--N1) * (long) N;

#ifdef SKIP_APPROACH_1
  /* If samples are to be skipped in output file, does it here */
  if (skip)
    start_byte += skip * sizeof(short);
#endif

  /* Check if is to process the whole file */
  if (N2 == 0)
  {
    struct stat     st;

    /* ... find the input file size ... */
    stat(FileIn, &st);
    N2 = ceil((st.st_size - start_byte) / (double)(N * sizeof(short)));
  }
  inp_size = N; /* samples */


  /* Allocate memory for delay buffer & initialize it */
  if (delay>0)
  {
    if ((zero=(short *)calloc(delay, sizeof(short)))==NULL)
    {
      HARAKIRI("Error allocating memory for delay buffer\n", 5);
    }    
    else
      memset(zero, 0, delay * sizeof(short));
  }

  /* Set flag to filter type: IIR or FIR; default is FIR */
  if (strncmp(F_type, "dc", 2) == 0 || strncmp(F_type, "DC",2) == 0)
    kernel_type = IIR_DIRECT;
  else if (strncmp(F_type, "iflat", 5) == 0||strncmp(F_type, "IFLAT", 5) == 0)
    kernel_type = IIR_CASCADE;
  else if (strncmp(F_type, "pcm", 3) == 0 || strncmp(F_type, "PCM", 3) == 0)
    kernel_type = IIR_PARALLEL;
  else
    kernel_type = FIR;


  /* ... CHOOSE CORRECT FILTER INITIALIZATION ... */

/*
  * Filter type: IRS - IRS weighting 1:1 factor:
  *                     . IRS8    -> factor: 1:1 (regular)
  *                     . IRS16   -> factor: 1:1 (regular | modified)
  *                     . IRS48   -> factor: 1:1 (modified)
  *                     . HIRS16  -> factor: 1:1
  *                     . TIRS    -> factor: 1:1
  *                     For FUTURE implementations:
  *                     . RXIRS8  -> factor: 1:1 (modified) NOT IMPLEMENTED
  *                     . RXIRS16 -> factor: 1:1 (modified) NOT IMPLEMENTED
  */
  if (strncmp(F_type, "irs", 3) == 0 || strncmp(F_type, "IRS", 3) == 0)
  {
    k = atoi(&F_type[3]);
    switch(k)
    {
    case 8:
      fir_state = irs_8khz_init();
      break;
    case 16:
      fir_state = modified_IRS
                   ? mod_irs_16khz_init()
      		   : irs_16khz_init() ;
      break;
    case 48:
      fir_state = mod_irs_48khz_init();
      break;
    default:
      HARAKIRI("Unimplemented: IRS rate not 8, 16 or 48 kHz\n", 15);
    }
  }
  if (strncmp(F_type, "rxirs", 5) == 0 || strncmp(F_type, "RXIRS", 5) == 0)
  {
    k = atoi(&F_type[5]);
    switch(k)
    {
    case 8:
      modified_IRS = 1; /* Only modified IRS rcx filter available */
      fir_state = rx_mod_irs_8khz_init();
      break;
    case 16:
      modified_IRS = 1; /* Only modified IRS rcx filter available */
      fir_state = rx_mod_irs_16khz_init();
      break;
    default:
      HARAKIRI("Unimplemented: Receive Mod-IRS rate not 8 or 16 kHz\n", 15);
    }
  }
  else if (strncmp(F_type, "hirs16", 6) == 0 || strncmp(F_type, "HIRS16", 6) == 0)
  {
      fir_state = ht_irs_16khz_init();
  }

  else if (strncmp(F_type, "tirs", 4) == 0 || strncmp(F_type, "TIRS", 4) == 0)
  {
      fir_state = tia_irs_8khz_init();
  }

/*
  * Filter type: DSM - Delta-SM: factor 1:1
  */
  else if (strncmp(F_type, "dsm", 3) == 0 || strncmp(F_type, "DSM", 3) == 0)
  {
      fir_state = delta_sm_16khz_init();
  }

/*
  * Filter type: PSO - Psophometric wheighting filter: factor 1:1 
  */
  else if (strncmp(F_type, "pso", 3) == 0 || strncmp(F_type, "PSO", 3) == 0)
  {
      fir_state = psophometric_8khz_init();
  }

/*
  * Filter type: GSM Mobile Station Input - Linear-phase, high-band
  * 1:1 factor added by Simao Campos after Kyrill Fisher [27/Feb/98]
  */
    else if (strncmp(F_type, "gsm1", 4) == 0 || 
	     strncmp(F_type, "GSM1", 4) == 0 ||
	     strncmp(F_type, "msin", 4) == 0 || 
	     strncmp(F_type, "MSIN", 4) == 0)
    {
      fir_state = msin_16khz_init();
    }    

/*
  * Filter type: FLAT - Linear-phase, pass-band 1:1, 2:1 or 1:2 factor:
  *                    . fs ==  8000 -> upsample: 1:2
  *                    . fs == 16000 -> downsample: 2:1, or
  *                                     keep rate: 1:1 (treated first)
  */
  else if (strncmp(F_type, "flat", 4) == 0 || strncmp(F_type, "FLAT", 4) == 0)
  {
      fir_state = F_type[4] == '1'
	          ? linear_phase_pb_1_to_1_init()
	          : (upsample
                     ? linear_phase_pb_1_to_2_init()
                     : linear_phase_pb_2_to_1_init());
  }

/*
  * Filter type: HQ2 - High quality 2:1 or 1:2 factor:
  *                    . fs ==  8000 -> upsample: 1:2
  *                    . fs == 16000 -> downsample: 2:1
  *              HQ3 - High quality 3:1 or 3:1 factor
  *                    . fs ==  8000 -> upsample: 1:3
  *                    . fs == 16000 -> downsample: 3:1
  */
  else if (strncmp(F_type, "hq", 2) == 0 || strncmp(F_type, "HQ", 2) == 0)
  {
    if (upsample)		/* It is up-sampling! */
      fir_state = F_type[2] == '2'
	? hq_up_1_to_2_init()
	: hq_up_1_to_3_init();
    else			/* It is down-sampling! */
      fir_state = F_type[2] == '2'
	? hq_down_2_to_1_init()
	: hq_down_3_to_1_init();
  }

/*
  * Filter type: P.341 send mask: factor 1:1
  */
  else if (strncmp(F_type, "p341", 4) == 0 || strncmp(F_type, "P341", 4) == 0)
  {
      fir_state = p341_16khz_init();
  }

/*
  * Filter type: 50-5000 Hz bandpass filter: factor 1:1
  */
  else if (strncmp(F_type, "5kbp", 4) == 0 || strncmp(F_type, "5KBP", 4) == 0 || strncmp(F_type, "5Kbp", 4) == 0 || strncmp(F_type, "5kBP", 4) == 0)
  {
      fir_state = bp5k_16khz_init();
  }

/*
  * Filter type: 100-5000 Hz bandpass filter: factor 1:1
  */
  else if (strncmp(F_type, "100_5kbp", 8) == 0 || strncmp(F_type, "100_5KBP", 8) == 0)
  {
      fir_state = bp100_5k_16khz_init();
  }

/*
  * Filter type: 50-14000 Hz bandpass filter (fs=32kHz): factor 1:1
  */
  else if (strncmp(F_type, "14kbp", 5) == 0 || strncmp(F_type, "14KBP", 5) == 0 || strncmp(F_type, "14Kbp", 5) == 0 || strncmp(F_type, "14kBP", 5) == 0)
  {
      fir_state = bp14k_32khz_init();
  }

  /*
  * Filter type: 20-20000 Hz bandpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "20kbp", 5) == 0 || strncmp(F_type, "20KBP", 5) == 0 || strncmp(F_type, "20Kbp", 5) == 0 || strncmp(F_type, "20kBP", 5) == 0)
  {
      fir_state = bp20k_48khz_init();
  }

/*
  * Filter type: 1.5kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP1p5", 5) == 0 || strncmp(F_type, "lp1p5", 5) == 0 || strncmp(F_type, "LP1p5", 5) == 0)
  {
      fir_state = LP1p5_48kHz_init();
  }

/*
  * Filter type: 3.5kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP35", 4) == 0 || strncmp(F_type, "lp35", 4) == 0)
  {
      fir_state = LP35_48kHz_init();
  }
/*
  * Filter type: 7kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP7", 3) == 0 || strncmp(F_type, "lp7", 3) == 0)
  {
      fir_state = LP7_48kHz_init();
  }
/*
  * Filter type: 10kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP10", 5) == 0 || strncmp(F_type, "lp10", 5) == 0)
  {
      fir_state = LP10_48kHz_init();
  }
// FILTER_12k48k_HW
  /*
  * Filter type: 12kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP12", 4) == 0 || strncmp(F_type, "lp12", 4) == 0)
  {
      fir_state = LP12_48kHz_init();
  }
// FILTER_12k48k_HW
/*
  * Filter type: 14kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP14", 4) == 0 || strncmp(F_type, "lp14", 4) == 0)
  {
      fir_state = LP14_48kHz_init();
  }

/*
  * Filter type: 20kHz lowpass filter (fs=48kHz): factor 1:1
  */
  else if (strncmp(F_type, "LP20", 4) == 0 || strncmp(F_type, "lp20", 4) == 0)
  {
      fir_state = LP20_48kHz_init();
  }

/*
  * Filter type: PCM  - Standard PCM quality 2:1 or 1:2 factor:
  *                    . fs ==  8000 -> upsample: 1:2
  *                    . fs == 16000 -> downsample: 2:1
  *              PCM1 - Standard PCM quality with 1:1 factor
  *                    . fs ==  8000 -> unimplemented
  *                    . fs == 16000 -> OK, 1:1 at 16 kHz
  */
  else if (strncmp(F_type, "pcm", 3) == 0 || strncmp(F_type, "PCM", 3) == 0)
  {
    if (strncmp(F_type, "pcm1", 4) == 0 || strncmp(F_type, "PCM1", 4) == 0)
    {
	parallel_iir_state = stdpcm_16khz_init();
    }
    else
      parallel_iir_state = upsample
	? stdpcm_1_to_2_init()	/* It is up-sampling! */
	: stdpcm_2_to_1_init();	/* It is down-sampling! */
  }

/*
  * Filter type: IFLAT - cascade-form IIR flat low-pass with 1:3
  *                    . fs == 16000 -> upsample: 1:3
  *                    . fs == 48000 -> downsample: 3:1
  */
  else if (strncmp(F_type, "iflat", 5) == 0 || 
	   strncmp(F_type, "IFLAT", 5) == 0)
  {
      cascade_iir_state = upsample
	? iir_casc_lp_1_to_3_init()  	/* It is up-sampling! */
	: iir_casc_lp_3_to_1_init();	/* It is down-sampling! */
  }

/*
  * Filter type: DC - IIR DC removal filter (a la RPE-LTP)
  */
  else if (strncmp(F_type, "dc", 2) == 0 || 
	   strncmp(F_type, "DC", 2) == 0)
  {
      direct_iir_state = iir_dir_dc_removal_init();
  }



  /* MEMORY ALLOCATION */

  /* Calculate Output buffer size and rate change factor */
  switch (kernel_type)
  {
  case FIR:
    factor = fir_state->dwn_up;
    out_size = (fir_state->hswitch=='U')
	       ? inp_size * factor
	       : ceil(inp_size / (double)factor); 
    break;
  case IIR_PARALLEL:
    factor = parallel_iir_state->idown;
    out_size = (parallel_iir_state->hswitch=='U')
	       ? inp_size * factor
	       : ceil(inp_size / (double)factor); 
    break;
  case IIR_CASCADE:
    factor = cascade_iir_state->idown;
    out_size = (cascade_iir_state->hswitch=='U')
	       ? inp_size * factor
	       : ceil(inp_size / (double)factor); 
    break;
  case IIR_DIRECT:
    factor = direct_iir_state->idown;
    out_size = (direct_iir_state->hswitch=='U')
	       ? inp_size * factor
	       : ceil(inp_size / (double)factor);
  }

  /* Check consistency once more */
  if (async && factor==1)
    HARAKIRI("INCONSISTENCY: async operation requires non-unity upsampling factor; aborting\n",10);

  /* Allocate memory for float input buffer */
  if ((InpBuff = (float *) calloc(inp_size, sizeof(float))) == NULL)
    HARAKIRI("Can't allocate memory for input data buffer\n", 10);

  /* Allocate memory for float output buffer */
  if ((OutBuff = (float *) calloc(out_size, sizeof(float))) == NULL)
    HARAKIRI("Can't allocate memory for output data buffer\n", 10);

  /* Allocate memory for short input/output buffer */
  if ((TmpBuff = (short *) calloc(max(inp_size, out_size), 
                                  sizeof(short))) == NULL)
    HARAKIRI("Can't allocate memory for short data buffer\n", 10);


/*
 * ......... PRINT INFO .......... 
 */
  if (factor==1)
    fprintf(stderr, "No-rate change operation\n");
  else
  {
    fprintf(stderr, "%s operation, ", async? "Asynchronization" : 
	    (upsample? "Upsampling" : "Downsampling"));
    fprintf(stderr, "factor %ld\n", async? 1l : factor);
  }
  if (modified_IRS)
    fprintf(stderr, "Using modified IRS\n");

  if (delay>0)
    fprintf(stderr, "Delaying output file by %ld samples\n", delay);
  else if (skip)
    fprintf(stderr, "Skipping %ld samples in output file\n", skip);

  fprintf(stderr, "Filter structure: %s\n", filter_type_str[(int)kernel_type]);


/*
 * ......... FILE PREPARATION .........
 */

  /* Handle VMS ideosyncrasies... */
#ifdef VMS
  sprintf(mrs, "mrs=%d", 2 * N);
#endif

  /* Opening input file; abort if there's any problem */
  if ((Fi = fopen(FileIn, RB)) == NULL)
    KILL(FileIn, 2);

  /* Creates output file */
  if ((Fo = fopen(FileOut, WB)) == NULL)
    KILL(FileOut, 3);

  /* Move pointer to 1st block of interest */
  if (fseek(Fi, start_byte, 0))
    KILL(FileIn, 4);


  /* FILTERING OPERATION! */

  /* One-time delay of output signal, if appropriate */
  if (async && delay>0)
    if ((smpno=fwrite(zero, sizeof(short), delay, Fo))==0 && ferror(Fo))
      KILL(FileOut, 6);

  /* Process regular frames */
  for (cur_blk = 0; cur_blk < N2; cur_blk++)
  {
    /* Print progress info */
    if (!quiet)
      fprintf(stderr, "%c\r", funny[cur_blk%8]);

    /* Reset output buffer */
    memset(OutBuff, '\0', out_size*sizeof(float));

    /* Read a block of samples */
    if ((smpno = fread(TmpBuff, sizeof(short), N, Fi)) == 0)
      KILL(FileIn, 5);

    /* ... and convert short to float, normalizing */
    sh2fl_16bit(smpno, TmpBuff, InpBuff, 1);

    /* Call the filtering routine */
    switch (kernel_type)
    {
    case FIR:
      smpno = hq_kernel(smpno, InpBuff, fir_state, OutBuff);
      break;
    case IIR_PARALLEL:
      smpno = stdpcm_kernel(smpno, InpBuff, parallel_iir_state, OutBuff);
      break;
    case IIR_CASCADE:
      smpno = cascade_iir_kernel(smpno, InpBuff, cascade_iir_state, OutBuff);
      break;
    case IIR_DIRECT:
      smpno = direct_iir_kernel(smpno, InpBuff, direct_iir_state, OutBuff);
      break;
    }

    /* Decimates to implement asynchronization process */
    if (async)
    {
      long k;

      /* Decrease output vector by `factor' */
      smpno /= factor;

      /* Shift samples implementing decimation process */
      for (k=0; k<smpno; k++)
	OutBuff[k] = OutBuff[k*factor];
    }

    /* Convert the filtered data back to short */
    satur += fl2sh_16bit(smpno, OutBuff, TmpBuff, (int) 1);

    /* Save to file, skipping any samples if necessary */
    if (skip >= smpno)
    {
      skip -= smpno; 
      continue;
    }
    else if (skip>0)
    {
      if ((smpno=fwrite(&TmpBuff[skip],sizeof(short),(smpno-skip),Fo))==0 &&
	  ferror(Fo))
	KILL(FileOut, 6);
      total += smpno;
      skip=0;
    }
    else
    {
      if ((smpno=fwrite(TmpBuff, sizeof(short), smpno, Fo))==0 && ferror(Fo))
	KILL(FileOut, 6);
      total += smpno;
    }
  }


  /* FINALIZATIONS */
  fprintf(stderr, "\n");

  /* Close open files */
  fclose(Fi);
  fclose(Fo);

  /* Release some memory */
  free(TmpBuff);
  free(OutBuff);
  free(InpBuff);

  /* Release filter structrues */
  switch (kernel_type)
  {
  case FIR:
    hq_free(fir_state);
    break;
  case IIR_PARALLEL:
    stdpcm_free(parallel_iir_state);
    break;
  case IIR_CASCADE:
    cascade_iir_free(cascade_iir_state);
    break;
  case IIR_DIRECT:
    direct_iir_free(direct_iir_state);
    break;
  }

  /* Release memory for delay buffer */
  if (delay>0)
    free(zero);

#ifndef VMS
  return(0);
#endif
}
