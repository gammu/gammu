.. _Discovering protocol:

Discovering protocol
====================

You need to get a communication dump to be able to understand protocol
or discover new commands. As most vendors provide some software for
Windows, all following sections assume you do the sniffing on Windows.

USB
---

For USB there exist various tools to dump USB communication. The dumps
can be later analyzed and used to discover protocol details or unknown
commands. One of the best free tools available currently is `UsbSnoop`_.

In directory ``contrib/usbsnoop`` in Gammu sources you can find some tools to
decode the output.

.. _UsbSnoop: http://www.pcausa.com/Utilities/UsbSnoop/

Serial port
-----------


Download `Portmon`_, which allows one to capture bytes sent and received by
ready binary software.

.. _Portmon: http://technet.microsoft.com/en-us/sysinternals/bb896644.aspx

If you have log saved by PortMon and protocol is the same to "old" Nokia
protocols, can use Gammu to decode it. It's simple::

    gammu --decodesniff MBUS2 file 6210 > log

saves in log decoded MBUS2 dump session. There is used phone module for
6210 and have you have debug info about 6210 specific frames (you don't have
to add model). Dump file for --decodesniff and MBUS should be specific:

1. without bytes sent to phone (in Portmon you set it here:
   "Edit", "Filter/Highlight")

2. in Hex format ("Options", "Show Hex")

3. without Date & Time ("Options", "Show Time" & "Clock Time")


Infrared
--------

First of all you need two computers with IrDA. One running linux, that will
sniff and one running windows, which will communicate with the phone and
whatever software you want (Nokia, Logomanager, Oxygen Phone Manager).
Then you have to get the software from
http://www.dev-thomynet.de/nokworld/noktrace/

You have to disable IrDA services on the linux machine and eventually you
have to change the default port the 'irda_intercept' program is sniffing
from (default ttyS1). On the windows machine you should decrease the
maximum transmission speed to 9600bps if possible, because the intercept
program doesn't seem to handle speed changes. (9600 is for searching
devices in range and then the highest possible speed is chosen) If it isn't
possible you have to change the default bitrate in intercept source code,
too. Then you won't see anything until the windows machine and the phone
start transmitting data, which isn't too bad. At least here in my setup I
could sniff the data coming from phone and sent to it in one go, like that::

    win    ------------------>   Nokia
  machine  <-----------------    phone
                  ^^
                  ||
                sniffing
                 device

You get a raw data file (.trc) from the intercept program, which you can
then decode to hex with the second program from the above mentioned page.
You should possibly be able to use Marcin's magnokii for decoding the trc
files, too, but it didn't work for me so I just figured things out from the
hex files. In the hex files you should look for primary frames with
00 01 00 in it, because this is the FBUS header which is in every valuable
frame sent to phone. It's not really joy to do that, but if it brings
support for a new phone it's worth it :-)
