
.SUFFIXES:
.SUFFIXES: .o .c .s


ENC_PROG = enc
DEC_PROG = dec
A2B_PROG = pcma2b
B2A_PROG = pcmb2a
A2P_PROG = atopcm
CMP_PROG = pcmcmp
G711_PROG = tst_g711
STRIP_PROG = stripInit
GPCM_PROG = genpcm
CVEC_PROG = check_vec
VECS_PROG = vec_strip

ALL_PROGS = $(DEC_PROG) $(ENC_PROG) $(A2B_PROG) $(B2A_PROG) $(STRIP_PROG) $(GPCM_PROG) $(A2P_PROG) $(CMP_PROG) $(G711_PROG) $(CVEC_PROG) $(VECS_PROG)

MAKEFILE = Makefile

CC = gcc
LD = gcc
LINT = lint
LPR = lpr

DEBUG = -g
CCFLAGS = -c ${DEBUG}
LDFLAGS = 
LIBFLAGS = -lm
LINTFLAGS = -abhnx
DEF =

#
#  Include Files
#

INCLUDES = encode.h audio.h

#
#  Source files
#
ENC_FILES = tbl.c enc.c cmn.c
DEC_FILES = tbl.c dec.c cmn.c
A2B_FILES = pcma2b.c
B2A_FILES = pcmb2a.c
A2P_FILES = atopcm.c
CMP_FILES = pcmcmp.c
G711_FILES = tbl.c tst_g711.c cmn.c
STRIP_FILES = stripInit.c
GPCM_FILES = genpcm.c
CVEC_FILES = check_vec.c
VECS_FILES = vec_strip.c

FILES = $(ENC_FILES) $(DEC_FILES) $(A2B_FILES) $(B2A_FILES) $(STRIP_FILES) $(GPCM_FILES) $(A2P_FILES) $(CMP_FILES) $(G711_FILES) $(CVEC_FILES) $(VECS_FILES)

#
#  objects files
#
ENC_OBJ = tbl.o enc.o cmn.o
DEC_OBJ = tbl.o dec.o cmn.o
A2B_OBJ = pcma2b.o
B2A_OBJ = pcmb2a.o
A2P_OBJ = atopcm.o
CMP_OBJ = pcmcmp.o
G711_OBJ = tbl.o tst_g711.o cmn.o
STRIP_OBJ = stripInit.o
GPCM_OBJ = genpcm.o
CVEC_OBJ = check_vec.o
VECS_OBJ = vec_strip.0

OBJ = $(ENC_OBJ) $(DEC_OBJ) $(A2B_OBJ) $(B2A_OBJ) $(STRIP_OBJ) $(GPCM_OBJ) $(A2P_OBJ) $(CMP_OBJ) $(G711_OBJ) $(CVEC_OBJ) $(VECS_OBJ)

#
#  listing files
#
ENC_LIST = tbl.l enc.l cmn.l
DEC_LIST = tbl.l dec.l cmn.l

default all: $(ALL_PROGS)

help:
	@echo "options: all, help, test, debug, $(ENC_PROG), $(DEC_PROG), $(STRIP_PROG), $(GPCM_PROG), $(A2P_PROG), $(CMP_PROG), $(G711_OBJ)"

$(DEC_PROG): $(DEC_OBJ)
	-$(LD)  $(LDFLAGS) $(DEC_OBJ) $(LIBFLAGS) -o $(DEC_PROG)
	-date

$(ENC_PROG): $(ENC_OBJ)
	-$(LD)  $(LDFLAGS) $(ENC_OBJ) $(LIBFLAGS) -o $(ENC_PROG)
	-date

$(A2B_PROG):

$(B2A_PROG):

$(STRIP_PROG):

$(GPCM_PROG):

$(A2P_PROG):

$(CMP_PROG):

$(CVEC_PROG):

$(VECS_PROG):

$(G711_PROG): $(G711_OBJ)
	-$(LD)  $(LDFLAGS) $(G711_OBJ) $(LIBFLAGS) -o $(G711_PROG)

printsrc: $(FILES) $(INCLUDES)	#print source listings
	-$(LPR) $(LPRFLAGS) $(INCLUDES) $(FILES) | lpr

test:	$(DEC_PROG) $(ENC_PROG)	$(STRIP_PROG)
	-./chk

debug:	$(DEC_PROG) $(ENC_PROG)
	-./chk -debug > world.log

lint:   $(FILES) $(INCLUDES)
	-$(LINT) $(LINTFLAGS) $(DEC_FILES)
	-$(LINT) $(LINTFLAGS) $(ENC_FILES)

$(OBJ): $(INCLUDES) $(MAKEFILE)

.c.o:
	-$(CC)  $(CCFLAGS) $<

.o :
	-$(LD) $(LDFLAGS) $< $(LIBFLAGS) -o $*

clean:
	/bin/rm -f $(ALL_PROGS)
	/bin/rm -f *.o
