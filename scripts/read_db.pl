#!/usr/bin/perl -w
#
######################################################################
#

use strict;
use DBI;
use Getopt::Long qw(:config auto_version);

my $exe = $0; # save just executable name
$exe =~ s!(/?([\w\.]+/)+)!!o; # remove leading path from scriptname
my $path = $1 ;
$exe =~ s/\s//g ;

$0 = "read_db.pl" ;

#
# external modules
#

my $debug = 0 ;              # set to one to print extra messages
my $help = 0 ;               # set to one to print usage message;

#
# get options
#
my $result = GetOptions (
    "debug"  => \$debug ,
    "help"  => \$help ,
);

if ($help) {
    print "${exe}\n\t[--help]\n\t[--debug]\n" ;
    exit ;
}

# Hash of modules with databases for testing purpose
my %testdatabases = (
	"APRSC"  => "APRSCTests",
	"ENCRTL" => "ENCRTLTests",
	"ENCNET" => "ENCNETTests",
	"DECRTL" => "DECRTLTests",
	"DECNET" => "DECNETTests",
);

# Enter test mode to be used
my $dbresponse;
my $input;
print "There are currently databases for: ";
foreach my $key (keys(%testdatabases)){
	print "$key ";
}
print "\n";
print "Which database do you want to read?: ";
$dbresponse = <>;
$input = substr($dbresponse,0,length($dbresponse)-1);

if(exists($testdatabases{$input})){
	print "\nPreparing to read database " . $testdatabases{$input} . "\n";
}
else {
	print "Invalid response. Exiting script.\n";
	exit;
}

my $toread = $testdatabases{$input};


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

#

my $os =  `uname -s` ;
chomp($os) ;
print "ok: found $os\n";

my $qh = $dbh->prepare("SELECT VERSION()");
$qh->execute;
my ($sql_vers) = $qh->fetchrow_array;
$qh->finish;

print "ok: found v$sql_vers\n";
print "Connected to database\n\n";

#read tests that have been done from table 
print "\nReading tests that have been done from database:\n";
my $code;
my $name;
my $starttime;
my $endtime;
my $TestResult;
my $num;
my $hash;
my $syntime;
$qh = $dbh->prepare("SELECT * FROM $toread");
$qh->execute;
while ( ($code,$name,$starttime,$endtime,$TestResult,$num,$hash,$syntime) = $qh->fetchrow_array() ) {
	print "$name did test $code starting at $starttime and it $TestResult.\n";
}
$qh->finish;

$dbh->disconnect if $dbh;
print "\nClosed database \n\n";
#
1;

