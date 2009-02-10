#!/usr/bin/perl
# GetRecentPhoneFiles v2.0
# 
# This Perl script downloads recent files from a phone using GAMMU.
# USAGE: GetRecentPhoneFiles FOLDERNAME DAYS_OLD_AS_NUMBER
# USAGE: GetRecentPhoneFiles Photo 14 # from folder Photos up to two weeks old
# USAGE: GetRecentPhoneFiles ".*" 365 # from all folders up to one year old
#
# This script changes into ./PhoneTmp directory, and calls GAMMU.
# Move this script to a correct directory and start it from there.
#
# Copyright Egor Kobylkin 2004 www.kobylkin.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

$dumpfile = "./PhoneTmp/PhoneFilesystem.txt";
@dumpfilecreator = "mkdir ./PhoneTmp; gammu --getfilesystem -flatall >./PhoneTmp/PhoneFilesystem.txt";

# @record array structure
$r_ID = 0;
$r_Type = 1; # never used here explicitly
$r_Folder = 2;
$r_File = 3;
$r_Date = 4;

use Time::Local;
use Date::Parse;
use POSIX qw(strftime);

$ok = (open DUMPFILE, "<$dumpfile");
if (! $ok) { 
	system (@dumpfilecreator);
	open DUMPFILE, "<$dumpfile" or die "could not open $dumpfile, could not call @dumpfilecreator, exiting!";
}

while(<DUMPFILE>) {
	chomp ; # strip newlines from $_
	if ( /File;\"@ARGV[0]\"/) { # file is in the specified folder
		my @record = split (";",$_);
		@record[$r_Date]=~s/\"//g;
		@record[$r_Date] = str2time(strftime ("%a %b %e %H:%M:%S %Y", localtime(str2time(@record[$r_Date]))));
		if ( @record[$r_Date] > time - 86400*@ARGV[1] ) { # file is no older than @ARGV[1] days
			push @files, @record[$r_ID];
#----------------------------------
#      print $_,"\n";
#      print "will fetch file ",@record[$r_File]," with id=",@record[$r_ID]," from folder ",@record[$r_Folder];
#      print " dated as ",@record[$r_Date],"\n";
#----------------------------------      
		}
	}
} 
@gammucmd = ( "gammu", "--getfiles");
push @gammucmd, join(", ", @files);
$gammustmt = join(" ", @gammucmd);
#---------------------------------
#print $gammustmt,"\n";
#---------------------------------
close DUMPFILE;
chdir "./PhoneTmp" or die "can not change to directory ./PhoneTmp, exiting";
exec $gammustmt;

# How should editor hadle tabs in this file? Add editor commands here.
# vim: noexpandtab sw=4 ts=4 sts=4:
