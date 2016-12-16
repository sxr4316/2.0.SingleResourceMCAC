/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: stack_profile.c
 Contents: Stack use profiler for checking the worst-case RAM use.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack_profile.h"

#ifdef ENABLE_STACK_PROFILING

#define MAX_STACK_DEPTH     512
#define MAX_FILENAME_LENGTH 512

struct stack_entry_t {
	const char   *file;
	unsigned int line;
	const char   *function_name;
	unsigned int size;
};

struct stack_t {
	scopeid_t scope;
	struct stack_entry_t stack[MAX_STACK_DEPTH];
	unsigned int size;
	unsigned int frame;
	char         input_file_name[MAX_FILENAME_LENGTH];
};

static struct stack_t current = { 0 }, worst = { 0 };

scopeid_t __spr_enter_scope(unsigned int size, const char *file, unsigned int line, const char *function_name)
{
	current.stack[current.scope].file = file;
	current.stack[current.scope].line = line;
	current.stack[current.scope].function_name = function_name;
	current.stack[current.scope].size = size;

	current.size += size;

	++current.scope;

	if (current.size > worst.size) {
		memcpy(&worst, &current, sizeof(struct stack_t));
	}
	if (current.scope >= MAX_STACK_DEPTH) {
		fputs("****************************************************\n", stderr);
		fprintf(stderr, "STACK PROFILING: Too deep stack in function %s (line %u in %s)\n", function_name, line, file);
		fputs("****************************************************\n", stderr);
		exit(1);
	}
	return current.scope;
}

void __spr_leave_scope(scopeid_t scopeid, const char *file, unsigned int line, const char *function_name)
{
	if (current.scope == 0 || scopeid == 0) {
		fputs("****************************************************\n", stderr);
		fprintf(stderr, "STACK PROFILING: Invalid scope leave in function %s (line %u in %s)\n", function_name, line, file);
		fprintf(stderr, "current scope = %u, scope provided = %u\n", current.scope, scopeid);
		fputs("****************************************************\n", stderr);
		exit(1);
	}
	if (scopeid != current.scope) {
		fputs("****************************************************\n", stderr);
		fprintf(stderr, "STACK PROFILING: Invalid scope leave in function %s (line %u in %s)\n", function_name, line, file);
		fprintf(stderr, "Unclosed scope began in function %s (line %u in %s)\n", current.stack[current.scope-1].function_name, current.stack[current.scope-1].line, current.stack[current.scope-1].file);
		fputs("****************************************************\n", stderr);
		exit(1);
	}
	--current.scope;
	current.size -= current.stack[current.scope].size;
	memset(&current.stack[current.scope], 0, sizeof(struct stack_entry_t));
}

void __spr_reset_stack_counters(const char *input_file_name)
{
	memset(&current, 0, sizeof(struct stack_t));
	strncpy(current.input_file_name, input_file_name, sizeof(current.input_file_name)-1);
	current.input_file_name[sizeof(current.input_file_name)-1] = '\0';
	memset(&worst, 0, sizeof(struct stack_t));
}

void __spr_next_frame(void)
{
	++current.frame;
}

static void print_stack(const struct stack_t *s, FILE *f)
{
	unsigned int i;
	for (i=s->scope; i>0; --i)
		fprintf(f, "[%u] %6u octets | %s (line %u in %s)\n", i, s->stack[i-1].size, s->stack[i-1].function_name, s->stack[i-1].line, s->stack[i-1].file);
}

void __spr_print_stack_profile(FILE *file, int frame_length)
{
	fprintf(file, "WORST STACK SIZE (octets):\n%u\n\n", worst.size);
	fprintf(file, "Reached for file: %s\n", worst.input_file_name);
	fprintf(file, "    frame length: %d\n", frame_length);
	fprintf(file, "        frame no: %u\n\n", worst.frame);
	fputs("*** Stack dump:\n", file);
	print_stack(&worst, file);

	if (current.scope != 0) {
		fputs("\n\n********** WARNING: CURRENT STACK NON-EMPTY:\n", file);
		print_stack(&current, file);
	}
}

void __spr_update_worst_stack_profile(const char *filename, int frame_length)
{
	unsigned int max;
	FILE *f;

	if ((f = fopen(filename, "rt")) != NULL) {
		int c;
		/* Skip the first line */
		while (((c = fgetc(f)) != '\n') && (c != EOF));
		if (fscanf(f, "%u", &max) != 1) {
			fclose(f);
			fprintf(stderr, "**** STACK PROFILING: file %s is in wrong format\n", filename);
			exit(1);
		}
		fclose(f);
	} else
		max = 0;

	if ((worst.size > max) || (max == 0)) {
		if ((f = fopen(filename, "wt")) != NULL) {
			__spr_print_stack_profile(f, frame_length);
			if (ferror(f)) {
				fclose(f);
				fprintf(stderr, "**** STACK PROFILING: could not write to file %s\n", filename);
				exit(1);
			}
			fclose(f);
		} else {
			fprintf(stderr, "**** STACK PROFILING: could not open file %s for writing\n", filename);
			exit(1);
		}
	}
}

#endif /* ENABLE_STACK_PROFILING */

