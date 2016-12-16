/*
  ----------------------------------------------------------------------

        FLTRESP.C
        ~~~~~~~~~

        Description:
        ~~~~~~~~~~~~

        Test program to evaluate the frequence response of filters.
        Depending on the function called, the program will be able
        to evaluate the frequence response for the range of
        frequencies specified. The algorithm here is to calculate the
        long-term energy of a sinewave before and after the filtering,
        converting the ratio to dB.

	--------------------------------------------------------------
        NOTE! the output dB values are ALWAYS relative to the given
        sampling frequency, not to the output one, whatever it is !!!
	--------------------------------------------------------------

        Compilation:
        ~~~~~~~~~~~~
        * VMS:
          $ cc fltresp.C
          $ link fltresp                      ! it is necessary to
          $ fltresp:==$disk:[dir]fltresp.EXE ! define as foreign command
	    * MSDOS:
          $ tcc fltresp.C
        * Unix:
          # cc -o fltresp fltresp.c -lm

        Usage:
        ~~~~~~
        $ fltresp Flt_type f0 ff fstep [fs]

        where:
        flt_type: 	is the filter type:
                        IRS, DSM, PSO, HQ2, HQ3, PCM, PCM1
	f0 		is the starting frequency [Hz]
        ff 		is the final frequency [Hz]
        fstep 		is the step in frequency from f0 to ff [Hz]
        [fs]		is the sampling frequency [Hz]; default is 8000 Hz.
        (*) may be the regular or the modified IRS!
        
        Options:
        -mod .......... uses the modified IRS characteristic instead of the
                        "regular" one.
        -fs ........... set the sampling frequency, in Hz [def: 8000]

	Valid combinations of filter and sampling rate:
		
	Flt_type   fs	Description
	  IRS     8000  (regular) IRS weighting with factor 1:1.
                 16000  (regular or modified) IRS weighting with factor 1:1.
                 48000  (modified) IRS weighting with factor 1:1.
          DSM    16000  Delta-SM filtering characteristic, 1:1
          PSO     8000  Psophometric wheighting filter, 1:1
          HQ2     8000  High quality with factor 1:2
                 16000  High quality with factor 2:1
          HQ3     8000  High quality with factor 1:3
                 16000  High quality with factor 3:1
          FLAT    8000  Linear-phase pass-band with factor 1:2
                 16000  Linear-phase pass-band with factor 2:1
	  PCM     8000  Standard PCM quality factor 1:2
                 16000  Standard PCM quality factor 2:1
          PCM1    8000  unimplemented!
                 16000  Standard PCM quality with factor 1:1 at 16 kHz

	Original author:
	~~~~~~~~~~~~~~~~
	Simao Ferraz de Campos Neto
        CPqD - Telebras                       Tel:    +55-192-39-6637
        Rod. Mogi Mirim - Campinas Km. 118,5  Fax:    +55-192-39-2179
        13.088-061 - Campinas - SP - Brazil   E-mail: simao@cpqd.ansp.br

        History:
        ~~~~~~~~
        30.Apr.92  1.0  Release of 1st version <tdsimao@venus.cpqd.ansp.br>
        20.May.92  2.0  Corrected bug in calculation of output level;
                        skipping start/stop of filtered sample buffer.
                        <tdsimao@venus.cpqd.ansp.br>
        20.Apr.94  3.0  Now displays tool information (previously not logged)
        02.Feb.10  3.1  Modified maximum string length for filenames (y.hiwasaki)

* ----------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memset() */
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


/* Other stufs */
#define TWO_PI (8*atan(1.0))


/*
 * Check if the specified filter is a valid one!
 *   (potentially: does not check consistency of fs and F_type
 * By: Simao in 30.Apr.92
 * Return: 1 -> OK
 *         0 -> invalid choice!
 */
int             valid_filter(F_type)
  char           *F_type;
{
  if (strncmp(F_type, "irs", 3) == 0 || strncmp(F_type, "IRS", 3) == 0 ||
      strncmp(F_type, "dsm", 3) == 0 || strncmp(F_type, "DSM", 3) == 0 ||
      strncmp(F_type, "pso", 3) == 0 || strncmp(F_type, "PSO", 3) == 0 ||
      strncmp(F_type, "hq", 2) == 0 || strncmp(F_type, "HQ", 2) == 0 ||
      strncmp(F_type, "flat", 4) == 0 || strncmp(F_type, "FLAT",4) == 0 ||
      strncmp(F_type, "pcm", 3) == 0 || strncmp(F_type, "PCM",3) == 0)
    return 1;
  else
    return 0;
}


/*
 * Function to display usage
 * By: Simao in 20.Apr.94
 */
void            display_usage()
{
  printf("FLTRESP -- Version 3.1 of 02.Feb.2010 --\n");
  printf("%s%s", "Test program to evaluate the frequence  ",
	  "response of filters.\n");
  printf("%s%s", "Depending on the function called, the program  ",
	  "will be able\n");
  printf("to evaluate the frequence response for the range of\n");
  printf("%s%s", "frequencies specified. The algorithm here is to  ",
	  "calculate the\n");
  printf("%s%s", "long-term energy of a sinewave before and  ",
	  "after the filtering,\n");
  printf("converting the ratio to dB.\n\n");

  printf("%s%s", "---------------------------------",
	  "-----------------------------\n");
  printf("%s%s", "NOTE! the output dB values are ALWAYS relative ",
	  "to the given\n");
  printf("%s%s", "sampling frequency, not to the output one,  ",
	  "whichever it is !!!\n");
  printf("%s%s", "---------------------------------",
	  "-----------------------------\n\n");

  printf("Usage:\n");
  printf("~~~~~~\n");
  printf("$ fltresp [-options] Flt_type f0 ff fstep [fs]\n\n");

  printf("where:\n");
  printf("flt_type: is the filter type:\n");
  printf("                IRS, HQ2, HQ3, PCM, PCM1\n");
  printf("f0 is the starting frequency [Hz]\n");
  printf("ff is the final frequency [Hz]\n");
  printf("fstep is the step in frequency from f0 to ff [Hz]\n");
  printf("%s%s", "[fs]is the sampling frequency [Hz]; ",
	  "default is 8000 Hz.\n\n");
  printf("Options:\n");
  printf("-fs: .... define sampling frequency, in Hz [def:8000Hz]\n");
  printf("-mod: ... use modified IRS filters\n");
  printf("-q: ..... quiet mode - don't print funny chars\n");

  printf("Valid combinations of filter and sampling rate:\n\n");

  printf("Flt_type   fs   Description \n");
  printf("  IRS     8000  (regular) IRS weighting with factor 1:2. \n");
  printf("         16000  (regular|modified) IRS weighting with factor 2:1.\n");
  printf("         48000  (modified) IRS weighting with factor 2:1.\n");
  printf("  DSM    16000  Delta-SM with factor 1:1\n");
  printf("  PSO     8000  Psophometric filter with factor 1:1\n");
  printf("  HQ2     8000  High quality with factor 1:2\n");
  printf("         16000  High quality with factor 2:1\n");
  printf("  HQ3     8000  High quality with factor 1:3\n");
  printf("         16000  High quality with factor 3:1\n");
  printf("  FLAT    8000  Linear-phase pass-band with factor 1:2\n");
  printf("         16000  Linear-phase pass-band with factor 2:1\n");
  printf("  PCM     8000  Standard PCM quality factor 1:2\n");
  printf("         16000  Standard PCM quality factor 2:1\n");
  printf("  PCM1    8000  unimplemented!\n");
  printf("%s%s", "         16000  Standard PCM quality with ",
	  "factor 1:1 at 16 kHz\n\n");
  exit(-128);
}


/*============================== */
int main(argc, argv)
  int             argc;
  char           *argv[];
/*============================== */
{
  /* DECLARATIONS */

  /* Algorithm variables */
  SCD_FIR        *fir_state;
  SCD_IIR        *iir_state;

  float          *BufInp, *BufOut;
  char            F_type[MAX_STRLEN];
  long            j, k, N, N2;
  char            modified_IRS = 0, quiet = 0;
  long            inp_size, out_size;
  double          f, f0, fstep, ff, fs=8000, inp_pwr;
  double          H_k, cur_f;
  static char     is_fir = 1;

  /* PREAMBLE */
  N = 256;
  N2 = 20;
  inp_size = N * N2;


  /* ......... GET PARAMETERS ......... */

  /* Check options */
  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1],"-mod")==0)
      {
	/* Get skip length */
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
  GET_PAR_S(1, "_Filter type: ................ ", F_type);
  
  if (!valid_filter(F_type))
  {
    fprintf(stderr, "Invalid filter chosen!\n\n");
    display_usage();
  }

  GET_PAR_D(2,  "_Start frequency [Hz]: ....... ", f0);
  GET_PAR_D(3,  "_Stop frequency [Hz]: ........ ", ff);
  GET_PAR_D(4,  "_Frequency step [Hz]: ........ ", fstep);
  FIND_PAR_D(5, "_Sampling Frequency [Hz]: .... ", fs, fs);


  /* Check consistency */

  /* Check upper frequency */
  if (ff >= fs / 2)
  {
    ff = fs / 2;
    fprintf(stderr, "Top frequency limited to 5%% of step below fs/2: %f ...\n",
                    ff - (0.05*fstep));
  }

  if (f0 < 2.0 / (double) inp_size * fs && f0 != 0.0)
  {
    f0 = 2.0 / (double) inp_size *fs;
    fprintf(stderr, "Lower frequency limited to fs/2: %f ...\n", f0);
  }

  /* normalization of frequencies */
  f0 /= fs;
  ff /= fs;
  fstep /= fs;

  /* set flag to filter type: IIR or FIR */
  if (strncmp(F_type, "pcm", 3) == 0 || strncmp(F_type, "PCM", 3) == 0)
    is_fir = 0;
  else
    is_fir = 1;


  /* ... CHOOSE CORRECT FILTER INITIALIZATION ... */


/*
  * Filter type: IRS - IRS weighting 1:1 factor:
  *                     . fs ==  8000 -> factor: 1:1
  *                     . fs == 16000 -> factor: 1:1 (regular | modified)
  *                     . fs == 48000 -> factor: 1:1
  */
  if (strncmp(F_type, "irs", 3) == 0 || strncmp(F_type, "IRS", 3) == 0)
  {
    if (fs == 8000)
      fir_state = irs_8khz_init();
    else if (fs == 16000)
      fir_state = modified_IRS? 
      		   mod_irs_16khz_init() :
      		   irs_16khz_init() ;
    else if (fs == 48000)
      fir_state = mod_irs_48khz_init();
    else
      HARAKIRI("Unimplemented: IRS at rate not 8, 16 or 48 kHz\n", 15);
  }

/*
  * Filter type: DSM - Delta-SM: factor 1:1
  */
  if (strncmp(F_type, "dsm", 3) == 0 || strncmp(F_type, "DSM", 3) == 0)
  {
    if (fs == 16000)
      fir_state = delta_sm_16khz_init();
    else
      HARAKIRI("Unimplemented: Delta-SM at rate not 16 kHz\n", 15);
  }

/*
  * Filter type: PSO - Psophometric wheighting filter: factor 1:1 
  */
  if (strncmp(F_type, "pso", 3) == 0 || strncmp(F_type, "PSO", 3) == 0)
  {
    if (fs == 8000)
      fir_state = psophometric_8khz_init();
    else
      HARAKIRI("Unimplemented: Psophometric filter only at fs=8kHz\n", 15);
  }


/*
  * Filter type: FLAT - Linear-phase, passband 2:1 or 1:2 factor:
  *                    . fs ==  8000 -> upsample: 1:2
  *                    . fs == 16000 -> downsample: 2:1
  */
  else if (strncmp(F_type, "flat", 4) == 0 || strncmp(F_type, "FLAT", 4) == 0)
  {
    if (fs == 8000)		/* It is up-sampling! */
      fir_state = linear_phase_pb_1_to_2_init();
    else if (fs == 16000)	/* It is down-sampling! */
      fir_state = linear_phase_pb_2_to_1_init();
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
    if (fs == 8000)		/* It is up-sampling! */
      fir_state = F_type[2] == '2'
	? hq_up_1_to_2_init()
	: hq_up_1_to_3_init();
    else			/* It is down-sampling! */
      fir_state = F_type[2] == '2'
	? hq_down_2_to_1_init()
	: hq_down_3_to_1_init();
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
      if (fs == 16000)
	iir_state = stdpcm_16khz_init();
      else
	HARAKIRI("Unimplemented: PCM with factor 1:1 for the given fs\n", 10);
    }
    else
      iir_state = (fs == 8000)
	? stdpcm_1_to_2_init()	/* It is up-sampling! */
	: stdpcm_2_to_1_init();	/* It is down-sampling! */
  }


  /* MEMORY ALLOCATION */

  /* Calculate Output buffer size */
  if (is_fir)
  {
    out_size = (fir_state->hswitch=='U') 
               ? inp_size * fir_state->dwn_up
               : inp_size / fir_state->dwn_up;
  }
  else
  {
    out_size = (iir_state->hswitch=='U') 
               ? inp_size * iir_state->idown
               : inp_size / iir_state->idown;
  }

  /* Allocate memory for input buffer */
  if ((BufInp = (float *) calloc(inp_size, sizeof(float))) == NULL)
    HARAKIRI("Can't allocate memory for data buffer\n", 10);

  /* Allocate memory for output buffer */
  if ((BufOut = (float *) calloc(out_size, sizeof(float))) == NULL)
    HARAKIRI("Can't allocate memory for data buffer\n", 10);


  /* FILTERING OPERATION! */

  for (k = 0, f = f0; f <= ff; f += fstep, k++)
  {

    /* Reset output buffer */
    memset(BufOut, '\0', out_size*sizeof(float));

    /* Current frequency, in Hz, and print */
    cur_f = f * fs;
    fprintf(stderr, "\nFrequency %f", cur_f);

    /* Adjust top (NORMALIZED!) frequency, if needed */
    if (fabs(f - 0.5) < 1e-8/fs)
      f -= (0.05*fstep);

    /* Calculate as a temporary the frequency in radians */
    inp_pwr = f * TWO_PI;

    /* Generate sine samples with peak 20000 ... */
    for (j = 0; j < inp_size; j++)
      BufInp[j] = 20000.0 * sin(inp_pwr * j);

    /* Calculate power of input signal */
    for (inp_pwr = 0, j = 0; j < inp_size; j++)
      inp_pwr += BufInp[j] * BufInp[j];

    /* Convert to dB */
    inp_pwr = 10.0 * log10(inp_pwr / (double) inp_size);

#ifdef OLD_WAY
    for (cur_frame = 0; cur_frame < N2; cur_frame++)
    {
      /* Information on processing phase */
      fprintf(stderr, "\r%c", funny[cur_frame % 9]);

      /* Copy data from big array to small one */
      for (l = cur_frame * N, j = 0; j < N; j++)
	inp[j] = (float) BufInp[l + j];

      /* Filtering ... */
      if (is_fir)
	j = hq_kernel(N, inp, fir_state, out);
      else
	j = stdpcm_kernel(N, inp, iir_state, out);

      /* Convert data from float to short */
      for (l = cur_frame * N, j = 0; j < N; j++)
	BufInp[l + j] = (float) out[j];
    }
#else  /* NEW_WAY */

      /* Filtering ... */
      if (is_fir)
	j = hq_kernel(inp_size, BufInp, fir_state, BufOut);
      else
	j = stdpcm_kernel(inp_size, BufInp, iir_state, BufOut);

#endif

    /* Compute power of output signal */
    for (H_k = 0, j = 2*N; j < out_size-2*N; j++)
      H_k += BufOut[j] * BufOut[j];

    /* Convert to dB */
    H_k = 10 * log10(H_k / (double) (out_size - 4*N) ) - inp_pwr;

    /* Printout of gain at the current frequency */
    printf("\nH( %4.0f ) \t = %7.3f dB\n", f * fs, H_k);

  }


  /* FINALIZATIONS */
  fprintf(stderr, "\n");

#ifndef VMS
  return(0);
#endif
}
