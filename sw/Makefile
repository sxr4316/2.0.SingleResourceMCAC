
.SUFFIXES:
.SUFFIXES: .o .c .s


SC_DIR = ./sc_model
MC_DIR = ./mc_model

SC_ENC_PROG = sc_enc
SC_DEC_PROG = sc_dec
SC_ENC_FLAT_PROG = sc_enc_flat
SC_DEC_FLAT_PROG = sc_dec_flat
MC_ENC_PROG = mc_enc
MC_DEC_PROG = mc_dec

ALL_PROGS = $(SC_ENC_PROG) $(MC_ENC_PROG) $(SC_DEC_PROG) $(MC_DEC_PROG)  $(SC_ENC_FLAT_PROG) $(SC_DEC_FLAT_PROG)

MAKEFILE = Makefile

CC = gcc
LD = gcc
LINT = lint
LPR = lpr

DEBUG = -g
CCFLAGS = -c ${DEBUG} -D MODEL
LDFLAGS = 
LIBFLAGS = -lm
LINTFLAGS = -abhnx
DEF =

#
#  Include Files
#

INCLUDE_DIR = ./include
INCLUDES = $(LIB_DIR)/encode.h

#
#  Source files
#
COMMON_DIR = ./lib
COMMON_FILES = $(COMMON_DIR)/enc.c $(COMMON_DIR)/dec.c $(COMMON_DIR)/tbl.c $(COMMON_DIR)/accum.c $(COMMON_DIR)/adda.c $(COMMON_DIR)/addb.c $(COMMON_DIR)/addc.c $(COMMON_DIR)/antilog.c $(COMMON_DIR)/compress.c $(COMMON_DIR)/expand.c $(COMMON_DIR)/filta.c $(COMMON_DIR)/filtb.c $(COMMON_DIR)/filtc.c $(COMMON_DIR)/filtd.c $(COMMON_DIR)/filte.c $(COMMON_DIR)/floata.c $(COMMON_DIR)/floatb.c $(COMMON_DIR)/functf.c $(COMMON_DIR)/fmult.c $(COMMON_DIR)/functw.c $(COMMON_DIR)/g711.c $(COMMON_DIR)/inv_g711.c $(COMMON_DIR)/lima.c $(COMMON_DIR)/limb.c $(COMMON_DIR)/limc.c $(COMMON_DIR)/limd.c $(COMMON_DIR)/llog.c $(COMMON_DIR)/mix.c $(COMMON_DIR)/quan.c $(COMMON_DIR)/reconst.c $(COMMON_DIR)/subta.c $(COMMON_DIR)/subtb.c $(COMMON_DIR)/subtc.c $(COMMON_DIR)/synch.c $(COMMON_DIR)/tone.c $(COMMON_DIR)/trans.c $(COMMON_DIR)/triga.c $(COMMON_DIR)/trigb.c $(COMMON_DIR)/upa1.c $(COMMON_DIR)/upa2.c $(COMMON_DIR)/upb.c $(COMMON_DIR)/xor.c
COMMON_FLAT_FILES = $(COMMON_DIR)/fmult.c $(COMMON_DIR)/accum.c $(COMMON_DIR)/tbl.c $(COMMON_DIR)/limd.c $(COMMON_DIR)/tone.c $(COMMON_DIR)/addc.c $(COMMON_DIR)/upa2.c $(COMMON_DIR)/limc.c $(COMMON_DIR)/addb.c $(COMMON_DIR)/floatb.c $(COMMON_DIR)/floata.c $(COMMON_DIR)/xor.c $(COMMON_DIR)/upb.c $(COMMON_DIR)/trigb.c $(COMMON_DIR)/upa1.c

SC_ENC_FILES = 		$(COMMON_DIR)/create_vecs_enc.c $(SC_DIR)/$(SC_ENC_PROG).c
SC_ENC_FLAT_FILES =  $(COMMON_DIR)/create_vecs_enc.c $(SC_DIR)/$(SC_ENC_FLAT_PROG).c
SC_DEC_FLAT_FILES =  $(COMMON_DIR)/create_vecs_dec.c $(SC_DIR)/$(SC_DEC_FLAT_PROG).c
SC_DEC_FILES = 		$(COMMON_DIR)/create_vecs_dec.c $(SC_DIR)/$(SC_DEC_PROG).c
MC_ENC_FILES = 		$(COMMON_DIR)/create_vecs_enc.c $(MC_DIR)/$(MC_ENC_PROG).c
MC_DEC_FILES = 		$(COMMON_DIR)/create_vecs_dec.c $(MC_DIR)/$(MC_DEC_PROG).c

FILES = $(SC_ENC_FILES) $(MC_ENC_FILES) $(SC_DEC_FILES) $(MC_DEC_FILES) $(SC_ENC_FLAT_FILES) $(SC_DEC_FLAT_FILES)

#
#  objects files
#

COMMON_OBJ = 		$(patsubst %.c,%.o,$(COMMON_FILES))
COMMON_FLAT_OBJ = 	$(patsubst %.c,%.o,$(COMMON_FLAT_FILES))
SC_ENC_OBJ = 		$(patsubst %.c,%.o,$(SC_ENC_FILES))
SC_ENC_FLAT_OBJ = 	$(patsubst %.c,%.o,$(SC_ENC_FLAT_FILES))
SC_DEC_FLAT_OBJ = 	$(patsubst %.c,%.o,$(SC_DEC_FLAT_FILES))
SC_DEC_OBJ = 		$(patsubst %.c,%.o,$(SC_DEC_FILES))
MC_ENC_OBJ = 		$(patsubst %.c,%.o,$(MC_ENC_FILES))
MC_DEC_OBJ = 		$(patsubst %.c,%.o,$(MC_DEC_FILES))
OBJ = $(COMMON_OBJ) $(SC_ENC_OBJ) $(SC_DEC_OBJ) $(MC_ENC_OBJ) $(MC_DEC_OBJ) $(SC_ENC_FLAT_OBJ) $(SC_DEC_FLAT_OBJ)

default all: $(SC_ENC_PROG) $(MC_ENC_PROG) $(SC_DEC_PROG) $(MC_DEC_PROG) $(SC_ENC_FLAT_PROG) $(SC_DEC_FLAT_PROG)

sc : $(SC_ENC_PROG) $(SC_DEC_PROG)

mc : $(MC_ENC_PROG) $(MC_DEC_PROG)

help:
	@echo "options: all, help, debug, sc, mc"

$(SC_ENC_PROG): $(COMMON_OBJ) $(SC_ENC_OBJ)
	-$(LD)  $(LDFLAGS) $(COMMON_OBJ) $(SC_ENC_OBJ) $(LIBFLAGS) -o $(SC_DIR)/$(SC_ENC_PROG)

$(SC_ENC_FLAT_PROG): $(COMMON_FLAT_OBJ) $(SC_ENC_FLAT_OBJ)
	-$(LD)  $(LDFLAGS) $(COMMON_FLAT_OBJ) $(SC_ENC_FLAT_OBJ) $(LIBFLAGS) -o $(SC_DIR)/$(SC_ENC_FLAT_PROG)

$(SC_DEC_FLAT_PROG): $(COMMON_FLAT_OBJ) $(SC_DEC_FLAT_OBJ)
	-$(LD)  $(LDFLAGS) $(COMMON_FLAT_OBJ) $(SC_DEC_FLAT_OBJ) $(LIBFLAGS) -o $(SC_DIR)/$(SC_DEC_FLAT_PROG)

$(SC_DEC_PROG): $(COMMON_OBJ) $(SC_DEC_OBJ)
	-$(LD)  $(LDFLAGS) $(COMMON_OBJ) $(SC_DEC_OBJ) $(LIBFLAGS) -o $(SC_DIR)/$(SC_DEC_PROG)

$(MC_ENC_PROG): $(COMMON_OBJ) $(MC_ENC_OBJ)
	-$(LD)  $(LDFLAGS) $(COMMON_OBJ) $(MC_ENC_OBJ) $(LIBFLAGS) -o $(MC_DIR)/$(MC_ENC_PROG)

$(MC_DEC_PROG): $(COMMON_OBJ) $(MC_DEC_OBJ)
	-$(LD)  $(LDFLAGS) $(COMMON_OBJ) $(MC_DEC_OBJ) $(LIBFLAGS) -o $(MC_DIR)/$(MC_DEC_PROG)

printsrc: $(FILES) $(INCLUDES)	$(COMMON_OBJ) #print source listings
	-$(LPR) $(LPRFLAGS) $(INCLUDES) $(FILES) | lpr

$(OBJ): $(MAKEFILE)

.c.o:
	-$(CC)  $(CCFLAGS) $< -o $@

.o :
	-$(LD) $(LDFLAGS) $< $(LIBFLAGS) -o $*

clean:
	/bin/rm -f $(OBJ)
	/bin/rm -f $(ALL_PROGS)
	/bin/rm -f $(SC_DIR)/$(SC_ENC_PROG)
	/bin/rm -f $(SC_DIR)/$(SC_DEC_PROG)
	/bin/rm -f $(MC_DIR)/$(MC_ENC_PROG)
	/bin/rm -f $(MC_DIR)/$(MC_DEC_PROG)
	/bin/rm -f *.o
