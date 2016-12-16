#!/usr/bin/python
import commands
import os
import sys

vectorFiles = ["a1p.t", "a1r.t", "a1.t", "a1t.t", "a2p.t", "a2r.t", "a2.t", "a2t.t", "aLaw.t", "al.t", "app.t", "apr.t", "ap.t", "ax.t", "b1p.t", "b1r.t", "b1.t", "b2p.t", "b2r.t", "b2.t", "b3p.t", "b3r.t", "b3.t", "b4p.t", "b4r.t", "b4.t", "b5p.t", "b5r.t", "b5.t", "b6p.t", "b6r.t", "b6.t", "dln.t", "dl.t", "dmlp.t", "dml.t", "dmsp.t", "dms.t", "dq0.t", "dq1.t", "dq2.t", "dq3.t", "dq4.t", "dq5.t", "dq6.t", "dqln.t", "dql.t", "dqs.t", "dq.t", "ds.t", "d.t", "fi.t", "I.t", "pcm.t", "pk0.t", "pk1.t", "pk2.t", "se.t", "sez.t", "sigpk.t", "sl.t", "sr0.t", "sr1.t", "sr2.t", "sr.t", "ss.t", "tdp.t", "tdr.t", "td.t", "tr.t", "u1.t", "u2.t", "u3.t", "u4.t", "u5.t", "u6.t", "vectorSize.t", "wa1.t", "wa2.t", "wb1.t", "wb2.t", "wb3.t", "wb4.t", "wb5.t", "wb6.t", "wi.t", "ylp.t", "yl.t", "y.t", "yup.t", "yu.t", "yut.t"]
rateDirs = ["16","24","32","40"]

PARENT_DIR = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)),os.path.pardir))
vecDir = os.path.join(PARENT_DIR, "sc_model/null_vectors/output/enc/reset/nrm/")

DIFF_COMMAND = "diff %s %s"

def Diff(dir1, dir2):
	for file in vectorFiles:
		file1 = os.path.join(dir1, file)
		file2 = os.path.join(dir2, file)
		command = DIFF_COMMAND % (file1, file2)
		output = commands.getstatusoutput(command)[1]
		if output:
			print file1, file2



def main():
	for i in range(0,4):
		for j in range(0,4):
			if i != j:
				brDir1 = os.path.join(vecDir, rateDirs[i])
				brDir2 = os.path.join(vecDir, rateDirs[j])
				Diff(brDir1, brDir2)

if __name__ == "__main__":
	main()