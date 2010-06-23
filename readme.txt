Gammu README version 20.10.2002
-------------------------------
Q. What is this ?

A. This is package with different tools and drivers for Nokia and other mobile
   phones released under GNU GPL license (see /copying file).

   It's created by Marcin Wiacek and other people and based on some experiences
   from Gnokii (www.gnokii.org) and MyGnokii (http://marcin-wiacek.topnet.pl)
   projects.

   Gammu (GNU All Mobile Management Utilities) was earlier (up to version 0.58)
   called MyGnokii2.

   NOTE: Name Gammu is not connected with Gammu from "Heretics of Dune" written
         by Frank Herbert
-------------------------------------------------------------------------------
Q. Can I use Gammu (Gammu dll or library) in own (commercial) applications ?

A. Gammu is currently licensed under GNU GPL license. It means, that each
   application using it have to be published with source. So, when link
   Gammu source in own, you should inform me and publish it with source.

   When use Gammu dll or library, you also should publish your source. This
   is how many people see GNU GPL. I won't make any problems, if your source
   will be unavailable (of course, still show somewhere link to Gammu source
   and info about it).

   If you think, I break GNU GPL and situation is not clear, I could maybe
   change license to different (like LGPL).
-------------------------------------------------------------------------------
Q. How can I help in writing this software ?

A. HELP IS VERY REQUIRED. Methods are many:

   1. writing support for some models or fixing source (I can give tips many
      times). Format and method of creating patch files is described in develop
      docs.
      In this I have in my library specs for many models (example:
      Morotola, Sagen, Siemens xx45) and it's only required to code few 
      functions into current source to have support for them...
   2. reporting bugs (see "How to report bugs ?" question)
   3. help in documentation or translations
   4. help in discovering phone protocols
   5. suggesting some changes to better :-)

   etc.
-------------------------------------------------------------------------------
Q. Where to get latest versions and support ?

A. Latest versions are on http://marcin-wiacek.topnet.pl. There are two
   packages: stable and developer. Second can be updated even once a day.

   For support contact Marcin Wiacek (marcin-wiacek@topnet.pl). You can also
   subscribe mailing list (see http://marcin-wiacek.topnet.pl and subpage
   about Gammu) and get help there. List is used also for annoucements about
   new versions.

   TIP: If you want to have only general info about new versions, you can
        subscibe http://freshmeat.net/projects/gammu/ annoucements.
-------------------------------------------------------------------------------
Q. How to compile it ?

A. Method 1:
     * under Unix (Linux)
         first do "./configure" and later use "make" or "make makelib"

         After it can use "make install" or "make installlib" to install it
         in your OS.
   Method 2:
     * change parameters in /cfg/config.h
     * compile it:
       * under MS Visual C++ 6.0 (7.0) for MS Windows
           enter into /gammu/msvc6 directory and run gammu.dsw
       * under Unix (Linux)
           change compiler in /cfg/makefile.cfg (if required) and use "make" or
           "make makelib"

           After it can use "make install" or "make installlib" to install it
           in your OS.
       * under Borland C++ 3.1 for MS-DOS
           I tried, but I can't do it :-(. Help required.
       * under DJGPP for MS-DOS
           simply use "make" :-)
           (I compiled it without problems in plain MS-DOS from Windows 98 SE.
            when tried from DOS Box in Windows XP, had memory problems).

           NOTE: in this early version you can't use Gammu compiled using
                 DJGPP - serial driver is not ready !
       * under CygWin for Windows
           I didn't try. Probably should work
       * under Borland C++ Builder 5 Trial
       * under Borland C++ Builder 6 Personal
           go into gammu/bcc and check if there is subdirectory "output"
           (if not, create it). Then run gammu.bpr
	   
           TIP: gammu.bpr was created using "Tools/Visual C++ Project Conve.."
                and little modified later.
       * under Borland C++ command line tools
           go into gammu/bcc and check if there is subdirectory "output"
           (if not, create it). Then make "make -fgammu.mak" from command
           line.
	   
           TIP: gammu.mak was created in Borland C++ Builder using
                "Project/Export makefiles" from main menu.
-------------------------------------------------------------------------------
Q. Where to get compilers to make Gammu ?

A. * MS Visual C++ 6.0
         go to the shop ;-). It's commercional software.
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
-------------------------------------------------------------------------------
Q. What are known problems and weak things of current version ?

A  1. Nokia 3310 (3315):

      * names from phonebook are empty.

      To avoid it you need to have higher than 4.06 firmware in phone.

      * can't save to phone calendar notes other than reminder (after upgrading
        firmware in service)

      5.11 and higher seems not to be able to save notes other than reminder
      (in older firmware it was unofficial too).

   2. Nokia 6210 (6250/7110 too ?):

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

   3. Nokia 7110: can't get binary ringtones from phone.

      upgrade firmware in phone. Last one is 5.01 and it can do it.

   4. all Nokia phones with caller groups: I had backup file from another
      model (with caller groups too). After restoring my backup I can't
      set ringtone from phone menu or they're different than in source phone.

      Each Nokia model can have different ID for (the same) ringtones.
      ID are not saved in Gammu source for each model, because now
      (for DCT3 phones) it's idiotic easy to change them and such list
      won't be actual. Because of it backup caller groups again to text
      backup file and remove lines with ringtone ID. When will restore it,
      will be able to select it from phone menu.

   5. Phone (or rather Gammu) doesn't inform about CB (Cell Broadcast)
      messages.

      By default this feature is disabled (see option in config.h), because
      some people had problems with it - their phones didn't answer for command
      enabling this feature or enabled after (too) long timeout. I personally
      didn't have such problems. If want, test it. If works, be happy :-)

   6. 3210 and 2'nd ringtone, which can't be set.

      Well, I don't know, how to avoid it. Use --nokiacomposer to enter
      manually ringtone.

   7. DCT3 phones (3210, 33xx, 3410, 51xx, 5210, 5510, 61xx, 62xx, ...)
      and WAP settings. 

      Getting/setting was done with 6210 5.56. It's very messy in these phones
      and probably in other models and some concrete firmware can be different.
      Please inform about incompatibilities.

   8. Restore for profiles

      TODO few details - caller groups and names

   9. after restore I've lost my voice tags

      Well, current version of Gammu contains backup/restore for voice
      tags for all Nokia phones with SMS memory (like 6210 or 6310). If you
      made backup using earlier versions, it can happen, that they're lost
      (not visible in phonebook and phone show problems during editing or
      using voice tags). This is very weak thing in Nokia firmware.

      When it happened with DCT3 phone (like 6210), the best solution is
      flashing phone again.

      With DCT4 (6310i 4.80) I resolved it this way (it can be tried with
      DCT3 too):

      made backup of phonebook. Added many voice tags info to backup file
      (with numbers 1,2,3,4,...,20). Later made restore. Entered in phone
      menu to each such entry and changed voice tag, later deleted it. After
      it voice tag memory was clean.

   10. Gammu shows more SMS saved than phone in menu (example: 6310i 4.80)

       Phone firmware is buggy. Gammu show the truth. You can get
       locations of these SMS and delete them, if want.

   11. Gammu read incorrectly SMS with alphanumeric numbers (Siemens M20)

       Well, M20 doesn't have correctly support for such numbers at all.
       Gammu tries to avoid it and construct SMS from such incorrect frame.
       Many times it ends with success in 99%

   12. Gammu shows in AT mode Outbox sms as Inbox...

       Well, in fact the only returned in phone info is, that SMS is
       Read/UnRead/Sent/UnSent. Normally Read/Unread can be only in Inbox
       and Sent/UnSent in Outbox.

       Firmware in Nokia 6310/6310i (probably other too) show received SMS
       from other than Inbox folders and there is no way to check, where they
       are saved. Gammu establish (like normally), that Read/Unread is Inbox
       and other Outbox. It can make such problems, but it's better than
       nothing.
   ---------
   TIP: if you want to know more about changes and bugs in firmware in Nokia
        phones, visit http://marcin-wiacek.topnet.pl and check firmware subpage
-------------------------------------------------------------------------------
Q. What models are supported ?

A. Here are all supported models:

   1.3210|3310|3330|3390|3410|5110|5110i|5130|5190|5210|5510|6110|6130|6150|
     8210|8250|8290|8850|8855|8890:

     6210|6250|7110:

     3510|6310|6310i|6510|8310|8910:

     9110|9210:

     They all should work (excluding maybe some details) now/in the future.
     I personally tested 3210, 3310, 5110, 6150, 6210, 6250, 6310i, 9210

   5.various Siemens (like M20, MC35), Nokia (like 6150, 6210, 6310i), Alcatel
     and IPAQ:

     Support written according to ETSI specs. Other compatible devices should
     work too. Made many things like 100% SMS support (both in text and PDU
     mode), phonebook...
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
   marcin-wiacek@topnet.pl (if you don't want to publish some data to
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
   * Walek - fixes
   * Marcel Holtmann - Bluetooth tests
   * Michal Cihar - general discussions and tries into making one
     big GSM manager
-------------------------------------------------------------------------------
Q. What file formats are supported ?

A. for logos              : xpm (only saving), bmp, nlm, nsl, ngg, nol
   for ringtones          : rttl, binary format created for Gammu,
                            mid (saving), re (reading), ott, communicator,
                            ringtones format found in fkn.pl, wav (saving),
                            ime/imy (saving)
   for backup/restore     : text format created for Gammu,
                            lmb (startup, phonebook, caller groups)
   for sms backup/restore : text format created for Gammu
-------------------------------------------------------------------------------
Q. How to connect Nokia phone to PC ?

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

                Gammu supports each of these protocols.

   Using infrared. There are two versions: with direct communicating with
                   phone or using some functions from operating system.

                   First ("infrared" in gammurc) can be used with 61xx.
                   Also 82xx use it, when enter infrared using 2 player mode.
                   Obsolete, in Linux requires disabling all infrared support
                   in kernel.

                   Second is much better, compatible with ALL modern operating
                   systems, in Linux requires enabling infrared in kernel.
                   Used in all new phones. "irda" in gammurc.

   Using bluetooth. Under Linux works 100% with Nokia 6210 (dlr3blue), works
   also with each AT phone (atblue).

   I will also write few words about flasher cable. It's different and
   connected to paraller (LPT) port and allows to read/write flash (internal
   memory). When use some software (called flashers) and such cable, you can
   even replace internal phone software to higher. Gammu as user software
   doesn't support flashing.
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
