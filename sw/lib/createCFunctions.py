#!/usr/bin/python

# Creates some c functions and stuff for stuff

DEC_VARS = "a1,a1p,a1r,a1t,a2,a2p,a2r,a2t,aLaw,al,ap,app,apr,ax,b1,b1p,b1r,b2,b2p,b2r,b3,b3p,b3r,b4,b4p,b4r,b5,b5p,b5r,b6,b6p,b6r,dlnx,dlx,dml,dmlp,dms,dmsp,dq,dq0,dq1,dq2,dq3,dq4,dq5,dq6,dql,dqln,dqs,dsx,dx,fi,I,pcm,pk0,pk1,pk2,sd,se,sez,sg,sigpk,slx,sp,sr,sr0,sr1,sr2,td,tdp,tdr,tr,u1,u2,u3,u4,u5,u6,wa1,wa2,wb1,wb2,wb3,wb4,wb5,wb6,wi,y,yl,ylp,yu,yup,yut"
ENC_VARS = "a1,a1p,a1r,a1t,a2,a2p,a2r,a2t,aLaw,al,ap,app,apr,ax,b1,b1p,b1r,b2,b2p,b2r,b3,b3p,b3r,b4,b4p,b4r,b5,b5p,b5r,b6,b6p,b6r,d,dl,dln,dml,dmlp,dms,dmsp,dq,dq0,dq1,dq2,dq3,dq4,dq5,dq6,dql,dqln,dqs,ds,fi,I,pcm,pk0,pk1,pk2,se,sez,sigpk,sl,sr,sr0,sr1,sr2,ss,td,tdp,tdr,tr,u1,u2,u3,u4,u5,u6,wa1,wa2,wb1,wb2,wb3,wb4,wb5,wb6,wi,y,yl,ylp,yu,yup,yut"

NEW_VARS = "input,output".split(',')

def createFile(fileName, variables):
	OUTPUT_FILE = open(fileName, "wb")

	lst = variables.split(',')

	OUTPUT_FILE.write("#include <stdio.h>\n")
	OUTPUT_FILE.write("#include <stdlib.h>\n")
	OUTPUT_FILE.write("#include <sys/stat.h>\n")
	OUTPUT_FILE.write("#include \"../include/encode.h\"\n\n")

	# Extern
	#OUTPUT_FILE.write("extern long %s;\n\n" % variables)

	# Write file pointers
	for var in lst:
		OUTPUT_FILE.write("FILE *f_%s;\n" % var)
	for var in NEW_VARS:
		OUTPUT_FILE.write("FILE *f_%s;\n" % var)

	# setOnlyTopLevel function
	OUTPUT_FILE.write("\nint ONLY_TOP_LEVEL = 0;\n")
	OUTPUT_FILE.write("void setOnlyTopLevel()\n{\n")
	OUTPUT_FILE.write("\tONLY_TOP_LEVEL = 1;\n")
	OUTPUT_FILE.write("}\n")


	# openFiles function
	OUTPUT_FILE.write("\nvoid openFiles()\n{\n")
	OUTPUT_FILE.write("\tconst char* userName = getenv(\"USER\");\n")
	OUTPUT_FILE.write("\tchar outPath[64];\n")
	OUTPUT_FILE.write("\tchar fileDir[64];\n")
	OUTPUT_FILE.write("\tsnprintf(outPath, sizeof(outPath), \"/tmp/%s_sr_vec_out/\", userName);\n")
	OUTPUT_FILE.write("\tchar* mode = \"w\";\n\n")

	OUTPUT_FILE.write("\tmkdir(outPath, (S_IRWXU | S_IRWXG | S_IRWXO));\n")
	
	OUTPUT_FILE.write("if (!ONLY_TOP_LEVEL) {\n")
	for var in lst:
		OUTPUT_FILE.write("\tsnprintf(fileDir, sizeof(fileDir), \"%%s%s.t\", outPath);\n" % var)
		OUTPUT_FILE.write("\tf_%s = fopen(fileDir, mode);\n" % (var))
	OUTPUT_FILE.write("}\n")
	for var in NEW_VARS:
		OUTPUT_FILE.write("\tsnprintf(fileDir, sizeof(fileDir), \"%%s%s.t\", outPath);\n" % var)
		OUTPUT_FILE.write("\tf_%s = fopen(fileDir, mode);\n" % (var))

	OUTPUT_FILE.write("}\n\n")

	# printVar function
	OUTPUT_FILE.write("int clockCount = 0;\n")
	OUTPUT_FILE.write("\nvoid printVars(struct channel* c)\n{\n")

	listCounter = 0
	for var in lst:
		OUTPUT_FILE.write("\tfprintf(%s, \"@%%04X\\t%%06X\\n\", clockCount, %s);\n" % (("f_"+var), "c->"+var))
		listCounter+=1

	OUTPUT_FILE.write("\tclockCount++;\n")
	OUTPUT_FILE.write("}\n")

	# printToConsole function
	OUTPUT_FILE.write("\nvoid printChannel(struct channel* c)\n{\n")

	listCounter = 0
	for var in lst:
		OUTPUT_FILE.write("\tprintf(\"%s = %%ld\\n\", %s);\n" % (var, "c->"+var))
		listCounter+=1

	OUTPUT_FILE.write("}\n")

	# Other var functions:

	for var in NEW_VARS:
		OUTPUT_FILE.write("\nint %sCounter = 0;\n" % var)
		OUTPUT_FILE.write("void print%s(long %s)\n{\n" % (var, var))
		OUTPUT_FILE.write("\tfprintf(%s, \"@%%04X\\t%%06X\\n\", %s, %s);\n" % (("f_"+var), var+"Counter", var))
		OUTPUT_FILE.write("\t%sCounter++;\n" % var)
		OUTPUT_FILE.write("}\n")


	# closeFiles function
	OUTPUT_FILE.write("\nvoid closeFiles()\n{\n")
	OUTPUT_FILE.write("if (!ONLY_TOP_LEVEL) {\n")
	for var in lst:
		OUTPUT_FILE.write("\tfclose(%s);\n" % ("f_"+var))
	OUTPUT_FILE.write("}\n")
	for var in NEW_VARS:
		OUTPUT_FILE.write("\tfclose(%s);\n" % ("f_"+var))

	
	OUTPUT_FILE.write("\n\tconst char* userName = getenv(\"USER\");\n")
	OUTPUT_FILE.write("\tchar outPath[64];\n")
	OUTPUT_FILE.write("\tsnprintf(outPath, sizeof(outPath), \"/tmp/%s_sr_vec_out/vectorSize.t\", userName);\n")

	OUTPUT_FILE.write("\n\tFILE* f_size;\n")
	OUTPUT_FILE.write("\tf_size = fopen(outPath, \"w\");\n")
	OUTPUT_FILE.write("\tif (ONLY_TOP_LEVEL) {\n")
	OUTPUT_FILE.write("\t\tfprintf(f_size, \"@0000\\t%06X\\n\", inputCounter);\n")
	OUTPUT_FILE.write("\t} else {\n")
	OUTPUT_FILE.write("\t\tfprintf(f_size, \"@0000\\t%06X\\n\", clockCount);\n")
	OUTPUT_FILE.write("\t}\n")
	OUTPUT_FILE.write("\tfclose(f_size);\n")

	OUTPUT_FILE.write("}\n\n")

	# Close file
	OUTPUT_FILE.close()

if __name__ == "__main__":
	createFile("create_vecs_enc.c", ENC_VARS)
	createFile("create_vecs_dec.c", DEC_VARS)