# ----------------------------------------------------------------
.DEFAULT_GOAL := all
DEFINES = -D CO_P
ENC_SRC = "enc.c"
ENC_OBJ = "enc.o"
DEC_SRC = "dec.c"
DEC_OBJ = "dec.o"

enc:
	-make -C ../lib DEFINES="$(DEFINES)" OTHER_SRC="$(ENC_SRC)" OTHER_OBJ="$(ENC_OBJ)"
	-make -f Makefile.enc DEFINES="$(DEFINES)"
	-date


enc_flat:
	-make -C ../lib FLATTENED=1
	-make -f Makefile_flat.enc DEFINES="$(DEFINES)"
	-date

dec:
	-make -C ../lib DEFINES="$(DEFINES)" OTHER_SRC="$(DEC_SRC)" OTHER_OBJ="$(DEC_OBJ)"
	-make -f Makefile.dec DEFINES="$(DEFINES)"
	-date

dec_flat:
	-make -C ../lib FLATTENED=1
	-make -f Makefile_flat.dec DEFINES="$(DEFINES)"
	-date
	
all:
	-make -C ../lib DEFINES="$(DEFINES)" OTHER_SRC="$(ENC_SRC) $(DEC_SRC)" OTHER_OBJ="$(ENC_OBJ) $(DEC_OBJ)"
	-make -f Makefile.enc DEFINES="$(DEFINES)"
	-make -f Makefile.dec DEFINES="$(DEFINES)"
	-date

clean:
	-make -C ../lib clean
	-make -f Makefile.enc clean
	-make -f Makefile_flat.enc clean
	-make -f Makefile.dec clean