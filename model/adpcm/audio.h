
/*
 *
 *	@(#) audio.h 1.1@(#)
 *	1/13/95  15:55:48
 *
 */

#ifndef u_32
typedef unsigned        u_32;
#endif

/* Define the audio magic number */
#define	AUDIO_FILE_MAGIC		((u_32)0x2e736e64)

/* Define the encoding fields */
#define	AUDIO_FILE_ENCODING_MULAW_8	(1)	/* 8-bit u-law pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_8	(2)	/* 8-bit linear pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_16	(3)	/* 16-bit linear pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_24	(4)	/* 24-bit linear pcm */
#define	AUDIO_FILE_ENCODING_LINEAR_32	(5)	/* 32-bit linear pcm */
#define	AUDIO_FILE_ENCODING_FLOAT	(6)	/* 32-bit IEEE floating point */
#define	AUDIO_FILE_ENCODING_DOUBLE	(7)	/* 64-bit IEEE floating point */

#ifndef SAMPLE
#define SAMPLE	8000		/* sample rate, Hz */
#endif
#ifndef AUDIO
#define AUDIO	".au"		/* audio file extension */
#endif

/* audio file header */
typedef struct {
	u_32		magic;		/* magic number */
	u_32		hdr_size;	/* size of this header */
	u_32		data_size;	/* length of data (optional) */
	u_32		encoding;	/* data encoding format */
	u_32		sample_rate;	/* samples per second */
	u_32		channels;	/* number of interleaved channels */
} Audio_filehdr;
