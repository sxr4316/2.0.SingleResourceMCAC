/*                                                              v3.5 02.Feb.10
  ============================================================================

  SV56DEMO.C
  ~~~~~~~~~~

  Description:
  ~~~~~~~~~~~~

  Example program that calculates the factor for equalizing, and
  equalizes, a file's active speech level "NdB" dBs below a 0 dB
  reference. Here, the default reference is 0 dB below system overload.
  This program, as an option, may normalize the output file using the 
  measure RMS (long term) level, instead of the active speech level.

  The calculation of speech file's active power uses the algorithm
  in ITU-T Recommendation P.56, and as a by-product of
  this mudule, also  calculates the activity [%], RMS power,
  positive and negative peaks for the file, and peak factors.

  In general, input and output files are in integer represent-
  ation, 16-bit words, 2's complement. In UGST convention, this
  data must be left-adjusted, RATHER than right-adjusted. Since
  the speech voltmeter uses `float' input data, it is necessary to
  convert from short (in the mentioned format) to float; this is
  carried out by the function `sh2fl()'. This function has the
  option of `normalize' the input data to the range -1..+1, what
  is done. After the equalization factor is found, the function
  `scale()' is called to carry out the equalization using single
  (rather than double) float precision. After equalized, data need
  to be converted back to integer (short, right-justified). This
  is done by function `fl2sh()', using: truncation, no
  zero-padding of the least significant bits, left-justification
  of data, hard-clipping of data outside the range -32768..+32767.
  After that, data is saved to file.

  The default values for the AD,DA systems resolution is 16
  bits, for the sampling rate is 16000 Hz. To change this
  on-line, just specify the parameters 6 and/or 7 conveniently
  in the command line. For example, 14 bits and 8000 Hz:
  $ sv56demo filein fileout 256 1 100 -15 8000 14 

  Usage:
  ~~~~~~
  $ sv56demo [-options] FileIn FileOut 
             [BlockSize [1stBlock [NoOfBlocks [DesiredLevel
             [SampleRate [Resolution] ] ] ] ] ]
  where:
  FileIn           is the input file to be analysed and equalized;
  FileOut          is the output equalized file
  BlockSize        is the block size in number of samples;
  1stBlock         the first block to be analysed/equalized
  NoOfBlocks       number of blocks to be analysed/equalized
  DesiredLevel     level desired to the output file, in dBov or dBm0
  SampleRate       sampling rate used for acquiring/generating the
                   file, in Hertz; default is 16000 Hz; THIS PARAMETER
                   IS OPTIONAL!
  Resolution       the digital system resolution (AD,DA systems), in
                   number of bits; default to 16 bits; THIS PARAMETER
                   IS OPTIONAL, but to specify it you need to specify
                   the former one!
  Options:
  ~~~~~~~~
  -bits n ........ change the default word length to n bits; equivalent
                   to parameter Resolution above [default: 16 bits]
  -lev ndB ....... equivalent to specifying DesiredLevel above, just that
                   here do not need to specify the former 3 parameters.
  -log file ...... print the statistics log into file rather than stdout
  -q ............. quit operation - does not print the progress flag. 
                   Saves time and avoids trash in batch processings.
  -qq ............ print short statistics summary; no progress flag.
  -rms ........... normalizes the output file using the RMS long-term level, 
                   instead of the active speech level.
  -sf f .......... set sampling frequency to `f' Hz; equivalent to parameter 
                   SampleRate above.
  -blk len  ...... is the block size in number of samples;
                   this parameter is optional, and the default is block size
	           of 256 samples; equivalent to parameter N above
  -start sb ...... define `sb' as the first block to be measured; equivalent 
                   to parameter N1 above [default: first block of the file] 
  -end eb ........ define `eb' as the last block to be measured
  -n nb .......... define `nb' as the number of blocks to be measured; 
                   equivalent to parameter N2 above [default: whole file]

  Modules used:
  ~~~~~~~~~~~~~
  > sv-P56.c: contains the functions related to active speech
              level measurement according to P.56,
              init_speech_voltmeter(), speech_voltmeter() and 
              bin_interp(). Their prototypesare in `sv-p56.h'.
  > ugst-utl.c: utility functions; here are used the gain/loss
              (scaling) algorithm of scale() and the data type
              conversion functions sh2fl() and fl2sh(). Prototypes 
              are in `ugst-utl.h'.

  Exit values:
  ~~~~~~~~~~~~
  0      success (all but VMS);
  1      success (only in VMS);
  2      error opening input file;
  3      error creating output file;
  4      error moving pointer to desired start of conversion;
  5      error reading input file;
  6      error writing to file;

  Compilation:
  ~~~~~~~~~~~~
  VaxC:   cc sv56demo.c 
          link sv56demo
  TurboC: tcc sv56demo.c
  Sun-OS: cc -o sv56demo sv56demo.c -lm 

  Author: 
  ~~~~~~~ 
  Simao Ferraz de Campos Neto
  DDS/Pr11                      Tel: +55-192-39-1396                
  CPqD/Telebras                 Fax: +55-192-53-4754                
  13085-061 Campinas SP Brazil  E-mail: <tdsimao@venus.cpqd.ansp.br>

  Log of changes:
  ~~~~~~~~~~~~~~~
  09.Mar.90     0.0        Release of first version of a C speech
                           voltmeter.
  08.Oct.91     1.0        Release of demo program for the speech
                           voltmeter module.
  19.Feb.92     2.0        Call to module using state variable instead of
                           individual variables. Compilation option 
                           between dB(overload) and dBm0.
  18.May.92     2.1        Removed references to dBm0; input data
                           is converted to the normalized range; 
                           speech voltmeter needs input data in
                           normalized range. <tdsimao@cpqd.ansp.br>
  10.Dec.94     3.0        Included additional input interface. NdB promoted
                           from long to double. <simao@ctd.comsat.com>
  21.Aug.95     3.1        Included additional option to normalize output
                           file using RMS long-term level instead of active 
                           level, and options for block size, first block, 
                           last block, number of blocks. <simao@ctd.comsat.com>
  29.May.97     3.2        moved summary statistics code to a function, and
                           created a short summary function as well. Added
                           command-line option to print short summary
                           (-qq). Add -log option so save statistical
                           summaries into a file <simao>
  06.Apr.98     3.3        solved small bug that occurred when the file 
                           size was not a multiple of the frame
                           size. The program was truncating the output
                           file size to a multiple of the current
                           block size. The fix was to introduce a
                           ceil() in the calculation of N2 when N2==0
                           <simao>
  08.Jul.99     3.4        fixed a bug in fwrite() call in main(); was 
                           saving N samples, rather than "l". This was
                           causing more samples to be written to the
                           end of the file when the file size was not
                           a multiple of the block size <simao>.
  02.Feb.10     3.5        Modified maximum string length to avoid
                           buffer overruns (y.hiwasaki)

  ============================================================================
*/
/* ... Include for all UGST demos -- should be the 1st include ... */
#include "ugstdemo.h"

/* ... Includes in general ... */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strstr() */
#include <math.h>

/* ... Includes for O.S. specific headers ... */
#if defined(VMS)
#include <perror.h>
#include <file.h>
#include <stat.h>
#else				/* Unix/DOS */
#include <sys/stat.h>
#if defined(MSDOS)
#include <fcntl.h>
#include <io.h>
#endif /* MSDOS */
#endif /* !VMS */

/* ... Include of speech-voltmeter-related routines ... */
#include "sv-p56.h"

/* ... Include of utilities ... */
#include "ugst-utl.h"

/* Local definitions */
#define MIN_LOG_OFFSET 1.0e-20 /*To avoid sigularity with log(0.0) */

/*
 -------------------------------------------------------------------------
 void display_usage(void);
 ~~~~~~~~~~~~~~~~~~
 Display proper usage for the demo program. Generated automatically from
 program documentation.

 History:
~~~~~~~~
 10.Dec.94 v1.0 Created. <simao>
 -------------------------------------------------------------------------
*/
#define P(x) printf x
void display_usage()
{
  P(("SV56DEMO.C: Version 3.5 of 02.Feb.2010 \n\n"));
  P(("  Program to level-equalize a speech file \"NdB\" dBs below\n"));
  P(("  the overload point for a linear n-bit (default: 16 bit) system.\n"));
  P(("  using the P.56 speech voltmeter algorithm.\n"));
  P(("\n"));
  P(("  Usage:\n"));
  P(("  $ sv56demo [-options] FileIn FileOut \n"));
  P(("             [BlockSize [1stBlock [NoOfBlocks [DesiredLevel\n"));
  P(("             [SampleRate [Resolution] ] ] ] ] ]\n"));
  P(("  FileIn: ..... is the input file to be analysed and equalized;\n"));
  P(("  FileOut: .... is the output equalized file\n"));
  P(("  BlockSize: .. is the block size [default: 256 samples]\n"));
  P(("  1stBlock: ... the first block to be manipulated [default: 1st]\n"));
  P(("  NoOfBlocks: . number of blocks to be manipulated [default: all]\n"));
  P(("  DesiredLevel: level desired to the output file, in dBov\n"));
  P(("                [default: -26 dBov].\n"));
  P(("  SampleRate: . sampling rate used for acquiring/generating the\n"));
  P(("                file, in Hertz [default: 16000 Hz].\n"));
  P(("  Resolution: . the digital system resolution (AD,DA systems), in\n"));
  P(("                number of bits [default: 16 bits].\n"));
  P(("  Options:\n"));
  P(("  -lev ndB .... equivalent to specifying DesiredLevel above, but\n"));
  P(("                here don't need to specify the former 3 parameters.\n"));
  P(("  -sf f ....... set sampling frequency to `f' Hz; equivalent to\n")); 
  P(("                parameter SampleRate above.\n"));
  P(("  -bits n ..... change the default word length to n bits; \n"));
  P(("                equivalent to parameter Resolution.\n"));
  P(("  -rms ........ normalizes the output file using the RMS long-term \n"));
  P(("                level, instead of the active speech level.\n"));
  P(("  -blk len .... is the block size in number of samples;\n"));
  P(("                this parameter is optional [default is block size\n"));
  P(("                of 256 samples]. Equiv. to param. BlockSize above\n"));
  P(("  -start sb ... define `sb' the first block to be measured; equiv.\n"));
  P(("                to param. 1stBlock above [dft: 1st block of file]\n"));
  P(("  -end eb ..... define `eb' as the last block to be measured\n"));
  P(("  -n nb ....... define `nb' as the number of blocks to be measured;\n"));
  P(("                equiv. to param.NoOfBlocks above [dft: whole file]\n"));
  P(("  -log file ... log statistics into file rather than stdout\n"));
  P(("  -q .......... quiet operation - does not print the progress flag.\n"));
  P(("                Saves time and avoids trash in batch processings.\n"));
  P(("  -qq ......... print short statistics summary; no progress flag.\n"));

  /* Quit program */
  exit(-128);
}
#undef P
/* .................... End of display_usage() ........................... */


/* 
  ============================================================================

       void print_p56_long_summary(char *file, SVP56_state state, double al_dB,
       ~~~~~~~~~~~~~~~~~~~~~~~~~~~ double NdB, double gain, double ratio,
                                   long N, long N1, long N2, long bitno);

       Print full summary of P.56 statistics

       Parameter:
       ~~~~~~~~~~
       file ..... File name
       state .... P.56 state variable
       al_dB .... active level in dB
       NdB ...... desired dB level for output
       gain ..... gain to equalize file to desired level
       ratio .... ratio of maximum number representable in the system (based on
                  the number of bits, or resolution, of the input speech file)
		  to the range of the input signal. E.g., for a 16 bit system
		  with a +-1 signal range, ratio = 32760 / 1.0.
       N ........ number of samples per block
       N1 ....... number of first block processed in the file
       N2 ....... total number of blocks processed from the file
       bitno .... number of bits per sample (input signal resolution)

       Returns
       ~~~~~~~
       None

       Original author
       ~~~~~~~~~~~~~~~
       simao@ctd.comsat.com

       Log of changes
       ~~~~~~~~~~~~~~
       01.Nov.96	v1.0	Creation.

  ============================================================================
*/
void print_p56_long_summary(out, file, state, al_dB, NdB, ratio, gain,
			    N, N1, N2, bitno)
  FILE *out;
  SVP56_state     state;
  char *file;
  double ratio, gain, al_dB, NdB;
  long N, N1, N2, bitno;
{
  static char     unity[5] = "dBov";
  double          abs_max_dB, max_gain, new_max;

  /* Calculate peak in dB */
  abs_max_dB = 20 * log10(SVP56_get_abs_max(state) + MIN_LOG_OFFSET)
               - state.refdB;

  /* Computes the maximum gain that will not cause saturation */
  max_gain = al_dB - 20 * log10(SVP56_get_abs_max(state));

  /* Print the relevant messages */
  fprintf(out, "%s%s", " ---------------------------",
	  "----------------------------");
  fprintf(out, "\n  Input file: ................... %s, ", file);
  fprintf(out, "%2ld bits, fs=%5.0f Hz", bitno, state.f);
  fprintf(out, "\n  Block Length: ................. %7ld [samples]", N);
  fprintf(out, "\n  Starting Block: ............... %7ld []", N1 + 1);
  fprintf(out, "\n  Number of Blocks: ............. %7ld []", N2);
  fprintf(out, "\n  %s desired for output: ...... %7.3f [%s]",
	  unity, NdB, unity);

  /* Skip if filesize is zero */
  if (state.n==0)
  {
    fprintf(out, "%s%s", "\n -***-----------------------",
	    "----------------------------\n");
    return;
  }

  /* If the activity factor is 0, don't report many things */
  if (SVP56_get_activity(state) == 0)
  {
    fprintf(out, "\n  Activity factor is ZERO -- the file is silence");
    fprintf(out, " or idle noise");
    fprintf(out, "%s%s", "\n ---------------------------",
	    "----------------------------");
    fprintf(out, "\n  DC level: ..................... %7.0f [PCM]", 
	    ratio * SVP56_get_DC_level(state));
    fprintf(out, "\n  Maximum positive value: ....... %7.0f [PCM]", 
	    ratio * SVP56_get_pos_max(state));
    fprintf(out, "\n  Maximum negative value: ....... %7.0f [PCM]", 
	    ratio * SVP56_get_neg_max(state));
    fprintf(out, "%s%s", "\n ---------------------------",
	    "----------------------------");
    fprintf(out, "\n  Noise/silence energy (rms): ... %7.3f [dB]", 
	    SVP56_get_rms_dB(state));
  }
  else
  {
    fprintf(out, "\n  Norm factor desired is: ....... %7.3f [times]", gain);
    fprintf(out, "\n  Max norm WITHOUT saturation: .. %7.3f [%s]",
	    max_gain, unity);
    fprintf(out, "%s%s", "\n ---------------------------",
	    "----------------------------");
    fprintf(out, "\n  DC level: ..................... %7.0f [PCM]", 
	    ratio * SVP56_get_DC_level(state));
    fprintf(out, "\n  Maximum positive value: ....... %7.0f [PCM]", 
	    ratio * SVP56_get_pos_max(state));
    fprintf(out, "\n  Maximum negative value: ....... %7.0f [PCM]", 
	    ratio * SVP56_get_neg_max(state));
    fprintf(out, "%s%s", "\n ---------------------------",
	    "----------------------------");
    fprintf(out, "\n  Long term energy (rms): ....... %7.3f [%s]", 
	    SVP56_get_rms_dB(state), unity);
    fprintf(out, "\n  Active speech level: .......... %7.3f [%s]",
	   al_dB, unity);
    fprintf(out, "\n  RMS peak-factor found: ........ %7.3f [dB]",
	    abs_max_dB - SVP56_get_rms_dB(state));
    fprintf(out, "\n  Active peak factor found: ..... %7.3f [dB]",
	    abs_max_dB - al_dB);
    fprintf(out, "\n  Activity factor: .............. %7.3f [%%]",
	    SVP56_get_activity(state));
  }
  fprintf(out, "%s%s", "\n ---------------------------",
	  "----------------------------");

  /* Tests if the equalization will cause saturation */
  new_max = SVP56_get_abs_max(state) * gain;
  if (new_max > 1.0)
  {
    /* Print message */
    fprintf(out, "\n%%SV-W-SAT, the dB level chosen causes SATURATION: ");
    fprintf(out, "old max=%5.0f; new max=%6.0f",
	    ratio * SVP56_get_abs_max(state), 
	    ratio * new_max);
    fprintf(out, "\n%%SV-I-MAXLEVDB, the maximum norm factor ");
    fprintf(out, "to PREVENT clipping is %7.3fdB; ", max_gain);
    fprintf(out, "\n -------------------------------------------------------");
  }
}
/* .................... End of print_p56_long_summary() ................... */


/* 
  ============================================================================

       void print_p56_short_summary (char *file, SVP56_state state,
       ~~~~~~~~~~~~~~~~~~~~~~~~  double al_dB, double ratio);

       Print one-line summary of P.56 statistics

       Parameter:
       ~~~~~~~~~~
       file ..... File name
       state .... P.56 state variable
       al_dB .... active level in dB
       ratio .... ratio of maximum number representable in the system (based on
                  the number of bits, or resolution, of the input speech file)
		  to the range of the input signal. E.g., for a 16 bit system
		  with a +-1 signal range, ratio = 32760 / 1.0.

       Returns
       ~~~~~~~
       None

       Original author
       ~~~~~~~~~~~~~~~
       simao@ctd.comsat.com

       Log of changes
       ~~~~~~~~~~~~~~
       01.Nov.96	v1.0	Creation.

  ============================================================================
*/
void print_p56_short_summary(out, file, state, al_dB, ratio, gain)
  FILE *out;
  SVP56_state     state;
  char *file;
  double ratio, al_dB, gain;
{
  /* Report number of samples */
  fprintf(out, "Samples: %5ld ", state.n);

  /* Skip if filesize is zero */
  if (state.n==0)
  {
    fprintf(out, "%s%s%s", "Min: ------ Max: ----- DC: ------- ",
	   "RMSLev[dB]: ------- ActLev[dB]: ------- %Active:  ------ ",
	    "Gain:  ------ ");
  }
  else
  {
    fprintf(out, "Min: %-5.0f ",       ratio * state.maxN);
    fprintf(out, "Max: %-5.0f ",       ratio * state.maxP);
    fprintf(out, "DC: %-7.2f ",        ratio * state.DClevel);
    fprintf(out, "RMSLev[dB]: %7.3f ", state.rmsdB);
    fprintf(out, "ActLev[dB]: %7.3f ", al_dB);
    fprintf(out, "%%Active: %7.3f",    state.ActivityFactor * 100);
    fprintf(out, "Gain[]: %7.3f",      gain);
  }
  fprintf(out, "\t%s\n", file);
}
/* ................... End of print_p56_short_summary() .................... */


/*
   **************************************************************************
   ***                                                                    ***
   ***        Demo-Program for testing the correct implementation         ***
   ***               and to show how to use the programs                  ***
   ***                                                                    ***
   **************************************************************************
*/
int main(argc, argv)
  int             argc;
  char           *argv[];
{
  /* DECLARATIONS */

  /* Parameters for operation */
  double          Overflow;	/* Max.positive value for AD_resolution bits */
  long            N=256, N1=1, N2=0, i, l;
  double          NdB=-26; /* dBov */

  /* Intermediate storage variables for speech voltmeter */
  SVP56_state     state;

  /* File-related variables */
  char            FileIn[MAX_STRLEN], FileOut[MAX_STRLEN];
  FILE           *Fi, *Fo;	/* input/output file pointers */
  FILE           *out=stdout;   /* where to print the statistical results */
#ifdef VMS
  char            mrs[15];
#endif

  /* Other variables */
  char	          quiet=0, use_active_level = 1, long_summary = 1;
  short           buffer[4096];
  float           Buf[4096];
  long            NrSat = 0, start_byte, bitno = 16;
  double          sf = 16000, factor;
  double          ActiveLeveldB, DesiredSpeechLeveldB;
  static char     funny[5] = {'/', '-', '\\', '|', '-'};
  static unsigned mask[5] = {0xFFFF,0xFFFE,0xFFFB,0xFFF8,0xFFF0};


  /* ......... GET PARAMETERS ......... */
 
  /* Getting options */
  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-lev") == 0)
      {
	/* Change default level normalization */
	NdB = atof(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-sf") == 0)
      {
	/* Change default sampling frequency */
	sf = atof(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-bits") == 0)
      {
	/* Change default sampling frequency */
	bitno = atol(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-rms") == 0)
      {
	/* Use the RMS level to normalize instead of the active level */
	use_active_level = 0;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-blk") == 0)
      {
	/* Change default sampling frequency */
	N = atol(argv[2]);

	/* Update argc/argv to next valid option/argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-start") == 0)
      {
	/* Change default sampling frequency */
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
      else if (strcmp(argv[1], "-log") == 0)
      {
	/* Log statistics into a file */
	if ((out=fopen(argv[2], WT)) == NULL)
	  KILL(argv[2], 2);
	else
	  fprintf(stderr, "Statitics will be logged in %s\n", argv[2]);

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
      else if (strcmp(argv[1], "-qq") == 0)
      {
	/* Don't print progress indicator and print short summary */
	quiet = 1;
	long_summary = 0;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-?") == 0 || strstr(argv[1], "-help"))
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


  /* Reads parameters for processing */
  GET_PAR_S(1, "_Input File: ........................... ", FileIn);
  GET_PAR_S(2, "_Output File: .......................... ", FileOut);
  FIND_PAR_L(3, "_Block Length: ......................... ", N, N);
  FIND_PAR_L(4, "_Start Block: .......................... ", N1, N1);
  FIND_PAR_L(5, "_No. of Blocks: ........................ ", N2, N2);
  FIND_PAR_D(6, "_dBs relative to system overload: ...... ", NdB, NdB);
  FIND_PAR_D(7, "_Sampling Frequency: ................... ", sf, sf);
  FIND_PAR_L(8, "_A/D resolution: ....................... ", bitno, bitno);


  /* ......... SOME INITIALIZATIONS ......... */
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

  /* Overflow (saturation) point */
  Overflow = pow((double)2.0, (double)(bitno-1));

  /* reset variables for speech level measurements */
  init_speech_voltmeter(&state, sf);


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

  /* Move pointer to 1st block of interest */
  if (fseek(Fi, start_byte, 0) < 0l)
    KILL(FileIn, 4);


  /* ... MEASUREMENT OF ACTIVE SPEECH LEVEL ACCORDING P.56 ... */

  /* Print info */
  if (!quiet)
    printf("  Processing \r");

  /* Process selected blocks */
  for (i = 0; i < N2; i++)
  {
    /* Read samples ... */
    if ((l = fread(buffer, sizeof(short), N, Fi)) > 0)
    {
      /* ... Convert samples to float */
      sh2fl((long) l, buffer, Buf, bitno, 1);

      /* ... Get the active level */
      ActiveLeveldB = speech_voltmeter(Buf, (long) l, &state);

      /* Print some preliminary information */
      if (!quiet) 
	printf("%c\r", funny[i % 5]);
    }
    else
    {
      KILL(FileIn, 5);
    }
  }

  /* Beautify screen ... */
  if (!quiet)
    printf("\n");


  /* ... COMPUTE EQUALIZATION FACTOR ... */

  /* Computes the equalization factor to be used in the output file */
  DesiredSpeechLeveldB = (double) NdB;
  if (use_active_level)
    factor = pow(10.0, (DesiredSpeechLeveldB-ActiveLeveldB) / 20.0);
  else
    factor = pow(10.0, (DesiredSpeechLeveldB-SVP56_get_rms_dB(state)) / 20.0);

  /* ... PRINT-OUT OF RESULTS ... */
  if (long_summary)
    print_p56_long_summary(out, FileIn, state, ActiveLeveldB, NdB,
			   Overflow, factor, N, N1, N2, bitno);
  else
    print_p56_short_summary(out, FileIn, state, ActiveLeveldB, 
			    Overflow, factor);

  /* EQUALIZATION: hard clipping (with truncation) */

  /* Move pointer to 1st desired block */
  if (fseek(Fi, start_byte, 0) < 0l)
    KILL(FileIn, 4);

  /* Get data of interest, equalize and de-normalize */
  for (i = 0; i < N2; i++)
  {
    if ((l = fread(buffer, sizeof(short), N, Fi)) > 0)
    {
      /* convert samples to float */
      sh2fl((long) l, buffer, Buf, bitno, 1);

      /* equalizes vector */
      scale(Buf, (long) l, (double) factor);

      /* Convert from float to short with hard clip and truncation */
      NrSat += fl2sh((long) l, Buf, buffer, (double) 0.0, mask[16-bitno]);

      /* write equalized, de-normalized and hard-clipped samples to file */
      if ((l = fwrite(buffer, sizeof(short), l, Fo)) < 0)
	KILL(FileOut, 6);
    }
    else
    {
      KILL(FileIn, 5);
    }
  }

  /* Log number of clipped samples */
  if (NrSat != 0)
    fprintf(out, "\n  Number of clippings: .......... %7ld []\n", NrSat);
  else
    fprintf(out, "\n");

  /* FINALIZATIONS */
  /* Beautify screen ... */
  if (!quiet)
    printf("---> DONE    \n");

  /* Close files ... */
  fclose(Fi);
  fclose(Fo);
  if (out != stdout)
    fclose(out);
#if !defined(VMS)
  return (0);
#endif
}
