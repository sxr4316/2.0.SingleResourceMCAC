# ----------------------------------------------------------------
.DEFAULT_GOAL := all

all:
	-make -C ../lib
	-make -f Makefile.enc
	-make -f Makefile.dec
	-date

clean:
	-make -C ../lib clean
	-make -f Makefile.enc clean
	-make -f Makefile.dec clean

