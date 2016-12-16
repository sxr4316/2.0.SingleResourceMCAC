/**************************************************************************

                (C) Copyright 1990, Motorola Inc., U.S.A.

Note:  Reproduction and use for the development of North American digital
       cellular standards or development of digital speech coding
       standards within the International Telecommunications Union -
       Telecommunications Standardization Sector is authorized by Motorola 
       Inc.  No other use is intended or authorized.

       The availability of this material does not provide any license
       by implication, estoppel, or otherwise under any patent rights
       of Motorola Inc. or others covering any use of the contents
       herein.

       Any copies or derivative works must incude this and all other
       proprietary notices.

       Permision has been granted to include this software in ITU-T
       Software Tool Library, for the only purpose of helping the
       development of new ITU standards.


Systems Research Laboratories
Chicago Corporate Research and Development Center
Motorola Inc.

************************************************************************* */

/*
  -------------------------------------------------------------------------
  vselp.c
  ~~~~~~~
  C Language Version of the IS54 VSELP Speech Coder. Reference
  implementation carried out by Motorola.

  Original author: Matt Hartman

  Adapted to ITU-TSS Software Tool Library by: Simao F.Campos Neto

  Usage:
  vselp [-urf urffile] [-bin obsfile] [-hex obsfile] [-log logfile]
        [-nolog] [-pf] [enc] [-dec]
        srcfile decfile logfile obsfile, OR (WHEN -enc USED)
        srcfile ibsfile logfile,         OR (WHEN -dec USED)
        srcfile decfile logfile          (DEFAULT)

  where:
  srcfile ........ name of input file
  decfile ........ name of output, decoded file [ignored for encode_only mode]
  ibsfile ........ input bitstream file [ignored for encode_only|encode+decode]
  logfile ........ log file name
  obsfile ........ bitstream gen'd for srcfile [only w/encode_only|encode+decode]
  Options:
  -urf urffile ... use the user's response file "urffile"
  -bin ........... coded bitstream should be saved to a binary file 
  -hex ........... coded bstream should be saved to an ascii hex file [default]
  -log logfile ... log of processing results is to be saved in file "logfile"
  -nolog ......... do not log results
  -enc ........... run encoder only [default: run encode + decode]
  -dec ........... run decoder only [default: run encode + decode]
  -pf ............ use post-filter in decoder

  History:
  08/May/90 v.1.0  Created <Matt Hartman>
  20/Mar/94 v.1.1  Adapted to UGST format <simao@cpqd.ansp.br>
  08/Feb/95 v.1.2  Fixed error in self-documentation and help message
                   <simao@ctd.comsat.com>
  22/Feb/96 v.1.2  Adapted code for operation in a DEC Alpha/APX (after 
                   STEGMANN, FI/DBP Telekom)
  02/Feb/10 v.1.3  Modified maximum string length (y.hiwasaki)
 -------------------------------------------------------------------------
*/

/* Include files */
#include "vselp.h"
#include "ugstdemo.h"
#include "vparams.h"

#ifdef __TURBOC__
unsigned        _stklen = 50000;/* Set Stack Size for Turbo C */
#endif

#ifdef VMS
#undef WT
#define WT "w" /* Plain write file definition for fopen() in VMS */
#endif

/* Global vars for communication with other routines */
int             apply_postfilter, encode_only, decode_only;


/* ------------------------------------------------------------------------ */
long swap_byte(shPtr, n)
short *shPtr;
long n;
{
  short *s_Ptr;
  short register tmp;
  long register count;
  
  for (count=0; count<n; count++)
  {
    tmp = (*shPtr << 8) + ((*shPtr >> 8) & 0x00ff);
    *shPtr++ = tmp;
  }
  return((long)count);
}
/* ......................... End of swap_byte() ............................ */


/* ------------------------------------------------------------------------ */
void initState()
{

  T_NEW.k = (FTYPE *) calloc(14 * NP, sizeof(FTYPE));	/* Note: it is  */
  T_NEW.a = T_NEW.k + NP;	/* necessary that these coef sets be */
  T_NEW.widen = T_NEW.a + NP;	/* stored sequentially. DONT REARRANGE. */

  T_OLD.k = T_NEW.widen + NP;	/* Note: it is necessary that these */
  T_OLD.a = T_OLD.k + NP;	/* coef sets be stored sequentially. */
  T_OLD.widen = T_OLD.a + NP;	/* DONT REARRANGE. */

  R_NEW.k = T_OLD.widen + NP;	/* Note: it is necessary that these */
  R_NEW.a = R_NEW.k + NP;	/* coef sets be stored sequentially. */
  R_NEW.widen = R_NEW.a + NP;	/* DONT REARRANGE. */
  R_NEW.sst = R_NEW.widen + NP;

  R_OLD.k = R_NEW.sst + NP;	/* Note: it is necessary that these */
  R_OLD.a = R_OLD.k + NP;	/* coef sets be stored sequentially. */
  R_OLD.widen = R_OLD.a + NP;	/* DONT REARRANGE. */
  R_OLD.sst = R_OLD.widen + NP;

}
/* ......................... End of initState() ............................ */

/* ------------------------------------------------------------------------ */
#define P(x) printf x
void            display_usage()
{
  P(("  vselp: version 1.10 of 14/Mar/1994 \n\n"));

  P(("  C Language Version of the IS54 VSELP Speech Coder. Reference\n"));
  P(("  implementation carried out by Motorola adapted to ITU-T Software\n"));
  P(("  Tool Library by Simao F. Campos Neto\n"));
  P(("\n"));
  P(("  Original author: Matt Hartman\n"));
  P(("\n"));
  P(("\n"));
  P(("  Usage:\n"));
  P(("  vselp [-urf urffile] [-bin obsfile] [-hex obsfile] [-log logfile] [-nolog]\n"));
  P(("        [-nolog] [-pf] [-enc] [-dec]\n"));
  P(("        srcfile decfile logfile obsfile, OR (WHEN -enc USED)\n"));
  P(("        srcfile ibsfile logfile,         OR (WHEN -dec USED)\n"));
  P(("        srcfile decfile logfile          (DEFAULT)\n\n"));

  P(("  where:\n"));
  P(("  srcfile ........ name of input file\n"));
  P(("  decfile ........ name of output, decoded file [ignored for encode_only mode]\n"));
  P(("  ibsfile ........ input bitstream file [ignored for encode_only|encode+decode]\n"));
  P(("  logfile ........ log file name\n"));
  P(("  obsfile ........ bstream gen'd for srcfile [only w/encode_only|encode+decode]\n"));
  P(("  Options:\n"));
  P(("  -urf urffile ... use the user's response file \"urffile\"\n"));
  P(("  -bin ........... coded bitstream should be saved to a binary file\n"));
  P(("  -hex ........... coded bstream should be saved to an ascii hex file [default]\n"));
  P(("  -log logfile ... log of processing results is to be saved in file \"logfile\"\n"));
  P(("  -nolog ......... do not log results\n"));
  P(("  -enc ........... run encoder only [default: run encode + decode]\n"));
  P(("  -dec ........... run decoder only [default: run encode + decode]\n"));
  P(("  -pf ............ use post-filter in decoder\n"));
  P(("\n"));

  /* Quit program */
  exit(-128);
}
#undef P
/* .......................... end of displau_usage() ......................... */


/*-------------------------------------------------------------------------*/
/* main program*/
int             main(argc, argv)
  int             argc;
  char           *argv[];
{
  /* Main's automatic variables */
  FTYPE          *coefBuf;	/* Points to memory allocated for T_NEW,
				 * T_OLD, R_NEW, and R_OLD coefficient
				 * storage */
  FTYPE           RQ_HOLD;	/* Temporary storage for the R0q value to use
				 * in the middle subframe on the receive side */
  FTYPE           RQ_TMP;	/* R0q value that gets passed to RES_ENG() */
  FTYPE           inScale = 1.0 / 32768.0;	/* Scale factor used to
						 * convert  integer input
						 * samples to floating point
						 * fractions */
  int             us;		/* Unstable flag returned by INTERPOLATE(),
				 * determines whether another RS will need to
				 * be calculated */
  short          *shBuf;	/* points to beginning of short I/O buffer */
  FILE           *fpprm;	/* points to parameter set-up file (if given) */

  FTYPE          *tmpPtr, *tmpPtr2, *endPtr, f1;
  int             i, numRead;
  short          *shPtr;
  long		(*get_codes)(), (*put_codes)();
  long            bs_read = 0, bs_saved = 0;
  char            use_user_resp_file = 0;	/* Don't use user's response
						 * file */
  char            InpFile[MAX_STRLEN], OutFile[MAX_STRLEN], LogFile[MAX_STRLEN], PackedFile[MAX_STRLEN];
#ifdef VMS
  char mrs[15];

  short *zero_vector;
  long zero_values,lwrite;  
#endif

  /* Diagnostic variables for debugging */
  int             finalCnt;

  /* Progress indication */
  static char     quiet, funny[9] = "|/-\\|/-\\";



  /* SETTING DEFAULT OPTIONS */
  decode_only = 0;		/* run encoder and decoder */
  encode_only = 0;
  packedStream = 1;		/* generate HEX-packed bitstreams */
  performMeas = 1;		/* calculate performance measures */
  makeLog = 1;
  apply_postfilter = 0;
  quiet = 0;
  finalCnt = 0;			/* always process the complete file */
  fpin = fpout = fpcode = fplog = fpstream = NULL;

#ifdef VMS
  sprintf (mrs, "mrs=%d", 512);
#endif

  /* GETTING OPTIONS */

  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-urf") == 0)
      {
	/* User response file */
	use_user_resp_file = 1;
	fpprm = fopen(argv[2], "r");
	getParams(fpprm);
	fclose(fpprm);

	/* Update argc/argv to next valid option/argument */
	argv += 2;
	argc -= 2;
      }
      else if (strcmp(argv[1], "-dec") == 0)
      {
	/* Run only the decoder */
	decode_only = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-enc") == 0)
      {
	/* Run only the encoder */
	encode_only = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-bin") == 0)
      {
	/* Save bitstream as an binary, 16-bit-word oriented file */
	packedStream = 0;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-hex") == 0)
      {
	/* Save bitstream as an ascii HEXadecimal file */
	packedStream = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-log") == 0)
      {
	/* Do create log file (default) */
	makeLog = 1;

	/* Scan log file name */
	strcpy(LogFile, argv[2]);
	if ((fplog = fopen(argv[2], "w")) == NULL)
	  HARAKIRI("ERROR creating log file\n", 2);

	/* Move argv over the option to the next argument */
	argv += 2;
	argc -= 2;
      }
      else if (strcmp(argv[1], "-nolog") == 0)
      {
	/* Don't create log file */
	makeLog = 0;
	fplog = stderr;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-pf") == 0)
      {
	/* Apply post-filtering in the decoder */
	apply_postfilter = 1;

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
      else
      {
	fprintf(stderr, "ERROR! Invalid option \"%s\" in command line\n\n",
		argv[1]);
	display_usage();
      }
  }



  /* WELCOME! */

  fprintf(stderr, "***************************************************************\n");
  fprintf(stderr, "\n           (C) Copyright 1990, Motorola Inc., U.S.A.\n\n");
  fprintf(stderr, "***************************************************************\n");


  /* Get values for calculated parameters */
  /* Three choices: */
  /*  - enc+dec:  read inp-lin-file out-lin-file logfile */
  /*  - enc-only: read inp-lin-file out-bitstream-file logfile */
  /*  - dec-only: read inp-bitstream-file out-lin-file logfile */
  if (!use_user_resp_file)
  {
    i = 1;
    if (!decode_only && fpin == NULL)
    {
      GET_PAR_S(i, "Input speech file: ........ ", InpFile);
      if ((fpin = fopen(InpFile, RB)) == NULL)
	KILL(InpFile, 2);
      i++;
    }

    if (decode_only && fpstream == NULL)
    {
      GET_PAR_S(i, "Input bit-stream file: .... ", InpFile);
      i++;
      if (packedStream)
      { 
        if ((fpstream = fopen(InpFile, "r")) == NULL)
	  HARAKIRI("Error opening code file\n", 2);
      }
      else
      {
        if ((fpstream = fopen(InpFile, RB)) == NULL)
	  HARAKIRI("Error opening code file\n", 2);
      }
    }

    if (encode_only && fpstream == NULL)
    {
      GET_PAR_S(i, "Output bit-stream file: ... ", OutFile);
      i++;
      if (packedStream)
      { 
        if ((fpstream = fopen(OutFile, "w")) == NULL)
	  HARAKIRI("Error creating code file\n", 2);
      }
      else
      {
        if ((fpstream = fopen(OutFile, WB)) == NULL)
	  HARAKIRI("Error creating code file\n", 2);
      }
    }

    if (!encode_only && fpout == NULL)
    {
      GET_PAR_S(i, "Output speech file: ....... ", OutFile);
      if ((fpout = fopen(OutFile, WB)) == NULL)
	KILL(OutFile, 3);
      i++;
    }

    if (makeLog || performMeas && fplog == NULL)
    {
      GET_PAR_S(i, "Log file name: .............", LogFile);
      i++;
      fplog = fopen(LogFile, "w");
    }
  }

  /* Define bitstream read/write functions */
  get_codes = packedStream
  		? getCodesHex
  		: getCodesBin;
  put_codes = packedStream
  		? putCodesHex
  		: putCodesBin;

  /* Initialize parameters that are fixed but need to be calculated */
  calcParams();

  /* Allocate short data buffer, allocate buffers and fill tables */
  shBuf = (short *) calloc((long)F_LEN, sizeof(short));
  initTables();

  /* allocation of coefficient space and initialization of pointers into */
  /* this space */
  initState();
  
  /* fill input buffer, excluding last frame (this doesn't get high-passed) */
  if (!decode_only)
  {
    fread(shBuf, sizeof(short), INBUFSIZ - F_LEN, fpin);
    tmpPtr = inBuf;
    shPtr = shBuf - 1;
#if INTEL_FORMAT
    swap_byte(shPtr, (long)(INBUFSIZ - F_LEN));
#endif
    for (endPtr = tmpPtr + INBUFSIZ - F_LEN; tmpPtr < endPtr; tmpPtr++)
      *tmpPtr = *++shPtr * inScale;
  }
  FILT4(inBuf, INBUFSIZ - F_LEN);	/* HPF 1st INBUFSIZ-F_LEN points */

  /*-------------------------------------------------------------------------*/
  /* main loop */
  while (1)
  {
    if (!quiet)
      fprintf(stderr, "%c\r", funny[frCnt % 8]);

    if (decode_only)
      goto receiveLabel;

    if (feof(fpin))
      break;

    /* read one frame into short buffer, scale, and transfer into floating- */
    /* point input buffer */
    if ((numRead = fread(shBuf, sizeof(short), F_LEN, fpin)) < F_LEN)
    {
        /* Zero-pad the input buffer when no.of samples less than F_LEN */
        short          *eshPtr = shBuf + F_LEN;
        shPtr = shBuf + numRead;
        for (; shPtr < eshPtr; shPtr++)
 	  *shPtr = 0;
    }
    tmpPtr = inBuf + INBUFSIZ - F_LEN;
    shPtr = shBuf - 1;

#if INTEL_FORMAT
    swap_byte(shBuf, F_LEN);
#endif
    for (endPtr = tmpPtr + F_LEN; tmpPtr < endPtr; tmpPtr++)
      *tmpPtr = *++shPtr * inScale;

    /* ENCODER SIDE */
    codes = codeBuf;		/* reset code pointer to beginning of buffer */

    /* hpf the last F_LEN of input */
    FILT4(inBuf + INBUFSIZ - F_LEN, F_LEN);

    FLATV();			/* get reflection coefs and rq0. */

    /* get direct-form coef's from rc's, calculate bandwidth- */
    /* widened coefs */
    RCTOA(T_NEW.k, T_NEW.a);
    widen(W_ALPHA, 't');

    /* This for loop does coefficient interpolation (stored in I_CBUFF) */
    /* and calculates the residual energy estimate, RS (stored in RS_BUFF), */
    /* for all subframes.  */
    for (i = 0; i < N_SUB; i++)
    {
      if (i == N_SUB - 1)
      {
	/* Final subframe, no interpolation done. Move coefs and */
	/* calculate RS.  Store a duplicate of RS in RS_BUFF */
	RQ_TMP = T_NEW.rq0;
	I_MOV(T_NEW, 2, RQ_TMP);
	*(RS_BUFF + 2 * N_SUB - 1) = *(RS_BUFF + 2 * N_SUB - 2);	/* copy last rs. */
      }
      else if (i * 2 == N_SUB - 2)
      {
	/* Middle subframe.  Interpolate coefs.  If result is */
	/* unstable, use coefs from frame with larger energy. */
	/* Perform a geometric average on last and current Rq's, */
	/* use this value to calculate RS. */
	/* Calculate another RS based on last or current rc's */
	/* (if it has not been calculated already due to instability, */
	/* if that is so, just copy it). */
	RQ_TMP = sqrt(T_NEW.rq0 * T_OLD.rq0);

	if (T_NEW.rq0 > T_OLD.rq0)
	{
	  us = INTERPOLATE(T_NEW, 2, T_OLD, i, RQ_TMP);
	  if (us)
	    *(RS_BUFF + 2 * i + 1) = *(RS_BUFF + 2 * i);
	  else
	    *(RS_BUFF + 2 * i + 1) = RES_ENG(RQ_TMP, T_NEW.k);
	}
	else
	{
	  us = INTERPOLATE(T_OLD, 2, T_NEW, i, RQ_TMP);
	  if (us)
	    *(RS_BUFF + 2 * i + 1) = *(RS_BUFF + 2 * i);
	  else
	    *(RS_BUFF + 2 * i + 1) = RES_ENG(RQ_TMP, T_OLD.k);
	}
      }
      else if (i * 2 < N_SUB - 2)
      {
	/* Subframe closer to last frame.  Interpolate coefs. */
	/* If result is unstable, use last frame's coefs. */
	/* Calculate RS based on last frame's Rq.  Calculate */
	/* another RS based on last rc's (if it has */
	/* not been calculated already due to instability, if that */
	/* is so, just copy it) */
	RQ_TMP = T_OLD.rq0;
	us = INTERPOLATE(T_OLD, 2, T_NEW, i, RQ_TMP);
	if (us)
	  *(RS_BUFF + 2 * i + 1) = *(RS_BUFF + 2 * i);
	else
	  *(RS_BUFF + 2 * i + 1) = RES_ENG(RQ_TMP, T_OLD.k);
      }
      else
      {
	/* Subframe closer to current frame.  Interpolate coefs. */
	/* If result is unstable, use current frame's coefs. */
	/* Calculate RS based on current frame's Rq.  Calculate */
	/* another RS based on current rc's (if it has */
	/* not been calculated already due to instability, if that */
	/* is so, just copy it) */
	RQ_TMP = T_NEW.rq0;
	us = INTERPOLATE(T_NEW, 2, T_OLD, i, RQ_TMP);
	if (us)
	  *(RS_BUFF + 2 * i + 1) = *(RS_BUFF + 2 * i);
	else
	  *(RS_BUFF + 2 * i + 1) = RES_ENG(RQ_TMP, T_NEW.k);
      }
    }

    for (i = 0; i < N_SUB; i++)
    {
      sfCnt = i + 1;
      /* Load/point-to the values that */
      /* are needed in the subframe processing. */
      RS = *(RS_BUFF + i * 2);
      COEF = I_CBUFF + i * 2 * NP;
      W_COEF = COEF + NP;

      T_SUB(i);			/* Do subframe processing. */
    }

    /* perform delay on input buffer. */
    tmpPtr2 = inBuf + F_LEN - 1;
    tmpPtr = inBuf;
    for (endPtr = tmpPtr + (INBUFSIZ - F_LEN); tmpPtr < endPtr; tmpPtr++)
      *tmpPtr = *++tmpPtr2;

    /* update xmt old values and pointers, */
    /* new pointers set to old space. */
    T_OLD.rq0 = T_NEW.rq0;
    tmpPtr = T_OLD.k;
    T_OLD.k = T_NEW.k;
    T_NEW.k = tmpPtr;
    tmpPtr = T_OLD.a;
    T_OLD.a = T_NEW.a;
    T_NEW.a = tmpPtr;
    tmpPtr = T_OLD.widen;
    T_OLD.widen = T_NEW.widen;
    T_NEW.widen = tmpPtr;

    /* output routine for packed ascii-hex output. */
    if (encode_only) 
      bs_saved += put_codes(fpstream, codeBuf);

receiveLabel:
    if (decode_only)
    {
      if ((i = get_codes(fpstream, codeBuf))==0)
      {
        /* Quit on end of file OR abort on error */
        if(feof(fpstream))
          break;
        else
          HARAKIRI("Error reading bitstream file\n", 3);
      }
      else
      bs_read += i;
    }

    /* output routine for diagnostic code output */
    if (makeLog)
      putCodesLog();


    /*------------------------------------------------------------------------*/
    /* --- DECODER SIDE --- */
    if (!encode_only)
    {
      /* get Rq0 and rc's from codes */
      codes = codeBuf;
      R_NEW.rq0 = lookup(0);
      tmpPtr = R_NEW.k;
      for (endPtr = tmpPtr + NP, i = 1; tmpPtr < endPtr; tmpPtr++, i++)
	*tmpPtr = lookup(i);

      /* get direct-form coef's from rc's, calculate bandwidth- */
      /* widened coefs (denominator of spectral post-filter) and spectrally- */
      /* smoothed numerator coefs. */
      RCTOA(R_NEW.k, R_NEW.a);
      widen(POST_W_D, 'r');
      A_SST(R_NEW.widen, R_NEW.sst);

      /* Calculate geometric average of Rq0's. */
      RQ_HOLD = sqrt(R_NEW.rq0 * R_OLD.rq0);

      /* Calculate interpolated coefs for all subframes */
      for (i = 0; i < N_SUB; i++)
      {
	if (i == N_SUB - 1)
	  I_MOV(R_NEW, 3, R_NEW.rq0);
	else if (i * 2 == N_SUB - 2)
	{
	  if (R_NEW.rq0 > R_OLD.rq0)
	    INTERPOLATE(R_NEW, 3, R_OLD, i, RQ_HOLD);
	  else
	    INTERPOLATE(R_OLD, 3, R_NEW, i, RQ_HOLD);
	}
	else if (i * 2 < N_SUB - 2)
	  INTERPOLATE(R_OLD, 3, R_NEW, i, R_OLD.rq0);
	else
	  INTERPOLATE(R_NEW, 3, R_OLD, i, R_NEW.rq0);
      }

      for (i = 0; i < N_SUB; i++)
      {
	sfCnt = i + 1;
	/* Load/point-to values needed for */
	/* subframe processing. */
	RS = *(RS_BUFF + 2 * i);
	COEF = I_CBUFF + i * 3 * NP;
	W_COEF = COEF + NP;
	N_COEF = W_COEF + NP;

	R_SUB();		/* Do subframe processing. */

	/* Scale and transfer synthesized speech to short buffer.  Write to */
	/* output file. */
	tmpPtr = outBuf;
	shPtr = shBuf - 1;
	for (endPtr = tmpPtr + S_LEN; tmpPtr < endPtr; tmpPtr++)
	{
	  if (*tmpPtr >= 0.0)
	  {
	    f1 = *tmpPtr * 32768.0 + 0.5;
	    *++shPtr = (f1 >= 32768.0) ? 32767 : (short) f1;
	  }
	  else
	  {
	    f1 = *tmpPtr * 32768.0 - 0.5;
	    *++shPtr = (f1 <= -32769.0) ? -32768 : (short) f1;
	  }
	}
	fwrite(shBuf, sizeof(short), S_LEN, fpout);
      }
    }

    /* update rcv pointers */
    R_OLD.rq0 = R_NEW.rq0;
    tmpPtr = R_OLD.k;
    R_OLD.k = R_NEW.k;
    R_NEW.k = tmpPtr;
    tmpPtr = R_OLD.a;
    R_OLD.a = R_NEW.a;
    R_NEW.a = tmpPtr;
    tmpPtr = R_OLD.widen;
    R_OLD.widen = R_NEW.widen;
    R_NEW.widen = tmpPtr;
    tmpPtr = R_OLD.sst;
    R_OLD.sst = R_NEW.sst;
    R_NEW.sst = tmpPtr;

    if (frCnt == finalCnt)
    {
      break;
    }
    frCnt++;
  }				/* main loop end */
  
  if (performMeas && !decode_only)
    printSnr(fplog);
  
  frCnt--;
  
  if (encode_only)
  {
    fprintf(stderr, "VSELP: %ld input samples encoded as %ld IS54 codes\n",
                    (long)frCnt*(long)F_LEN, bs_saved);
  }
  else if (decode_only)
  {
    fprintf(stderr, "VSELP: %ld IS54 codes generated %ld output samples\n",
                    bs_read, (long)frCnt*(long)F_LEN);
  }
  else
  {
    fprintf(stderr, "VSELP: %ld samples processed\n",
                    (long)frCnt*(long)F_LEN);
  }

  /* Free memory */
  free(shBuf);
  free(T_NEW.k);
  freeSpace();

#ifdef VMS
    /*	  
    **  Fill the rest of the file with zeros
    */	  

    if ((((long)frCnt*(long)F_LEN) % 256) != 0)
    {
    	zero_values = 256 - (((long)frCnt*(long)F_LEN) % 256);
	zero_vector = (short *) calloc(zero_values,sizeof(short));
	lwrite = fwrite(zero_vector,sizeof (short),zero_values,fpout);
        if (lwrite != zero_values)
	{
	    printf (" Error writing zero vector \n");
	    exit (1);
	}
    }
#endif

  /* Close files, if open */
  if (fpin) fclose(fpin);
  if (fpout) fclose(fpout);
  if (fpcode) fclose(fpcode);
  if (fplog) fclose(fplog);
  if (fpstream) fclose(fpstream);

  /* Return OK status to the OS */
  return 0;
}
/* ............................. End of main() ........................... */
/*  DEC/CMS REPLACEMENT HISTORY, Element VSELP.C */
/*  *4    28-AUG-1995 15:56:00 SCHROEDER "Change the date due th some changes in h-files" */
/*  *3    24-AUG-1995 11:52:08 STEGMANN "2nd update considering VMS 512-Byte record structure for output speech file" */
/*  *2    23-AUG-1995 14:15:37 KIRCHHERR "Update to run under OpenVMS/AXP" */
/*  *1    23-AUG-1995 11:02:30 KIRCHHERR "Original by UGST" */
/*  DEC/CMS REPLACEMENT HISTORY, Element VSELP.C */
