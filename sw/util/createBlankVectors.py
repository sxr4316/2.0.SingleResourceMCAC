#!/usr/bin/python
import os
import sys

# Creates the blank vectors for multi-channel

def main(file):
	counter = 0
	with open(file, "rb") as inputFile:
		with open(os.path.splitext(file)[0]+"_null.I", "wb") as outputFile:
			for char in inputFile.read():
				if char == '\n':
					outputFile.write('\n')
				else:
					outputFile.write('0')
					counter += 1

	counter = counter/2
	print "Nullified %s bytes" % hex(counter)

if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "Usage: %s <file>" % sys.argv[0]
		sys.exit(1)
	file = os.path.abspath(sys.argv[1])
	if not os.path.isfile(file):
		print "ERROR: Could not find file: %s" % file
		sys.exit(1)
	main(file)