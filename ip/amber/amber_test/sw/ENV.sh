#
# source this file for command line work with kernel
#
target=arm-unknown-eabi

# Change /proj/amber to where you saved the amber package on your system
#export AMBER_BASE=/<your amber install path>/trunk
export AMBER_BASE=/home/maieee/work/opencores/amber/amber/amber_soc
# # Change /opt/Sourcery to where the package is installed on your system
#PATH=/<your code sourcery install path>/bin:${PATH}
PATH=/classes/eeee720/tools/x-tools/arm-unknown-eabi/bin:${PATH}
# # AMBER_CROSSTOOL is the name added to the start of each GNU tool in
# # the Code Sourcery bin directory. This variable is used in various makefiles to set
# # the correct tool to compile code for the Amber core
export AMBER_CROSSTOOL=${target}
# # Xilinx ISE installation directory
# # This should be configured for you when you install ISE.
# # But check that is has the correct value
# # It is used in the run script to locate the Xilinx library elements.
#export XILINX=/opt/Xilinx/14.5/ISE
export XILINX=/opt/Xilinx/ISE/14.7/ISE_DS/ISE/
#

#setenv ARCH arm
#setenv UBOOT_ARCH arm
#setenv CROSS_COMPILE ${target}-
#
#setenv STRIP "${target}-strip"
#setenv CPP "${target}-gcc -E"
#setenv OBJCOPY "${target}-objcopy"
#setenv AR "${target}-ar"
#setenv F77 "${target}-g77 -march=armv2a \
#			-mno-thumb-interwork -mno-thumb"
#setenv RANLIB "${target}-ranlib"
#setenv LD "${target}-ld"
#setenv CCLD "${target}-gcc -march=armv2a \
#			-mno-thumb-interwork -mno-thumb"
#setenv OBJDUMP "${target}-objdump"
#setenv CC "${target}-gcc -march=armv2a \
#			-mno-thumb-interwork -mno-thumb"
#setenv CXX "${target}-g++ -march=armv2a \
#		-mno-thumb-interwork -mno-thumb"
#setenv NM "${target}-nm"
#setenv AS "${target}-as"
#
#
