#ifdef COMPILE_ME
/* This file is just an scheleton */


/* ..... Generic include files ..... */
#include "ugstdemo.h"		/* general UGST definitions */
#include <stdio.h>		/* Standard I/O Definitions */
#include <math.h>
#include <stdlib.h>
#include <string.h>		/* memset */
#include <ctype.h>		/* toupper */

/* ..... OS-specific include files ..... */
#if defined (unix) && !defined(MSDOS)
/*                 ^^^^^^^^^^^^^^^^^^ This strange construction is necessary
                                      for DJGPP, because "unix" is defined,
				      even it being MSDOS! */
#include <sys/unistd.h>		/* for SEEK_... definitions used by fseek() */
#endif

/* ..... Module definition files ..... */
#include "eid.h"		/* EID functions */
#include "eid_io.h"		/* EID I/O functions */

/* ..... Definitions used by the program ..... */

/* Operating modes */
enum BS_types {byte, g192, compact};
enum BS_types {BER, FER};

/* Definitions for byte mode (defs for G192-mode are in eid_io.h) */
#define BYTE_ZERO       (char)0X7F
#define BYTE_ONE	(char)0X81
#define BYTE_SYNC	(char)0x21
#define BYTE_FER	(char)0x20

/* Other definitions */
#define EID_BUFFER_LENGTH 256
#define OUT_RECORD_LENGTH 512

/* 
   -------------------------------------------------------------------------
   long read_g192 (short *patt, long n, FILE *F);
   ~~~~~~~~~~~~~~

   Read a G.192-compliant 16-bit serial bitstream error pattern.

   Parameter:
   ~~~~~~~~~~
   patt .... G.192 array with the softbits representing 
             the bit error/frame erasure pattern
   n ....... number of softbits in the pattern
   F ....... pointer to FILE where the pattern should be readd

   Return value: 
   ~~~~~~~~~~~~~
   Returns a long with the number of shorts softbits from file. On error, 
   returns -1.

   Original author: <simao.campos@comsat.com>
   ~~~~~~~~~~~~~~~~

   History:
   ~~~~~~~~
   13.Aug.97  v.1.0  Created.
   -------------------------------------------------------------------------
 */
long read_g192(patt, n, F)
short *patt;
long n;
FILE *F;
{
  long i;

  /* Read words from file */
  i=fread(patt, sizeof(short), n, F);

  /* Return no.of samples read or error */
  return(ferror(F)? -1l : i);
}
/* ....................... End of read_g192() ....................... */


/*
  ---------------------------------------------------------------------------
  char *format_str (int fmt);
  ~~~~~~~~~~~~~~~~~

  Function to return a string with the description of the current
  bitstream format (g192, byte, or bit).

  Parameters:
  fmt ... integer with the bitstream format

  Returned value:
  ~~~~~~~~~~~~~~~
  Returns a pointer to the format string, or a NULL pointer
  if fmt is invalid. 

  Original author: <simao.campos@comsat.com>
  ~~~~~~~~~~~~~~~~

  History:
  ~~~~~~~~
  21.Aug.97  v1.00 created
  ---------------------------------------------------------------------------
*/
char *format_str(fmt)
int fmt;
{
  switch(fmt)
  {
  case byte:
    return "byte";
    break;
  case g192:
    return "g192";
    break;
  case compact:
    return "bit";
    break;
  }
  return "";
}
/* ....................... End of format_str() ....................... */ 

/*
  --------------------------------------------------------------------------
  char check_eid_format (FILE *F, char *file, char *type);
  ~~~~~~~~~~~~~~~~~~~~~

  Function that checks the format (g192, byte, bit) in a given
  bitstream, and tries to guess the type of data (bit stream or frame
  erasure pattern)

  Parameter:
  ~~~~~~~~~~
  F ...... FILE * structure to file to be checked
  file ... name of file to be checked
  type ... pointer to guessed data type (FER or BER) in file

  Returned value:
  ~~~~~~~~~~~~~~~
  Returns the data format (g192, byte, bit) found in file.

  Original author: <simao.campos@comsat.com>
  ~~~~~~~~~~~~~~~~

  History:
  ~~~~~~~~
  15.Aug.97  v.1.0  Created.
  -------------------------------------------------------------------------- 
*/
char check_eid_format(F, file, type)
FILE *F;
char *file;
char *type;
{

  short word;
  char ret_val;

  /* Get a 16-bit word from the file */
  fread(&word, sizeof(short), 1, F);

  /* Use some heuristics to determine what type of file is this */
  switch((unsigned)word)
  {
  case 0x7F7F:
  case 0x7F81:
  case 0x817F:
  case 0x8181:
    /* Byte-oriented G.192 bitstream */ 
    *type = BER;
    ret_val = byte;
    break;

  case 0x2020:
  case 0x2021:
  case 0x2120:
  case 0x2121:
    /* Byte-oriented G.192 syncs */
    *type = FER;
    ret_val = byte;
    break;

  case 0x007F:
  case 0x0081:
    /* G.192 bitstream in natural order */
    *type = BER;
    ret_val = g192;
    break;

  case 0x6B21:
  case 0x6B20:
    /* G.192 sync header in natural order */
    *type = FER;
    ret_val = g192;
    break;

  case 0x7F00:
  case 0x8100:
  case 0x216B:
  case 0x206B:
    /* G.192 format that needs to be byte-swapped */
    fprintf(stderr, "File %s needs to be byte-swapped! Aborted.\n", file);
    exit(8);

  default:
    /* Assuming it is compact bit mode */
   *type = nil; /* Not possible to infer type for binary format! */
    ret_val = compact;
  }

  /* Final check to see if the input bitstream is a byte-oriented G.192
     bitstream. In this case, the first byte is 0x2n (n=0..F) and the
     second byte must be the frame length */
  if ((((unsigned)word>>8) & 0xF0) == 0x20)
  {
     *type = FER;
     ret_val = byte;
  }
  /* Rewind file & and return format identifier */
  fseek(F, 0l, SEEK_SET);
  return(ret_val);
}
/* ...................... End of check_eid_format() ...................... */



int             main (argc, argv)
  int             argc;
  char           *argv[];
{
  /* Command line parameters */
  char            format = g192;       /* BS format: g192, byte, bit */
  char            type = FER;          /* BS type: BER or FER */
  double          ber_master, ber_new;

  /* File I/O parameter */
  char            master_ep[MAX_STRLEN]; /* Master error pattern file */
  char            new_ep[MAX_STRLEN];    /* New error pattern file */
  FILE           *Fi, *Fo;
  long            smpno=0, start_frame=1;

  /* EID-related variables */
  long fr_len = 256;
  char sync_header = 1;

  /* Data arrays */
  short          *master, *new;	       /* Bit error buffer */

  /* Aux. variables */
  double          FER;		       /* frame erasure rate */
  double          BER;		       /* bit error rate */
  double          BER_gamma = 0.0;     /* burst factors; NOT NEEDED */
  double          FER_gamma = 0.0;     /* burst factors; NOT NEEDED */
  double          ber1;		       /* returns values from BER_generator */
  double          disturb_master;	       /* # of distorted bits/frames */
  double          proc_master;	       /* # of processed bits/frames */
  double          generated;	       /* # of generated bits/frames */
  double          percentage;
  double          tolerance = -1;  /* Tolerance for actual rates; disabled */
  char            percent = '%';
  long            i, j, k, iteraction = 0;
  long            items;	       /* Number of output elements */
  long            itot;
  long            index;
#if defined(VMS)
  char            mrs[15] = "mrs=512";
#endif
  char            quiet = 0, reset = 0, save_format = byte;

  /* Pointer to a function */
  long            (*read_patt)() = read_g192;	/* To read master EP */
  long            (*save_patt)() = save_g192;	/* To save new EP */

  /* ......... GET PARAMETERS ......... */

  /* Check options */
  if (argc < 2)
    display_usage ();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp (argv[1], "-start") == 0)
      {
	/* Define starting sample/frame for error insertion */
	start_frame = atol (argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp (argv[1], "-n") == 0)
      {
	/* Define number of samples to extract */
	smpno = atoi (argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp (argv[1], "-tol") == 0)
      {
	/* Define number of samples to extract */
	tolerance = atof (argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp (argv[1], "-frame") == 0)
      {
	/* Define input & output encoded speech bitstream format */
	fr_len = atoi(argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp (argv[1], "-ber") == 0 ||
	       strcmp (argv[1], "-BER") == 0)
      {
	/* BS type */
        bs_type = BER;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp (argv[1], "-fer") == 0 ||
	       strcmp (argv[1], "-FER") == 0)
      {
	/* BS type */
        bs_type = FER;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp (argv[1], "-g192") == 0)
      {
	/* Save bitstream as a G.192-compliant serial bitstream */
	save_format = g192;
	save_patt = save_g192;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp (argv[1], "-byte") == 0)
      {
	/* Save bitstream as a byte-oriented serial bitstream */
	save_format = byte;
	save_patt = save_byte;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp (argv[1], "-compact") == 0 ||
	       strcmp (argv[1], "-bit") == 0)
      {
	/* Save bitstream as a compact binary bitstream */
	save_format = compact;
	save_patt = save_bit;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else if (strcmp (argv[1], "-q") == 0)
      {
	/* Set quiet mode */
	quiet = 1;

	/* Move arg{c,v} over the option to the next argument */
	argc--;
	argv++;
      }
      else
      {
	fprintf (stderr, "ERROR! Invalid option \"%s\" in command line\n\n",
		 argv[1]);
	display_usage ();
      }
  }

  /* Get command line parameters */
  GET_PAR_S (1, "_Master error pattern file ...............: ", master_ep);
  GET_PAR_S (2, "_Decimated error pattern file ............: ", new_ep);
  FIND_PAR_L (3, "_Decimation factor .......................: ",
	      dec_fac, dec_fac);

  /* Starting frame is from 0 to number_of_frames-1 */
  start_frame--;

  /* Open files */
  if ((Fi= fopen (master_ep, RB)) == NULL)
    HARAKIRI ("Could not open input bitstream file\n", 1);
  if ((Fo= fopen (new_ep, WB)) == NULL)
    HARAKIRI ("Could not create output file\n", 1);

  /* *** CHECK CONSISTENCY *** */

  /* Do preliminary inspection in the INPUT BITSTREAM FILE to check
     its format (byte, bit, g192) */
  i = check_eid_format(Fibs, ibs_file, &tmp_type);

  /* Check whether the specified BS format matches with the one in the file */
  if (i != bs_format)
  {
    /* The input bitstream format is not the same as specified */
    fprintf (stderr, "*** Switching bitstream format from %s to %s ***\n",
	     format_str((int)bs_format), format_str(i));
    bs_format = i;
  }

  /* Check whether the BS has a sync header */
  if (tmp_type == FER)
  {
    /* The input BS may have a G.192 synchronism header - verify */
    if (bs_format == g192)
    {
      short tmp[2];

      /* Get presumed first G.192 sync header */
      fread(tmp, sizeof(short), 2, Fibs);
      /* tmp[1] should have the frame length */
      i = tmp[1];
      /* advance file to the (presumed) next G.192 sync header */
      fseek(Fibs, (long)(tmp[1])*sizeof(short), SEEK_CUR);
      /* get (presumed) next G.192 sync header */
      fread(tmp, sizeof(short), 2, Fibs);
      /* Verify */
      if (((tmp[0] & 0xFFF0) == 0x6B20) && (i == tmp[1]))
      {
	fr_len = i;
	sync_header = 1;
      }
      else
	sync_header = 0;
    }
    else if (bs_format == byte)
    {
      char tmp[2];

      /* Get presumed first byte-wise G.192 sync header */
      fread(tmp, sizeof(char), 2, Fibs);
      /* tmp[1] should have the frame length */
      i = tmp[1];
      /* advance file to the (presumed) next byte-wise G.192 sync header */
      fseek(Fibs, (long)tmp[1], SEEK_CUR);
      /* get (presumed) next G.192 sync header */
      fread(tmp, sizeof(char), 2, Fibs);
      /* Verify */
      if (((tmp[0] & 0xF0) == 0x20) && (i == tmp[1]))
      {
	fr_len = i;
	sync_header = 1;
      }
      else
	sync_header = 0;
    }
    else
      sync_header = 0;

    /* Rewind file */
    fseek(Fibs, 0l, SEEK_SET);
  }

  /* If input BS is headerless, any frame size will do; using default */
  if (fr_len == 0)
    fr_len = blk;


  /* Also, reset sync if BS is compact */
  if (bs_format == compact && sync_header)
  {
    fprintf (stderr, "*** Disabling SYNC header for compact bitstream ***\n");
    sync_header = 0;
  }

  /* *** FINAL INITIALIZATIONS *** */

  /* Use the proper data I/O functions */
  read_patt = ep_format==byte? read_byte
              : (ep_format==g192? read_g192 :
		 (ep_type==BER? read_bit_ber : read_bit_fer));
  save_patt = ep_format==byte? save_byte
              : (ep_format==g192? save_g192 : save_bit);


  /* Define how many samples are read for each frame */
  /* Bitstream may have sync headers, which are 2 samples-long */
  bs_len = sync_header? fr_len + 2 : fr_len;
  ep_len = fr_len;

  /* Allocate memory for data buffers */
  if ((master = (short *)calloc(fr_len, sizeof(short))) == NULL)
    HARAKIRI("Can't allocate memory for master error pattern. Aborted.\n",6);
  if ((new = (short *)calloc(fr_len, sizeof(short))) == NULL)
    HARAKIRI("Can't allocate memory for new error pattern. Aborted.\n",6);

  /* Initializes to the start of the payload in input bitstream */
  payload = sync_header? bs + 2: bs;

  /* *** START ACTUAL WORK *** */

  switch(ep_type)
  {
  case FER:
    k = 0;
    while(1)
    {
      /* Read one frame from bitstream */
      items = read_data (bs, bs_len, Fibs);

      /* Stop when reaches end-of-file */
      if (items==0)
	break;

      /* Aborts on error */
      if (items < 0)
	KILL(ibs_file, 7);

      /* Check if read all expected samples; if not, take a special action */
      if (items < bs_len)
      {
	if (sync_header)
	{
	  /* If the bitstream has sync header, this situation should
	     not occur, since the length of the input bitstream file
	     should be a multiple of the frame size! The file is
	     either invalid otr corrupt. Execution is aborted at this
	     point */
	  HARAKIRI("File size not multiple of frame length. Aborted!\n", 9);
	}
	else if (feof(Fibs))
	{
	  /* EOF reached. Since the input bitstream is headerless,
             this maybe a corrupt file, or the user simply specified
             the wrong frame size. Warn the user and continue */
	  fprintf(stderr, "*** File size not multiple of frame length ***\n");
	  bs_len = fr_len = items;
	}
	else /* An unknown error happened! */
	  KILL(ibs_file, 7);
      }
      /* Read a number of erasure flags from file */
      while (k==0)
      {
	/* No EP flags in buffer; read a number of them */
	k = read_patt (ep, ep_len, Fep);

	/* No flags read - either error or EOF */
	/* Go back to beginning of EP & fill up EP buffer */
	if (k<=0)
	{
	  if (k < 0)
	    KILL(ep_file, 7);	    /* Error: abort */
	  fseek(Fep, 0l, SEEK_SET); /* EOF: Rewind */
	}

	/* Count how many times wrapped the EP file */
	wraps++;
      }

      /* Update frame counters */
      proc_master++;

      /* Save original or erased frame, as appropriate */
      if (ep[ep_len - k] == G192_FER)
      {
	items = save_data (erased_frame, bs_len, Fobs);
	disturb_master ++;
      }
      else
	items = save_data (bs, bs_len, Fobs);

      /* Abort on error */
      if (items < bs_len)
	KILL(obs_file, 7);

      /* Decrement counter of number of flags in EP buffer */
      k--;
    }
    break;

  case BER:
    while(1)
    {
      /* Read one frame from bitstream */
      items = read_data (bs, bs_len, Fibs);

      /* Stop when reaches end-of-file */
      if (items==0)
	break;

      /* Aborts on error */
      if (items < 0)
	KILL(ibs_file, 7);

      /* Check if read all expected samples; if not, probably hit EOF */
      if (items < bs_len)
      {
	if (sync_header)
	{
	  /* If the bitstream has sync header, this situation should
	     not occur, since the length of the input bitstream file
	     should be a multiple of the frame size! The file is
	     either invalid otr corrupt. Execution is aborted at this
	     point */
	  HARAKIRI("File size not multiple of frame length in header. Aborted!\n", 9);
	}
	else if (feof(Fibs))
	{
	  /* EOF reached. Since the input bitstream is headerless,
             this maybe a corrupt file, or the user simply specified
             the wrong frame size. Warn the user and continue */
	  fprintf(stderr, "*** File size not multiple of given frame length ***\n");
	  bs_len = fr_len = items;
	}
	else /* An unknown error happened! */
	  KILL(ibs_file, 7);
      }

      /* Read one error pattern frame from file */
      items = read_patt (ep, ep_len, Fep);

      /* Treat case when EP finishes before BS: */
      /* Go back to beginning of EP & fill up EP buffer */
      if (items < ep_len)
      {
	if (items < 0)
	  KILL(ep_file, 7);
	k = ep_len - items; /* Number of missing EP samples */
	fseek(Fep, 0l, SEEK_SET); /* Rewind */
	items = read_patt (&ep[items], k, Fep); /* Fill-up EP buffer */

	/* Count how many times wrapped the EP file */
	wraps++; 
      }

      /* Convolve errors */
      items = insert_errors (payload, ep, payload, fr_len);

      /* Update BER counters */
      disturb_master += items;
      proc_master += fr_len;

     /* Save disturb_master bitstream to file */
      items = save_data (bs, bs_len, Fobs);

      /* Abort on error */
      if (items < bs_len)
	KILL(obs_file, 7);
    }
    break;
  }

  /* ***  PRINT SUMMARY OF OPTIONS & RESULTS ON SCREEN *** */


  /* Print summary */
  fprintf (stderr, "# Pattern format %s....... : %s\n",
	   ep_type == FER? "(Frame erasure) " : "(Bit error) ....", 
	   format_str((int)ep_format));
  fprintf (stderr, "# Frame size ............................: %ld\n",
	   fr_len);
  fprintf (stderr, "# Processed %s..................... : %.0f \n",
	   ep_type==BER? "bits .." : "frames ", proc_master);
  fprintf (stderr, "# Master EP - Distorted %s.......... : %.0f \n",
	   ep_type==BER? "bits .." : "frames ", disturb_master);
  fprintf (stderr, "# Master EP %s...........: %f %%\n",
	   ep_type==BER? "Bit error rate ..." : "Frame erasure rate", 
	   100.0 * disturb_master / proc_master);
  fprintf (stderr, "# Output EP - Distorted %s.......... : %.0f \n",
	   ep_type==BER? "bits .." : "frames ", disturb_new);
  fprintf (stderr, "# Output EP %s...........: %f %%\n",
	   ep_type==BER? "Bit error rate ..." : "Frame erasure rate", 
	   100.0 * disturb_new / proc_new);


  /* *** FINALIZATIONS *** */

  /* Free memory allocated */
  free(new);
  free(master);

  /* Close the output file and quit *** */
  fclose (Fi);
  fclose (Fo);

#ifndef VMS			/* return value to OS if not VMS */
  return 0;
#endif
}

#endif /* Scheleton */

