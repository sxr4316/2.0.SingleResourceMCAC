-------------------------
Description
-------------------------
This package can be used to verify that an executable build of the G.711.0
Reference Software produces bit-perfect output compared to the output produced
by the reference build.

-------------------------
Instructions
-------------------------
(1) Place the g711llc.exe (build of the G.711.0 Reference Software) to the
    "CuT" folder.
(2) Run runtests.bat and check the console output.
(3) In case of failures, check "tmp/failures.txt" for a list of the failed
    files.

-------------------------
Contents
-------------------------
CuT: placeholder folder for the g711llc.exe executable.

test_vectors: folder containing test vectors
    \ref_in:
        \testvec_0.lca  :  A-law bit-stream that decodes to 0 samples

        \testvec_0.lcm  : Mu-law bit-stream that decodes to 0 samples

        \testvec_40.al8 :  A-law input signal that triggers all the possible
                          encoder tools if encoded with -n40

        \testvec_40.mu8 : Mu-law input signal that triggers all the possible
                          encoder tools if encoded with -n40

        \testvec_80.al8 :  A-law input signal that triggers all the possible
                          encoder tools if encoded with -n80

        \testvec_80.mu8 : Mu-law input signal that triggers all the possible
                          encoder tools if encoded with -n80

        \testvec_160.al8:  A-law input signal that triggers all the possible
                          encoder tools if encoded with -n160

        \testvec_160.mu8: Mu-law input signal that triggers all the possible
                          encoder tools if encoded with -n160

        \testvec_240.al8:  A-law input signal that triggers all the possible
                          encoder tools if encoded with -n240

        \testvec_240.mu8: Mu-law input signal that triggers all the possible
                          encoder tools if encoded with -n240

        \testvec_320.al8:  A-law input signal that triggers all the possible
                          encoder tools if encoded with -n320

        \testvec_320.mu8: Mu-law input signal that triggers all the possible
                          encoder tools if encoded with -n320

        \testvec_all.al8: The following files appended together in the
                          following order:
                              testvec_320.al8
                              filler160.al8
                              testvec_240.al8
                              testvec_160.al8
                              testvec_80.al8
                              testvec_40.al8
                              filler840.al8
                          This serves a universal input signal that triggers
                          all the possible encoder tools regardless of the
                          frame length.

        \testvec_all.lca: The following files appended together in the
                          following order:
                              testvec_320.lca [testvec_320.al8 / -n320]
                              filler160.lca   [filler.al8      / -n160]
                              testvec_240.lca [testvec_240.al8 / -n240]
                              testvec_160.lca [testvec_160.al8 / -n160]
                              testvec_80.lca  [testvec_80.al8  /  -n80]
                              testvec_40.lca  [testvec_40.al8  /  -n40]
                              testvec_0.lca
                              filler840.lca   [filler_840.al8  /  -n40]
                          This serves a universal input signal that triggers
                          all the possible encoder tools regardless of the
                          frame length.

        \testvec_all.mu8: The following files appended together in the
                          following order:
                              testvec_320.mu8
                              filler160.mu8
                              testvec_240.mu8
                              testvec_160.mu8
                              testvec_80.mu8
                              testvec_40.mu8
                              filler840.mu8
                          This serves a universal input signal that triggers
                          all the possible encoder tools regardless of the
                          frame length.

        \testvec_all.lcm: The following files appended together in the
                          following order:
                              testvec_320.lcm [testvec_320.mu8 / -n320]
                              filler160.lcm   [filler.mu8      / -n160]
                              testvec_240.lcm [testvec_240.mu8 / -n240]
                              testvec_160.lcm [testvec_160.mu8 / -n160]
                              testvec_80.lcm  [testvec_80.mu8  /  -n80]
                              testvec_40.lcm  [testvec_40.mu8  /  -n40]
                              testvec_0.lcm
                              filler840.lcm   [filler_840.mu8  /  -n40]
                          This serves a universal input signal that triggers
                          all the possible encoder tools regardless of the
                          frame length.

        \speech01.al8 : A-law speech signal (Chinese, female)

        \speech01.mu8 : Mu-law speech signal (Chinese, female)

        \speech02.al8 : A-law speech signal (Chinese, male)

        \speech02.mu8 : Mu-law speech signal (Chinese, male)

        \speech03.al8 : A-law speech signal (English, female)

        \speech03.mu8 : Mu-law speech signal (English, female)

        \speech04.al8 : A-law speech signal (English, male)

        \speech04.mu8 : Mu-law speech signal (English, male)

        \speech05.al8 : A-law speech signal (Japanese, female)

        \speech05.mu8 : Mu-law speech signal (Japanese, female)

        \speech06.al8 : A-law speech signal (Japanese, male)

        \speech06.mu8 : Mu-law speech signal (Japanese, male)

    \ref_out:
        \testvec_0.alo      : Output of "g711llc.exe testvec_0.lca"
        \testvec_0.muo      : Output of "g711llc.exe testvec_0.lcm"
        \testvec_40.lca     : Output of "g711llc.exe -n40 testvec_40.al8"
        \testvec_40.lcm     : Output of "g711llc.exe -n40 testvec_40.mu8"
        \testvec_80.lca     : Output of "g711llc.exe -n80 testvec_80.al8"
        \testvec_80.lcm     : Output of "g711llc.exe -n80 testvec_80.mu8"
        \testvec_160.lca    : Output of "g711llc.exe -n160 testvec_160.al8"
        \testvec_160.lcm    : Output of "g711llc.exe -n160 testvec_160.mu8"
        \testvec_240.lca    : Output of "g711llc.exe -n240 testvec_240.al8"
        \testvec_240.lcm    : Output of "g711llc.exe -n240 testvec_240.mu8"
        \testvec_320.lca    : Output of "g711llc.exe -n320 testvec_320.al8"
        \testvec_320.lcm    : Output of "g711llc.exe -n320 testvec_320.mu8"
        \testvec_all.40.lca : Output of "g711llc.exe -n40 testvec_all.al8"
        \testvec_all.40.lcm : Output of "g711llc.exe -n40 testvec_all.mu8"
        \testvec_all.80.lca : Output of "g711llc.exe -n80 testvec_all.al8"
        \testvec_all.80.lcm : Output of "g711llc.exe -n80 testvec_all.mu8"
        \testvec_all.160.lca: Output of "g711llc.exe -n160 testvec_all.al8"
        \testvec_all.160.lcm: Output of "g711llc.exe -n160 testvec_all.mu8"
        \testvec_all.240.lca: Output of "g711llc.exe -n240 testvec_all.al8"
        \testvec_all.240.lcm: Output of "g711llc.exe -n240 testvec_all.mu8"
        \testvec_all.320.lca: Output of "g711llc.exe -n320 testvec_all.al8"
        \testvec_all.320.lcm: Output of "g711llc.exe -n320 testvec_all.mu8"
        \testvec_all.alo    : Output of "g711llc.exe testvec_all.lca"
        \testvec_all.muo    : Output of "g711llc.exe testvec_all.lcm"
        \speech01.40.lca    : Output of "g711llc.exe -n40 speech01.al8"
        \speech01.40.lcm    : Output of "g711llc.exe -n40 speech01.mu8"
        \speech01.80.lca    : Output of "g711llc.exe -n80 speech01.al8"
        \speech01.80.lcm    : Output of "g711llc.exe -n80 speech01.mu8"
        \speech01.160.lca   : Output of "g711llc.exe -n160 speech01.al8"
        \speech01.160.lcm   : Output of "g711llc.exe -n160 speech01.mu8"
        \speech01.240.lca   : Output of "g711llc.exe -n240 speech01.al8"
        \speech01.240.lcm   : Output of "g711llc.exe -n240 speech01.mu8"
        \speech01.320.lca   : Output of "g711llc.exe -n320 speech01.al8"
        \speech01.320.lcm   : Output of "g711llc.exe -n320 speech01.mu8"
        \speech02.40.lca    : Output of "g711llc.exe -n40 speech02.al8"
        \speech02.40.lcm    : Output of "g711llc.exe -n40 speech02.mu8"
        \speech02.80.lca    : Output of "g711llc.exe -n80 speech02.al8"
        \speech02.80.lcm    : Output of "g711llc.exe -n80 speech02.mu8"
        \speech02.160.lca   : Output of "g711llc.exe -n160 speech02.al8"
        \speech02.160.lcm   : Output of "g711llc.exe -n160 speech02.mu8"
        \speech02.240.lca   : Output of "g711llc.exe -n240 speech02.al8"
        \speech02.240.lcm   : Output of "g711llc.exe -n240 speech02.mu8"
        \speech02.320.lca   : Output of "g711llc.exe -n320 speech02.al8"
        \speech02.320.lcm   : Output of "g711llc.exe -n320 speech02.mu8"
        \speech03.40.lca    : Output of "g711llc.exe -n40 speech03.al8"
        \speech03.40.lcm    : Output of "g711llc.exe -n40 speech03.mu8"
        \speech03.80.lca    : Output of "g711llc.exe -n80 speech03.al8"
        \speech03.80.lcm    : Output of "g711llc.exe -n80 speech03.mu8"
        \speech03.160.lca   : Output of "g711llc.exe -n160 speech03.al8"
        \speech03.160.lcm   : Output of "g711llc.exe -n160 speech03.mu8"
        \speech03.240.lca   : Output of "g711llc.exe -n240 speech03.al8"
        \speech03.240.lcm   : Output of "g711llc.exe -n240 speech03.mu8"
        \speech03.320.lca   : Output of "g711llc.exe -n320 speech03.al8"
        \speech03.320.lcm   : Output of "g711llc.exe -n320 speech03.mu8"
        \speech04.40.lca    : Output of "g711llc.exe -n40 speech04.al8"
        \speech04.40.lcm    : Output of "g711llc.exe -n40 speech04.mu8"
        \speech04.80.lca    : Output of "g711llc.exe -n80 speech04.al8"
        \speech04.80.lcm    : Output of "g711llc.exe -n80 speech04.mu8"
        \speech04.160.lca   : Output of "g711llc.exe -n160 speech04.al8"
        \speech04.160.lcm   : Output of "g711llc.exe -n160 speech04.mu8"
        \speech04.240.lca   : Output of "g711llc.exe -n240 speech04.al8"
        \speech04.240.lcm   : Output of "g711llc.exe -n240 speech04.mu8"
        \speech04.320.lca   : Output of "g711llc.exe -n320 speech04.al8"
        \speech04.320.lcm   : Output of "g711llc.exe -n320 speech04.mu8"
        \speech05.40.lca    : Output of "g711llc.exe -n40 speech05.al8"
        \speech05.40.lcm    : Output of "g711llc.exe -n40 speech05.mu8"
        \speech05.80.lca    : Output of "g711llc.exe -n80 speech05.al8"
        \speech05.80.lcm    : Output of "g711llc.exe -n80 speech05.mu8"
        \speech05.160.lca   : Output of "g711llc.exe -n160 speech05.al8"
        \speech05.160.lcm   : Output of "g711llc.exe -n160 speech05.mu8"
        \speech05.240.lca   : Output of "g711llc.exe -n240 speech05.al8"
        \speech05.240.lcm   : Output of "g711llc.exe -n240 speech05.mu8"
        \speech05.320.lca   : Output of "g711llc.exe -n320 speech05.al8"
        \speech05.320.lcm   : Output of "g711llc.exe -n320 speech05.mu8"
        \speech06.40.lca    : Output of "g711llc.exe -n40 speech06.al8"
        \speech06.40.lcm    : Output of "g711llc.exe -n40 speech06.mu8"
        \speech06.80.lca    : Output of "g711llc.exe -n80 speech06.al8"
        \speech06.80.lcm    : Output of "g711llc.exe -n80 speech06.mu8"
        \speech06.160.lca   : Output of "g711llc.exe -n160 speech06.al8"
        \speech06.160.lcm   : Output of "g711llc.exe -n160 speech06.mu8"
        \speech06.240.lca   : Output of "g711llc.exe -n240 speech06.al8"
        \speech06.240.lcm   : Output of "g711llc.exe -n240 speech06.mu8"
        \speech06.320.lca   : Output of "g711llc.exe -n320 speech06.al8"
        \speech06.320.lcm   : Output of "g711llc.exe -n320 speech06.mu8"

    \misc: other files used for the construction of test vectors (not used
           for the test itself)

tmp: folder for temporary files and results

ReadMe.txt: this file

runtests.bat: main test batch file

--- Last update: 2009-07-24