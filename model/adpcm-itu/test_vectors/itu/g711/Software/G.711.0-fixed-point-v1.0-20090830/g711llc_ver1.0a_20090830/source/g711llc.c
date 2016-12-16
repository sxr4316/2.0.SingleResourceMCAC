/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc.c
 Contents: Command line interface to the encoder/decoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utility/wmops_timer.h"
#include "utility/stack_profile.h"
#include "g711llc/g711llc_defines.h"
#include "utility/g711llc_encode_file.h"
#include "utility/g711llc_decode_file.h"

#define MODULE_NAME             "g711llc"
#define MODULE_LONG_NAME        "ITU-T G.711.0 ANSI-C Source Code"
#define MODULE_VERSION          "Release 1.00 [July 24, 2009]"

#define MAX_FILENAME_LENGTH 1024

/* Environment variable names */
#define ENV_SPR_ENC_NAME        "G711LLC_SPR_PREFIX_ENC"
#define ENV_SPR_DEC_NAME        "G711LLC_SPR_PREFIX_DEC"

enum operation_type { operation_unknown, operation_encode, operation_decode, operation_help };
enum law_type { undefined_law, mu_law, a_law };

/* processing options */
static enum operation_type  operation = operation_unknown;
static BOOL                 verbose_mode = FALSE;
static int                  frame_length = g711llc_default_frame_length;
static enum law_type        law = undefined_law;
static char                 input_file_name[MAX_FILENAME_LENGTH] = { 0 },
                            output_file_name[MAX_FILENAME_LENGTH] = { 0 };

static int parse_command_line(int argc, char *argv[]);
static int encode(void);
static int decode(void);
static void display_help(void);

#define countof(x) (sizeof(x)/sizeof(x[0]))

int main(int argc, char *argv[])
{
	int error = parse_command_line(argc, argv);
	if (error != 0)
		return EXIT_FAILURE;

	switch (operation) {
		case operation_encode:
			error = encode();
			break;
		case operation_decode:
			error = decode();
			break;
		default:
			display_help();
			break;
	}

	return (error == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void display_help(void)
{
	printf("<<<<< " MODULE_LONG_NAME " " MODULE_VERSION " >>>>>\n"
	       "Usage:\n"
	       "  Encoding: " MODULE_NAME " [-v] [-e|-enc] [-u|-a] [-n#] infile.[ul8|mu8|mu|al8|al] [outfile.[lcm|lca]]\n"
	       "  Decoding: " MODULE_NAME " [-v] [-u|-a] infile.[lcm|lca] [outfile.[muo|alo]]\n"
	       "General options:\n"
	       "  -h         : Help (this message)\n"
	       "  -v         : Verbose mode\n"
	       "  -e or -enc : force encode mode\n"
	       "  -u         : force mu-law mode\n"
	       "  -a         : force a-law mode\n"
	       "Encoding options:\n"
	       "  -n#        : Frame length (default=%d)\n"
		   "\n\n", (int)g711llc_default_frame_length);
}

static const char *get_extension(const char *filename) {
	const char *last_dot = strrchr(filename, '.');
	if (last_dot == NULL)
		return "";

	return last_dot+1;
}

int parse_command_line(int argc, char *argv[])
{
	int arg;
	if (argc <= 1 || argv == 0) {
		fputs("ERROR: Invalid number of arguments\n", stderr);
		display_help();
		return -1;
	}

	operation = operation_unknown;

	/* Parse command line arguments */
	for (arg=1; arg<argc; ++arg) {
		const char *a = argv[arg];
		if (a == NULL)
			continue;

		if (strcmp(a, "-h") == 0) {
			operation = operation_help;
			break;
		} else if (strcmp(a, "-v") == 0) {
			verbose_mode = TRUE;
		} else if ((strcmp(a, "-e") == 0) || (strcmp(a, "-enc") == 0)) {
			if (operation != operation_help)
				operation = operation_encode;
		} else if (strcmp(a, "-u") == 0) {
			if (law == a_law) {
				fputs("ERROR: Conflicting -a and -u options\n", stderr);
				return -2;
			}
			law = mu_law;
		} else if (strcmp(a, "-a") == 0) {
			if (law == mu_law) {
				fputs("ERROR: Conflicting -a and -u options\n", stderr);
				return -2;
			}
			law = a_law;
		} else if (strncmp(a, "-n", 2) == 0) {
			frame_length = atoi(a+2);
		} else {
			/* this must be infile or outfile */
			if ((input_file_name[0] != '\0') && (output_file_name[0] != '\0')) {
				fputs("ERROR: Unexpected argument specified\n", stderr);
				return -3;
			}

			if (input_file_name[0] == '\0') {
				strncpy(input_file_name, a, countof(input_file_name)-1);
				input_file_name[countof(input_file_name)-1] = '\0';
			} else {
				strncpy(output_file_name, a, countof(output_file_name)-1);
				output_file_name[countof(output_file_name)-1] = '\0';
			}
		}
	}

	if (operation == operation_help)
		return 0;

	/* Check arguments, set defaults */
	if (input_file_name[0] == '\0') {
		fputs("ERROR: Input file name was not specified\n", stderr);
		return -4;
	}

	if (operation == operation_unknown) {
		const char *extension = get_extension(input_file_name);
			
		if ((strcmp(extension, "ul8") == 0) ||
		    (strcmp(extension, "mu8") == 0) ||
		    (strcmp(extension, "al8") == 0) ||
		    (strcmp(extension, "mu") == 0) ||
		    (strcmp(extension, "al") == 0))
			operation = operation_encode;
		else
			operation = operation_decode;
	}

	if ((frame_length != 40) && (frame_length != 80) && (frame_length != 160) && (frame_length != 240) && (frame_length != 320)) {
		fputs("ERROR: invalid frame length (-n#). Valid frame lengths are: 40, 80, 160, 240, 320.\n", stderr);
		return -9;
	}

	/* Guess the law mode from the file names (decoding) */
	if ((operation == operation_decode) && (law == undefined_law)) {
		const char *extension = get_extension(input_file_name);
			
		if (strcmp(extension, "lcm") == 0)
			law = mu_law;
		else if (strcmp(extension, "lca") == 0)
			law = a_law;

		if (law == undefined_law) {
			/* Guessing from the input file name failed. Try from the output file name. */
			extension = get_extension(output_file_name);

			if (strcmp(extension, "muo") == 0)
				law = mu_law;
			else if (strcmp(extension, "alo") == 0)
				law = a_law;
		}

		if (law == undefined_law) {
			fputs("ERROR: Can't guess the law mode from the input/output file names! Please specify either the -u or the -a option.\n", stderr);
			return -14;
		}
	}

	/* Guess the law mode from the file names (encoding) */
	if ((operation == operation_encode) && (law == undefined_law)) {
		const char *extension;

		/* Try to guess the law mode from the input file name. */
		extension = get_extension(input_file_name);

		if ((strcmp(extension, "mu") == 0) || (strcmp(extension, "ul8") == 0) || (strcmp(extension, "mu8") == 0))
			law = mu_law;
		else if ((strcmp(extension, "al") == 0) || (strcmp(extension, "al8") == 0))
			law = a_law;

		if (law == undefined_law) {
			/* Guessing from the input file name failed. Try from the output file name. */
			extension = get_extension(output_file_name);

			if (strcmp(extension, "lcm") == 0)
				law = mu_law;
			else if (strcmp(extension, "lca") == 0)
				law = a_law;
		}

		if (law == undefined_law) {
			fputs("ERROR: Can't guess the law mode from the input/output file names! Please specify either the -u or the -a option.\n", stderr);
			return -15;
		}
	}

	/* generate the output file name if it was not specified */
	if (output_file_name[0] == '\0') {
		const char *extension = get_extension(input_file_name);
		const char *new_extension;
		const size_t filename_and_dot_length = strlen(input_file_name) - strlen(extension);
		int remaining_size = (int)(countof(output_file_name)-1-filename_and_dot_length);
		if (remaining_size < 0)
			remaining_size = 0;

		switch (operation) {
			case operation_encode:
				if ((strcmp(extension, "mu") == 0) || (strcmp(extension, "ul8") == 0) || (strcmp(extension, "mu8") == 0))
					new_extension = "lcm";
				else if ((strcmp(extension, "al") == 0) || (strcmp(extension, "al8") == 0))
					new_extension = "lca";
				else
					new_extension = ((law == mu_law) ? ".lcm" : ".lca");
				break;
			case operation_decode:
				if (strcmp(extension, "lcm") == 0)
					new_extension = "muo";
				else if (strcmp(extension, "lca") == 0)
					new_extension = "alo";
				else
					new_extension = (law == mu_law ? "muo" : "alo");
				break;
			default:
				return -1; /* Should not happen */
		}

		strncpy(output_file_name, input_file_name, filename_and_dot_length);
		output_file_name[filename_and_dot_length] = '\0';
		strncat(output_file_name, new_extension, remaining_size);
		output_file_name[countof(output_file_name)-1] = '\0';
	}

	return 0;
}

#ifdef ENABLE_STACK_PROFILING
static void update_spr_files(const char *prefix, int frame_length)
{
	char *filename;
	size_t prefix_len;
	if (prefix == NULL || frame_length < 0 || frame_length > 999)
		return;

	prefix_len = strlen(prefix);
	if ((filename = (char*)malloc(strlen(prefix) + 5)) != NULL) {
		sprintf(filename, "%s.all", prefix);
		SPR_UPDATE_WORST(filename, frame_length);
		sprintf(filename, "%s.%d", prefix, frame_length);
		SPR_UPDATE_WORST(filename, frame_length);
		free(filename);
	} else {
		fputs("Out of memory!\n", stderr);
		exit(EXIT_FAILURE);
	}
}
#endif /* ENABLE_STACK_PROFILING */

int encode(void)
{
	int error = 0, encoder_error;
	FILE *input_file = NULL, *output_file = NULL;
	long input_file_size, output_file_size, encoded_size;

	SPR_RESET(input_file_name);

	/* Open input file */
	if ((input_file = fopen(input_file_name, "rb")) == NULL) {
		fputs("input file was not found\n", stderr);
		error = -1;
		goto cleanup;
	}

	/* Open output file */
	if ((output_file = fopen(output_file_name, "wb")) == NULL) {
		fputs("output file was not found\n", stderr);
		error = -2;
		goto cleanup;
	}
	
	wmops_timer_init("encoder");

	/* Encode! */
	if ((encoder_error = g711llc_encode_file(input_file, output_file, law == mu_law, frame_length, verbose_mode)) < 0) {
		fprintf(stderr, "ERROR: The encoder returned error code: %d\n", encoder_error);
		error = -6;
		goto cleanup;
	}

	input_file_size = ftell(input_file);
	output_file_size = ftell(output_file);
	encoded_size = output_file_size;

	/* Display encoding info */
	puts("<<<< " MODULE_LONG_NAME " " MODULE_VERSION " >>>>\n");
	printf("  Law Type     : %s\n", ((law == mu_law) ? "mu-law" : "a-Law"));
	printf("  InFile       : %s\n", input_file_name);
	printf("  OutFile      : %s\n", output_file_name);
	printf("  Frame Size   : %d\n", frame_length);
	printf("  Infile Size  : %ld bytes\n", input_file_size);
	printf("  Outfile Size : %ld bytes\n", output_file_size);
	printf("  Encoded Size : %ld bytes\n", encoded_size);
	printf("  Comp ratio   : %f %%\n", (input_file_size > 0 ? (100.0-(100.0*encoded_size) / input_file_size) : 0.0));

	/* Display timing info */
	wmops_timer_show_default(input_file_size / (float)g711llc_default_hz);

#ifdef ENABLE_STACK_PROFILING
	update_spr_files(getenv(ENV_SPR_ENC_NAME), frame_length);
#endif /* ENABLE_STACK_PROFILING */

cleanup:
	if (input_file != NULL)
		fclose(input_file);
	if (output_file != NULL)
		fclose(output_file);

	return error;
}

int decode(void)
{
	int error = 0, decoder_error;
	FILE *input_file = NULL, *output_file = NULL;
	long input_file_size, output_file_size, encoded_size;

	SPR_RESET(input_file_name);

	/* Open the input file */
	if ((input_file = fopen(input_file_name, "rb")) == NULL) {
		fputs("input file was not found\n", stderr);
		error = -1;
		goto cleanup;
	}

	/* Open the output file */
	if ((output_file = fopen(output_file_name, "wb")) == NULL) {
		fputs("output file was not found\n", stderr);
		error = -2;
		goto cleanup;
	}

	/* Set up the decoder */
	wmops_timer_init("decoder");

	/* Decode! */
	if ((decoder_error = g711llc_decode_file(input_file, output_file, law == mu_law, verbose_mode, &frame_length)) < 0) {
		fprintf(stderr, "ERROR: The decoder returned error code: %d\n", decoder_error);
		error = -6;
		goto cleanup;
	}

	input_file_size = ftell(input_file);
	output_file_size = ftell(output_file);
	encoded_size = input_file_size;

	/* Display decoding info */
	puts("<<<< " MODULE_LONG_NAME " " MODULE_VERSION " >>>>\n");
	printf("  Law Type     : %s\n", ((law == mu_law) ? "mu-law" : "a-law"));
	printf("  InFile       : %s\n", input_file_name);
	printf("  OutFile      : %s\n", output_file_name);
	printf("  Frame Size   : %d (informative)\n", frame_length);
	printf("  Infile Size  : %ld bytes\n", input_file_size);
	printf("  Outfile Size : %ld bytes\n", output_file_size);
	printf("  Encoded Size : %ld bytes\n", encoded_size);
	printf("  Comp ratio   : %f %%\n", (output_file_size > 0 ? ((100.0*encoded_size) / output_file_size) : 0.0));

	/* Display timing info */
	wmops_timer_show_default(output_file_size / (float)g711llc_default_hz);

#ifdef ENABLE_STACK_PROFILING
	update_spr_files(getenv(ENV_SPR_DEC_NAME), frame_length);
#endif /* ENABLE_STACK_PROFILING */

cleanup:
	if (input_file != NULL)
		fclose(input_file);
	if (output_file != NULL)
		fclose(output_file);

	return error;
}
