#!/usr/bin/perl

# Nokia To Android SMS Converter
# Copyright (C) 2009 Thilo-Alexander Ginkel <thilo@ginkel.com>
#
# For instructions, refer to:
# http://blog.ginkel.com/2009/12/transferring-sms-from-nokia-to-android/
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use Date::Parse;
use HTML::Entities;

print <<HEADER;
<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<smses>
HEADER

while (<>) {
	chomp;
	s/^(.*)\r$/$1/;

	if (/^Sent = (.*)/) {
		$time = str2time($1);
	} elsif (/^Number = "(.*)"/) {
		$address = $1;
	} elsif (/^UDH = 050003(.*)(\p{XDigit}{2})/) {
		$msgid = $1;
		$seqno = $2;
	} elsif (/^State = (.*)/) {
		if ($1 eq "Sent") {
			$type = 2;
		} elsif ($1 eq "Read") {
			$type = 1;
		}
	} elsif (/^Text\d+ = (.*)/) {
		my $encoded = $1;
		$encoded =~ s/([0-9A-F]{4})/&utf8(hex($1))/gei;
		$text .= $encoded;
		# print $text, "\n";
	} elsif (/^$/ and defined($address)) {
		unless ($msgid) {
			print_message($address, $time, $type, $text);
		} else {
			my $msg = {};
			my $parts = {};

			$id = $address . $msgid;

			if (defined($multipart{$id})) {
				$msg = $multipart{$id};
			} else {
				$msg->{'date'} = $time;
				$msg->{'address'} = $address;
				$msg->{'type'} = $type;
			}

			if (defined($msg->{'parts'})) {
				$parts = $msg->{'parts'};
			}

			$parts->{$seqno} = $text;

			$msg->{'parts'} = $parts;
			$multipart{$id} = $msg;
		}

		$time = undef;
		$address = undef;
		$msgid = undef;
		$seqno = undef;
		$type = undef;
		$text = undef;
	}
}

# output any multipart messages
foreach $msg (values %multipart) {
	my $body;
	my $parts = $msg->{'parts'};
	foreach $seqno (sort keys %$parts) {
		$body .= $parts->{$seqno};
	}
	print_message($msg->{'address'}, $msg->{'date'}, $msg->{'type'}, $body);
}

print "</smses>\n";

sub utf8 { local($_)=@_;
    return $_ < 0x80 ? chr($_) : 
	$_ < 0x800 ? chr($_>>6&0x3F|0xC0) . chr($_&0x3F|0x80) :
	    chr($_>>12&0x0F|0xE0).chr($_>>6&0x3F|0x80).chr($_&0x3F|0x80);
}

sub print_message {
	my ($address, $date, $type, $text) = @_;
	print "<sms protocol=\"0\" address=\"" . encode_entities($address, "<>&'\"") . "\" date=\"${date}000\" type=\"$type\" subject=\"null\" body=\"" . encode_entities($text, "<>&'\"") . "\" toa=\"null\" sc_toa=\"null\" service_center=\"\" read=\"1\" status=\"-1\" />\n";
} 
