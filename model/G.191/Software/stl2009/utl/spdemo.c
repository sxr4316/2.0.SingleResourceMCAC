/*                                                            02.Feb.2010 v3.2 
  ============================================================================

  SPDEMO.C
  ~~~~~~~~~

  Description: 
  ~~~~~~~~~~~~

  DEMO program to show the use of the serialization/
  parallelization functions.

  A input file in the parallel format will be converted to serial
  in the form of 16-bit words whose least significant bits will be
  `1' or `0', as defined below. If the inclusion of a synchronism
  bit is requested, then at the beginning of each data frame a
  sync word (defined below) is added.

                      Bit '0' is represented as '0x007F'
                      Bit '1' is represented as '0x0081'
                      A SYNC-word is defined as '0x6B21'

  This bit definition was chosen to be compatible in the future 
  with the so called 'soft-bit'-format, where the  channel decoder
  outputs probabilities that the  received bit is '0' or '1'. 

  The output bit-stream will then be a binary file where the first
  word on the file is the SYNC-word, followed by LFRAME
  words with the data bits (LFRAME=number of bits in one frame);
  then the next SYNC-word, followed by the next frame, ... and so
  on (if the sync word inclusion is selected). 


  Usage:
  ~~~~~~
  $ spdemo oper finp fout N N1 N2 resolution [sync [just]]
  where:
  oper ......... operation desired:
                 ps input is in parallel format, output is serial
                 sp input is a bit-stream file, output is parallel.
  finp ......... input filename
  fout ......... output filename
  N ............ frame size
  N1 ........... 1st frame to convert
  N2 ........... number of frames from N1 on.
  resolution ... number of bits per sample in input file.
  sync ......... flag for using sync words:
                 1: each frame begin with a SYNC_WORD in the
                    output file; this is the DEFAULT.
                 0: no SYNC_WORDs are used in the output file.
  just ......... flag for choosing justification: 
                 left:  the parallel data is supposed to be right-
		        justified; this is the DEFAULT.
                 right: the parallel data is supposed to be left-
                        justified;
                 (PS: bit-stream 16-bit words are always right-justified!)
  Options:
  -n ............. number of frames to process
  -start ......... first frame to start operation
  -frame ......... payload size in no. of softbits
  -res # ......... bit resolution for file (default 16)
  -left .......... data is left-justified
  -right ......... data is right-justified (the default)
  -nosync ........ don't use sync headers
  -sync .......... use sync headers
  -q ............. quiet operation
  -help, -? ...... display help message

  Compilation:
  ~~~~~~~~~~~~

  VAX/VMS: 
  $ cc spdemo
  $ link spdemo 
  $ spdemo   :== $scd_disk:[scd_dir]spdemo 
  $ spdemo 

  Turbo-C, Turbo-C++: 
  > tcc -ml spdemo
  > spdemo 

  HighC (MetaWare, version R2.32): 
  > hc386 -ml spdemo.c  
  > run386 spdemo 

  Sun-C (BSD-Unix)
  # cc -o spdemo spdemo.c
  # spdemo

        
  Original author: 
  ~~~~~~~~~~~~~~~~
  Simao Ferraz de Campos Neto       
  CPqD/Telebras         		             
  DDS/Pr.11                             Phone : +55-192-39-6396
  Rd. Mogi Mirim-Campinas Km.118        Fax   : +55-192-53-6125
  13.088-061 - Campinas - SP (Brazil)   EMail : simao@cpqd.ansp.br
                                          
  History
  ~~~~~~~
  20.Mar.92 v1.0 1st release to UGST.
                 <tdsimao@venus.cpqd.ansp.br>
  18.May.92 v1.1 Introduction of changes from v1.2 of ugst-utl.c.
                 <tdsimao@venus.cpqd.ansp.br>
  14.Apr.94 v1.2 Improved user interface
  06.Jun.95 v2.0 Fixed for correct '1' and '0' softbit definition
                 <simao.campos@comsat.com>
  20.Aug.97 v3.0 Several changes: <simao.campos@comsat.com>
                 - Modified code to recognize the G.192 bitstream format
		 - Removed obsolete code portions (memory allocation for
		   input & output files being both parallel - this is
		   prevented by a previous piece of code!)
  16.Jan.98 v3.1 Added conditional compile for Ultrix with cc <simao>
  18.Jan.99 v3.2 Fixed bug in calculation of total no.of samples 
                 to process; fixed a bug in check_sync() which
                 unnecessarily zeroed the frame length variable fr_len
                 causing errors when the user specifies the frame length
                 <simao>
  02.Feb.10 v3.3 Modified maximum string length to avoid buffer overruns
                 (y.hiwasaki)
  ============================================================================
*/

/* ..... Generic include files ..... */
#include <stdio.h>                      /* Standard I/O Definitions */
#include <stdlib.h>                     /* for atoi(), atol() */
#include <string.h>                     /* for strstr() */
#include <math.h>                       /* for ceil() */
#include "ugstdemo.h"                   /* general UGST definitions */

/* ..... Specific include files ..... */
#if defined(VMS)		/* for checking file sizes */
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#if defined (unix) && !defined(MSDOS)
/*                 ^^^^^^^^^^^^^^^^^^ This strange construction is necessary
                                      for DJGPP, because "unix" is defined,
				      even it being MSDOS! */
#if defined(__ALPHA)
#include <unistd.h>		/* for SEEK_... definitions used by fseek() */
#else
#include <sys/unistd.h>		/* for SEEK_... definitions used by fseek() */
#endif
#endif


/* ..... Module definition files ..... */
#include "ugst-utl.h"                   /* format conversion functions */


/* ... Local function prototypes ... */
char check_bs_format ARGS((FILE *F, char *file, char *type));
int check_sync ARGS((FILE *F, char *file, char *bs_type, long *fr_len,
		     char *bs_format));
void display_usage ARGS((int level));
/* ......... End of local function prototypes ......... */ 


/* ... Local defines, pseudo-functions ... */
#define is_serial(x) ((x)=='S'||(x)=='s')

#ifdef STL92
#define OVERHEAD 1 /* Overhead is sync word */
#else
#define OVERHEAD 2 /* Overhead is sync word and length word*/
#endif

/* Operating modes */
enum BS_formats {byte, g192, compact, nil};
enum BS_types {NO_HEADER, HAS_HEADER};


/* ************************* AUXILIARY FUNCTIONS ************************* */

/*
  --------------------------------------------------------------------------
  char check_bs_format (FILE *F, char *file, char *type);
  ~~~~~~~~~~~~~~~~~~~~

  Function that checks the format (g192, byte, bit) in a given
  bitstream, and tries to guess the type of data (bit stream or frame
  erasure pattern)

  Parameter:
  ~~~~~~~~~~
  F ...... FILE * structure to file to be checked
  file ... name of file to be checked
  type ... pointer to guessed data type (NO_HEADER or HAS_HEADER) in file

  Returned value:
  ~~~~~~~~~~~~~~~
  Returns the data format (g192, byte, bit) found in file.

  Original author: <simao.campos@comsat.com>
  ~~~~~~~~~~~~~~~~

  History:
  ~~~~~~~~
  20.Aug.97  v.1.0  Created.
  -------------------------------------------------------------------------- 
*/
char check_bs_format(F, file, type)
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
    *type = NO_HEADER;
    ret_val = byte;
    break;

  case 0x2020:
  case 0x2021:
  case 0x2120:
  case 0x2121:
    /* Byte-oriented G.192 syncs */
    *type = HAS_HEADER;
    ret_val = byte;
    break;

  case 0x007F:
  case 0x0081:
    /* G.192 bitstream in natural order */
    *type = NO_HEADER;
    ret_val = g192;
    break;

  case 0x6B21:
  case 0x6B20:
    /* G.192 sync header in natural order */
    *type = HAS_HEADER;
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
     *type = HAS_HEADER;
     ret_val = byte;
  }
  /* Rewind file & and return format identifier */
  fseek(F, 0l, SEEK_SET);
  return(ret_val);
}
/* ...................... End of check_bs_format() ...................... */


int check_sync(F, file, bs_type, fr_len, bs_format)
FILE *F;
char *file;
char *bs_type, *bs_format;
long *fr_len;
{
  long i;
  char sync_header;

  /* Initialize frame length to "unknown" */
  /* *fr_len = 0; */

  /* Get info on bitstream */
  *bs_format = check_bs_format(F, file, bs_type);

  /* If the BS seems to have a header, search for two consecutive
     ones. If it finds, determines which is the frame size */
  if (*bs_type == HAS_HEADER)
  {
    /* The input BS may have a G.192 synchronism header - verify */
    if (*bs_format == g192)
    {
      short tmp[2];

      /* Get presumed first G.192 sync header */
      fread(tmp, sizeof(short), 2, F);
      /* tmp[1] should have the frame length */
      i = tmp[1];
      /* advance file to the (presumed) next G.192 sync header */
      fseek(F, (long)(tmp[1])*sizeof(short), SEEK_CUR);
      /* get (presumed) next G.192 sync header */
      fread(tmp, sizeof(short), 2, F);
      /* Verify */
      if (((tmp[0] & 0xFFF0) == 0x6B20) && (i == tmp[1]))
      {
	*fr_len = i;
	sync_header = 1;
      }
      else
	sync_header = 0;
    }
    else if (*bs_format == byte)
    {
      char tmp[2];

      /* Get presumed first byte-wise G.192 sync header */
      fread(tmp, sizeof(char), 2, F);
      /* tmp[1] should have the frame length */
      i = tmp[1];
      /* advance file to the (presumed) next byte-wise G.192 sync header */
      fseek(F, (long)tmp[1], SEEK_CUR);
      /* get (presumed) next G.192 sync header */
      fread(tmp, sizeof(char), 2, F);
      /* Verify */
      if (((tmp[0] & 0xF0) == 0x20) && (i == tmp[1]))
      {
	*fr_len = i;
	sync_header = 1;
      }
      else
	sync_header = 0;
    }
    else
      sync_header = 0;

    /* Rewind file */
    fseek(F, 0l, SEEK_SET);
  }
  else
    sync_header = 0;

  /* Return info on whether or not a frame sync header was found */
  return(sync_header);
}

/*
  --------------------------------------------------------------------------
  display_usage()

  Shows program usage.

  History:
  ~~~~~~~~
  20/Aug/1997  v1.0 Created <simao>
  --------------------------------------------------------------------------
*/
#define P(x) printf x
void display_usage(level)
int level;
{
  P(("spdemo.c - version 3.2 of 02.Feb.2010\n"));

  P(("  Demo program to convert between serial and parallel data formats.\n"));

  if (level == 1)
  {
    P(("  A input file in the parallel format will be converted to serial\n"));
    P(("  in the form of 16-bit words whose least significant bits will be\n"));
    P(("  `1' or `0', as defined below. If the inclusion of a synchronism\n"));
    P(("  bit is requested, then at the beginning of each data frame a\n"));
    P(("  sync word (defined below) is added.\n"));
    P(("\n"));
    P(("                      Bit '0' is represented as '0x007F'\n"));
    P(("                      Bit '1' is represented as '0x0081'\n"));
    P(("                      A SYNC-word is defined as '0x6B21'\n"));
    P(("\n"));
    P(("  This bit definition was chosen to be compatible in the future \n"));
    P(("  with the so called 'soft-bit'-format, where the  channel decoder\n"));
    P(("  outputs probabilities that the  received bit is '0' or '1'. \n"));
    P(("\n"));
    P(("  The output bit-stream will then be a binary file where the first\n"));
    P(("  word on the file is the SYNC-word, followed by LFRAME\n"));
    P(("  words with the data bits (LFRAME=number of bits in one frame);\n"));
    P(("  then the next SYNC-word, followed by the next frame, ... and so\n"));
    P(("  on (if the sync word inclusion is selected). \n"));
    P(("\n"));
  }

  P(("  Usage:\n"));
  P(("  $ spdemo oper finp fout N N1 N2 resolution [sync [just]]\n"));
  P(("  where:\n"));
  P(("  oper ......... \"ps\": parallel -> serial; \"sp\": serial -> parallel\n"));
  P(("  finp ......... input filename\n"));
  P(("  fout ......... output filename\n"));
  P(("  N ............ frame size\n"));
  P(("  N1 ........... 1st frame to convert\n"));
  P(("  N2 ........... number of frames from N1 on.\n"));
  P(("  resolution ... number of bits per sample in input file.\n"));
  P(("  sync ......... Enable (1) or disable (0) use of sync headers\n"));
  P(("  just ......... data justification: left or right (default)\n\n"));
  P(("  Options:\n"));
  P(("  -n # ........... number of frames to process\n"));
  P(("  -start # ....... first frame to start operation\n"));
  P(("  -frame # ....... payload size in no. of softbits\n"));
  P(("  -res # ......... bit resolution for file (default 16)\n"));
  P(("  -left .......... data is left-justified\n"));
  P(("  -right ......... data is right-justified (the default)\n"));
  P(("  -nosync ........ don't use sync headers\n"));
  P(("  -sync .......... use sync headers\n"));
  P(("  -q ............. quiet operation\n"));
  P(("  -? ............. display short help message\n"));
  P(("  -help, ......... display long help message\n"));

  /* Quit program */
  exit(-128);
}
#undef P
/* .................. End of display_usage() ....................... */ 

char *format_str(i)
int i;
{
  switch(i)
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


/* ------------------------------------------------------------------------ */
/*                              Main Program                                */
/* ------------------------------------------------------------------------ */
int main(argc, argv)
int argc;
char *argv[];
{
  long            N = 256, N1 = 1, N2 = 0, smpno, size, i;
  long            fr_len = 0;           /* Frame length in number of bits */
  long            bitno, cur_blk, resolution = 16;
  char            bs_type = HAS_HEADER; /* Type for bitstream */
  char            bs_format = g192;     /* Format for bitstream */
  long            Nin, Nout;      /* Frame length for input and input files */
  short          *par_buf;	        /* parallel-aligned samples' buffer */
  short          *bit_stm;	        /* bit-stream data */
  char            inpfil[MAX_STRLEN], outfil[MAX_STRLEN];
  FILE           *inpfilptr, *outfilptr;
  int             inp, out;
  long            start_byte;
  char            sync=1, oper[MAX_STRLEN];
  static char     just[8] = "right";
  short           inp_type, out_type;
  /*   char            format_str[4][5] = {{"byte"}, {"g192"}, {"bit"}, {""}}; */
#ifdef VMS
  char            mrs[15];	/* for correct mrs in VMS environment */
#endif
  long		 (*serialize_f)();    /* pointer to serialization routine */
  long           (*parallelize_f)();  /* pointer to parallelization routine */
  char quiet=0;


  /* ......... GET PARAMETERS ......... */

  /* Check options */
  if (argc < 2)
    display_usage(0);
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp (argv[1], "-start") == 0)
      {
	/* Define starting sample for s/p operation */
	N1 = atol (argv[2]);

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
      else if (strcmp (argv[1], "-n") == 0)
      {
	/* Define number of frames to process */
	N2 = atoi(argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp(argv[1],"-r")==0 || strcmp(argv[1],"-res")==0)
      {
	/* Define resolution */
	resolution = atol(argv[2]);

	/* Move arg{c,v} over the option to the next argument */
	argc -= 2;
	argv += 2;
      }
      else if (strcmp(argv[1], "-left") == 0 || 
               strcmp(argv[1], "-LEFT") == 0)
      {
	/* Data is left-justified */
	strcpy(just, "left");
	
	/* Update arg[vc] */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-right") == 0 || 
               strcmp(argv[1], "-RIGHT") == 0)
      {
	/* Data is right-justified */
	strcpy(just, "right");
	
	/* Update arg[vc] */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-sync") == 0)
      {
	/* Use sync header */
	sync=1;
	
	/* Update arg[vc] */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-nosync") == 0)
      {
	/* Don't use sync header */
	sync=0;
	
	/* Update arg[vc] */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-q") == 0)
      {
	/* quiet operation - don't print progress flag */
	quiet=1;
	
	/* Update arg[vc] */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0)
      {
	/* Display help */
	display_usage(0);
      }
      else if (strstr(argv[1], "-help"))
      {
	/* Display help */
	display_usage(1);
      }
      else if (strcmp(argv[1], "-") == 0)
      {
	/* End of option parsing */
	break;
      }
      else
      {
	fprintf(stderr, "ERROR! Invalid option \"%s\" in command line\n\n",
		argv[1]);
	display_usage(0);
      }
  }

  /* Read parameters for processing */

  GET_PAR_S(1,  "_Operation (ps,sp): .......... ", oper);
  GET_PAR_S(2,  "_File to be converted: ....... ", inpfil);
  GET_PAR_S(3,  "_Output File: ................ ", outfil);
  FIND_PAR_L(4, "_Block Length: ............... ", N, N);
  FIND_PAR_L(5, "_Start Block: ................ ", N1, N1);
  FIND_PAR_L(6, "_No. of Blocks: .............. ", N2, N2);
  FIND_PAR_L(7, "_Resolution of input data: ... ", resolution, resolution);
  FIND_PAR_I(8, "_Sync header? (1=Yes,0=No) ... ", sync, sync);
  FIND_PAR_S(9, "_Left of right adjusted? ..... ", just, just);

 /* Classification of the conversion desired */
  inp_type = is_serial(oper[0])? IS_SERIAL : IS_PARALLEL;
  out_type = is_serial(oper[1])? IS_SERIAL : IS_PARALLEL;

  /* Verify if valid input/output combination */
  /* If input is parallel, output must be serial */
  /* If input is serial, output must be parallel */
  if (inp_type == out_type)
    HARAKIRI(" Invalid input/output combination of option [sp|ps]!\n", 7);

 /* Verify if sync is valid; if not, change to 1 */
  if (sync!=1 && sync!=0)
    sync = 1;

 /* Definition of the routines in function of the justification */
  if (strcmp(just, "right")==0 || strcmp(just, "RIGHT")==0)
  {
     serialize_f = serialize_right_justified;
     parallelize_f = parallelize_right_justified;
  }
  else
  {
     serialize_f = serialize_left_justified;
     parallelize_f = parallelize_left_justified;
  }

  /* ***** FILE OPERATIONS ***** */

#ifdef VMS
  sprintf(mrs, "mrs=%d", 512);
#endif

  /* Open input file */
  if ((inpfilptr = fopen(inpfil, RB)) == NULL)
    KILL(inpfil, 2);
  inp = fileno(inpfilptr);

  /* Open (create) output file */
  if ((outfilptr = fopen(outfil, WB)) == NULL)
    KILL(outfil, 3);
  out = fileno(outfilptr);

  /* Define 1st byte to process */
  start_byte = N * (--N1) * sizeof(short);
  
  /* Move file's pointer to 1st desired block */
  if (fseek(inpfilptr, start_byte, 0) < 0l)
    KILL(inpfil, 4);

  /* ***** DEFINE FRAME SIZES AND FILE TYPES ***** */

  /* Get input and output frame sizes */
  if (inp_type==IS_SERIAL && out_type==IS_PARALLEL)
  { /* SP mode: */
    /* Find the actual frame size for the serial bit stream from the
       serial bitstream. If this is a headerless bitstream, assumes that
       the frame size is based on the provided block size. */

    /* Find the number of words per frame (payload only!), 
       check whether a sync header was found, and also returns the type 
       of bitstream (g192, byte, compact) found */
    i = check_sync(inpfilptr, inpfil, &bs_type, &fr_len, &bs_format);

    /* If bitstream is not g192, abort */
    if (bs_format != g192)
      HARAKIRI("Bitstream needs to be in G.192 format. Aborted.\n", 7);

    /* If sync info in file differs from the users' provided
       input, warns and changes to the value found in file */
    if (sync != i)
    {
      fprintf (stderr, "*** Switching sync from %d to %ld ***\n",
	       sync, i);
      sync = i;
    }

    /* If input BS is headerless, any frame size will do; use default */
    if (fr_len==0)
      fr_len = N * resolution;

    /* Set Nin to length of payload + header */
    Nin = fr_len + (sync ? OVERHEAD : 0);

    /* Increase Ni by size of header, if present */
    /* Output frame size is the # of bits per frame divided
       by the resolution */
    Nout = Nin/resolution;

    /* Print info */
    fprintf(stderr, "# Input has %ld bits/frame %s sync header\n", 
	    Nin, sync?"with":"without");
    fprintf(stderr, "# Output data has %ld %ld-bit parallel samples/frame\n",
	    Nout, resolution);
  }
  else
  { /* PS mode: Input is parallel and output is serial */

    /* Input frame size is as provided by the user */
    Nin = N; 

    /* Output frame size defined by Nin and the resolution */
    fr_len = N*resolution; /* Payload only */
    Nout = fr_len  + (sync ? OVERHEAD : 0); /* Payload + header */

    /* Print info */
    fprintf(stderr, "# Input data has %ld %ld-bit parallel samples/frame\n",
	    Nin, resolution);
    fprintf(stderr, "# Output has %ld bits/frame %s sync header\n", 
	    Nout, sync?"with":"without");
  }

  /* The following is a hook for future extensions */
  switch(bs_format)
  {
  case g192:
    size = sizeof(short);
    break;
  case byte:
    size = sizeof(char);
    break;
  case compact:
    size = sizeof(char);
    break;
  }

  /* ***** ALLOCATION OF BUFFERS ***** */

  /* Allocate memory for input and output buffers */
  if (inp_type==IS_SERIAL && out_type==IS_PARALLEL)
  {
    if ((bit_stm = (short *) calloc(Nin, size)) == NULL)
          HARAKIRI("Can't allocate memory for input buffer\n", 10);
    if ((par_buf = (short *) calloc(Nout, sizeof(short))) == NULL)
          HARAKIRI("Can't allocate memory for output buffer\n", 11);
  }
  else if (inp_type==IS_PARALLEL && out_type==IS_SERIAL) 
  {
    if ((par_buf = (short *) calloc(Nin, sizeof(short))) == NULL)
          HARAKIRI("Can't allocate memory for input buffer\n", 10);
    if ((bit_stm = (short *) calloc(Nout, size)) == NULL)
          HARAKIRI("Can't allocate memory for output buffer\n", 11);
  }


  /* Check if is to process the whole file */
  if (N2 == 0)
  {
    struct stat     st;

    stat(inpfil, &st);
    N2 = ceil((st.st_size - start_byte) / (double)(Nin * size));
  }


  /* ***** PRINT PRELIMINARY INFORMATION ***** */
  fprintf(stderr, "# Operation: %s\n", oper);
  fprintf(stderr, "# Resolution: %ld\n", resolution);
  fprintf(stderr, "# Bitstream format: %s\n", format_str((int)bs_format));
  fprintf(stderr, "# Header: %s\n", sync? "present" : "none");
  fprintf(stderr, "# Payload/total frame size: %ld / %ld\n",
	  fr_len, fr_len + (sync ? OVERHEAD : 0) );


  /* ***** CARRY OUT CONVERSION ***** */
  
  smpno=bitno=0;

  if (inp_type==IS_SERIAL && out_type==IS_PARALLEL)
  {
    /* Performs the serial-to-parallel conversion */
    for (cur_blk = 0; cur_blk < N2; cur_blk++)
      {
        if (!quiet)
	  fprintf(stderr, "\rProcessing block %ld\t", cur_blk+1);

	if ((bitno = fread(bit_stm, size, Nin, inpfilptr)) < 0)
	  KILL(inpfil, 5);

	i += parallelize_f
  	     (bit_stm,    /* input buffer pointer */
	      par_buf,    /* output buffer pointer */
	      bitno,      /* number of bits (not samples) per frame */
	      resolution, /* number of bits per sample */
	      sync);      /* whether sync header is present or not */

	/* Check if error was returned */
	if (i<0)
	{
	  fprintf(stderr, "Returned error %ld in S/P function; aborting\n", i);
	  exit(10);
	}
	if (i==0)
	  fprintf(stderr, "Detected erased frame at frame no. %ld; skipping\n",
		 cur_blk);
	else
	  smpno += i;

	if ((bitno = fwrite(par_buf, sizeof(short), Nout, outfilptr)) < 0)
	  KILL(outfil, 6);
      }
      fprintf(stderr, "# Total %ld samples parallelized\n", smpno);
  }
  else if (inp_type==IS_PARALLEL && out_type==IS_SERIAL)
  {
    /* Performs the parallel-to-serial conversion */
    for (cur_blk = 0; cur_blk < N2; cur_blk++)
      {
        if (!quiet)
	  fprintf(stderr, "\rProcessing block %ld\t", cur_blk+1);
	if ((smpno = fread(par_buf, sizeof(short), Nin, inpfilptr)) < 0)
	  KILL(inpfil, 5);
	bitno += serialize_f
                  (par_buf,     /* input buffer pointer */
                   bit_stm,     /* output buffer pointer */
                   N,           /* number of samples (not bits!) per frame */
                   resolution,  /* number of bits per sample */
                   sync);       /* whether sync header is present or not */

	if ((smpno = fwrite(bit_stm, size, Nout, outfilptr)) < 0)
	  KILL(outfil, 6);
      }
      fprintf(stderr, "# Total %ld bits produced\n", bitno);
  }


  /* ***** EXITING ***** */

  /* Release memory */
  free(bit_stm);
  free(par_buf); 

  /* And close files! */
  fclose(outfilptr);
  fclose(inpfilptr);
  
  /* Return OK when not VMS */
#ifndef VMS
  return(0);
#endif
}
/* ....................... End of main() program ....................... */ 
