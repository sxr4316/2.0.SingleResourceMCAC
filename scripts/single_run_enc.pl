#!/usr/bin/perl -w
#
######################################################################
#

use strict;
use DBI;
use Getopt::Long qw(:config auto_version);

my $exe = $0; # save just executable name
$exe =~ s!(/?([\w\.]+/)+)!!o; # remotve leading path from scriptname
my $path = $1 ;
$exe =~ s/\s//g ;

$0 = "single_run_enc.pl" ;

#
# external modules
#

my $debug = 0 ;              # set to one to print extra messages
my $help = 0 ;               # set to one to print usage message;

my $netlist = 0; #default to rtl simulation
my $t = "";
my $force = 0;
#
# get options
#
my $result = GetOptions (
    "debug"  => \$debug ,
    "help"  => \$help ,
    "n"      => \$netlist , 
    "t=s"      => \$t, "test=s"      => \$t,
    "f" => \$force, "force" => \$force
);

if ($help) {
    print "${exe}\n\t[--help]\n\t[--debug]\n\t[--n]\n" ;
    exit ;
}

if ($netlist) {
   print "Executing Netlist simulation\n";
}
else {
   print "Executing RTL simulation\n";
}


# Enter test mode to be used
my $test;
if ($t eq "") {
	print "Enter the test code (ex: ERNAA16): ";
	$test = <>;
} else {
	$test = $t;
}

# formatted like EROMM32 (system, test type, input type, law, rate)

# Gather info about the test
my $system = substr($test,0,1); # E or D (encoder or decoder)
my $testtype = substr($test,1,1); # R or H (reset or homing)
my $inputtype = substr($test,2,1); # N, O, or I (normal, overload, or I)
my $law = substr($test,3,2); # AA, MM, AM, or MA
my $rate = substr($test,5,2); # 16, 24, 32, or 40

# Print info about the test
print "\nTest info:\n";
if($system eq 'E'){
	print "Encoder test\n";
}
else{
	print "Invalid system in test code: not encoder. Exiting script\n";
	exit;
}
if($testtype eq 'R'){
	print "Reset test\n";
}
elsif($testtype eq 'H'){
	print "Homing test\n";
}
else{
	print "Invalid test type in test code: not reset or homing. Exiting script\n";
	exit;
}
if($inputtype eq 'N'){
	print "Normal input\n";
}
elsif($inputtype eq 'O'){
	print "Overload input\n";
}
#elsif($inputtype eq 'I'){
#	print "I input\n";
#}
else{
#	print "Invalid input in test code: not normal, overload, or I. Exiting script\n";
	print "Invalid input in test code: not normal or overload. Exiting script\n";
	exit;
}
if($law eq 'AA'){
	print "Law AA\n";
}
elsif($law eq 'MM'){
	print "Law MM\n";
}
elsif($law eq 'AM'){
	print "Law AM\n";
}
elsif($law eq 'MA'){
	print "LAW MA\n";
}
else{
	print "Invalid input in test code: not a valid law. Exiting script\n";
	exit;
}
if($rate eq '16'){
	print "Rate 16\n";
}
elsif($rate eq '24'){
	print "Rate 24\n";
}
elsif($rate eq '32'){
	print "Rate 32\n";
}
elsif($rate eq '40'){
	print "Rate 40\n";
}
else{
	print "Invalid input in test code: not a valid rate. Exiting script\n";
	exit;
}

# Create output log file for test (./enc_test_out/testname.out)
my $destfilepath = "./enc_test_out/";
mkdir $destfilepath unless -d $destfilepath;
my $testname = substr($test,0,7);
my $destfilename;
if($netlist){
	$destfilename = $testname . "NET.out";
}
else{
	$destfilename = $testname . "RTL.out";
}
my $outputpath = $destfilepath . $destfilename;
print "\nCreating output log file for test: \n" . $outputpath . "\n";
open (LOGFILE, ">", $outputpath) or die("Cannot open " . $outputpath . "\n");
print "-----------------------------------------------------------------------------\n";
print LOGFILE "Running test " . $testname . "\n";
my $start = localtime();
print LOGFILE "Started test at local date and time $start\n";


# The SQL database:
#
my $db_base = 'mysql';

#
# How to access the SQL database:
#
my $mysql_db_host = "mysqldb.indovina.us";         # where is the database?

#
# How to access the SQL database:
#
my $mysql_db_port = 3306;                # which port to use

#
# How to access the SQL database:
#
my $mysql_db_name = "eeee720_s2016_sr";          # name of the MySQL database

#
# Remove (limited to modify/alter ) access to the SQL database:
#
my $mysql_root_db_user = "eeee720_s2016_sr";  # user to attach to the MySQL database

#
# Some people actually use passwords with their MySQL database ...
#
my $mysql_root_db_pass = "cf331E54a66d888340d881a036b8359a";


# get a handle to the low-level DBD driver
my $drh = DBI->install_driver($db_base)
    or die "Can't connect to the $db_base. Is the database installed and up and running?\n";

my $dsn = "dbi:$db_base:$mysql_db_name:host=$mysql_db_host:port=$mysql_db_port";
my $dbh = DBI->connect($dsn, $mysql_root_db_user, $mysql_root_db_pass)
    or die "Can't connect to the datebase '$dsn'.\n";

my $os =  `uname -s` ;
chomp($os) ;
print "ok: found $os\n";

my $qh = $dbh->prepare("SELECT VERSION()");
$qh->execute;
my ($sql_vers) = $qh->fetchrow_array;
$qh->finish;

print "ok: found v$sql_vers\n";
print "Connected to database\n\n";


# Read database for what team member is assigned to the test and where the email will be sent
print "Reading info on team member assigned to this test:\n";
my ($assignedcode, $assignedsystem, $assignedtest, $assignedinput, $assignedlaw, $assignedrate);
my ($assignednum, $assignedtester, $assignedemail);

$qh = $dbh->prepare("SELECT * FROM APRSCTesters");
$qh->execute;
while ( ($assignedcode,$assignedsystem,$assignedtest,$assignedinput,$assignedlaw, $assignedrate, $assignednum, $assignedtester, $assignedemail) = $qh->fetchrow_array() ) {
	if($testname eq $assignedcode){
		print "This module is assigned to " . $assignedtester . ". An email will be sent to him/her at: " . $assignedemail . "\@rit.edu\n";
		print LOGFILE "This module is assigned to " . $assignedtester . ". An email will be sent to him/her at: " . $assignedemail . "\@rit.edu\n";
		last; # exit while loop once find test in database
	}
}

$qh->finish;

# grab username of current user (ex: ads3385 for Adam)
my $currentusername = getpwuid( $< );
my $currenttester;
# look up name of current user based on their username (start of RIT email)
my ($currentcode, $currentsystem, $currenttest, $currentinput, $currentlaw, $currentrate);
my ($currentnum, $currentname, $currentemail);
$qh = $dbh->prepare("SELECT * FROM APRSCTesters");# WHERE EMAIL = $currentusername");
$qh->execute;
while ( ($currentcode,$currentsystem,$currenttest,$currentinput,$currentlaw,$currentrate, $currentnum,
$currentname, $currentemail) = $qh->fetchrow_array() ) {
	if($currentemail eq $currentusername){
		$currenttester = $currentname;
		last; # exit with current name
	}
}
$qh->finish;

print "\nCurrent tester: $currenttester \(username: $currentusername\)\n";
print LOGFILE "\nCurrent tester: $currenttester \(username: $currentusername\)\n";

# Disconnect from database
$dbh->disconnect if $dbh;
print "\nClosed database \n\n";

my $continue;
my $choice;
if($netlist){
	print "\nMake sure that you have synthesized the most recent version of the code. \n";
}

if ($force) {
	$choice = 'y';
} else {
	print "Do you want to continue and start the test (y/n)?: ";
	$continue = <>;
	$choice = substr($continue,0,1);
}

if($choice eq 'y'){
	print "-----------------------------------------------------------------------------\n";
	print "Starting simulation. Do not exit out of this window.\n";
}
elsif($choice eq 'n'){
	print "Exiting script.\n";
	exit;
}
else{
	print "Invalid response. Exiting script.\n";
	exit;
}

# Grab some info on the local repository
my $currentgithash = `git rev-parse HEAD`;
my $currentsyntime;
if($netlist){
	$currentsyntime = `date -r ../enc/dc_shell_cmp_scan.log +"%D_%R:%S"`;
}
else{
	$currentsyntime = "";
}
print LOGFILE "\n-----------------------------------------------------------------------------\n";
print LOGFILE "Current git version: $currentgithash \n";
if($netlist){
	print LOGFILE "Last time synthesis was ran: $currentsyntime \n";
}

chdir('../') or die "$!"; # create temporary directory at same level as ENC
# create temporary directory to run simulations in
my $testfoldername;
if($netlist){
	$testfoldername = "enc" . $testname . "NET";
}
else{
	$testfoldername = "enc" . $testname . "RTL";
}
my $testfolder = "./$testfoldername/";
mkdir $testfolder unless -d $testfolder;
chdir $testfolder or die "$!";
system("ln -s ../enc/sim.csh");
system("ln -s ../enc/etc");
system("ln -s ../enc/src");
system("ln -s ../enc/netlist");
system("ln -s ../enc/sdf");

#print "\n Creating output file in current temporary directory for test bench to read to determine the test.\n";
my $testfilename = "ENCTests.t";
open (TESTFILE, ">", $testfilename) or die("Cannot open " . $testfilename . "\n");

if($system eq 'E'){
	print TESTFILE "\@0000\t000000\n";
}
else{ #D
	print TESTFILE "\@0000\t000001\n";
}
if($testtype eq 'R'){
	print TESTFILE "\@0001\t000000\n";
}
else{ #H
	print TESTFILE "\@0001\t000001\n";
}
if($inputtype eq 'N'){
	print TESTFILE "\@0002\t000000\n";
}
elsif($inputtype eq 'O'){
	print TESTFILE "\@0002\t000001\n";
}
else{  #I
	print TESTFILE "\@0002\t000002\n";
}
if($law eq 'AA'){
	print TESTFILE "\@0003\t000000\n";
}
elsif($law eq 'MM'){
	print TESTFILE "\@0003\t000001\n";
}
elsif($law eq 'AM'){
	print TESTFILE "\@0003\t000002\n";
}
else{ #MA
	print TESTFILE "\@0003\t000003\n";
}
if($rate eq '16'){
	print TESTFILE "\@0004\t000003\n";
}
elsif($rate eq '24'){
	print TESTFILE "\@0004\t000002\n";
}
elsif($rate eq '32'){
	print TESTFILE "\@0004\t000001\n";
}
else{ #40
	print TESTFILE "\@0004\t000000\n";
}
# close test file
close(TESTFILE);

my $netlistlogname;
my $rtllogname;
if($netlist){
	# call sim script to run normal test bench for ENC. Eventually want RTL and Netlist
	#my $netlistlogname = $testname . "netlist.out";
	$netlistlogname = "ncverilog.log";
	print LOGFILE "\n-----------------------------------------------------------------------------\n";
	print LOGFILE "Starting netlist simulation.\n\n";
	print "\nStarting netlist simulation. If cannot remove warnings show up they are fine; it just means other people are simulating at the same time.\n";
	my $simoutput = system("./sim.csh -n -v -ng -run >$netlistlogname");
	print "\nNetlist simulation script has completed.\n\n";
	print LOGFILE "\nNetlist simulation script has completed.\n\n";
}
else{
	#my $rtllogname = $testname . "rtl.out";
	$rtllogname = "ncverilog.log";
	print LOGFILE "Starting RTL simulation.\n\n";
	print "\nStarting RTL simulation. If cannot remove warnings show up they are fine; it just means other people are simulating at the same time.\n\n";
	my $simoutput2 = system("./sim.csh -r -v -ng -run >/dev/null"); # /dev/null on computer is black hole
	print "\nRTL simulation script has completed.\n\n";
	print LOGFILE "\nRTL simulation script has completed.\n\n";
}
# once test is done, result is printed and put in log file and info is written to database

print "\n-----------------------------------------------------------------------------\n";
print LOGFILE "\n-----------------------------------------------------------------------------\n";

my $emailbody = "Test " . $testname . " finished. ";

my $emailbody1;
my $emailbody2;
my $netlisterror;
my $rtlerror;
if($netlist){
	# check output file to make sure that no errors were in the test bench for Netlist simulation
	open (NETLOGFILE, "< $netlistlogname") or die "Cannot open $netlistlogname\n";
	my $line;
	$netlisterror = 1;
	while ($line = <NETLOGFILE>){ # loop through lines of file
		if($line =~ /TESTBENCHERROR/){
			$netlisterror = 1;
			last; # no point continuing to loop if found error
		}
		elsif($line =~ /readmem/){
			$netlisterror = 1;
			last;
		}
		elsif($line =~ /TESTBENCHFINISHED/){
			$netlisterror = 0;
			last;
		}
	}
	if($netlisterror == 1){
		print "\nFound error in netlist simulation. Check the logs.\n";
		print LOGFILE "\nFound error in netlist simulation. Check the logs.\n";
		$emailbody1 = $emailbody . "Netlist simulation failed. ";
	}
	else{
		print "\nNetlist simulation passed.\n";
		print LOGFILE "\nNetlist simulation passed.\n";
		$emailbody1 = $emailbody . "Netlist simulation passed. ";
	}
	close(NETLOGFILE);
	$emailbody2 = $emailbody1 . "";
}
else{
	$emailbody1 = "";
	# check output file to make sure that no errors were in the test bench for RTL simulation
	open (RTLLOGFILE, "< $rtllogname") or die "Cannot open $rtllogname\n";
	my $line2;
	$rtlerror = 1;
	while ($line2 = <RTLLOGFILE>){ # loop through lines of file
		if($line2 =~ /TESTBENCHERROR/){
			$rtlerror = 1;
			last; # no point continuing to loop if found error
		}
		elsif($line2 =~ /readmem/){
			$rtlerror = 1;
			last;
		}
		elsif($line2 =~ /TESTBENCHFINISHED/){
			$rtlerror = 0;
			last;
		}
	}

	if($rtlerror == 1){
		print "\nFound error in rtl simulation. Check the logs.\n";
		print LOGFILE "\nFound error in rtl simulation. Check the logs.\n";
		$emailbody2 = $emailbody1 . "RTL simulation failed. ";
	}
	else{
		print "\nRTL simulation passed.\n\n";
		print LOGFILE "\nRTL simulation passed.\n\n";
		$emailbody2 = $emailbody1 . "RTL simulation passed. ";
	}
	close(RTLLOGFILE);
}

# change back to scripts directory
chdir('../scripts/') or die "$!";

# get end time
my $end = localtime();
print LOGFILE "\nEnded test at local date and time $end\n";

my $emailbody3;
$emailbody3 = $emailbody2 . "Check log file " . $outputpath . " for more information. " . "Test was performed by " . $currenttester . " starting at " . $start . " and ending at " . $end . ".";

#re-connect to database
# get a handle to the low-level DBD driver
$drh = DBI->install_driver($db_base)
    or die "Can't connect to the $db_base. Is the database installed and up and running?\n";

$dsn = "dbi:$db_base:$mysql_db_name:host=$mysql_db_host:port=$mysql_db_port";
$dbh = DBI->connect($dsn, $mysql_root_db_user, $mysql_root_db_pass)
    or die "Can't connect to the datebase '$dsn'.\n";

$os =  `uname -s` ;
chomp($os) ;
print "ok: found $os\n";

$qh = $dbh->prepare("SELECT VERSION()");
$qh->execute;
($sql_vers) = $qh->fetchrow_array;
$qh->finish;

print "ok: found v$sql_vers\n";
print "Connected to database\n\n";

#add results to test database
if($netlist){
	print "\n-----------------------------------------------------------------------------\n";
	print "\nWriting results to ENCNETTests database\n";
	if($netlisterror == 1){
		$qh = $dbh->prepare("INSERT INTO ENCNETTests VALUES ('$testname', '$currenttester', '$start', '$end', 'failed', '$assignednum', '$currentgithash', '$currentsyntime');");
		$qh->execute;
	}
	else{ # passed
		$qh = $dbh->prepare("INSERT INTO ENCNETTests VALUES ('$testname', '$currenttester', '$start', '$end', 'passed', '$assignednum', '$currentgithash', '$currentsyntime');");
		$qh->execute;
	}
}
else{
	print "\n-----------------------------------------------------------------------------\n";
	print "\nWriting results to ENCRTLTests database\n";
	if($rtlerror == 1){
		$qh = $dbh->prepare("INSERT INTO ENCRTLTests VALUES ('$testname', '$currenttester', '$start', '$end', 'failed', '$assignednum', '$currentgithash');");#, '$currentsyntime');");
		$qh->execute;
	}
	else{ # passed
		$qh = $dbh->prepare("INSERT INTO ENCRTLTests VALUES ('$testname', '$currenttester', '$start', '$end', 'passed', '$assignednum', '$currentgithash');");#, '$currentsyntime');");
		$qh->execute;
	}
}

#read tests that have been done from table 
print "\n-----------------------------------------------------------------------------\n";
print LOGFILE "\n-----------------------------------------------------------------------------\n";
print "\nReading tests that have been done from database:\n";
print LOGFILE "\n\nReading tests that have been done from database:\n";
my $code;
my $name;
my $starttime;
my $endtime;
my $TestResult;
my $num;
my $hash;
my $syntime;

if($netlist){
	$qh = $dbh->prepare("SELECT * FROM ENCNETTests");
	$qh->execute;
	while ( ($code,$name,$starttime,$endtime,$TestResult,$num,$hash,$syntime) = $qh->fetchrow_array() ) {
		print "$name did test $code starting at $starttime and it $TestResult.\n";
		print LOGFILE "$name did test $code starting at $starttime and it $TestResult.\n";
	}
	$qh->finish;
}
else{
	$qh = $dbh->prepare("SELECT * FROM ENCRTLTests");
	$qh->execute;
	while ( ($code,$name,$starttime,$endtime,$TestResult,$num,$hash) = $qh->fetchrow_array() ) {
		print "$name did test $code starting at $starttime and it $TestResult.\n";
		print LOGFILE "$name did test $code starting at $starttime and it $TestResult.\n";
	}
	$qh->finish;
}

$dbh->disconnect if $dbh;
print "\nClosed database \n\n";

if($netlist){
	print "\n-----------------------------------------------------------------------------\n";
	print LOGFILE "\n-----------------------------------------------------------------------------\n";
	print LOGFILE "NETLIST SIMULATION LOG CONTENTS:\n\n";

	# add sim log file content here
	open (NETLOGFILE2, "< ../$testfolder/$netlistlogname") or die "Cannot open $netlistlogname\n";
	my $line3;
	while ($line3 = <NETLOGFILE2>){ # loop through lines of file
		print LOGFILE $line3 . "\n"; # print line to log file
	}
	close(NETLOGFILE2);
}
else{
	print LOGFILE "\n-----------------------------------------------------------------------------\n";
	print LOGFILE "RTL SIMULATION LOG CONTENTS:\n\n";
	open (RTLLOGFILE2, "< ../$testfolder/$rtllogname") or die "Cannot open $rtllogname\n";
	my $line4;
	while ($line4 = <RTLLOGFILE2>){ # loop through lines of file
		print LOGFILE $line4 . "\n"; # print line to log file
	}
	close(RTLLOGFILE2);
}
print LOGFILE "\n-----------------------------------------------------------------------------\n";


# go back to top level and delete the temporary directory
chdir('../') or die "$!";
system("rm -fr ./$testfolder/");


# change back to scripts directory
chdir('./scripts/') or die "$!";

# close log file
close(LOGFILE);
print "\nClosed log file " . $outputpath . "\n";

# when test complete, email sent to tester
# Call SABRE.py -a address -b body -s subject -f
# now add -l /pathtofile/filenamewithextension to add an attachment
my $address = $assignedemail . "\@rit.edu";
my $subject = "Automated ENC Test Finished: Test " . $testname;
print "\n-----------------------------------------------------------------------------\n";
print "\nSending email to " . $address . "\n";
my $emailscript = system("./SABRE.py -a $address -b $emailbody3 -s $subject -l $outputpath -f");
# email now sent with an attachment of the log file

#
1;

