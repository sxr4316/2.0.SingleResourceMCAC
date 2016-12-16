
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

ALL_PROGS = $(DEC_PROG) $(ENC_PROG) $(A2B_PROG) $(B2A_PROG) $(STRIP_PROG) $(GPCM_PROG) $(A2P_PROG) $(CMP_PROG) $(G711_PROG)

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
ENC_FILES = create_vecs_enc.c tbl.c enc.c cmn.c accum.c adda.c addb.c addc.c antilog.c compress.c expand.c filta.c filtb.c filtc.c filtd.c filte.c floata.c floatb.c functf.c fmult.c functw.c g711.c inv_g711.c lima.c limb.c limc.c limd.c llog.c mix.c quan.c reconst.c subta.c subtb.c subtc.c synch.c tone.c trans.c triga.c trigb.c upa1.c upa2.c upb.c xor.c
DEC_FILES = create_vecs_dec.c tbl.c dec.c cmn.c accum.c adda.c addb.c addc.c antilog.c compress.c expand.c filta.c filtb.c filtc.c filtd.c filte.c floata.c floatb.c functf.c fmult.c functw.c g711.c inv_g711.c lima.c limb.c limc.c limd.c llog.c mix.c quan.c reconst.c subta.c subtb.c subtc.c synch.c tone.c trans.c triga.c trigb.c upa1.c upa2.c upb.c xor.c
A2B_FILES = pcma2b.c
B2A_FILES = pcmb2a.c
A2P_FILES = atopcm.c
CMP_FILES = pcmcmp.c
G711_FILES = tbl.c tst_g711.c cmn.c g711.c inv_g711.c
STRIP_FILES = stripInit.c
GPCM_FILES = genpcm.c

FILES = $(ENC_FILES) $(DEC_FILES) $(A2B_FILES) $(B2A_FILES) $(STRIP_FILES) $(GPCM_FILES) $(A2P_FILES) $(CMP_FILES) $(G711_FILES)

#
#  objects files
#
ENC_OBJ = create_vecs_enc.o tbl.o enc.o cmn.o accum.o adda.o addb.o addc.o antilog.o compress.o expand.o filta.o filtb.o filtc.o filtd.o filte.o floata.o floatb.o fmult.o functf.o functw.o g711.o inv_g711.o lima.o limb.o limc.o limd.o llog.o mix.o quan.o reconst.o subta.o subtb.o subtc.o synch.o tone.o trans.o triga.o trigb.o upa1.o upa2.o upb.o xor.o
DEC_OBJ = create_vecs_dec.o tbl.o dec.o cmn.o accum.o adda.o addb.o addc.o antilog.o compress.o expand.o filta.o filtb.o filtc.o filtd.o filte.o floata.o floatb.o fmult.o functf.o functw.o g711.o inv_g711.o lima.o limb.o limc.o limd.o llog.o mix.o quan.o reconst.o subta.o subtb.o subtc.o synch.o tone.o trans.o triga.o trigb.o upa1.o upa2.o upb.o xor.o
A2B_OBJ = pcma2b.o
B2A_OBJ = pcmb2a.o
A2P_OBJ = atopcm.o
CMP_OBJ = pcmcmp.o
G711_OBJ = tbl.o tst_g711.o cmn.o g711.o inv_g711.o
STRIP_OBJ = stripInit.o
GPCM_OBJ = genpcm.o

OBJ = $(ENC_OBJ) $(DEC_OBJ) $(A2B_OBJ) $(B2A_OBJ) $(STRIP_OBJ) $(GPCM_OBJ) $(A2P_OBJ) $(CMP_OBJ) $(G711_OBJ)

#
#  listing files
#
ENC_LIST = tbl.l enc.l cmn.l
DEC_LIST = tbl.l dec.l cmn.l

default all: $(DEC_PROG) $(ENC_PROG) $(A2B_PROG) $(B2A_PROG) $(STRIP_PROG) $(GPCM_PROG) $(A2P_PROG) $(CMP_PROG) $(G711_PROG)

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
