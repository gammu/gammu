#!/usr/bin/perl
#
# Author:       Peter Nixon <codemonkey@peternixon.net>
# Date:         July 2003
# Summary:      Command Line SMS queuer for Gammu SMSD
# Copy Policy:  GNU Public Licence Version 2 or later
# URL:          http://www.peternixon.net/code/
# Supported:    Gammu
# Copyright:    2003 Peter Nixon <codemonkey@petenixon.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# $Id: queuesms.pl,v 1.3 2003/11/04 15:04:40 peter Exp $
#


# Modules that we use to get things done.
require Getopt::Long;

## Program and File locations
# 
$SMSSPOOLDIR = "/var/spool/sms/outbox/";


#### You should not have to modify anything below here
$progname = "Command Line SMS queuer";
$version = "1.2";

# Set up some basic variables
$message; $verbose = 0; $priority = "C";


# The filename should be one of the following formats
#
# OUT<phone number>.txt
# OUT<priority>_<phone number>_<serialno>.txt
# OUT<priority><date>_<time>_<serialno>_<phone number>_<anything>.txt
# where
# <priority> is an alphabetic character (A-Z) A = highest priority
# Other fields as above.
# SMSes will be transmitted sequentially based on the file name
sub write_sms {
	my $number = $_[0]; my $process = $$;
	my $filename = "$SMSSPOOLDIR" . "OUT" . "$priority" . "_" . "$number" . "_" . "$process" . ".txt";

	if ($verbose > 0) { print "Message: $message\n"; }
	if ($verbose > 0) { print "Filename: $filename\n"; }

	open(SMS, ">$filename") or die "ERROR: Oops thats torn it! I Cant open $filename: $!\n";
	print SMS "$message\n";
        close(SMS);
	if ($verbose > 0) { print "Message successfully queued...\n"; }
}


sub print_usage_info {
	print "\n";
	$leader = "$progname $version Usage Information";
	$underbar = $leader;
	$underbar =~ s/./-/g;
	print "$leader\n$underbar\n";
	print "\n";
	print "  Syntax:   queuesms.pl [ options ] numbers(s)\n";
	print "\n";
	print "    -h --help                        Show this usage information\n";
	print "    -v --verbose                     Turn on verbose\n";
	print "    -x --debug                       Turn on debugging\n";
	print "    -V --version                     Show version and copyright\n";
	print "    -m --message                     SMS message to send\n";
	print "\n";
}

sub main {
        # Parse the command line for options
        if (!scalar(@ARGV)) {
        	&print_usage_info();
		exit(SUCCESS);
	};

	# See the Getopt::Long man page for details on the syntax of this line
	@valid_opts = ("h|help", "V|version", "m|message=s", "x|debug", "v|verbose+" => \$verbose, "q|quiet+" => \$quiet);
	Getopt::Long::Configure("no_getopt_compat", "bundling", "no_ignore_case");
	Getopt::Long::GetOptions(@valid_opts);

	# Post-parse the options stuff
	select STDOUT; $| = 1;
	if ($opt_V) {
		# Do not edit this variable.  It is updated automatically by CVS when you commit
		my $rcs_info = 'CVS Revision $Revision: 1.3 $ created on $Date: 2003/11/04 15:04:40 $ by $Author: peter $ ';

		$rcs_info =~ s/\$\s*Revision: (\S+) \$/$1/;
		$rcs_info =~ s/\$\s*Date: (\S+) (\S+) \$/$1 at $2/;
		$rcs_info =~ s/\$\s*Author: (\S+) \$ /$1/;

		print "\n";
		print "$progname Version $version by Peter Nixon <codemonkey\@peternixon.net>\n";
		print "Copyright (c) 2003 Peter Nixon\n";
		print "  ($rcs_info)\n";
		print "\n";
		return SUCCESS;
	} elsif ($opt_h) {
	        &print_usage_info();
	        exit(SUCCESS);
	}

	if ($opt_x) { 
		print "DEBUG: Debug mode is enabled.\n"; 
		$verbose = 2;
	} elsif ($quiet) { $verbose -= $quiet; }

	if ($opt_m) { 
		$message = $opt_m;
	} else {
		print "ERROR: Please specify a message\n";
		exit(FAILURE);
	}

	if (@ARGV) {
        	# Loop through the defined numbers
	        foreach $number (@ARGV) {
			&write_sms($number);
	        }

	} else {
		print "ERROR: Please specify one or more mobile numbers to send the message to.\n";
		exit(FAILURE);
	}

}


exit &main();
