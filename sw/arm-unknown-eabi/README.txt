
This build area is for cross compile of software for the ARM core.

To build software, you'll need to:

Source the environment setup:

Bourne or Bash Shell (the default): . ./ENV.sh (use this!!) 
C-Shell: source ./ENV.csh

Then build to supporting tools and libraries in the following
directories:

cd mini-libc
make

cd boot-loader-serial
make 

cd tools
make

Then build the ADPCM software:

cd lib
make

cd sc_adpcm
make

