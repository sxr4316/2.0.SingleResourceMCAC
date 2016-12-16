#!/usr/bin/python
import argparse
import commands
import os
import sys

PARENT_DIR = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)),os.path.pardir))
INPUT_DIR = os.path.join(PARENT_DIR, "sc_model", "null_vectors", "input")
OUTPUT_DIR = os.path.join(PARENT_DIR, "sc_model", "null_vectors", "output")
ENCODER = os.path.join(PARENT_DIR, "sc_model", "sc_enc")
RATES = ["16", "24", "32", "40"]
LAWS = ["M", "A"]

FILE_TO_DIR = {	"H_OVR_null.I": os.path.join("homing", "ovr"), 
				"R_OVR_null.I": os.path.join("reset", "ovr"), 
				"H_NRM_null.I": os.path.join("homing", "nrm"), 
				"R_NRM_null.I": os.path.join("reset", "nrm")}

if not os.path.isfile(ENCODER):
	print "ERROR: sc_enc does not exist. Please compile the encoder."
	os.sys.exit(1)

sys.stdout.write("Generated: ")
sys.stdout.flush()
counter = 1
totalLoops = 32
previousPrintLine = ""
for file, extDir in FILE_TO_DIR.iteritems():
	inputFilePath = os.path.join(INPUT_DIR, file)
	for law in LAWS:
		for rate in RATES:
			commandsList = []
			outputVectorPath = os.path.join(OUTPUT_DIR, law, extDir, rate)
			lawFlag = " -a" if law == 'A' else ''
			commandsList.append("rm -f /tmp/${USER}_sr_vec_out/*")
			commandsList.append("%s -i %s -o NULL.O -r %s -v%s" % (ENCODER, inputFilePath, rate, lawFlag))
			commandsList.append("mkdir -p %s" % outputVectorPath)
			commandsList.append("rm -f %s/*" % outputVectorPath)
			commandsList.append("mv /tmp/${USER}_sr_vec_out/* %s" % outputVectorPath)
			commandsList.append("mv NULL.O %s" % outputVectorPath)
			for cmd in commandsList:
				os.system(cmd)
			printLine = os.path.join(law, extDir, rate) + " (%s / %s)" % (counter, totalLoops)
			for char in previousPrintLine:
				sys.stdout.write('\010')
				sys.stdout.flush()
			counter += 1
			previousPrintLine = printLine
			sys.stdout.write(printLine)
			sys.stdout.flush()

print "\nDone!"
