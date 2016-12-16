#!/usr/bin/python
import argparse
import os
import sys
import random

NUM_CHANNELS = 32

BytesToTestType = {	19880 : "H_NRM",
					5544 : "H_OVR",
					16384 : "R_NRM",
					2048 : "R_OVR"
					}

class Channel(object):
	def __init__(self, data, index):
		self.Data = data
		self.Index = index
		self.ByteCount = len(self.Data)/2

class MultiChannel(object):
	def __init__(self):
		self.Channels = [None for i in range(0,NUM_CHANNELS)]
		self.LargestChannelLength = 0
		self.MergedData = None
		self.NullFile = None

	def AddSingle(self, channel):
		if type(channel) is not Channel:
			Error("Invalid type: %s" % type(channel))
		else:
			self.Channels[channel.Index] = channel
			if len(channel.Data) > self.LargestChannelLength:
				self.LargestChannelLength = len(channel.Data)

	def Decode(self, outputFile, channelNumber):
		lineCounter = 0
		channel = self.Channels[channelNumber]
		for i in range(0, len(channel.Data)-1, 2):
			byte = channel.Data[i:i+2]
			outputFile.write(byte)
			lineCounter += 1

			if lineCounter == NUM_CHANNELS:
				outputFile.write('\n')
				lineCounter = 0

	def Encode(self, outputFile):
		nullChannel = None
		if self.NullFile != None:
			with open(self.NullFile, "rb") as nullFile:
				nullChannel = Channel(nullFile.read().replace('\n',''), 0)
		for i in range(0, self.LargestChannelLength-1, 2):
			for channel in self.Channels:
				if channel == None or i >= len(channel.Data):
					if self.NullFile != None:
						byte = nullChannel.Data[i:i+2]
						outputFile.write(byte)
					else:
						outputFile.write("00")
				else:
					byte = channel.Data[i:i+2]
					outputFile.write(byte)
			outputFile.write('\n')

	def SplitMergedData(self):
		n = 0
		self.LargestChannelLength = len(self.MergedData)/2
		for i in range(0, len(self.MergedData)-1, 2):
			byte = self.MergedData[i:i+2]
			if self.Channels[n] == None:
				self.Channels[n] = Channel(byte, n)
			else:
				self.Channels[n].Data += byte
			n += 1
			if n == NUM_CHANNELS:
				n = 0

def Error(string, fatal=True):
	print "ERROR: " + string
	if fatal:
		sys.exit(1)

def main(args):
	channelList = range(0,NUM_CHANNELS)
	multiChannel = MultiChannel()

	if args["channel"] != None:
		i = 0
		for val in args["channel"]:
			if val < 0 or val > NUM_CHANNELS-1:
				Error("channel number: %s not in valid range [0,31]" % args["channel"])
			else:
				channelList[i] = val
				i += 1
	
	with open(args["output"], "wb") as outputFile:
		i = 0
		numFiles = len(args["input"])
		for filePath in args["input"]:
			if not os.path.isfile(filePath):
				Error("could not find input file: %s" % args["input"])
			else:
				with open(filePath, "rb") as inputFile:
					channel = Channel(inputFile.read().replace('\n',''), channelList[i])
					byteKeys = BytesToTestType.keys()
					byteKeys.sort()
					if numFiles == 1:
						if channel.ByteCount <= byteKeys[-1]:
							multiChannel.NullFile = args["null"]
							multiChannel.AddSingle(channel)
							multiChannel.Encode(outputFile)
						else:
							multiChannel.MergedData = channel.Data
							multiChannel.SplitMergedData()
							if args["channel"] == None or len(args["channel"]) > 1:
								Error("Please enter a single channel value for decoding")
							multiChannel.Decode(outputFile, args["channel"][0])
					else:
						if args["null"] != None:
							Error("Can only use null mode if a single input is provided")
						if channel.ByteCount > byteKeys[-1]:
							Error("Multiple file mode can only be used for the creation of vectors")
						else:
							multiChannel.AddSingle(channel)
			i += 1

		if numFiles > 1:
			multiChannel.Encode(outputFile)

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Create or decode multichannel vectors")
	parser.add_argument("-i", "--input", nargs='+', required=True, metavar="input-file", type=str, help="The input vector to be read")
	parser.add_argument("-o", "--output", required=True, metavar="output-file", type=str, help="The output vector to be written")
	parser.add_argument("-c", "--channel", nargs='+', required=False, metavar="channel-number", type=int, \
						help="The channel number to be encoded/decoded")
	parser.add_argument("-n", "--null", required=False, metavar="null-file", type=str, help="Store null vector instead of 0s")
	args = vars(parser.parse_args())
	main(args)