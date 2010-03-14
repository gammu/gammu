The MGNETD (MyGnokiiNETDdaemon) program is intended 
to provide a network api for mygnokii. With MGNETD
running in the background and a small netapi,
you can use the features of mygnokii in your own programs.
(See mg_demo_client.c)
I plan a 'plugin' with mg_net_api Interface
for jpilot, to sync phonebook (and later calender)
between phone and palm.

I provide it for free use under GPL.
I provide the program AS IS without any warranty.

For MGNETD you need a running mygnokii installation !


To start MGNETD, chose a passwort and type:

	mgnet password

If all is OK, you see a small message:
	'init done, daemonizing'
and mgnet runs in the background and serv udp
requests from any client who knows the password.


By default MGNETD listen at udp Port 50963.
You can add a entry to your '/etc/services' file
to change this, eg. to listen port 21007 add this line:

mygnokii      21007/udp


--------------------------------------------

In Version 0.3 you could:
- read IMEI,REVISION,HW of connected phone
- read phonebookentrys
- write phonebookentrys
- delete phonebookentrys
- send sms messages

TODOS:
- implement timeout for client
- handle receive sms

Ralf Thelen <ralf@mythelen.de>
