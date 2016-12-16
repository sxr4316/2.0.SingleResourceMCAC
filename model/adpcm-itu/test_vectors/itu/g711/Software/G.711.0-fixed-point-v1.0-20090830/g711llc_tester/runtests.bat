@REM %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@REM % ITU-T G.711.0 Fixed point code - Approved 2009-10-22                  %
@REM %                                                                       %
@REM % Filename: runtests.bat                                                %
@REM % Contents: G.711.0 test batch file                                     %
@REM % Version: 1.01                                                         %
@REM % Revision Date: July 10, 2009                                          %
@REM %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

@ECHO OFF

SET G711LLC=CuT\g711llc.exe
SET TMP=tmp
SET TEST_VECTORS=test_vectors

@REM **************************************************************************

SET REF_IN=%TEST_VECTORS%\ref_in
SET REF_OUT=%TEST_VECTORS%\ref_out
SET FAILURES=%TMP%\failures.txt
SET LASTLOG=%TMP%\lastlog.txt
SET ALL_PASSED=YES

@REM **************************************************************************

@REM Basic checks
IF NOT EXIST %G711LLC% (
    ECHO.
    ECHO.ERROR: %G711LLC% is missing!
    ECHO.
    ECHO.       Please copy the G.711-LLC Reference Software binary
    ECHO.       to %G711LLC% and run this batch file again.
    GOTO :EOF
)

IF NOT EXIST %REF_IN% (
    ECHO.ERROR: %REF_IN% folder is missing!
    GOTO :EOF
)

IF NOT EXIST %REF_OUT% (
    ECHO.ERROR: %REF_OUT% folder is missing!
    GOTO :EOF
)

@REM Make sure the TMP folder exists
MKDIR %TMP% > NUL 2>&1

@REM List of temp files to delete
SET DELETEME=

@REM Delete the list of failures
DEL /F /Q %FAILURES%

@REM **************************************************************************
@REM *** Encoder tests
@REM **************************************************************************
ECHO.*** Running G.711-LLC A-law single encoder tests ***
FOR %%N IN (40 80 160 240 320) DO (
    CALL :SingleEncodeTest %%N %REF_IN%\testvec_%%N.al8 %TMP%\testvec_%%N.lca %REF_OUT%\testvec_%%N.lca
)
ECHO.

ECHO.*** Running G.711-LLC mu-law single encoder tests ***
FOR %%N IN (40 80 160 240 320) DO (
    CALL :SingleEncodeTest %%N %REF_IN%\testvec_%%N.mu8 %TMP%\testvec_%%N.lcm %REF_OUT%\testvec_%%N.lcm
)
ECHO.

FOR %%X IN (testvec_all speech01 speech02 speech03 speech04 speech05 speech06) DO (
    ECHO.*** Running G.711-LLC A-law "%%X" encoder tests ***
    FOR %%N IN (40 80 160 240 320) DO (
        CALL :SingleEncodeTest %%N %REF_IN%\%%X.al8 %TMP%\%%X.%%N.lca %REF_OUT%\%%X.%%N.lca
    )
    ECHO.

    ECHO.*** Running G.711-LLC mu-law "%%X" encoder tests ***
    FOR %%N IN (40 80 160 240 320) DO (
        CALL :SingleEncodeTest %%N %REF_IN%\%%X.mu8 %TMP%\%%X.%%N.lcm %REF_OUT%\%%X.%%N.lcm
    )
    ECHO.
)

@REM **************************************************************************
@REM *** Decoder tests
@REM **************************************************************************
ECHO.*** Running G.711-LLC A-law "reverse single" decoder tests ***
FOR %%N IN (40 80 160 240 320) DO (
    CALL :SingleDecodeTest %REF_OUT%\testvec_%%N.lca %TMP%\testvec_%%N.alo %REF_IN%\testvec_%%N.al8
)
ECHO.

ECHO.*** Running G.711-LLC mu-law "reverse single" decoder tests ***
FOR %%N IN (40 80 160 240 320) DO (
    CALL :SingleDecodeTest %REF_OUT%\testvec_%%N.lcm %TMP%\testvec_%%N.muo %REF_IN%\testvec_%%N.mu8
)
ECHO.

FOR %%X IN (testvec_all speech01 speech02 speech03 speech04 speech05 speech06) DO (
    ECHO.*** Running G.711-LLC A-law "reverse %%X" decoder tests ***
    FOR %%N IN (40 80 160 240 320) DO (
        CALL :SingleDecodeTest %REF_OUT%\%%X.%%N.lca %TMP%\%%X.%%N.alo %REF_IN%\%%X.al8
    )
    ECHO.

    ECHO.*** Running G.711-LLC mu-law "reverse %%X" decoder tests ***
    FOR %%N IN (40 80 160 240 320) DO (
        CALL :SingleDecodeTest %REF_OUT%\%%X.%%N.lcm %TMP%\%%X.%%N.muo %REF_IN%\%%X.mu8
    )
    ECHO.
)

ECHO.*** Running G.711-LLC A-law "0" decoder tests ***
CALL :SingleDecodeTest %REF_IN%\testvec_0.lca %TMP%\testvec_0.alo %REF_OUT%\testvec_0.alo
ECHO.

ECHO.*** Running G.711-LLC mu-law "0" decoder tests ***
CALL :SingleDecodeTest %REF_IN%\testvec_0.lcm %TMP%\testvec_0.muo %REF_OUT%\testvec_0.muo
ECHO.

ECHO.*** Running G.711-LLC A-law "all" decoder tests ***
CALL :SingleDecodeTest %REF_IN%\testvec_all.lca %TMP%\testvec_all.alo %REF_OUT%\testvec_all.alo
ECHO.

ECHO.*** Running G.711-LLC mu-law "all" decoder tests ***
CALL :SingleDecodeTest %REF_IN%\testvec_all.lcm %TMP%\testvec_all.muo %REF_OUT%\testvec_all.muo
ECHO.

@REM **************************************************************************
@REM *** Clean up
@REM **************************************************************************

FOR %%F IN (%DELETEME%) DO (
    DEL /F /Q %%F > NUL 2>&1
)

IF %ALL_PASSED%==NO (
    ECHO.***********************************************
    ECHO.*          !!! THERE WERE FAILURES !!!        *
    ECHO.*                                             *
    ECHO.* List of failures:                           *
    ECHO.* %FAILURES%                            *
    ECHO.***********************************************
) ELSE (
    ECHO.***********************************************
    ECHO.***            ALL TESTS PASSED             ***
    ECHO.***********************************************
)

GOTO :EOF

@REM **************************************************************************
@REM *** Subroutines
@REM **************************************************************************

:SingleEncodeTest
@REM %1: Frame length
@REM %2: Reference Input
@REM %3: Generated Output
@REM %4: Reference Output
%G711LLC% -n%1 %2 %3 > %LASTLOG% 2>&1
IF ERRORLEVEL 1 (
    ECHO.!!! FAILURE FOR INPUT !!!: %2
    ECHO.ENCODER ERROR: Referece input: %2, Generated output: %3, Reference output: %4 >> %FAILURES%
    TYPE %LASTLOG% >> %FAILURES%
    ECHO.>> %FAILURES%
    SET ALL_PASSED=NO
) ELSE (
    FC /B %3 %4 > NUL 2>&1
    IF ERRORLEVEL 1 (
        ECHO.!!! FAILURE FOR INPUT !!!: %2
        ECHO.ENCODER MISMATCH: Referece input: %2, Generated output: %3, Reference output: %4 >> %FAILURES%
        TYPE %LASTLOG% >> %FAILURES%
        ECHO.>> %FAILURES%
        SET ALL_PASSED=NO
    ) ELSE (
        ECHO.Passed for input: %2
        SET DELETEME=%3 %DELETEME%
    )
)
DEL /F /Q %LASTLOG% > NUL 2>&1
GOTO :EOF

:SingleDecodeTest
@REM %1: Reference Input
@REM %2: Generated Output
@REM %3: Reference Output
%G711LLC% %1 %2 > %LASTLOG% 2>&1
IF ERRORLEVEL 1 (
    ECHO.!!! FAILURE FOR INPUT !!!: %1
    ECHO.DECODER ERROR: Referece input: %1, Generated output: %2, Reference output: %3 >> %FAILURES%
    TYPE %LASTLOG% >> %FAILURES%
    ECHO.>> %FAILURES%
    SET ALL_PASSED=NO
) ELSE (
    FC /B %2 %3 > NUL 2>&1
    IF ERRORLEVEL 1 (
        ECHO.!!! FAILURE FOR INPUT !!!: %1
        ECHO.DECODER MISMATCH: Referece input: %1, Generated output: %2, Reference output: %3 >> %FAILURES%
        TYPE %LASTLOG% >> %FAILURES%
        ECHO.>> %FAILURES%
        SET ALL_PASSED=NO
    ) ELSE (
        ECHO.Passed for input: %1
        SET DELETEME=%2 %DELETEME%
    )
)
DEL /F /Q %LASTLOG% > NUL 2>&1
GOTO :EOF
