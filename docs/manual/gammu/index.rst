.. _gammu:

Gammu Utility
=============

.. program:: gammu

Synopsis
++++++++


.. code-block:: text

    gammu [parameters] <command> [options]

Description
+++++++++++

This program is a tool for mobile phones. Many vendors and phones
are supported, for actual listing see <http://wammu.eu/phones/>.


Options
-------

Parameters before command configure gammu behaviour:

.. option:: -c, --config <filename> 
   
    name of configuration file

.. option:: -s, --section <confign> 
   
   section of config file to use, eg. 42

.. option:: -d, --debug <level>
   
   debug level (see :ref:`gammurc` for possible values)

.. option:: -f, --debug-file <filename>
   
   file for logging debug messages


Commands
--------

Commands actually indicate which operation should Gammu perform. They can be
specified with or without leading ``--``.


Calls
~~~~~

.. option:: --answercall [id]

   Answer incoming call.

.. option:: --cancelcall [id]

    Cancel incoming call

.. option:: --canceldiverts

    Cancel all existing call diverts.

.. option:: --conferencecall id

    Initiates a conference call.

.. option:: --dialvoice number [show|hide]

    Make voice call from SIM card line set in phone.

    ``show|hide`` - optional parameter whether to disable call number indication.

.. option:: --divert get|set all|busy|noans|outofreach all|voice|fax|data [number timeout]

    Manage or display call diverts.

    ``get`` or ``set``
        whether to get divert information or to set it.
    ``all`` or ``busy`` or ``noans`` or ``outofreach`` 
        condition when apply divert
    ``all`` or ``voice`` or ``fax`` or ``data`` 
        call type when apply divert
    number 
        number where to divert
    timeout 
        timeout when the diversion will happen


.. option:: --getspeeddial start [stop]

    Gets speed dial choices.

.. option:: --getussd code

    Retrieves USSD information - dials a service number and reads response.

.. option:: --holdcall id

    Holds call.

.. option:: --maketerminatedcall number length [show|hide]

    Make voice call from SIM card line set in phone which will 
    be terminated after ``length`` seconds.

.. option:: --monitor [times]

    Get phone status and writes continuously to standard output. Press Ctrl+C
    to break this state.

.. option:: --senddtmf sequence

    Plays DTMF sequence. In some phones available only during calls

.. option:: --splitcall id

    Splits call.

.. option:: --switchcall [id]

    Switches call.

.. option:: --transfercall [id]

    Transfers call.

.. option:: --unholdcall id

    Unholds call.

SMS and EMS
~~~~~~~~~~~

Sending messages might look a bit complicated on first attempt to use.
But be patient, the command line has been written in order to allow
almost every usage. See EXAMPLE section for some hints on usage.

There is also an option to use gammu-smsd(1) when you want to send or 
receive more messages and process them automatically.

Common parameters for sendsms and savesms
_________________________________________

.. option:: -smscset number

    SMSC will be taken from set \fBnumber\fR. Default set: 1

.. option:: -smscnumber number

    SMSC number

.. option:: -reply

    reply SMSC is set

.. option:: -maxnum number

    Limit maximal number of messages which will be
    created. If there are more messages, Gammu will terminate with failure.

.. option:: -folder number

    save to specified folder. 

    Folders are numerated from 1.

    The most often folder 1 = "Inbox", 2 = "Outbox",etc. Use \fBgetsmsfolders\fR to get folder list.

.. option:: -unread

    makes message unread. In some phones (like 6210) you won't see 
    unread sms envelope after saving such sms. In some phones with internal 
    SMS memory (like 6210) after using it with folder 1 SIM SMS memory will be used

.. option:: -read

    makes message read. In some phones with internal
    SMS memory (like 6210) after using it with folder 1 SIM SMS memory will be used

.. option:: -unsent

    makes message unsent

.. option:: -sent

    makes message sent

.. option:: -smsname name

    set message name

.. option:: -sender number

    set sender number (default: "Gammu")

.. option:: -report

    request delivery report for message

.. option:: -validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX

    sets how long will be the
    message valid (SMSC will the discard the message after this time if it could
    not deliver it).

.. option:: -save

    will also save message which is being sent

Introduction to SMS formats
___________________________

Gammu has support for many SMS formats like:

Nokia Smart Messaging
    used for monochromatic picture images, downloadable profiles, monochromatic operator logos, monochromatic caller logos and monophonic ringtones
Linked SMS
    both with 8 and 16-bit identification numbers in headers
EMS
    this is SMS format used for saving monochromatic images, monophonic ringtones, animations, text formatting and others
MMS notifications
    contains links where phone should download MMS
Alcatel logo messages
    proprietary format for logos


You need to ensure that the target phone supports message type you want to
send. Otherwise the phone will not be able to display it or will even crash,
because firmware of phone did not expect this possibility.

Encoding chars in SMS text
__________________________

Text in SMS can be coded using two ways:

with \fBGSM Default Alphabet\fR - in single SMS you can have maximally 160 chars (Gammu doesn't support compressing such texts according to GSM standards, but it isn't big limit, because there are no phones supporting them), but they're from limited set:

* all Latin small and large
* all digits
* some Greek
* some other national
* some symbols like  @ ! " # & / ( ) % * + = - , . : ; < > ?
* few others

with \fBUnicode\fR - in single SMS you can save at most 70 chars, but these can be
any chars including all national and special ones. Please note, that some
older phones might have problems displaying such message.

Gammu tries to do the best to handle non ASCII characters in your message.
Everything is internally handled in Unicode (the input is converted depending
on your locales configuration) and in case message uses Unicode the text will
be given as such to the message. 

Should the message be sent in GSM Default Alphabet, Gammu will try to convert
all characters to keep message readable. Gammu does support multi byte
encoding for some characters in GSM Default Alphabet (it is needed for ^ { }
\\ [ ] ~ |). The characters which are not present in GSM Default Alphabet
are transliterated to closest ASCII equivalent (accents are removed).
Remaining not known characters are replaced by question mark.

.TP
.. option:: --addsms folder file [-yes]
Adds SMSes from file (format like backupsms uses) to
selected folder in phone.
.TP
.. option:: --addsmsfolder name

.TP
.. option:: --backupsms file [-yes|-all]
Stores all SMSes from phone to file. 

Use -yes for answering yes to all questions (backup all messages and 
delete them from phone), or -all to just backup all folders while keeping
messages in phone.

.TP
.. option:: --deleteallsms folder
Delete all SMS from specified SMS folder.

.TP
.. option:: --deletesms folder start [stop]
Delete SMS from phone. See description for \fBgetsms\fR for info about
sms folders naming convention. 

Locations are numerated from 1.

.TP
.. option:: --displaysms ... (options like in sendsms)

Displays PDU data of encoded SMS messages. It accepts same parameters and 
behaves same like sendsms.

.TP
.. option:: --getallsms -pbk
Get all SMS from phone. In some phones you will have also SMS templates
and info about locations used to save Picture Images. With each sms you
will see location. If you want to get such sms from phone alone, use
\fBgammu getsms 0 location\fR

.TP
.. option:: --geteachsms -pbk
Similiary to \fBgetallsms\fR. Difference is, that
links all concatenated sms

.TP
.. option:: --getsms folder start [stop]
Get SMS. 

Locations are numerated from 1.

Folder 0 means that sms is being read from "flat" memory (all sms from all
folders have unique numbers). It's sometimes emulated by Gammu. You 
can use it with all phones.

Other folders like 1, 2, etc. match folders in phone such as Inbox, Outbox, etc.
and each sms has unique number in his folder. Name of folders can
depend on your phone (the most often 1="Inbox", 2="Outbox", etc.).
This method is not supported by all phones (for example, not supported
by Nokia 3310, 5110, 6110). If work with your phone, use
\fBgetsmsfolders\fR to get folders list.

.TP
.. option:: --getsmsc [start [stop]]
Get SMSC settings from SIM card. 

Locations are numerated from 1.

.TP
.. option:: --getsmsfolders
Get names for SMS folders in phone

.TP
.. option:: --nokiagett9
This option should display T9 dictionary content from
DCT4 phones.

.TP
.. option:: --restoresms file [-yes]
Restores SMSes from file (format like backupsms uses) to
selected folder in phone. Please note that this overwrites existing
messages in phone (if it supports it).
.TP
.. option:: --savesms ANIMATION frames file1 file2... [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Save an animation as a SMS. You need to give
number of frames and picture for each frame. Each picture can be in any
picture format which Gammu supports (B/W bmp, gif, wbmp, nol, nlm...).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms BOOKMARK file location [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read WAP bookmark from file created by \fBbackup\fR option and saves in
Nokia format as SMS


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms CALENDAR file location [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read calendar note from file created by \fBbackup\fR option and saves in
VCALENDAR 1.0 format as SMS. The location identifies position of calendar item 
to be read in backup file (usually 1, but can be useful in case the backup contains 
more items).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms CALLER file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Save caller logo as sms in Nokia (Smart Messaging)
format - size 72x14, two colors.

Please note, that it isn't designed for colour logos available for example in
DCT4/TIKU - you need to put bitmap file there inside phone using filesystem
commands.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms EMS [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-unicode] [-16bit] [-format lcrasbiut] [-text text] [-unicodefiletext file] [-defsound ID] [-defanimation ID] [-tone10 file] [-tone10long file] [-tone12 file] [-tone12long file] [-toneSE file] [-toneSElong file] [-fixedbitmap file] [-variablebitmap file] [-variablebitmaplong file] [-animation frames file1 ...] [-protected number]

Saves EMS sequence. All parameters after \fB-unread\fR (like \fB-defsound\fR) can be used few times.

\fB-text\fR - adds text

\fB-unicodefiletext\fR - adds text from Unicode file

\fB-defanimation\fR - adds default animation with ID specified by user.ID for different phones are different.

\fB-animation\fR - adds "frames" frames read from file1, file2, etc.

\fB-defsound\fR - adds default sound with ID specified by user. ID for different phones are different.

\fB-tone10\fR - adds IMelody version 1.0 read from RTTL or other compatible file

\fB-tone10long\fR - IMelody version 1.0 saved in one of few SMS with UPI. Phones compatible with UPI (like SonyEricsson phones) will read such ringtone as one

\fB-tone12\fR - adds IMelody version 1.2 read from RTTL or other compatible file

\fB-tone12long\fR - IMelody version 1.2 saved in one of few SMS with UPI. Phones compatible with UPI (like SonyEricsson phones) will read such ringtone as one

\fB-toneSE\fR - adds IMelody in "short" form supported by SonyEricsson phones

\fB-toneSElong\fR - add SonyEricsson IMelody saved in one or few SMS with UPI

\fB-variablebitmap\fR - bitmap in any size saved in one SMS

\fB-variablebitmaplong\fR - bitmap with maximal size 96x128 saved in one or few sms

\fB-fixedbitmap\fR - bitmap 16x16 or 32x32

\fB-protected\fR - all ringtones and bitmaps after this parameter (excluding default ringtones and logos) will be "protected" (in phones compatible with ODI like SonyEricsson products it won't be possible to forward them from phone menu)

\fB-16bit\fR - Gammu uses SMS headers with 16-bit numbers for saving linking info in SMS (it means less chars available for user in each SMS)

\fB-format\fR lcrasbiut - last text will be formatted. You can use combinations of chars:
    l - left aligned
    c - centered
    r - right aligned
    a - large font
    s - small font
    b - bold font
    i - italic font
    u - underlined font
    t - strikethrough font

\fIExample:\fR gammu savesms EMS -text "Greetings" -defsound 1 -text "from Gammu" -tone10 axelf.txt -animation 2 file1.bmp file2.bmp

will create EMS sequence with text "Greetings" and default sound 1 and text "from Gammu" and ringtone axelf.txt and 2 frame animation read from (1'st frame) file1.bmp and (2'nd frame) file2.bmp

\fIExample:\fR gammu savesms EMS -protected 2 -variablebitmaplong ala.bmp -toneSElong axelf.txt -toneSE ring.txt

ala.bmp and axelf.txt will be "protected


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms MMSINDICATOR URL Title Sender [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]

Saves a message with MMS indication. The recipient phone will then download
MMS from given URL and display it.

Please note that you should provide valid smil data on that URL.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms MMSSETTINGS file location  [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Saves a message with MMS configuration. The
configuration will be read from Gammu backup file from given location.

For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms OPERATOR file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-netcode netcode] [-biglogo]
Save operator logo as sms in Nokia (Smart
Messaging) format - size 72x14 or 78x21 after using \fB-biglogo\fR, all in
two colors.

Please note, that it isn't designed for colour logos available for example in
DCT4/TIKU - you need to put bitmap file there inside phone using filesystem
commands.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms PICTURE file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-text text] [-unicode] [-alcatelbmmi]

Read bitmap from 2 colors file (bmp, nlm, nsl, ngg, nol, wbmp, etc.), format
into bitmap in Smart Messaging (72x28, 2 colors, called often Picture Image
and saved with text) or Alcatel format and send/save over SMS.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms PROFILE [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-name name] [-bitmap bitmap] [-ringtone ringtone]

Read ringtone (RTTL) format, bitmap (Picture Image size) and name, format into
Smart Messaging profile and send/save as SMS. Please note, that this format is
abandomed by Nokia and supported by some (older) devices only like Nokia 3310.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms RINGTONE file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-long] [-scale]
Read RTTL ringtone from file and save as SMS
into SIM/phone memory. Ringtone is saved in Nokia (Smart Messaging) format.

\fB-long\fR - ringtone is saved using Profile style. It can be longer (and saved
in 2 SMS), but decoded only by newer phones (like 33xx)

\fB-scale\fR - ringtone will have Scale info for each note. It will allow to edit
it correctly later in phone composer (for example, in 33xx)


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms SMSTEMPLATE [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-unicode] [-text text] [-unicodefiletext file] [-defsound ID] [-defanimation ID] [-tone10 file] [-tone10long file] [-tone12 file] [-tone12long file] [-toneSE file] [-toneSElong file] [-variablebitmap file] [-variablebitmaplong file] [-animation frames file1 ...]
Saves a SMS template (for Alcatel phones).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms TEXT [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-inputunicode] [-16bit] [-flash] [-len len] [-autolen len] [-unicode] [-enablevoice] [-disablevoice] [-enablefax] [-disablefax] [-enableemail] [-disableemail] [-voidsms] [-replacemessages ID] [-replacefile file] [-text msgtext] [-textutf8 msgtext]
Take text from stdin (or commandline if -text 
specified) and save as text SMS into SIM/phone memory.

\fB-flash\fR - Class 0 SMS (should be displayed after receiving on recipients' phone display after receiving without entering Inbox)

\fB-len len\fR - specify, how many chars will be read. When use this
option and text will be longer than 1 SMS, will be split into more
linked SMS

\fB-autolen len\fR - specify, how many chars will be read. When use this
option and text will be longer than 1 SMS, will be split into more
linked SMS.Coding type (SMS default alphabet/Unicode) is set according
to input text

\fB-enablevoice\fR | \fB-disablevoice\fR | \fB-enablefax \fR |
\fB-disablefax \fR | \fB-enableemail \fR | \fB-disableemail \fR -
sms will set such indicators. Text will be cut to 1 sms.

\fB-voidsms\fR - many phones after receiving it won't display anything,
only beep, vibrate or turn on light. Text will be cut to 1 sms.

\fB-unicode\fR - SMS will be saved in Unicode format

\fB-inputunicode\fR - input text is in Unicode.

\fB-text\fR - get text from command line instead of stdin.

\fB-textutf8\fR - get text in UTF-8 from command line instead of stdin.

\fB-16bit\fR - Gammu uses SMS headers with 16-bit numbers for saving linking info in SMS (it means less chars available for user in each SMS)

\fITIP:\fR
You can create Unicode file using WordPad in Windows (during saving select
"Unicode Text Document" format). In Unix can use for example YUdit.

\fB-replacemessages ID\fR - \fBID\fR can be 1..7. When you will use option and
send more single SMS to one recipient with the same ID, each another SMS will
replace each previous with the same ID

\fB-replacefile file\fR  - when you want, you can make file in such format:
\fBsrc_unicode_char1, dest_unicode_char1, src_unicode_char2, dest_unicode_char2\fR
(everything in one line). After reading text for SMS from stdin there will
be made translation and each src char will be converted to dest char. In docs
there is example file (\fIreplace.txt\fR), which will change all "a" chars to "1

\fITIP:\fR when use ~ char in sms text and \fB-unicode\fR option
(Unicode coding required), text of sms after ~ char will blink in some phones
(like N33xx)

\fIExample:\fR echo some_text | gammu savesms TEXT


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms TODO file location [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Saves a message with a todo entry. The content will
be read from any backup format which Gammu supports and from given location.

For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms VCARD10|VCARD21 file SM|ME location [-nokia] [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read phonebook entry from file created by \fBbackup\fR option and saves in
VCARD 1.0 (only name and default number) or VCARD 2.1 (all entry details with
all numbers, text and name) format as SMS. The location identifies position of contact item 
to be read in backup file (usually 1, but can be useful in case the backup contains 
more items).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms WAPINDICATOR URL Title [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Saves a SMS with a WAP indication for given
URL and title.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms WAPSETTINGS file location DATA|GPRS [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read WAP settings from file created by \fBbackup\fR option and saves in Nokia format as SMS


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms ANIMATION destination frames file1 file2... [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms ANIMATION\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms BOOKMARK destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms BOOKMARK\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms CALENDAR destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms CALENDAR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms CALLER destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms CALLER\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms EMS destination [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-unicode] [-16bit] [-format lcrasbiut] [-text text] [-unicodefiletext file] [-defsound ID] [-defanimation ID] [-tone10 file] [-tone10long file] [-tone12 file] [-tone12long file] [-toneSE file] [-toneSElong file] [-fixedbitmap file] [-variablebitmap file] [-variablebitmaplong file] [-animation frames file1 ...] [-protected number]
Sends a message, for description of message specific parameters see \fBsavesms EMS\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms MMSINDICATOR destination URL Title Sender [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms MMSINDICATOR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms MMSSETTINGS destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms MMSSETTINGS\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms OPERATOR destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-netcode netcode] [-biglogo]
Sends a message, for description of message specific parameters see \fBsavesms OPERATOR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms PICTURE destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-text text] [-unicode] [-alcatelbmmi]
Sends a message, for description of message specific parameters see \fBsavesms PICTURE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms PROFILE destination [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-name name] [-bitmap bitmap] [-ringtone ringtone]
Sends a message, for description of message specific parameters see \fBsavesms PROFILE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms RINGTONE destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-long] [-scale]
Sends a message, for description of message specific parameters see \fBsavesms RINGTONE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms SMSTEMPLATE destination [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-unicode] [-text text] [-unicodefiletext file] [-defsound ID] [-defanimation ID] [-tone10 file] [-tone10long file] [-tone12 file] [-tone12long file] [-toneSE file] [-toneSElong file] [-variablebitmap file] [-variablebitmaplong file] [-animation frames file1 ...]
Sends a message, for description of message specific parameters see \fBsavesms SMSTEMPLATE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms TEXT destination [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-inputunicode] [-16bit] [-flash] [-len len] [-autolen len] [-unicode] [-enablevoice] [-disablevoice] [-enablefax] [-disablefax] [-enableemail] [-disableemail] [-voidsms] [-replacemessages ID] [-replacefile file] [-text msgtext] [-textutf8 msgtext]
Sends a message, for description of message specific parameters see \fBsavesms TEXT\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms TODO destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms TODO\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms VCARD10|VCARD21 destination file SM|ME location [-nokia] [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms VCARD10|VCARD21\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms WAPINDICATOR destination URL Title [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms WAPINDICATOR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms WAPSETTINGS destination file location DATA|GPRS [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms WAPSETTINGS\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --setsmsc location number
Set SMSC settings on SIM card. This keeps all SMSC configuration
intact, it just changes the SMSC number.

Locations are numerated from 1.


.SS "Memory (phonebooks and calls)
.TP
.. option:: --addcategory TODO|PHONEBOOK text

.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --deleteallmemory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL
Deletes all entries from specified memory type.
.TP
.. option:: --deletememory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL start [stop]
Deletes entries in specified range from specified memory type.
.TP
.. option:: --getallcategory TODO|PHONEBOOK

.TP
.. option:: --getallmemory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL
Get all memory locations from phone. For memory
types see \fBgetmemory\fR.
.TP
.. option:: --getcategory TODO|PHONEBOOK start [stop]

.TP
.. option:: --getmemory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL start [stop [-nonempty]]
Get memory location from phone. 

Locations are numerated from 1.

\fBDC\fR = Dialled calls
.br
\fBMC\fR = Missed calls
.br
\fBRC\fR = Received calls
.br
\fBON\fR = Own numbers
.br
\fBVM\fR = voice mailbox
.br
\fBSM\fR = SIM phonebook
.br
\fBME\fR = phone internal phonebook
.br
\fBFD\fR = fixed dialling
.br
\fBSL\fR = sent SMS log

.TP
.. option:: --getspeeddial start [stop]
Gets speed dial choices.

.TP
.. option:: --listmemorycategory text|number

.TP
.. option:: --nokiagetpbkfeatures memorytype

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.

.TP
.. option:: --savefile VCARD10|VCARD21 target.vcf file SM|ME location
Allows to convert between various backup formats which gammu
supports for phonebook events. The file type is guessed (for input file
guess is based on extension and file content, for output solely on 
extension).

For example if you want to convert single entry from gammu native 
backup to vCard, you need following command:

    gammu savefile CALENDAR output.vcf myPhonebook.backup ME 42

.TP
.. option:: --savesms VCARD10|VCARD21 file SM|ME location [-nokia] [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read phonebook entry from file created by \fBbackup\fR option and saves in
VCARD 1.0 (only name and default number) or VCARD 2.1 (all entry details with
all numbers, text and name) format as SMS. The location identifies position of contact item 
to be read in backup file (usually 1, but can be useful in case the backup contains 
more items).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --searchmemory text
Scans all memory entries for given text. It performs
case insensitive substring lookup. You can interrupt searching by pressing
Ctrl+C.
.TP
.. option:: --sendsms VCARD10|VCARD21 destination file SM|ME location [-nokia] [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms VCARD10|VCARD21\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.



.SS "Filesystem
.P

Gammu allows to access phones using native protocol (Nokias) or OBEX. Your
phone can also support usb storage, which is handled on the operating system
level and Gammu does not use that.

.TP
.. option:: --addfile folderID name [-type JAR|BMP|PNG|GIF|JPG|MIDI|WBMP|AMR|3GP|NRT] [-readonly] [-protected] [-system] [-hidden] [-newtime]
Add file with specified name to folder with specified folder ID.

.B -type 
- file type was required for filesystem 1 in Nokia phones (current filesystem 2 doesn't need this)

.B -readonly, -protected, -system, -hidden
- you can set readonly, protected (file can't be for example forwarded from phone menu), system and hidden (file is hidden from phone menu) file attributes

.B -newtime 
- after using it date/time of file modification will be set to moment of uploading 


.TP
.. option:: --addfolder parentfolderID name
Create a folder in phone with specified name in a
folder with specified folder ID.
.TP
.. option:: --deletefiles fileID
Delete files with given IDs.
.TP
.. option:: --deletefolder name
Delete folder with given ID.
.TP
.. option:: --getfilefolder fileID, fileID, ...
Retrieve files or all files from folder with given IDs from a phone filesytem.
.TP
.. option:: --getfiles fileID, fileID, ...
Retrieve files with given IDs from a phone filesytem.
.TP
.. option:: --getfilesystem [-flatall|-flat]
Display info about all folders and files in phone memory/memory card. By default there is tree displayed, you can change it:

.B -flatall
there are displayed full file/folder details like ID (first parameter in line)

.B -flat

Please note, that in some phones (like N6230) content of some folders (with more files) can be cut (only part of files will be displayed) for example on infrared connection. This is not Gammu issue, but phone firmware problem.

.TP
.. option:: --getfilesystemstatus
Display info filesystem status - number of
bytes available, used or used by some specific content.
.TP
.. option:: --getfolderlisting folderID

Display files and folders available in folder with given folder ID. You can get ID's using getfilesystem -flatall.

Please note, that in some phones (like N6230) content of some folders (with more files) can be cut (only part of files will be displayed) for example on infrared connection. This is not Gammu issue, but phone firmware problem.

.TP
.. option:: --getrootfolders

Display info about drives available in phone/memory card.

.TP
.. option:: --nokiaaddfile APPLICATION|GAME file [-readonly] [-overwrite] [-overwriteall]
Install the *.jar/*.jad file pair of a midlet in the application or game menu of the phone. You need to specify filename without the jar/jad suffix, both will be added automatically.

The option
.I -overwrite
deletes the application's .jad and .jar files bevor installing, but doesn't delete the application data. Option
.I -overwriteall
will also delete all data. Both these options work only for Application or Game upload.

\fIExample:\fR gammu nokiaaddfile Application Alien will read Alien.JAD and Alien.JAR and add to Applications

\fIExample:\fR gammu nokiaaddfile Tones file.mid will read file.mid and add to Tones folder

You can use jadmaker(1) to generate a .jad file from a .jar file.

.TP
.. option:: --nokiaaddfile GALLERY|GALLERY2|CAMERA|TONES|TONES2|RECORDS|VIDEO|PLAYLIST|MEMORYCARD file [-name name] [-protected] [-readonly] [-system] [-hidden] [-newtime]

.TP
.. option:: --nokiaaddplaylists

Goes through phone memory and generated playlist for all music files found.

To manually manage playlists:

    gammu addfile a:\\predefplaylist filename.m3u

Will add playlist filename.m3u

    gammu getfilesystem

Will get list of all files (including names of files with playlists)

    gammu deletefiles a:\\predefplaylist\\filename.m3u

Will delete playlist filename.m3u

Format of m3u playlist is easy (standard mp3 playlist):

First line is #EXTM3U, next lines contain  names of files (b:\\file1.mp3,
b:\\folder1\\file2.mp3, etc.). File needs t have '\\r\\n' terminated lines. So
just run unix2dos on the resulting file before uploading it your your phone.

.TP
.. option:: --sendfile name
Sends file to a phone. It's up to phone to decide where
to store this file and how to handle it (for example when you send vCard or
vCalendar, most of phones will offer you to import it.

.TP
.. option:: --setfileattrib folderID [-system] [-readonly] [-hidden] [-protected]


.SS "Logo and pictures
.P

These options are mainly (there are few exceptions) for monochromatic logos and
images available in older phones. Recognized file formats: xpm (only saving),
2-colors bmp, nlm, nsl, ngg, nol, wbmp, gif (for Samsung).

In new models all bitmaps are saved in filesystem and should go into filesystem section

.TP
.. option:: --copybitmap inputfile [outputfile [OPERATOR|PICTURE|STARTUP|CALLER]]
Allow to convert logos files to another. When give ONLY inputfile, output will
be written to stdout using ASCII art. When give output file and format, in
some file formats (like NLM) will be set indicator informing about logo type
to given.

.TP
.. option:: --getbitmap CALLER location [file]
Get caller group logo from phone. Locations 1-5.

.TP
.. option:: --getbitmap DEALER
In some models it's possible to save dealer welcome note - text displayed
during enabling phone, which can't be edited from phone menu.  Here you can
get it.

.TP
.. option:: --getbitmap OPERATOR [file]
Get operator logo (picture displayed instead of operator name) from phone.

.TP
.. option:: --getbitmap PICTURE location [file]
Get Picture Image from phone.

.TP
.. option:: --getbitmap STARTUP [file]
Get static startup logo from phone. Allow to save it in file.

.TP
.. option:: --getbitmap TEXT
Get startup text from phone.

.TP
.. option:: --savesms ANIMATION frames file1 file2... [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Save an animation as a SMS. You need to give
number of frames and picture for each frame. Each picture can be in any
picture format which Gammu supports (B/W bmp, gif, wbmp, nol, nlm...).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms CALLER file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Save caller logo as sms in Nokia (Smart Messaging)
format - size 72x14, two colors.

Please note, that it isn't designed for colour logos available for example in
DCT4/TIKU - you need to put bitmap file there inside phone using filesystem
commands.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms OPERATOR file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-netcode netcode] [-biglogo]
Save operator logo as sms in Nokia (Smart
Messaging) format - size 72x14 or 78x21 after using \fB-biglogo\fR, all in
two colors.

Please note, that it isn't designed for colour logos available for example in
DCT4/TIKU - you need to put bitmap file there inside phone using filesystem
commands.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms PICTURE file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-text text] [-unicode] [-alcatelbmmi]

Read bitmap from 2 colors file (bmp, nlm, nsl, ngg, nol, wbmp, etc.), format
into bitmap in Smart Messaging (72x28, 2 colors, called often Picture Image
and saved with text) or Alcatel format and send/save over SMS.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms ANIMATION destination frames file1 file2... [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms ANIMATION\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms CALLER destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms CALLER\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms OPERATOR destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-netcode netcode] [-biglogo]
Sends a message, for description of message specific parameters see \fBsavesms OPERATOR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms PICTURE destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-text text] [-unicode] [-alcatelbmmi]
Sends a message, for description of message specific parameters see \fBsavesms PICTURE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --setbitmap CALLER location [file]
Set caller logo.
.TP
.. option:: --setbitmap COLOUROPERATOR [fileID [netcode]]
Sets color operator logo in phone.
.TP
.. option:: --setbitmap COLOURSTARTUP [fileID]

.TP
.. option:: --setbitmap DEALER text
Sets welcome message configured by dealer, which usually can not be changed in phone menus.
.TP
.. option:: --setbitmap OPERATOR [file [netcode]]
Set operator logo in phone. When won't give file and netcode, operator logo
will be removed from phone. When will give only filename, operator logo will
be displayed for your current GSM operator. When give additionally network
code, it will be displayed for this operator: \fBgammu setbitmap OPERATOR file "260 02"\fR

.TP
.. option:: --setbitmap PICTURE file location [text]
Sets picture image in phone.
.TP
.. option:: --setbitmap STARTUP file|1|2|3
Set startup logo in phone. It can be static (then you will have to give file
name) or one of predefined animated (only some phones like Nokia 3310 or 3330
supports it, use location 1, 2 or 3 for these).

.TP
.. option:: --setbitmap TEXT text
Sets startup text in phone.
.TP
.. option:: --setbitmap WALLPAPER fileID
Sets wallpaper in phone.

.SS "Ringtones
.P

Ringtones are mostly supported only for older phones. For recent phones you
usually just upload them to some folder in phone filesystem.

There are recognized various file formats by options described below: rttl,
binary format created for Gammu, mid (saving), re (reading), ott, communicator,
ringtones format found in fkn.pl, wav (saving), ime/imy (saving), rng, mmf (for
Samsung).


.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --copyringtone source destination [RTTL|BINARY]
Copy source ringtone to destination.

.TP
.. option:: --getphoneringtone location [file]

Get one of "default" ringtones and saves into file

.TP
.. option:: --getringtone location [file]
Get ringtone from phone in RTTL or BINARY format. 

Locations are numerated from 1.

.TP
.. option:: --getringtoneslist

.TP
.. option:: --nokiacomposer file
Show, how to enter RTTL ringtone in composer existing in many Nokia phones
(and how should it look like).

.TP
.. option:: --playringtone file
Play aproximation of ringtone over phone buzzer. File can be in RTTL or BINARY (Nokia DCT3) format.

.TP
.. option:: --playsavedringtone number

Play one of "built" ringtones. This option is available for DCT4 phones. For getting ringtones list use getringtoneslist.

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.

.TP
.. option:: --savesms RINGTONE file [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-long] [-scale]
Read RTTL ringtone from file and save as SMS
into SIM/phone memory. Ringtone is saved in Nokia (Smart Messaging) format.

\fB-long\fR - ringtone is saved using Profile style. It can be longer (and saved
in 2 SMS), but decoded only by newer phones (like 33xx)

\fB-scale\fR - ringtone will have Scale info for each note. It will allow to edit
it correctly later in phone composer (for example, in 33xx)


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms RINGTONE destination file [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-long] [-scale]
Sends a message, for description of message specific parameters see \fBsavesms RINGTONE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --setringtone file [-location location] [-scale] [-name name]
Set ringtone in phone. When don't give location, it will be written
"with preview" (in phones supporting this feature like 61xx or 6210).
When use RTTL ringtones, give location and use \fB-scale\fR, there will be written
scale info with each note. It will avoid scale problems available during
editing ringtone in composer from phone menu (for example, in N33xx).

\fITIP:\fR when use ~ char in ringtone name, in some phones (like 
33xx) name will blink later in phone menus


.SS "Calendar notes
.P

In Nokia 3310, 3315 and 3330 these are named "Reminders" and have some limitations (depending on phone firmware version).

.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --deletecalendar start [stop]
Deletes selected calendar entries in phone. 
.TP
.. option:: --getallcalendar
Retrieves all calendar entries from phone.
.TP
.. option:: --getcalendar start [stop]
Retrieves selected calendar entries from phone.
.TP
.. option:: --getcalendarsettings
Displays calendar settings like first day of
week or automatic deleting of old entries.

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.

.TP
.. option:: --savefile CALENDAR target.vcs file location
Allows to convert between various backup formats which gammu
supports for calendar events. The file type is guessed (for input file
guess is based on extension and file content, for output solely on 
extension).

For example if you want to convert single entry from gammu native 
backup to vCalendar, you need following command:

    gammu savefile CALENDAR output.vcs myCalendar.backup 260

.TP
.. option:: --savesms CALENDAR file location [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read calendar note from file created by \fBbackup\fR option and saves in
VCALENDAR 1.0 format as SMS. The location identifies position of calendar item 
to be read in backup file (usually 1, but can be useful in case the backup contains 
more items).


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms CALENDAR destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms CALENDAR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.



.SS "To do lists
.TP
.. option:: --addcategory TODO|PHONEBOOK text

.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --deletetodo start [stop]
Deletes selected todo entries in phone. 
.TP
.. option:: --getallcategory TODO|PHONEBOOK

.TP
.. option:: --getalltodo
Retrieves all todo entries from phone.
.TP
.. option:: --getcategory TODO|PHONEBOOK start [stop]

.TP
.. option:: --gettodo start [stop]
Retrieves selected todo entries from phone.
.TP
.. option:: --listtodocategory text|number

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.

.TP
.. option:: --savefile TODO target.vcs file location
Allows to convert between various backup formats which gammu
supports for todo events. The file type is guessed (for input file
guess is based on extension and file content, for output solely on 
extension).

For example if you want to convert single entry from gammu native 
backup to vCalendar, you need following command:

    gammu savefile CALENDAR output.vcs myCalendar.backup 260

.TP
.. option:: --savesms TODO file location [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Saves a message with a todo entry. The content will
be read from any backup format which Gammu supports and from given location.

For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms TODO destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms TODO\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.



.SS "Notes
.TP
.. option:: --getallnotes
Reads all notes from the phone.

Note: Not all phones supports this function, especially most Sony Ericsson 
phones even if they have notes inside phone.


.SS "Date, time and alarms
.TP
.. option:: --getalarm [start]
Get alarm from phone, if no location is specified,
1 is used.

.TP
.. option:: --getdatetime
Get date and time from phone

.TP
.. option:: --setalarm hour minute
Sets repeating alarm in phone on selected time.
.TP
.. option:: --setdatetime [HH:MM[:SS]] [YYYY/MM/DD]
Set date and time in phone to date and time set in computer. Please 
note, that this option doesn't show clock on phone screen. It only set
date and time.

\fITIP:\fR you can make such synchronization each time, when will connect
your phone and use Gammu. See gammurc(5) for details.


.SS "Categories
.TP
.. option:: --addcategory TODO|PHONEBOOK text

.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --getallcategory TODO|PHONEBOOK

.TP
.. option:: --getcategory TODO|PHONEBOOK start [stop]

.TP
.. option:: --listmemorycategory text|number

.TP
.. option:: --listtodocategory text|number

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.


.SS "Backing up and restoring
.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --addsms folder file [-yes]
Adds SMSes from file (format like backupsms uses) to
selected folder in phone.
.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --backupsms file [-yes|-all]
Stores all SMSes from phone to file. 

Use -yes for answering yes to all questions (backup all messages and 
delete them from phone), or -all to just backup all folders while keeping
messages in phone.

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.

.TP
.. option:: --restoresms file [-yes]
Restores SMSes from file (format like backupsms uses) to
selected folder in phone. Please note that this overwrites existing
messages in phone (if it supports it).
.TP
.. option:: --savefile BOOKMARK target.url file location
Converts backup format supported by
Gammu to vBookmark file.
.TP
.. option:: --savefile CALENDAR target.vcs file location
Allows to convert between various backup formats which gammu
supports for calendar events. The file type is guessed (for input file
guess is based on extension and file content, for output solely on 
extension).

For example if you want to convert single entry from gammu native 
backup to vCalendar, you need following command:

    gammu savefile CALENDAR output.vcs myCalendar.backup 260

.TP
.. option:: --savefile TODO target.vcs file location
Allows to convert between various backup formats which gammu
supports for todo events. The file type is guessed (for input file
guess is based on extension and file content, for output solely on 
extension).

For example if you want to convert single entry from gammu native 
backup to vCalendar, you need following command:

    gammu savefile CALENDAR output.vcs myCalendar.backup 260

.TP
.. option:: --savefile VCARD10|VCARD21 target.vcf file SM|ME location
Allows to convert between various backup formats which gammu
supports for phonebook events. The file type is guessed (for input file
guess is based on extension and file content, for output solely on 
extension).

For example if you want to convert single entry from gammu native 
backup to vCard, you need following command:

    gammu savefile CALENDAR output.vcf myPhonebook.backup ME 42


.SS "Nokia specific
.TP
.. option:: --nokiaaddfile APPLICATION|GAME file [-readonly] [-overwrite] [-overwriteall]
Install the *.jar/*.jad file pair of a midlet in the application or game menu of the phone. You need to specify filename without the jar/jad suffix, both will be added automatically.

The option
.I -overwrite
deletes the application's .jad and .jar files bevor installing, but doesn't delete the application data. Option
.I -overwriteall
will also delete all data. Both these options work only for Application or Game upload.

\fIExample:\fR gammu nokiaaddfile Application Alien will read Alien.JAD and Alien.JAR and add to Applications

\fIExample:\fR gammu nokiaaddfile Tones file.mid will read file.mid and add to Tones folder

You can use jadmaker(1) to generate a .jad file from a .jar file.

.TP
.. option:: --nokiaaddfile GALLERY|GALLERY2|CAMERA|TONES|TONES2|RECORDS|VIDEO|PLAYLIST|MEMORYCARD file [-name name] [-protected] [-readonly] [-system] [-hidden] [-newtime]

.TP
.. option:: --nokiaaddplaylists

Goes through phone memory and generated playlist for all music files found.

To manually manage playlists:

    gammu addfile a:\\predefplaylist filename.m3u

Will add playlist filename.m3u

    gammu getfilesystem

Will get list of all files (including names of files with playlists)

    gammu deletefiles a:\\predefplaylist\\filename.m3u

Will delete playlist filename.m3u

Format of m3u playlist is easy (standard mp3 playlist):

First line is #EXTM3U, next lines contain  names of files (b:\\file1.mp3,
b:\\folder1\\file2.mp3, etc.). File needs t have '\\r\\n' terminated lines. So
just run unix2dos on the resulting file before uploading it your your phone.

.TP
.. option:: --nokiacomposer file
Show, how to enter RTTL ringtone in composer existing in many Nokia phones
(and how should it look like).

.TP
.. option:: --nokiadebug filename [[v11-22] [,v33-44]...]

.TP
.. option:: --nokiadisplayoutput

.TP
.. option:: --nokiadisplaytest number

.TP
.. option:: --nokiagetadc

.TP
.. option:: --nokiagetoperatorname
6110.c phones have place for name for one GSM network (of course, with flashing it's
possible to change all names, but Gammu is not flasher ;-)). You can get this
name using this option.

.TP
.. option:: --nokiagetpbkfeatures memorytype

.TP
.. option:: --nokiagetscreendump

.TP
.. option:: --nokiagett9
This option should display T9 dictionary content from
DCT4 phones.

.TP
.. option:: --nokiagetvoicerecord location

Get voice record from location and save to WAV file. File is
coded using GSM 6.10 codec (available for example in win32). Name
of file is like name of voice record in phone.

Created WAV files require GSM 6.10 codec to be played. In Win XP it's included
by Microsoft. If you deleted it by accident in this operating system, make such
steps:

1. Control Panel
.br
2. Add hardware
.br
3. click Next
.br
4. select "Yes. I have already connected the hardware
.br
5. select "Add a new hardware device
.br
6. select "Install the hardware that I manually select from a list
.br
7. select "Sound, video and game controllers
.br
8. select "Audio codecs
.br
9.  select "windows\\system32" directory and file "mmdriver.inf
.br
10. if You will be asked for file msgsm32.acm, it should unpacked from Windows CD
.br
11. now You can be asked if want to install unsigned driver (YES), about select codec configuration (select what you want) and rebotting PC (make it)

.TP
.. option:: --nokiamakecamerashoot

.TP
.. option:: --nokianetmonitor test
Takes output or set netmonitor for Nokia DCT3 phones.

\fITIP:\fR For more info about this option, please visit
\fIhttp://www.mwiacek.com\fR and read netmonitor manual.

\fITIP:\fR test \fB243\fR enables all tests (after using command
\fBgammu nokianetmonitor 243\fR in some phones like 6210 or 9210 have to
reboot them to see netmonitor menu)

.TP
.. option:: --nokianetmonitor36
Reset counters from netmonitor test 36 in Nokia DCT3 phones.

\fITIP:\fR For more info about this option, please visit
\fIhttp://www.mwiacek.com\fR and read netmonitor manual.

.TP
.. option:: --nokiasecuritycode
Get/reset to "12345" security code

.TP
.. option:: --nokiaselftests

Perform tests for Nokia DCT3 phones.

Note: EEPROM test can show an error when your phone has an EEPROM in 
flash (like 82xx/7110/62xx/33xx). The clock test will show an error 
when the phone doesn?t have an internal battery for the clock (like
3xxx).

.TP
.. option:: --nokiasetlights keypad|display|torch on|off

.TP
.. option:: --nokiasetoperatorname [networkcode name]

.TP
.. option:: --nokiasetphonemenus
Enable all (?) possible menus for DCT3 Nokia phones:

1. ALS (Alternative Line Service) option menu
2. vibra menu for 3210
3. 3315 features in 3310 5.45 and higher
4. two additional games (React and Logic) for 3210 5\.31 and higher
5. WellMate menu for 6150
6. NetMonitor

and for DCT4:

1. ALS (Alternative Line Service) option menu
2. Bluetooth, WAP bookmarks and settings menu, ... (6310i)
3. GPRS Always Online
4. ...

.TP
.. option:: --nokiasetvibralevel level
Set vibra power to "level" (given in percent)

.TP
.. option:: --nokiatuneradio

.TP
.. option:: --nokiavibratest


.SS "Siemens specific
.TP
.. option:: --siemensnetmonact netmon_type

Enables network monitor in Siemens phone. Currently known values for type
are 1 for full and 2 for simple mode.
.TP
.. option:: --siemensnetmonitor test

.TP
.. option:: --siemenssatnetmon


.SS "Network
.TP
.. option:: --getgprspoint start [stop]

.TP
.. option:: --listnetworks [country]
Show names/codes of GSM networks known for Gammu

.TP
.. option:: --monitor [times]
Get phone status and writes continuously to standard output. Press Ctrl+C
to break this state.

.TP
.. option:: --networkinfo
Show information about network status from the phone.

.TP
.. option:: --nokiadebug filename [[v11-22] [,v33-44]...]

.TP
.. option:: --nokiagetoperatorname
6110.c phones have place for name for one GSM network (of course, with flashing it's
possible to change all names, but Gammu is not flasher ;-)). You can get this
name using this option.

.TP
.. option:: --nokianetmonitor test
Takes output or set netmonitor for Nokia DCT3 phones.

\fITIP:\fR For more info about this option, please visit
\fIhttp://www.mwiacek.com\fR and read netmonitor manual.

\fITIP:\fR test \fB243\fR enables all tests (after using command
\fBgammu nokianetmonitor 243\fR in some phones like 6210 or 9210 have to
reboot them to see netmonitor menu)

.TP
.. option:: --nokiasetoperatorname [networkcode name]

.TP
.. option:: --setautonetworklogin

.TP
.. option:: --siemensnetmonact netmon_type

Enables network monitor in Siemens phone. Currently known values for type
are 1 for full and 2 for simple mode.
.TP
.. option:: --siemensnetmonitor test

.TP
.. option:: --siemenssatnetmon


.SS "WAP settings and bookmarks
.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --deletewapbookmark start [stop]
Delete WAP bookmarks from phone. 

Locations are numerated from 1.

.TP
.. option:: --getchatsettings start [stop]

.TP
.. option:: --getsyncmlsettings start [stop]

.TP
.. option:: --getwapbookmark start [stop]
Get WAP bookmarks from phone. 

Locations are numerated from 1.

.TP
.. option:: --getwapsettings start [stop]
Get WAP settings from phone. 

Locations are numerated from 1.

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.

.TP
.. option:: --savefile BOOKMARK target.url file location
Converts backup format supported by
Gammu to vBookmark file.
.TP
.. option:: --savesms BOOKMARK file location [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read WAP bookmark from file created by \fBbackup\fR option and saves in
Nokia format as SMS


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms WAPINDICATOR URL Title [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Saves a SMS with a WAP indication for given
URL and title.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms WAPSETTINGS file location DATA|GPRS [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Read WAP settings from file created by \fBbackup\fR option and saves in Nokia format as SMS


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms BOOKMARK destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms BOOKMARK\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms WAPINDICATOR destination URL Title [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms WAPINDICATOR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms WAPSETTINGS destination file location DATA|GPRS [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms WAPSETTINGS\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.



.SS "MMS and MMS settings
.TP
.. option:: --getallmms [-save]

.TP
.. option:: --geteachmms [-save]

.TP
.. option:: --getmmsfolders

.TP
.. option:: --getmmssettings start [stop]

.TP
.. option:: --readmmsfile file [-save]

.TP
.. option:: --savesms MMSINDICATOR URL Title Sender [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]

Saves a message with MMS indication. The recipient phone will then download
MMS from given URL and display it.

Please note that you should provide valid smil data on that URL.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --savesms MMSSETTINGS file location  [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Saves a message with MMS configuration. The
configuration will be read from Gammu backup file from given location.

For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms MMSINDICATOR destination URL Title Sender [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms MMSINDICATOR\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms MMSSETTINGS destination file location [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]
Sends a message, for description of message specific parameters see \fBsavesms MMSSETTINGS\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.



.SS "Phone tests
.TP
.. option:: --nokiadisplaytest number

.TP
.. option:: --nokiagetadc

.TP
.. option:: --nokiaselftests

Perform tests for Nokia DCT3 phones.

Note: EEPROM test can show an error when your phone has an EEPROM in 
flash (like 82xx/7110/62xx/33xx). The clock test will show an error 
when the phone doesn?t have an internal battery for the clock (like
3xxx).

.TP
.. option:: --nokiasetlights keypad|display|torch on|off

.TP
.. option:: --nokiavibratest


.SS "FM radio
.TP
.. option:: --addnew file [-yes] [-memory ME|SM|..]
Adds data written in file created using \fBbackup option\fR. All things
backup'ed by \fBbackup\fR can be restored (when made backup to Gammu text
file).

Please note that this adds all content of backup file to phone and
does not care about current data in the phone (no duplicates are 
detected).

Use -yes parameter to answer yes to all questions (you want to automatically 
restore all data).

Use -memory parameter to force usage of defined memory type for storing entries 
regardless what backu format says.

.TP
.. option:: --backup file [-yes]
Backup your phone to file. It's possible to backup (depends on phone and backup format):

* phonebook from SIM and phone memory
* calendar notes
* SMSC settings
* operator logo
* startup (static) logo or startup text
* WAP bookmarks
* WAP settings
* caller logos and groups
* user ringtones

There are various backup formats supported and the backup format is guessed
based on file extension:

* .lmb - Nokia backup, supports contacts, caller logos and startup logo.
* .vcs - vCalendar, supports calendar and todo.
* .vcf - vCard, supports contacts.
* .ldif - LDAP import, supports contacts.
* .ics - iCalendar, supports calendar and todo.
* Any other extension is Gammu backup file and it supports all data mentioned above.

See \fBEXAMPLE\fR section for some hints on manipulating Gammu backup format.


.TP
.. option:: --clearall
Deletes all private data from the phone.

.TP
.. option:: --getfmstation start [stop]
Show info about FM stations in phone

.TP
.. option:: --nokiatuneradio

.TP
.. option:: --restore file [-yes]
Restore settings written in file created using \fBbackup\fR option. 

Please note that restoring deletes all current content in phone. If you
want only to add entries to phone, use addnew.

In some phones restoring calendar notes will not show error, but won't
be done, when phone doesn't have set clock inside.


.SS "Phone information
.TP
.. option:: --battery
Displays information about battery and power source.

.TP
.. option:: --getdisplaystatus

.TP
.. option:: --getlocation
Gets network information from phone (same as networkinfo)
and prints location (latitude and longitude) based on information from 
OpenCellID <http://opencellid.org>.

.TP
.. option:: --getsecuritystatus
Show, if phone wait for security code (like PIN, PUK, etc.) or not

.TP
.. option:: --identify
Show the most important phone data.

.TP
.. option:: --monitor [times]
Get phone status and writes continuously to standard output. Press Ctrl+C
to break this state.

.TP
.. option:: --nokiasecuritycode
Get/reset to "12345" security code


.SS "Phone settings
.TP
.. option:: --getcalendarsettings
Displays calendar settings like first day of
week or automatic deleting of old entries.

.TP
.. option:: --getprofile start [stop]

.TP
.. option:: --resetphonesettings PHONE|DEV|UIF|ALL|FACTORY
Reset phone settings. \fIBE CAREFULL !!!!\fR

\fBPHONE:\fR
.br

\fBDEV:\fR
.br

\fBALL:\fR 

Clear user settings
.br
* removes or set logos to default
.br
* set default phonebook and other menu settings
.br
* clear T9 words,
.br
* clear call register info
.br
* set default profiles settings 
.br
* clear user ringtones

\fBUIF:\fR
.br
* changes like after \fBALL\fR
.br
* disables netmon and PPS (all "hidden" menus)

\fBFACTORY:\fR
.br
* changes like after \fBUIF\fR
.br
* clear date/time

.TP
.. option:: --savesms PROFILE [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-name name] [-bitmap bitmap] [-ringtone ringtone]

Read ringtone (RTTL) format, bitmap (Picture Image size) and name, format into
Smart Messaging profile and send/save as SMS. Please note, that this format is
abandomed by Nokia and supported by some (older) devices only like Nokia 3310.


For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.


.TP
.. option:: --sendsms PROFILE destination [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]] [-smscset number] [-smscnumber number] [-reply] [-maxsms num] [-name name] [-bitmap bitmap] [-ringtone ringtone]
Sends a message, for description of message specific parameters see \fBsavesms PROFILE\fR.



For description of shared parameters see \fBCommon parameters for sendsms and savesms\fR.



.SS "Dumps decoding
.TP
.. option:: --decodebinarydump file [phonemodel]
Decodes a dump made by Gammu with 
\fBlogformat\fR se to \fBbinary\fR (see \fIREADME\fR for
info about this method of reporting bugs).

This option is available only if Gammu was compiled with debuging options.


.TP
.. option:: --decodesniff MBUS2|IRDA file [phonemodel]
Option available only, if Gammu was compiled with debug. Allows to decode
sniffs. See \fI/docs/manual/\fR for more details.


.SS "Functions that don't fit elsewhere
.TP
.. option:: --batch [file]
Starts Gammu in a batch mode. In this mode you can issue
several commands each on one line. Lines starting with # are treated as a
comments.

By default, commands are read from standard input, but you can optionally
specify a file from where they would be read (special case - means standard
input).

.TP
.. option:: --entersecuritycode PIN|PUK|PIN2|PUK2|PHONE|NETWORK code|-
Allow to enter security code from PC. When code is -, it is read from stdin.

.TP
.. option:: --nokiagetscreendump

.TP
.. option:: --nokiagetvoicerecord location

Get voice record from location and save to WAV file. File is
coded using GSM 6.10 codec (available for example in win32). Name
of file is like name of voice record in phone.

Created WAV files require GSM 6.10 codec to be played. In Win XP it's included
by Microsoft. If you deleted it by accident in this operating system, make such
steps:

1. Control Panel
.br
2. Add hardware
.br
3. click Next
.br
4. select "Yes. I have already connected the hardware
.br
5. select "Add a new hardware device
.br
6. select "Install the hardware that I manually select from a list
.br
7. select "Sound, video and game controllers
.br
8. select "Audio codecs
.br
9.  select "windows\\system32" directory and file "mmdriver.inf
.br
10. if You will be asked for file msgsm32.acm, it should unpacked from Windows CD
.br
11. now You can be asked if want to install unsigned driver (YES), about select codec configuration (select what you want) and rebotting PC (make it)

.TP
.. option:: --nokiamakecamerashoot

.TP
.. option:: --nokiasetphonemenus
Enable all (?) possible menus for DCT3 Nokia phones:

1. ALS (Alternative Line Service) option menu
2. vibra menu for 3210
3. 3315 features in 3310 5.45 and higher
4. two additional games (React and Logic) for 3210 5\.31 and higher
5. WellMate menu for 6150
6. NetMonitor

and for DCT4:

1. ALS (Alternative Line Service) option menu
2. Bluetooth, WAP bookmarks and settings menu, ... (6310i)
3. GPRS Always Online
4. ...

.TP
.. option:: --nokiasetvibralevel level
Set vibra power to "level" (given in percent)

.TP
.. option:: --presskeysequence mMnNpPuUdD+-123456789*0#gGrR<>[]hHcCjJfFoOmMdD@
Press specified key sequence on phone keyboard

``mM``
    Menu
``nN``
    Names key
``pP``
    Power
``uU``
    Up
``dD``
    Down
``+-``
    +-
``gG``
    Green
``rR``
    Red
``123456789*0#``
    numeric keyboard

.TP
.. option:: --reset SOFT|HARD
Make phone reset: soft (without asking for PIN) or hard (with PIN).

\fINote:\fR some phones will make hard reset even with \fBSOFT\fR option.

.TP
.. option:: --searchphone [-debug]
Attempts to search for a connected phone. Please note
that this can take a very long time, but in case you have no clue how to
configure phone connection, this is a convenient way to find working setup for
Gammu.

Gammu information
-----------------

.. option:: --checkversion [STABLE]

    Checks whether there is newer Gammu version
    available online (if Gammu has been compiled with CURL). If you pass
    additional parameter ``STABLE``, only stable versions will be checked.

.. option:: --features

    Print information about compiled in features.

.. option:: --help [topic]

    Print help. By default general help is printed, but you can
    also specify a help category to get more detailed help on some topic.

.. option:: --version

    Print version information and license.


Return values
+++++++++++++

gammu returns 0 on success. In case of failure non zero code is 
returned. 

1
    Out of memory or other critical error.
2
    Invalid command line parameters.
3
    Failed to open file specified on command line.
4
    Program was interrupted.
98
    Gammu library version mismatch.
99
    Functionality has been moved. For example to gammu-smsd(1).

Errors codes greater than 100 map to the GSM_Error 
values increased by 100:

101
    No error.
102
    Error opening device. Unknown, busy or no permissions.
103
    Error opening device, it is locked.
104
    Error opening device, it doesn't exist.
105
    Error opening device, it is already opened by other application.
106
    Error opening device, you don't have permissions.
107
    Error opening device. No required driver in operating system.
108
    Error opening device. Some hardware not connected/wrongly configured.
109
    Error setting device DTR or RTS.
110
    Error setting device speed. Maybe speed not supported.
111
    Error writing to the device.
112
    Error during reading from the device.
113
    Can't set parity on the device.
114
    No response in specified timeout. Probably phone not connected.
115
    Frame not requested right now. See <http://wammu.eu/support/bugs/> for information how to report it.
116
    Unknown response from phone. See <http://wammu.eu/support/bugs/> for information how to report it.
117
    Unknown frame. See <http://wammu.eu/support/bugs/> for information how to report it.
118
    Unknown connection type string. Check config file.
119
    Unknown model type string. Check config file.
120
    Some functions not available for your system (disabled in config or not implemented).
121
    Function not supported by phone.
122
    Entry is empty.
123
    Security error. Maybe no PIN?
124
    Invalid location. Maybe too high?
125
    Functionality not implemented. You are welcome to help authors with it.
126
    Memory full.
127
    Unknown error.
128
    Can not open specified file.
129
    More memory required...
130
    Operation not allowed by phone.
131
    No SMSC number given. Provide it manually or use the one configured in phone.
132
    You're inside phone menu (maybe editing?). Leave it and try again.
133
    Phone is not connected.
134
    Function is currently being implemented. If you want to help, please contact authors.
135
    Phone is disabled and connected to charger.
136
    File format not supported by Gammu.
137
    Nobody is perfect, some bug appeared in protocol implementation. Please contact authors.
138
    Transfer was canceled by phone, maybe you pressed cancel on phone.
139
    Phone module need to send another answer frame.
140
    Current connection type doesn't support called function.
141
    CRC error.
142
    Invalid date or time specified.
143
    Phone memory error, maybe it is read only.
144
    Invalid data given to phone.
145
    File with specified name already exists.
146
    File with specified name doesn't exist.
147
    You have to give folder name and not file name.
148
    You have to give file name and not folder name.
149
    Can not access SIM card.
150
    Wrong GNAPPLET version in phone. Use version from currently used Gammu.
151
    Only part of folder has been listed.
152
    Folder must be empty.
153
    Data were converted.
154
    Gammu is not configured.
155
    Wrong folder used.
156
    Internal phone error.
157
    Error writing file to disk.
158
    No such section exists.
159
    Using default values.
160
    Corrupted data returned by phone.
161
    Bad feature string in configuration.
162
    Desired functionality has been disabled on compile time.
163
    Bluetooth configuration requires channel option.
164
    Service is not runnig.
165
    Service configuration is missing.
166
    Command rejected because device was busy. Wait and restart.
167
    Could not connect to the server.
168
    Could not resolve the host name.
169
    Failed to get SMSC number from phone.


Examples
++++++++

Configuration
-------------

To check it out, you need to have configuration file for gammu, see 
:ref:`gammurc` for more details about it.

Sending messages
----------------

Save text message up to standard 160 chars:

.. code-block:: sh

    echo "All your base are belong to us" | gammu savesms TEXT

or 

.. code-block:: sh

    gammu savesms TEXT -text "All your base are belong to us

Save long text message:

.. code-block:: sh

    echo "All your base are belong to us" | gammu savesms TEXT -len 400

or 

.. code-block:: sh

    gammu savesms TEXT -len 400 -text "All your base are belong to us

or

.. code-block:: sh

    gammu savesms EMS -text "All your base are belong to us

Save some funky message with predefined sound and animation from 2 bitmaps:

.. code-block:: sh

    gammu savesms EMS -text "Greetings" -defsound 1 -text "from Gammu -tone10 axelf.txt -animation 2 file1.bmp file2.bmp

Save protected message with ringtone:

.. code-block:: sh

    gammu savesms EMS -protected 2 -variablebitmaplong ala.bmp -toneSElong axelf.txt -toneSE ring.txt

Backups
-------

If you will backup settings to Gammu text file, it will be possible to edit
it. It's easy: many things in this file will be written double - once in Unicode,
once in ASCII. When you will remove Unicode version Gammu will use ASCII
on \fBrestore\fR (and you can easy edit ASCII text) and will convert it
according to your OS locale. When will be available Unicode version of text,
it will be used instead of ASCII (useful with Unicode phones - it isn't important,
what locale is set in computer and no conversion Unicode -> ASCII and ASCII ->
Unicode is done).

You can use any editor with regular expressions function to edit backup text
file. Examples of such editors can be vim <http://www.vim.org/> or TextPad
<http://www.textpad.com/> which both do support regular expressions.

Remove info about voice tags

Find::

    ^Entry\\([0-9][0-9]\\)VoiceTag = \\(.*\\)\\n

Replace::

    <blank>

Change all numbers starting from +3620, +3630, +3660, +3670 to +3620

Find::

    Type = NumberGeneral\\nEntry\\([0-9][0-9]\\)Text = "\\+36\\(20\\|30\\|60\\|70\\)\\n

Replace::

    Type = NumberMobile\\nEntry\\1Text = "\\+3620

Change phone numbers type to mobile for numbers starting from +3620, +3630,... and removing the corresponding TextUnicode line

Find::

    Type = NumberGeneral\\nEntry\\([0-9][0-9]\\)Text = "\\+36\\([2367]0\\)\\([^\\"]*\\)"\\nEntry\\([0-9][0-9]\\)TextUnicode = \\([^\\n]*\\)\\n

Replace::

    Type = NumberMobile\\nEntry\\1Text = "\\+36\\2\\3"\\n

Reporting bugs
--------------

There are definitely many bugs, reporting to author is welcome. Please include
some useful information when sending bug reports (especially debug logs,
operating system, it's version and phone information are needed).

To generate debug log, enable it in gammurc (alternatively you can
do it on command line using -d textall -f /tmp/gammu.log):

.. code-block:: ini

    [gammu]
    YOUR CONNECTION SETTINGS
    logfile = /tmp/gammu.log
    logformat = textall

With this settings, Gammu generates /tmp/gammu.log on each connection to
phone and stores dump of communication there. You can also find some
hints for improving support for your phone in this log.

See <http://wammu.eu/support/bugs/> for more information on reporting bugs.

Please report bugs to <http://bugs.cihar.com>.
