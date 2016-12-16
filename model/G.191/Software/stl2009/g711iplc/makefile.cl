# -----------------------------------------------------------------------------
# MS C makefile for compiling and testing the G.711 Appendix I implementation
# The executable must be defined by variable below.
# 01.Feb.2002 - Makefile adapted by <simao.campos@labs.comsat.com> from original
#               Makefile provided by AT&T
# -----------------------------------------------------------------------------
#
# Packet Loss Concealment code for G.711 Appendix I
# Copyright (c) 1997-2000 AT&T Corp.
# All rights reserved.
# 
# Makefile for Microsoft Visual C++: nmake -f Make.ms
.SUFFIXES: .o .c .cc

# ------------------------------------------------
# Choose compiler and options.
# ------------------------------------------------
CC=cl
RFLAGS=-O2 -D__STDC__ -I../utl -I../eid
DFLAGS= -Od -Zi -D "_DEBUG" -D__STDC__
CFLAGS= -nologo -G5 -Gr $(RFLAGS) -W3 -DUSEDOUBLES

# ------------------------------------
#  Lists of files
# ------------------------------------
OFILESC=g711iplc.obj lowcfe.obj plcferio.obj softbit.obj error.obj
OAF= asc2g192.obj error.obj softbit.obj

CTARG=g711iplc.exe asc2g192.exe
ALL=$(CTARG)

# ------------------------------------------------
# Choose a file comparison utility:
#       - cf compares,
#       - sub shows the difference of the different samples
# Default is cf.
# ------------------------------------------------
DIFF = fc /b
#DIFF = sub
#DIFF_OPT = 256 1 30

# ------------------------------------------------
# Choose an archiving utility: 
#       - public domain unzip, or [PC/MS Visual C/VMS]
#       - shareware pkunzip [PC only]
# ------------------------------------------------
#UNZIP = pkunzip
UNZIP = unzip -o

# ------------------------------------------------
# Generic rules
# ------------------------------------------------

.c.obj:
	$(CC) -c $(CFLAGS) $(*B).c

# ------------------------------------
# Targets
# ------------------------------------
all: $(ALL)

clean:
	rm -f *.obj

cleantest:
	rm -f *.raw
	rm -f *.g192
	rm -f *.le

veryclean: clean cleantest
	rm -f *.exe

# ------------------------------------
# Dependencies and rules
# ------------------------------------
g711iplc: g711iplc.exe
g711iplc.exe: $(OFILESC)
	$(CC) $(CFLAGS) -Fe$(*B).exe -Fd$(*B).pdb $(OFILESC)
asc2g192: asc2g192.exe
asc2g192.exe: $(OAF)
	$(CC) $(CFLAGS) -Fe$(*B).exe -Fd$(*B).pdb $(OAF)

softbit.obj: ../eid/softbit.c
        $(CC) $(CFLAGS) -c -o softbit.obj ../eid/softbit.c

# -----------------------------------------
# Test implementation with provided script
# -----------------------------------------
#test: $(ALL)
#	testit.bat

# ------------------------------------------------
# Test portability
# ------------------------------------------------
test: proc comp

proc: fe10.g192
	g711iplc -stats fe10.g192 f2.le f2_10_c.raw
	g711iplc -noplc -stats fe10.g192 f2.le f2_10m_c.raw
	g711iplc -stats fe10_2.g192 f2.le f2_10_2_c.raw
	g711iplc -noplc -stats fe10_2.g192 f2.le f2_10_2m_c.raw

comp: f2_10.raw
	$(DIFF) f2_10.raw f2_10_c.raw
	$(DIFF) f2_10m.raw f2_10m_c.raw
	$(DIFF) f2_10_2.raw f2_10_2_c.raw
	$(DIFF) f2_10_2m.raw f2_10_2m_c.raw

# ------------------------------------------------
# Extract from archive, if necessary
# ------------------------------------------------
fe10.g192:	tst-g711iplc.zip
	$(UNZIP) tst-g711iplc.zip *.g192
	swapover *.g192
	$(UNZIP) tst-g711iplc.zip f2.le
	swapover f2.le

f2_10.raw:	tst-g711iplc.zip
	$(UNZIP) tst-g711iplc.zip f2_10.raw f2_10_2.raw f2_10_2m.raw f2_10m.raw
	swapover f2_10.raw f2_10_2.raw f2_10_2m.raw f2_10m.raw