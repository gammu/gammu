Gammu README version 10.10.2003
-------------------------------
Q. What is this ?

A. This is package with different tools and drivers for Nokia and other mobile
   phones released under GNU GPL/LGPL license (see /copying file).

   It's created by Marcin Wiacek and other people and based on some experiences
   from Gnokii (www.gnokii.org) and MyGnokii (www.mwiacek.com) projects.

   Gammu (GNU All Mobile Management Utilities) was earlier (up to version 0.58)
   called MyGnokii2.

   NOTE: Name Gammu is not connected with Gammu from "Heretics of Dune" written
         by Frank Herbert
-------------------------------------------------------------------------------
Q. Can I use Gammu (Gammu dll or library) in own (commercial) applications ?

A. Gammu is currently licensed under GNU GPL license for normal users. It
   means, that each application using it have to be published with source.
   So, when you link Gammu source in own, you should inform me and publish
   it with source. When use Gammu dll or library, you also HAVE to publish
   your source.

   In the future probably it will be possible to get LGPL for Gammu or next
   project from Marcin Wiacek called Gammu+.

   NOTE: If you want to use Gammu in own applications, it will be fair to give
         some gift like small sum of money, old phone, etc. to author(s). It
         will be used for improving this software.
-------------------------------------------------------------------------------
Q. How can I help in writing this software ?

A. HELP IS VERY REQUIRED. Methods are many:

   1. writing support for some models or fixing source (I can give tips many
      times). Format and method of creating patch files is described in develop
      docs.
   2. reporting bugs (see "How to report bugs ?" question)
   3. help in documentation or translations
   4. help in discovering phone protocols
   5. suggesting some changes to better :-)
   6. giving some software (specs, tools, etc.) or hardware help

   etc.
-------------------------------------------------------------------------------
Q. What are things ToDo/planned ?

A. * these things can be done without anything (it will be done everything
     to make them):
     * MMS
     * giving support for more formats
     * writing really usefull GUI
     * ending win32 dll
     * uploading some things over WAP
     * AT modem emulator
     * DKU-5 driver
     * Symbian support
     * SyncML
     * .... (other)
   * this things can be done after getting some hardware help:
     * Nokia:
       * N3410 Java support
       * N5510 radio/MP3 support
       * for Series 40 getting/setting/sending PustToTalk settings,
         support for some types of files and MMC (3300, 6230, etc.), 
	 probably some other things connected with OMA, shutdown bitmap
     * Other:
       * more EMS features: polyphonic ringtones, more text features, etc.
       * all Siemens features
       * all Motorola features
     * .... (other)
-------------------------------------------------------------------------------
Q. Where to get latest versions and support ?

A. * Latest versions are on www.mwiacek.com (win32 binaries, gzip sources,
     ChangeLog)
   * Michal Cihar prepared mirror (win32 binaries, gzip sources, bzip2 sources,
     md5 checksums, ChangeLog) under www.cihar.com/gammu/zips/
   * ready Debian packages (thanks to Michal Cihar): http://cihar.com/debian/
   * ready RPM packages (thanks to Troy Engel):
     ftp://ftp.falsehope.com/home/tengel/gammu/
     and mirrors updated during 24 hours:
     ftp://rpmfind.net/linux/falsehope/home/tengel/gammu/
     ftp://ftp.pbone.net/mirror/ftp.falsehope.net/home/tengel/gammu/

   * For support contact Marcin Wiacek (currently marcin@mwiacek.com,
     DO NOT USE marcin-wiacek@topnet.pl).
   * You can also subscribe mailing list (see www.mwiacek.com and subpage
     about Gammu) and get help there. List is used also for annoucements about
     new versions. Archive of list (starting 28 Sept 2002) is under
     http://news.gmane.org/thread.php?group=gmane.linux.drivers.gammu

   TIP: If you want to have only general info about new versions, you can
        subscribe http://freshmeat.net/projects/gammu/ annoucements.

   UPDATE: there are now some additional projects based on Gammu:
   * Wammu: graphic manager using Python
         http://freshmeat.net/projects/wammu/
   * Snofs: software to mount phone filesystem under some Linux directory
            (something like Phone Browser in PC Suite)
         http://snofs.sourceforge.net/
-------------------------------------------------------------------------------
Q. How to compile it ?

A. Method 1:
     * under Unix (Linux)
         first do "./configure" and later use "make" or "make makelib"

         After it can use "make install" or "make installshared" to install it
         in your OS.
   Method 2:
     * under Linux based on RPM
         first do "./configure" and then "make makerpm"
         resulting rpm package could be installed via rpm --install <package>
	     or
         run rpmbuild -ta gammu-x.y.z.tar.gz
   Method 3:
     * under Linux based on Debian packages
         Just running make deb should do the job. Note you will need at least
         fakeroot, debhelper, libc6-dev and autoconf installed.
   Method 4:
     * change parameters in /cfg/config.h
     * compile it:
       * under MS Visual C++ 6.0 for MS Windows
           enter into /gammu/msvc6.mak directory and run gammu.dsw
       * under MS Visual C++ 7.0 (.NET) for MS Windows
           enter into /gammu/msvc7.mak directory and run gammu.vcproj
       * under MS Visual C++ Toolkit 2003
           not tested yet. It should be similiar to MS VC .NET
       * under MS Visual C++ Express
           not tested yet. It should be similiar to MS VC .NET        
       * under Unix (Linux)
           change compiler in /cfg/makefile.cfg (if required) and use "make" or
           "make makelib"

           After it can use "make install" or "make installlib" to install it
           in your OS.
       * under Borland C++ 3.1 for MS-DOS
           I tried, but I can't do it :-(. Help required.
       * under DJGPP for MS-DOS
           * set CFLAGS and LDLIBS in cfg/Makefile.glo
           * simply use "make djgpp" :-)
       * under CygWin for Windows
           I didn't try. Probably should work
       * under Borland C++ Builder 5 Trial
       * under Borland C++ Builder 6 Personal
           go into gammu/bcc.mak and check if there is subdirectory "output"
           (if not, create it). Then run gammu.bpr
	   
           TIP: gammu.bpr was created using "Tools/Visual C++ Project Conve.."
                and little modified later.
       * under Borland C++ command line tools
           go into gammu/bcc and check if there is subdirectory "output"
           (if not, create it). Then make "make -fgammu.mak" from command
           line.
	   
           TIP: gammu.mak was created in Borland C++ Builder using
                "Project/Export makefiles" from main menu.
       * under Borland C++ X - you need to create new makefile.
         It was added to some older versions too.
-------------------------------------------------------------------------------
Q. Where to get compilers and libraries to make Gammu ?

A. compilers
   * MS Visual C++ 6.0
   * MS Visual C++ 7.0 (.NET)
         go to the shop ;-). It's commercional software.
   * MS Visual C++ Toolkit 2003
         http://msdn.microsoft.com/visualc/vctoolkit2003/
   * MS Visual C++ Express
         search on microsoft.com
   * compilers for Unix (Linux) like gcc or kgcc (for free)
         available in Net and in Linux distributions included into CD
         for many newspapers. Installation depends on distribution.
   * DJGPP (for free)
         see http://www.midpec.com/djgpp/ (you have many info there)

         OR

         1. go directly into http://www.simtel.net
         2. find section for DJGPP
         3. download packages
              v2/readme.1st       - Instructions for DJGPP setup 
              v2/faq230b.zip      - The DJGPP Frequently Asked Questions 

              v2/djdev203.zip     - The DJGPP Development kit 
              v2misc/csdpmi4b.zip - The DPMI Server 
              v2gnu/bnu210b.zip   - The GNU Binary Utilities 
              v2gnu/gcc2952b.zip  - The GNU C Compiler 
              v2gnu/mak279b.zip   - The GNU Make Utility 
            or newer
         4. unzip to one directory (let's say C:\DJGPP)
         5. add to autoexec.bat these lines:
              PATH C:\DJGPP\BIN;%PATH%
              SET DJGPP=C:\DJGPP\DJGPP.ENV
   * Borland C++ Builder 5/6
         Personal version is given for free (see www.borland.com). You can
         download it from Borland, take from CD added to many computer
         magazines or use Trial versions also put into many magazines CD.
   * Borland C++ compiler (for free)
         See www.borland.com. It was added to many computer magazines with CD.
   * Borland C++ X
         www.borland.com
	 
   libraries
   * MySQL
         www.mysql.com
   * MS Platform SDK
         www.microsoft.com/msdownload/platformsdk/sdkupdate/psdk-full.htm
	 Please note, that will need probably MS Internet Explorer with enabled
	 ActiveX to download it. Platform SDK is added to MSDN subscriptions 
         too
   * BlueZ
         www.bluez.org
-------------------------------------------------------------------------------
Q. What are known problems and weak things of current version ?

A  Nokia section

    1.3210
      * 2'nd ringtone can't be set
      Well, I don't know, how to avoid it. Use --nokiacomposer to enter
      manually ringtone.

    2.3310/3315
      * names from phonebook are empty.
      To avoid it you need to have higher than 4.06 firmware in phone.

      * can't save to phone calendar notes other than reminder
      5.11 and higher seems not to be able to save notes other than reminder
      (in older firmware it was unofficial too).

    3.6210 (6250/7110 too ?)
      * when save some SMS like logo or ringtone to phone and forward them
        from phone's menu, they're damaged.
      It's NOT Gammu fault. Easy test show it: save such SMS to SIM
      inside older Nokia phone like 6150. Forward from 6150 menu. It's OK.
      Put SIM to 6210. Forward from 6210 menu. It's NOT OK.

      * when save unread sms to phone, no unread sms envelope is shown. 
      Sorry. Ask Nokia, if it's possible to have it.

      * after removing operator logo, it's still visible
      Have to reboot phone. Thank Nokia for it.

      * No info about incoming calls or SMS.
      Use option in config.h to enable it. Prepare for some small problems
      with phone then. This series is very weak here.

      * Phone restarts few seconds after ending connection (before restart
      phone display is frozen)
      I don't know how to avoid it. It's visible only sometimes.

    4.6310i 4.80
      * Gammu shows more SMS saved than phone in menu
      Phone firmware is buggy. Gammu show the truth. You can get
      locations of these SMS and delete them, if want. Update firmware to 5.10
      to avoid problems

    5.7110
      * can't get binary ringtones from phone.
      upgrade firmware in phone. Last one is 5.01 and it can do it.

    6.all Nokia phones with caller groups
      * I had backup file from another model (with caller groups too).
        After restoring my backup I can't set ringtone from phone menu or
        they're different than in source phone.
      Each Nokia model can have different ID for (the same) ringtones.
      ID are not saved in Gammu source for each model, because now
      (for DCT3 phones) it's idiotic easy to change them and such list
      won't be actual. Because of it backup caller groups again to text
      backup file and remove lines with ringtone ID. When will restore it,
      will be able to select it from phone menu.

    7.all Nokia phones with profiles
      TODO few details - caller groups and names

    8.all Nokia phones with voice tags
      * after restore I've lost my voice tags
      Well, current version of Gammu contains backup/restore for voice
      tags for all Nokia phones with SMS memory (like 6210 or 6310). If you
      made backup using earlier versions, it can happen, that they're lost
      (not visible in phonebook and phone show problems during editing or
      using voice tags). This is very weak thing in Nokia firmware.

      When it happened with DCT3 phone (like 6210):
      * the best solution is flashing phone again.

      With DCT4 (6310i 4.80) I resolved it this way (it can be tried with
      DCT3 too):
      * made backup of phonebook. Added many voice tags info to backup file
        (with numbers 1,2,3,4,...,20). Later made restore. Entered in phone
        menu to each such entry and changed voice tag, later deleted it. After
        it voice tag memory was clean.

    9.all Nokia phones with SMS memory
      * Gammu shows in AT mode Outbox sms as Inbox
      Well, in fact the only returned in phone info is, that SMS is
      Read/UnRead/Sent/UnSent. Normally Read/Unread can be only in Inbox
      and Sent/UnSent in Outbox.

      Firmware in Nokia 6310/6310i (probably other too) show received SMS
      from other than Inbox folders and there is no way to check, where they
      are saved. Gammu establish (like normally), that Read/Unread is Inbox
      and other Outbox. It can make such problems, but it's better than
      nothing.

   10.all DCT3 phones
      * Phone (or rather Gammu) doesn't inform about CB (Cell Broadcast)
      By default this feature is disabled (see option in config.h), because
      some people had problems with it - their phones didn't answer for command
      enabling this feature or enabled after (too) long timeout. I personally
      didn't have such problems. If want, test it. If works, be happy :-)

   11.DCT3 phones (3330, 3410, 5210, 5510, 62xx, ...) with WAP settings
      It's very messy (random) in these phones and in some situations it can
      give incorrect results (like mixing sets from different sets).

   12.can't transfer ringtones in binary format from DCT3 to DCT4 phone
      * You can only try to convert binary DCT3 ringtone to RTTL (using
      --copyringtone or other than Gammu software) and upload to DCT4 phone
      as RTTL ringtone. No binary DCT3 -> binary DCT4 converter available
      for now.

   14.DCT4 phones
      * I don't have all info about calendar notes included in Gammu backup
        and output
      When make ./configure, use --newcalendar. If this works for Your
      phone, please report model and firmware version.

   15.DCT4 phones with filesystem
      * I uploaded file to phone, phone says "format unknown".
      Unfortunately, if there is communication problem during uploading file
      and (for example) some file part is resent by Gammu, phone can recognize
      it incorrectly. Please try to upload file again and (if problem still
      happen) report to Gammu author(s).

   AT section

    1.Siemens M20
      * Gammu read incorrectly SMS with alphanumeric numbers
      Well, M20 doesn't have correctly support for such numbers at all.
      Gammu tries to avoid it and construct SMS from such incorrect frame.
      Many times it ends with success in 99%

   SonyEricsson

    1.T310
      * I can't backup Notes & I don't have numbers in PhoneCall ToDo
      Phone firmware issues. Thanks SE
   ---------
   TIP: if you want to know more about changes and bugs in firmware in Nokia
        phones, visit www.mwiacek.com and check firmware subpage

   Samsung

    1.S300
      * Only GIF pictures supported. Transfer pictures using --setlogo PICTURE,
      leave location parameter as 0. Data upload from phone not supported,
      --getbitmap and --getlogo only show location info.

    2.All other models:
      * Not tested, must verify wallpaper size for different models.
-------------------------------------------------------------------------------
Q. What models are supported ?

A. 1.Nokia DCT3 (3210|3310|3330|3390|3410|5110|5110i|5130|5190|5210|5510|6110|
     6130|6150|6190|8210|8250|8290|8850|8855|8890
     6210|6250|7110
     9110) and compatible.

     There is supported MBUS, FBUS, DLR3, Infrared, Bluetooth, at19200.
     For all (excluding 9110) there is supported almost everything.

   2.Nokia DCT4 (1100|1100a|1100b|3100|3100b|3108|3200|3200a|3300|3510|3510i|
     3530|3589i|3590|3595|5100|5140|6100|6200|6220|6230|6310|6310i|6385|6510|
     6610|6610i|6800|6810|6820|7210|7250|7250i|7600|8310|8390|8910|8910i
     3320) and compatible

     There is supported MBUS, FBUS, DLR3, Infrared, at19200, BlueTooth.
     No full support for DKU-5. No support for locking WAP settings, locking
     WAP bookmarks, enabling netmonitor and few others.

   3.Siemens M20, MC35, SL45 and compatible

     Support for AT mode and many extensions (calendar, logos, netmonitor...)

   4.Alcatel OT 500, 501, 701, 715, 535, 735
     
   5.Falcom A2D-1

   6.IPAQ

   7.AT compatible devices

     If device is compatible with ETSI specs, SMS (both in text and PDU),
     phonebook and some other will work with AT module.

   8.OBEX devices

     Tested with Nokia Series40/Series60 and SonyEricsson. Works according to
     device capabilities (can be folder browsing/getting/uploading files/
     uploading files to Inbox).

   9.Sony Ericsson

     Tested with T310 (or rather T300 with R3B firmware from T310),
     and R320s. Connections can be at* (serial), irda* and blue*.
     Supported as AT or OBEX device above. New supported features are in 
     development now.

  10.Samsung

     Experimental support for Samsung AT extensions (ringtones and logos).
     Pictures are GIF format only. Tested with S300.
-------------------------------------------------------------------------------
Q. How to report bugs ?

A. There are few methods of reporting errors. I will describe, which is the
   best for your error:

   1. some function doesn't work. Use "logfile" option in "gammurc" (for
      example "gammurc" see /docs/examples directory). Set it for example to
      "logfile". Option "logformat" set to "textall". Then run incorrectly
      working function. There will be "logfile" file created. Look inside.

      If you see some lines starting from "[ERROR:", stop here and read below,
      what todo with "logfile" file.

      If no such errors, set "logformat" option to "binary". Run again
      incorrectly working function. There will be binary logfile file created.
      See below what todo with this file.

   2. Gammu can't at all communicate with phone. Similiary like in point 1
      set "logfile" option to "logfile", "logformat" to "textall" and make dump
      for example from "gammu --identify".

   Created log files can be sent to mailing list or directly to
   marcin@mwiacek.com (if you don't want to publish some data to
   more people). Please compress file(s), if they're big.

   To debug files you can (should) add:

   1. (when use Linux or Unix): name of your disribution, kernel version
   2. at least short description of problem

   You don't have to include:

   1. config file (important settings will be in debug files)

   Thank you very much.

   TIP: when set "logformat" option to "errors", there will be reported only
        errors. After long using Gammu you simply check, if there were any
        problems with communication with your phone.
-------------------------------------------------------------------------------
Q. Who helped in writing it ?

A. Here are some people, who somehow helped in this project. Probably not
   everyone is mentioned. If you think, you're missed, please let us know:

   * all people developing Gnokii (some ideas are from it) like Pavel Janik,
     Pawel Kot, Manfred Jonsson, ... (see CREDITS from gnokii)
   * all people responsible for MyGnokii (Gammu is based on it too) like
     Gabriele Zappi, Ralf Thelen, .... (see CREDITS from mygnokii)
   * Marcin Wiacek - author of all this mess ;-)
   * Markus Plail & Tibor Harsszegi - for DCT4 help
   * Frederick Ros - 5210 help
   * Jan Kratochvil - AT commands
   * Walek - fixes, progress in many things
   * Marcel Holtmann - Bluetooth tests
   * Michal Cihar - general discussions, tries into making one
     big and good GSM manager (many changes and suggestions), Alcatel support
   * Ondrej Rybin & Tomasz Motylewski & Tom Nelson - hardware help
   * Joergen Thomsen - SMS daemon
   * Claudio Matsuoka - Samsung support, some hacks and fixes
-------------------------------------------------------------------------------
Q. What file formats are supported ?

A. for logos              : xpm (only saving), bmp, nlm, nsl, ngg, nol, wbmp,
			    gif (for Samsung)
   for ringtones          : rttl, binary format created for Gammu,
                            mid (saving), re (reading), ott, communicator,
                            ringtones format found in fkn.pl, wav (saving),
                            ime/imy (saving), rng, mmf (for Samsung)
   for backup/restore     : text format created for Gammu, vcs (VCALENDAR),
                            vcf (VCARD), lmb (Logo Manager - startup,
                            phonebook, caller groups entries),
                            ldif (Mozilla/Netscape Address Book)
   for sms backup/restore : text format created for Gammu
-------------------------------------------------------------------------------
Q. How to connect Nokia phone to PC and gammu?

A. Using cable. Each phone has connector on bottom (like in 5110) or under
                battery (like in 3310) or under back cover (like in 3210).
                In first version cable has "normal" connector, in second it's
                special and put between phone and battery or under back cover.
                There are three protocols used with these phones: MBUS
                (called M2BUS too), FBUS, AT mode. Cables supporting these
                protocols are different:
                1. universal FBUS/MBUS. The best, because should work with
                   each phone (of course you need correct connector for it)
                   in one or second protocol. Sometimes called DAU9P (it was
                   name of original universal cable for 61xx/51xx). Same
                   universal cables have manual switch. IMHO, it's not
                   necessary
                2. MBUS only. Required for some concrete phones to allow them
                   work with this protocol (like my 6310i - it didn't want to
                   work in MBUS with my universal FBUS/MBUS cable without
                   switch).
                3. FBUS only. IMHO, wasting money.
                4. AT/FBUS cable (originally called DLR3 or DLR3P). Will not
                   work with older phones (like 51xx/61xx) even in FBUS. You
                   can use it with 62xx/63xx/71xx and work in AT or FBUS mode.
                   Good choice, if won't use MBUS protocol and have modern
                   model.

                Gammu supports each of these protocols. Just give access to
                used serial device, set it as "port" in gammurc and set
                "connection" according to protocol

   Using infrared. There are two versions: with direct communicating with
                   phone or using some functions from operating system.

                   First ("infrared" in gammurc) can be used with 61xx.
                   Also 82xx use it, when enter infrared using 2 player mode.
                   Obsolete, in Linux requires disabling all infrared support
                   in kernel.

                   Second is much better, compatible with ALL modern operating
                   systems, in Linux requires enabling infrared in kernel.
                   Called socket infrared. Used in all new phones.
                   "irda" in gammurc.

                   Example of using socket infrared in Linux (based on USB
                   dongle):
                   1.compile kernel with USB and infrared protocols
                   3.if required, start USB module (example:
                     "modprobe irda-usb")
                   2.use "irattach irda0 -s" or similiar
                   3.set "connection" in gammurc to "irda", "port" to
                     "/dev/ircomm0" or similiar (you have to add access to
                     this device to user)

   Using bluetooth. Works OK in all situations

                    Example of using USB dongle for Linux (Bluez stack from
                    http://bluez.sourceforge.net):
                    1.compile kernel with USB and Bluetooth (don't use
                      BT in USB section !)
                    2.load kernel driver ("modprobe hci_usb")
                    3."hciconfig hci0 up" enables dongle
                    4.in /etc/bluetooth create "givepin" file. It should be
                      executable ("chmod a+x /etc/bluetooth/givepin")
                    5.write inside:
                      #!/bin/sh
                      echo "PIN:5432"
                    6.in /etc/bluetooth/hcid.conf set "pin_helper" to
                      "/etc/bluetooth/givepin;"
                    7."hcid" (will send options from hcid.conf to BT device)
                    8.set "device" in gammurc to phone address (see "hcitool
                      scan" for it) and protocol to "bluephonet" or similiar
                    9.during first connection give PIN 5432 in phone

   I will also write few words about flasher cable. It's different and
   connected to paraller (LPT) port and allows to read/write flash (internal
   memory). When use some software (called flashers) and such cable, you can
   even replace internal phone software to higher. Gammu as user software
   doesn't support flashing.
-------------------------------------------------------------------------------
Q. How to connect Ericsson/Siemens/Samsung/Alcatel phone to PC and gammu?

A. Using cable: Get serial cable for the appropriate model, plug it into the
                phone and in a free serial port of the PC. In gammu, set 
                the correct port ("COM1:" .. "COM9:" in Windows, 
                "/dev/ttyS*" in Linux). Set "connection=at115200" (or 
                lower) and "model=at". You can also try "model=obex".
   Using infrared: You can map your infrared connection to a virtual 
                   COM port in Win9x and XP. Then give "port=COM4:" (or 
                   check the correct port in Control Panels/Infrared 
                   transfer) and the connection will look like normal serial 
                   connection. Continue as above.
                
                   If you wish to use true infrared connection, follow the 
                   steps in the Nokia answer above.

   Using bluetooth: see Nokia answer above.
-------------------------------------------------------------------------------
Q. What is "FIRMWARE"?

A. FIRMWARE is the Operating System of the phone, the program which allows
   phone to work. Infact, a mobile phone is just a computer which can handle
   voice communications between two (or more) users, and, just like any other
   computer, it needs programs to work: the FIRMWARE, infact.
   This is a special kind of software, at half way between HARDWARE, that is
   the physical, electronic part of the computer, and the SOFTWARE, the
   programs which allow the hardware to work; the firmware is a kind of
   software, "fixed" on the hardware, and it cannot be modified by users.
   By "normal" users, at least.
   Actually, it IS possible to modify the firmware, to upgrade it to newest
   releases, but it requires a technical skill which not all people have.
   Usually, only the phone manufacturer can upgrade the firmware, using special
   devices. So, if you want to upgrade your firmware, you should bring your
   phone to your dealer, and wait for him to give the phone to the
   manufacturer, which, often after "months" ;-)))), will give you back your
   phone, upgraded, and your wallet, without a copule of 20$ banknote...
   BUT, it is also possible to do all this mess by yourself: you can find on
   Internet the required hardware, a Flasher Device (which could often cost
   MORE than 40$!), a flasher program, and the latest release of the firmware..
   which is copyrighted by the phone manufacturer, and so COULDN'T be
   distributed on the Net....
   On Gammu site you can find a lot of info about Mobile Phones Firmwares,
   BUT nothing about flashing: you should use a search engine using keywords
   like "flasher", "firmware", "upgrade", "mobile phone" etc.
-------------------------------------------------------------------------------
Q. Can Gammu backup voice tags (in meaning: sound saved in phone as voice
   record) ?

A. Sorry, but not. I don't have Nokia software able to do it.
-------------------------------------------------------------------------------
Q. I can't connect to Nokia 9210 using original cable provided with phone...

A. Use or "at19200"/"at115200" connection (have to enable modem in phone then)
   or "fbus" after enabling FBUS with phone netmonitor 31.

   Note: to enable netmonitor in phone you need MBUS cable (not DLR2 provided
         with phone)
-------------------------------------------------------------------------------
Q. What phone could be recommended by Gammu author(s) to make SMS gateway ?

A. The best is always the most simple device. So, I think that using Nokia 3310
   or 3410 can be very good choice. They have removed some SMS bugs (for
   example, 3330 has buggy firmware - you have to flash it with 3310 firmware
   to avoid any problems). Especially 3410 is interesting - has got LiIon
   battery.

   More expensive Nokia phones (with own SMS memory) are more complicated
   and using them is not always good idea (of course, Gammu can do it).

   Another way is is to use special cellular modem especially equiped to
   continuous work. I recommend Siemens devices - for example Siemens MC35. It
   has GPRS and can be used to many tasks.
-------------------------------------------------------------------------------
Q. What phone could be recommended by Gammu author(s) ? (generally) (19.11.02)

A. It's difficult to say. Everyone needs something different according to own
   needs.

   Start from Nokia:
     If you want to be visible, buy 7650 or 9210. Very visible, very expensive,
     and parts for it are very expensive. Many small stability problems,
     no GPRS (in 9210). Weak battery in 7650.

     If you need phone for calls, no WAP, but requires extended pbk, buy 6210
     or 6250 (second is for any weather and hard environment). Will be able
     to "repair" (flash) it - it repairs each phone in majority problems.
     These phones are very slow and has some other small problems.

     When want WAP, GPRS, buy rather 6310i than 6310. Remember, that firmware
     inside has still many small bugs and it possible, phone can go often to
     service to flashing. Some things are still not useable (like CB messages
     - can't change tone to it), no netmon in this moment. This family has
     Bluetooth.

     For WAP, GPRS is also good 8310 - first DCT4 phone.

     When don't have too much money and need calendar/extended pbk, buy 3510
     (the same flashing and netmon story to 6310, 6310i). 

     The cheapiest 3410 is for people, who don't need anything - only device
     for making calls, nothing more. It's (like old 3310 or 5110) very good to
     making SMS gateway.

     Phones with colour display are not very good for me. IMHO, I will wait
     some time, until they will have all problems removed.

  Siemens:
     Very good AT support, very stable with it, available netmon and many,
     many other functions (not like with Nokia). Worse user interface than
     Nokia.

  Panasonic:
     GD96 - weak battery, many, many functions. Nice for many people.

  Ericsson:
     T65 - very small, good, if you will like its' interface. T68i - MMS.

  I will not say about other - for example, I don't like Morotola or Alcatel
  user interfaces at all.

-------------------------------------------------------------------------------
Q. How to make national version of Gammu ?

A. 1. go into docs/docs/locale
   2. rename gammu_us.txt to gammu_XX.txt (where XX is language ID)
   3. open gammu_XX.txt in editor supporting Unicode (in win32 Notepad)
   4. to each "Fxxxx = "text"" line add "Txxxx = "text"" line
   5. send to me updated file
-------------------------------------------------------------------------------
Q. Where can I find additional info ?

A. * www.mwiacek.com
     * manual for Nokia netmonitor
     * info about firmware in Nokia phones (changes, bugs, ...)
     * info about protocols in Nokia phones
     * default ringtones and logos from Nokia phones
   * www.gnokii.org
     * another "free" manager for GSM phones. It was core for first manager
       created by Marcin Wiacek (the main reason of creating it was eliminating
       Gnokii problems) called MyGnokii. Gammu was created from rewriting
       MyGnokii. You can now compare quality in Gnokii and Gammu meaning ;-)
   * http://gsm-technology.com
     * GSM DATA AND SERVICE CABLES
     * GSM EQUIPMENT & HARDWARE
     * FREE FLASHES, LOGO'S, RINGTONES
     * FREE INSTRUCTIONS, SCHEMES
   * http://zope.achterklap.nl:8080/nokia/
     * reversing DCT3 phones
-------------------------------------------------------------------------------
Q. Nokia and other manufactures provides original software. What original
   applications can be replaced with Gammu ?

A. Nokia:
   * Nokia Cellular Data Suite 2.0 - 3.0 (N5110, N6110)
   * PC-Suite for N5210
   * PC-Suite 4.06 (N6210)
   * PC-Suite 4.3 - 4.88 (DCT4 mono: N6310, N6310i, N6510, N8310, N8910)
   * PC-Suite 5.0 - 5.1 (DCT4 color: N5100, N6610, N6800, N7210)
     * PC-Suite 5.12 (N6800)
     * PC-Suite 5.16 (N7210)
-------------------------------------------------------------------------------
Q. Does Gammu support Chinese SMS ?

A. If I understand correctly, they're saved with Unicode alphabet in SMS. Gammu
   is prepared for it. For example for sending/saving SMS just create Unicode
   file and use switch -inputunicode, when use send/savesms
-------------------------------------------------------------------------------
Q. Can I use Nokia 6110/6130/6150 and infrared ?

A. Unfortunately, connecting to Nokia:Phonet service in these phones doesn't
   end with success. Probably there is no other way and you can't use socket
   infrared here.

   In Linux disable infrared in kernel, connect your infrared device to serial
   port (other won't be supported in this mode) and try Gammu with "infrared"
   connection. If doesn't work, there will be required some modifications of
   sources. Please contact authors.

   In Windows ANY infrared device can be assigned to serial port with driver
   written by Jan Kiszka.
-------------------------------------------------------------------------------
Q. I can't play files produced by --nokiagetvoicerecord

A. They require GSM 6.10 codec to be played. In Win XP it's included by
   Microsoft. If you deleted it by accident in this operating system, make
   such steps:
    1. Control Panel
    2. Add hardware
    3. click Next
    4. select "Yes. I have already connected the hardware"
    5. select "Add a new hardware device"
    6. select "Install the hardware that I manually select from a list"
    7. select "Sound, video and game controllers"
    8. select "Audio codecs"
    9. select "windows\system32" directory and file "mmdriver.inf"
   10. if You will be asked for file msgsm32.acm, it should unpacked from
       Windows CD
   11. now You can be asked if want to install unsigned driver (YES), about
       select codec configuration (select what you want) and rebotting PC (make
       it)
-------------------------------------------------------------------------------
Q. I have problems with GPRS Online in my Nokia phone (Series 40, not Symbian)

A. In first Nokia phones with GPRS (like 8310, 6310, 6310i, 6510, 3510 and
   maybe some other) phone was able to display "G" char instead of char
   displayed below network level. It was clear, that phone is in continuous
   GPRS session and downloading some data won't be connected with starting new
   session (in many networks connected with new money for operator).
   If phone end GPRS session after leaving WAP browser or data connection
   (no "G" char), it had disabled continuous GPRS sessions (it was done
   for example after upgrading firmware in service). Simple
   "gammu --nokiasetphomenus" "repaired" it.

   Nokia 3510i seems to have disabled possibility of using continous GPRS
   sessions at all. Probably the same will be in other cheap Nokia models.

   All other new models seems to have ability of making continuous GPRS
   sessions - at least some documents from Nokia say, that the same setting
   (the same, which was set for first models) should be set to enable
   this behaviour. "gammu --nokiasetphonemenus" sets it, but it looks, that or
   this info is wrong or new models continues GPRS sessions without informing
   users about it (no "G" char). I haven't checked it deeply. Maybe in free
   moment...

   Short summary: GPRS was chance to save some money. It was wrong for
   cellular operators. Probably on their requests Nokia made sessions more
   enpensive (it's not possible to use continuous sessions or they're more
   hidden). Welcome in new connected world :-(
-------------------------------------------------------------------------------
Q. How to manage playlists in 6230 ?

A. 1. gammu --addfile a:\predefplaylist filename.m3u

      Will add playlist filename.m3u

   2. gammu --getfilesystem

      Will get list of all files (including names of files with playlists)

   3. gammu --deletefiles a:\predefplaylist\filename.m3u

      Will delete playlist filename.m3u

   Format of m3u files is easy:

   first line is #EXTM3U
   in next you give names of files (b:\file1.mp3, b:\folder1\file2.mp3, etc.)
