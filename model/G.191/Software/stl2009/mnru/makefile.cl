# -----------------------------------------------------------------------------
#	MS Visual C makefile for compiling and testing the UGST MNRU 
#       implementation
#	Implemented by <simao.campos@labs.comsat.com> -- 31.Jul.95
# -----------------------------------------------------------------------------

# ------------------------------------------------
# Choose compiler.
# ------------------------------------------------
GCC = cl
CC_OPT = -I../utl -W1
PROGRAMS = mnrudemo
MNRU_EXE = mnrudemo -q

# ------------------------------------------------
# Choose an archiving utility: 
#       - public domain unzip, or [PC/MS Visual C/VMS]
#       - shareware pkunzip [PC only]
# ------------------------------------------------
#UNZIP = pkunzip
UNZIP = unzip -o

# ------------------------------------------------
# Choose an AWK; suggest use GNU version 
#                (available via anonymous ftp) 
# ------------------------------------------------
AWK = gawk
AWK_CMD = '$$6~/[0-9]+:[0-9][0-9]/ {print "sb -over",$$NF};END{print "exit"}'

# ------------------------------------------------
# Choose a file comparison utility: 
#       - cf compares, 
#       - sub shows the difference of the different samples
# Default is cf.
# ------------------------------------------------
DIFF = cf -q
#DIFF = sub
#DIFF_OPT = 256 1 30

# ------------------------------------------------
# Define other tools
# ------------------------------------------------
RM=rm -f

# ------------------------------------------------
# List of files (source and object)
# ------------------------------------------------
SOURCES = mnru.c mnrudemo.c ugst-utl.c 
OBJECTS = mnru.obj mnrudemo.obj ugst-utl.obj 

# ------------------------------------------------
# Generic rules
# ------------------------------------------------
.c.obj:
	$(CC) $(CC_OPT) -c $<

# ------------------------------------------------
# Targets
# ------------------------------------------------
all: $(PROGRAMS) 

clean:
	$(RM) $(OBJECTS)

cleantest:
	$(RM) sine*.unx *.q?? sine.src

veryclean: clean cleantest
	$(RM) mnrudemo.exe

# ------------------------------------------------
# Object & executable code
# ------------------------------------------------
mnru.obj: mnru.c
	$(CC) $(CC_OPT) -c mnru.c

mnrudemo: mnrudemo.exe
mnrudemo.exe: mnrudemo.obj mnru.obj ugst-utl.obj
	$(CC) $(CC_OPT) -o mnrudemo mnrudemo.obj mnru.obj ugst-utl.obj  

ugst-utl.obj: ../utl/ugst-utl.c
	$(CC) $(CC_OPT) -c ../utl/ugst-utl.c

calc-snr.obj:	calc-snr.c snr.c
snr:	calc-snr.obj
	$(CC) $(CC_OPT) -o snr calc-snr.obj 

# ------------------------------------------------
# Testing portability
# Note: there are no compliance test vectors associated with this module
# ------------------------------------------------
sine-q99.unx: sine-ref.zip
	$(UNZIP) sine-ref.zip s*.unx
	swapover s*.unx
#	$(UNZIP) -v sine-ref.zip *.unx | $(AWK) $(AWK_CMD) | command

sine.src: sine-ref.zip
	$(UNZIP) sine-ref.zip sine.src
	sb -over sine.src

test: proc comp

proc:	sine.src
	$(MNRU_EXE) sine.src sine.q00 256 1 20 00
	$(MNRU_EXE) sine.src sine.q05 256 1 20 05
	$(MNRU_EXE) sine.src sine.q10 256 1 20 10
	$(MNRU_EXE) sine.src sine.q15 256 1 20 15
	$(MNRU_EXE) sine.src sine.q20 256 1 20 20
	$(MNRU_EXE) sine.src sine.q25 256 1 20 25
	$(MNRU_EXE) sine.src sine.q30 256 1 20 30
	$(MNRU_EXE) sine.src sine.q35 256 1 20 35
	$(MNRU_EXE) sine.src sine.q40 256 1 20 40
	$(MNRU_EXE) sine.src sine.q45 256 1 20 45
	$(MNRU_EXE) sine.src sine.q50 256 1 20 50
	$(MNRU_EXE) sine.src sine.q99 256 1 20 150

comp: sine-q99.unx
	$(DIFF) sine-q00.unx sine.q00
	$(DIFF) sine-q05.unx sine.q05
	$(DIFF) sine-q10.unx sine.q10
	$(DIFF) sine-q15.unx sine.q15
	$(DIFF) sine-q20.unx sine.q20
	$(DIFF) sine-q25.unx sine.q25
	$(DIFF) sine-q30.unx sine.q30
	$(DIFF) sine-q35.unx sine.q35
	$(DIFF) sine-q40.unx sine.q40
	$(DIFF) sine-q45.unx sine.q45
	$(DIFF) sine-q50.unx sine.q50
	$(DIFF) sine-q99.unx sine.q99

run:	proc calc-snr

calc-snr: sine.qinf
	snr -q sine.qinf sine.q00 
	snr -q sine.qinf sine.q05 
	snr -q sine.qinf sine.q10 
	snr -q sine.qinf sine.q15 
	snr -q sine.qinf sine.q20 
	snr -q sine.qinf sine.q25 
	snr -q sine.qinf sine.q30 
	snr -q sine.qinf sine.q35 
	snr -q sine.qinf sine.q40 
	snr -q sine.qinf sine.q45 
	snr -q sine.qinf sine.q50 

sine.qinf: sine.src
	$(MNRU_EXE) -Q 150 sine.src sine.qinf
