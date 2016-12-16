# ----------------------------------------------------------------
.DEFAULT_GOAL := all

ifdef FLATTENED
SRC = tbl.c communication.c
OBJ = tbl.o communication.o
else
SRC = communication.c tbl.c expand.c subta.c llog.c quan.c subtb.c adda.c antilog.c reconst.c filtd.c filte.c functw.c g711.c inv_g711.c limb.c mix.c filta.c filtb.c filtc.c functf.c lima.c subtc.c triga.c accum.c addb.c addc.c floata.c floatb.c fmult.c limc.c limd.c trigb.c upa1.c upa2.c upb.c tone.c trans.c xor.c compress.c synch.c
OBJ = communication.o tbl.o expand.o subta.o llog.o quan.o subtb.o adda.o antilog.o reconst.o filtd.o filte.o functw.o g711.o inv_g711.o limb.o mix.o filta.o filtb.o filtc.o functf.o lima.o subtc.o triga.o accum.o addb.o addc.o floata.o floatb.o fmult.o limc.o limd.o trigb.o upa1.o upa2.o upb.o tone.o trans.o xor.o compress.o synch.o
endif

ifdef OTHER_SRC
SRC += $(OTHER_SRC)
endif

ifdef OTHER_OBJ
OBJ += $(OTHER_OBJ)
endif

ADPCM_LIB = libadpcm.a

USE_MINI_LIBC = 1
CREATE_FLT_OUTPUT = 1

include ../include/common.mk

all: $(ADPCM_LIB)

$(ADPCM_LIB): $(OBJ)
	-$(AR) rcv $(ADPCM_LIB) $(OBJ)
	-$(RANLIB) $(ADPCM_LIB)
	-date

