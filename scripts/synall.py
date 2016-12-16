#!/usr/bin/python
import argparse
import commands
import os
import shlex
import subprocess
import sys
import threading

GREP_COMMAND = "grep -irn tlat %s"

MODULE_LIST = [	"ACCUM",
				"ADDB",
				"ADDC",
				"APRSC",
				"boot_mem128",
				"CFG_INT",
				"CLK_GEN",
				"dec",
				"DELAY",
				"enc",
				"FLOATA",
				"FLOATB",
				"FMULT",
				"FMULT_ACCUM",
				"interrupt_controller",
				"LIMC",
				"LIMD",
				"mcac",
				"TDMI",
				"TDMO",
				"test_module",
				"timer_module",
				"TRIGB",
				"uart",
				"UPA1",
				"UPA2",
				"UPB",
				"wishbone_arbiter",
				"XOR" ]

PARENT_DIR = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)),os.path.pardir))
DEV_NULL = open(os.devnull, 'w')

def syn(module):
	moduleDir = os.path.abspath(os.path.join(PARENT_DIR,module))

	if not os.path.isdir(moduleDir):
		print "Warning: Module directory for %s does not exist" % module
		return

	synFile = os.path.join(moduleDir, "syn.csh")
	if not os.path.isfile(synFile):
		print "ERROR: Synthesis script for module %s does not exist" % module
		return

	os.chdir(moduleDir)
	command = "%s -b" % synFile
	command = shlex.split(command)

	subprocess.call(command, stdout=DEV_NULL, stderr=DEV_NULL)
	print "Finished synthesis for: %s" % module

def synall():
	threads = [threading.Thread(target=syn, args=(module,)) for module in MODULE_LIST]
	for t in threads:
		t.start()

	for t in threads:
		t.join()

def latchCheck():
	for module in MODULE_LIST:
		netlistDir = os.path.join(os.path.abspath(os.path.join(PARENT_DIR,module)),"netlist")
		if not os.path.isdir(netlistDir):
			print "Warning: Netlist directory for %s does not exist" % module
			continue
		else:
			command = GREP_COMMAND % netlistDir
			output = commands.getstatusoutput(command)[1]
			if output:
				print output

def main(args):
	if args["synthesize"]:
		synall()

	if args["latch_check"]:
		latchCheck()

	if not args["synthesize"] and not args["latch_check"]:
		synall()
		latchCheck()

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Create or decode multichannel vectors")
	parser.add_argument("-s", "--synthesize", action="store_true", required=False, help="Run synthesis")
	parser.add_argument("-l", "--latch-check", action="store_true", required=False, help="Check for latches")
	args = vars(parser.parse_args())
	main(args)