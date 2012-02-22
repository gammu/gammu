.. _gammu:

Gammu Utility
=============

.. program:: gammu

Synopsis
++++++++


.. code-block:: text

    gammu [parameters] <command> [options]

Commands actually indicate which operation should Gammu perform. They can be
specified with or without leading ``--``.

Description
+++++++++++

This program is a tool for mobile phones. Many vendors and phones
are supported, for actual listing see `Gammu Phones Database <http://wammu.eu/phones/>`_.


Options
-------

Parameters before command configure gammu behaviour:

.. option:: -c, --config <filename>

    name of configuration file

.. option:: -s, --section <confign>

   section of config file to use, eg. 42

.. option:: -d, --debug <level>

   debug level (see :config:option:`LogFormat` in :ref:`gammurc` for possible values)

.. option:: -f, --debug-file <filename>

   file for logging debug messages


Call commands
-------------

.. option:: answercall [id]

   Answer incoming call.

.. option:: cancelcall [id]

    Cancel incoming call

.. option:: canceldiverts

    Cancel all existing call diverts.

.. option:: conferencecall id

    Initiates a conference call.

.. option:: dialvoice number [show|hide]

    Make voice call from SIM card line set in phone.

    ``show|hide`` - optional parameter whether to disable call number indication.

.. option:: divert get|set all|busy|noans|outofreach all|voice|fax|data [number timeout]

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

.. option:: getussd code

    Retrieves USSD information - dials a service number and reads response.

.. option:: holdcall id

    Holds call.

.. option:: maketerminatedcall number length [show|hide]

    Make voice call from SIM card line set in phone which will
    be terminated after ``length`` seconds.

.. option:: senddtmf sequence

    Plays DTMF sequence. In some phones available only during calls

.. option:: splitcall id

    Splits call.

.. option:: switchcall [id]

    Switches call.

.. option:: transfercall [id]

    Transfers call.

.. option:: unholdcall id

    Unholds call.

.. _gammu-sms:

SMS and EMS commands
--------------------

Sending messages might look a bit complicated on first attempt to use.
But be patient, the command line has been written in order to allow
almost every usage. See EXAMPLE section for some hints on usage.

There is also an option to use :ref:`gammu-smsd` when you want to send or
receive more messages and process them automatically.

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

GSM Default Alphabet
~~~~~~~~~~~~~~~~~~~~

With `GSM Default Alphabet` you can fit at most 160 chars into single SMS
(Gammu doesn't support compressing such texts according to GSM standards, but
it isn't big limit, because there are no phones supporting them), but they're
from limited set:

* all Latin small and large
* all digits
* some Greek
* some other national
* some symbols like  @ ! " # & / ( ) % * + = - , . : ; < > ?
* few others

Unicode
~~~~~~~

With `Unicode` single SMS can contain at most 70 chars, but these can be
any chars including all national and special ones.

.. warning::

   Please note, that some older phones might have problems displaying such
   message.

Conversion
~~~~~~~~~~

Gammu tries to do the best to handle non ASCII characters in your message.
Everything is internally handled in Unicode (the input is converted depending
on your locales configuration) and in case message uses Unicode the text will
be given as such to the message.

Should the message be sent in GSM Default Alphabet, Gammu will try to convert
all characters to keep message readable. Gammu does support multi byte
encoding for some characters in GSM Default Alphabet (it is needed for ``^`` ``{`` ``}``
``\`` ``[`` ``]`` ``~`` ``|``). The characters which are not present in GSM Default Alphabet
are transliterated to closest ASCII equivalent (accents are removed).
Remaining not known characters are replaced by question mark.

SMS commands
____________

.. option:: addsmsfolder name

.. option:: deleteallsms folder

    Delete all SMS from specified SMS folder.

.. option:: deletesms folder start [stop]

    Delete SMS from phone. See description for :option:`gammu getsms` for info about
    sms folders naming convention.

    Locations are numerated from 1.

.. option:: displaysms ... (options like in sendsms)

    Displays PDU data of encoded SMS messages. It accepts same parameters and
    behaves same like sendsms.

.. option:: getallsms -pbk

    Get all SMS from phone. In some phones you will have also SMS templates
    and info about locations used to save Picture Images. With each sms you
    will see location. If you want to get such sms from phone alone, use
    :option:`gammu getsms`.

.. option:: geteachsms -pbk

    Similiary to :option:`gammu getallsms`. Difference is, that
    links all concatenated sms

.. option:: getsms folder start [stop]

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
    :option:`gammu getsmsfolders` to get folders list.

.. option:: getsmsc [start [stop]]

    Get SMSC settings from SIM card.

    Locations are numerated from 1.

.. option:: getsmsfolders

    Get names for SMS folders in phone

.. option:: savesms TYPE [type parameters] [type options] [-folder id] [-unread] [-read] [-unsent] [-sent] [-sender number] [-smsname name] [-smscset number] [-smscnumber number] [-reply] [-maxsms num]

   Saves SMS to phone, see bellow for ``TYPE`` options.

    .. option:: -smscset number

        SMSC number will be taken from phone stored SMSC configuration ``number``.

        Default: 1

    .. option:: -smscnumber number

        SMSC number

    .. option:: -reply

        reply SMSC is set

    .. option:: -folder number

        save to specified folder.

        Folders are numerated from 1.

        The most often folder 1 = "Inbox", 2 = "Outbox",etc. Use :option:`gammu getsmsfolders` to get folder list.

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

        set sender number (default: ``Gammu``)

    .. option:: -maxsms num

        Limit maximal number of messages which will be
        created. If there are more messages, Gammu will terminate with failure.

    Types of messages:

    .. option:: ANIMATION frames file1 file2...

        Save an animation as a SMS. You need to give
        number of frames and picture for each frame. Each picture can be in any
        picture format which Gammu supports (B/W bmp, gif, wbmp, nol, nlm...).

    .. option:: BOOKMARK file location

        Read WAP bookmark from file created by :option:`gammu backup` command and saves in
        Nokia format as SMS


    .. option:: CALENDAR file location

        Read calendar note from file created by :option:`gammu backup` command and saves in
        VCALENDAR 1.0 format as SMS. The location identifies position of calendar item
        to be read in backup file (usually 1, but can be useful in case the backup contains
        more items).


    .. option:: CALLER file

        Save caller logo as sms in Nokia (Smart Messaging)
        format - size 72x14, two colors.

        .. warning::

           Please note, that it isn't designed for colour logos available for
           example in DCT4/TIKU - you need to put bitmap file there inside
           phone using filesystem commands.



    .. option:: EMS [-unicode] [-16bit] [-format lcrasbiut] [-text text] [-unicodefiletext file] [-defsound ID] [-defanimation ID] [-tone10 file] [-tone10long file] [-tone12 file] [-tone12long file] [-toneSE file] [-toneSElong file] [-fixedbitmap file] [-variablebitmap file] [-variablebitmaplong file] [-animation frames file1 ...] [-protected number]

        Saves EMS sequence. All format specific parameters (like :option:`-defsound`) can be used few times.

        .. option:: -text

            adds text

        .. option:: -unicodefiletext

            adds text from Unicode file

        .. option:: -defanimation

            adds default animation with ID specified by user. ID for different phones are different.

        .. option:: -animation

            adds "frames" frames read from file1, file2, etc.

        .. option:: -defsound

            adds default sound with ID specified by user. ID for different phones are different.

        .. option:: -tone10

            adds IMelody version 1.0 read from RTTL or other compatible file

        .. option:: -tone10long

            IMelody version 1.0 saved in one of few SMS with UPI. Phones
            compatible with UPI (like Sony-Ericsson phones) will read such
            ringtone as one

        .. option:: -tone12

            adds IMelody version 1.2 read from RTTL or other compatible file

        .. option:: -tone12long

            IMelody version 1.2 saved in one of few SMS with UPI. Phones
            compatible with UPI (like Sony-Ericsson phones) will read such
            ringtone as one

        .. option:: -toneSE

            adds IMelody in "short" form supported by Sony-Ericsson phones

        .. option:: -toneSElong

            add Sony-Ericsson IMelody saved in one or few SMS with UPI

        .. option:: -variablebitmap

            bitmap in any size saved in one SMS

        .. option:: -variablebitmaplong

            bitmap with maximal size 96x128 saved in one or few sms

        .. option:: -fixedbitmap

            bitmap 16x16 or 32x32

        .. option:: -protected

            all ringtones and bitmaps after this parameter (excluding default
            ringtones and logos) will be "protected" (in phones compatible with
            ODI like SonyEricsson products it won't be possible to forward them
            from phone menu)

        .. option:: -16bit

            Gammu uses SMS headers with 16-bit numbers for saving linking info
            in SMS (it means less chars available for user in each SMS)

        .. option:: -format lcrasbiut

            last text will be formatted. You can use combinations of chars:

            =========  ==================
            Character  Formating
            =========  ==================
            ``l``      left aligned
            ``c``      centered
            ``r``      right aligned
            ``a``      large font
            ``s``      small font
            ``b``      bold font
            ``i``      italic font
            ``u``      underlined font
            ``t``      strikethrough font
            =========  ==================


    .. option:: MMSINDICATOR URL Title Sender

        Creates a MMS indication SMS. It contains URL where the actual MMS
        payload is stored which needs to be SMIL encoded. The phone usually
        downloads the MMS data using special APN, which does not count to
        transmitted data, however there might be limitations which URLs can
        be accessed.

    .. option:: MMSSETTINGS file location

        Saves a message with MMS configuration. The
        configuration will be read from Gammu backup file from given location.

    .. option:: OPERATOR file  [-netcode netcode] [-biglogo]

        Save operator logo as sms in Nokia (Smart Messaging) format - size
        72x14 in two colors.

        .. option:: -biglogo

            Use 78x21 formatted logo instead of standard 72x14.

        .. note::

           This isn't designed for colour logos available for example in newer
           phones - you need to put bitmap file there inside phone using
           filesystem commands.


    .. option:: PICTURE file  [-text text] [-unicode] [-alcatelbmmi]

        Read bitmap from 2 colors file (bmp, nlm, nsl, ngg, nol, wbmp, etc.), format
        into bitmap in Smart Messaging (72x28, 2 colors, called often Picture Image
        and saved with text) or Alcatel format and send/save over SMS.


    .. option:: PROFILE  [-name name] [-bitmap bitmap] [-ringtone ringtone]

        Read ringtone (RTTL) format, bitmap (Picture Image size) and name, format into
        Smart Messaging profile and send/save as SMS.

        .. warning::

           Please note, that this format is abandoned by Nokia and supported
           by some (older) devices only like Nokia 3310.


    .. option:: RINGTONE file  [-long] [-scale]

        Read RTTL ringtone from file and save as SMS
        into SIM/phone memory. Ringtone is saved in Nokia (Smart Messaging) format.

        .. option:: -long

            ringtone is saved using Profile style. It can be longer (and saved
            in 2 SMS), but decoded only by newer phones (like 33xx)

        .. option:: -scale

            ringtone will have Scale info for each note. It will allow one to edit
            it correctly later in phone composer (for example, in 33xx)


    .. option:: SMSTEMPLATE  [-unicode] [-text text] [-unicodefiletext file] [-defsound ID] [-defanimation ID] [-tone10 file] [-tone10long file] [-tone12 file] [-tone12long file] [-toneSE file] [-toneSElong file] [-variablebitmap file] [-variablebitmaplong file] [-animation frames file1 ...]

        Saves a SMS template (for Alcatel phones).


    .. option:: TEXT  [-inputunicode] [-16bit] [-flash] [-len len] [-autolen len] [-unicode] [-enablevoice] [-disablevoice] [-enablefax] [-disablefax] [-enableemail] [-disableemail] [-voidsms] [-replacemessages ID] [-replacefile file] [-text msgtext] [-textutf8 msgtext]

        Take text from stdin (or commandline if -text
        specified) and save as text SMS into SIM/phone memory.

        .. option:: -flash

            Class 0 SMS (should be displayed after receiving on recipients' phone display after receiving without entering Inbox)

        .. option:: -len len

            specify, how many chars will be read. When use this option and text
            will be longer than 1 SMS, will be split into more linked SMS

        .. option:: -autolen len

            specify, how many chars will be read. When use this option and text
            will be longer than 1 SMS, will be split into more linked
            SMS.Coding type (SMS default alphabet/Unicode) is set according to
            input text

        .. option:: -enablevoice

            sms will set voice mail indicator. Text will be cut to 1 sms.

        .. option:: -disablevoice

            sms will not set voice mail indicator. Text will be cut to 1 sms.

        .. option:: -enablefax

            sms will set fax indicator. Text will be cut to 1 sms.

        .. option:: -disablefax

            sms will not set fax indicator. Text will be cut to 1 sms.

        .. option:: -enableemail

            sms will set email indicator. Text will be cut to 1 sms.

        .. option:: -disableemail

            sms will not set email indicator. Text will be cut to 1 sms.

        .. option:: -voidsms

            many phones after receiving it won't display anything, only beep,
            vibrate or turn on light. Text will be cut to 1 sms.

        .. option:: -unicode

            SMS will be saved in Unicode format

            .. note::

                The ``~`` char in SMS text and :option:`-unicode` option
                (Unicode coding required) can cause text of SMS after ``~``
                char  blink in some phones (like Nokia 33xx).

        .. option:: -inputunicode

            input text is in Unicode.

            .. note::

                You can create Unicode file using WordPad in Windows (during
                saving select "Unicode Text Document" format). In Unix can use
                for example YUdit or vim.

        .. option:: -text

            get text from command line instead of stdin.

        .. option:: -textutf8

            get text in UTF-8 from command line instead of stdin.

            .. note::

                Gammu detects your locales and uses by default encoding based
                on this. Use this option only when you know the input will be
                in UTF-8 in all cases.

        .. option:: -16bit

            Gammu uses SMS headers with 16-bit numbers for saving linking info
            in SMS (it means less chars available for user in each SMS)

        .. option:: -replacemessages ID

            ``ID`` can be 1..7. When you will use option and send more single
            SMS to one recipient with the same ID, each another SMS will
            replace each previous with the same ID

        .. option:: -replacefile file

            File with replacement table in unicode (UCS-2), preferably with
            byte order mark (BOM). It contains pairs of chars, first one is to
            replace, second is replacement one. The replacement is done after
            reading text for the message.

            For example replacement ``1`` (0x0061) with ``a`` (0x0031) would
            be done by file with following content (hex dump, first two bytes
            is BOM)::

                ff fe 61 00 31 00

    .. option:: TODO file location

        Saves a message with a todo entry. The content will
        be read from any backup format which Gammu supports and from given location.

    .. option:: VCARD10|VCARD21 file SM|ME location [-nokia]

        Read phonebook entry from file created by :option:`gammu backup`
        command and saves in VCARD 1.0 (only name and default number) or VCARD
        2.1 (all entry details with all numbers, text and name) format as SMS.
        The location identifies position of contact item to be read in backup
        file (usually 1, but can be useful in case the backup contains more
        items).


    .. option:: WAPINDICATOR URL Title

        Saves a SMS with a WAP indication for given
        URL and title.


    .. option:: WAPSETTINGS file location DATA|GPRS

        Read WAP settings from file created by :option:`gammu backup` command
        and saves in Nokia format as SMS


.. option:: sendsms TYPE destination [type parameters] [type options] [-smscset number] [-smscnumber number] [-reply] [-report] [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX] [-save [-folder number]]

    Sends a message to a ``destination`` number, most parameters are same as for :option:`gammu savesms`.

    .. option:: -save

        will also save message which is being sent

    .. option:: -report

        request delivery report for message

    .. option:: -validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX

        sets how long will be the
        message valid (SMSC will the discard the message after this time if it could
        not deliver it).

.. option:: setsmsc location number

    Set SMSC settings on SIM card. This keeps all SMSC configuration
    intact, it just changes the SMSC number.

    Locations are numerated from 1.


Memory (phonebooks and calls) commands
--------------------------------------

.. _memory types:

Memory types
____________

Gammu recognizes following memory types:

``DC``
     Dialled calls
``MC``
     Missed calls
``RC``
     Received calls
``ON``
     Own numbers
``VM``
     voice mailbox
``SM``
     SIM phonebook
``ME``
     phone internal phonebook
``FD``
     fixed dialling
``SL``
     sent SMS log


Memory commands
_______________

.. option:: deleteallmemory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL

    Deletes all entries from specified memory type.

    For memory types description see :ref:`memory types`.

.. option:: deletememory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL start [stop]

    Deletes entries in specified range from specified memory type.

    For memory types description see :ref:`memory types`.

.. option:: getallmemory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL

    Get all memory locations from phone.

    For memory types description see :ref:`memory types`.

.. option:: getmemory DC|MC|RC|ON|VM|SM|ME|MT|FD|SL start [stop [-nonempty]]

    Get memory location from phone.

    For memory types description see :ref:`memory types`.

    Locations are numerated from 1.

.. option:: getspeeddial start [stop]

    Gets speed dial choices.

.. option:: searchmemory text

    Scans all memory entries for given text. It performs
    case insensitive substring lookup. You can interrupt searching by pressing
    :kbd:`Ctrl+C`.

Filesystem commands
-------------------

Gammu allows one to access phones using native protocol (Nokias) or OBEX. Your
phone can also support usb storage, which is handled on the operating system
level and Gammu does not use that.

.. option:: addfile folderID name [-type JAR|BMP|PNG|GIF|JPG|MIDI|WBMP|AMR|3GP|NRT] [-readonly] [-protected] [-system] [-hidden] [-newtime]

    Add file with specified name to folder with specified folder ID.

    .. option:: -type

       File type was required for filesystem 1 in Nokia phones (current filesystem 2 doesn't need this).

    .. option:: -readonly

       Sets the read only attribute.

    .. option:: -protected

       Sets the protected attribute (file can't be for example forwarded
       from phone menu).

    .. option:: -system

       Sets the system attribute.

    .. option:: -hidden

       Sets the hidden attribute (file is hidden from phone menu).

    .. option:: -newtime

        After using it date/time of file modification will be set to moment of uploading.

.. option:: addfolder parentfolderID name

    Create a folder in phone with specified name in a
    folder with specified folder ID.

.. option:: deletefiles fileID

    Delete files with given IDs.

.. option:: deletefolder name

    Delete folder with given ID.

.. option:: getfilefolder fileID, fileID, ...

    Retrieve files or all files from folder with given IDs from a phone filesytem.

.. option:: getfiles fileID, fileID, ...

    Retrieve files with given IDs from a phone filesytem.

.. option:: getfilesystem [-flatall|-flat]

    Display info about all folders and files in phone memory/memory card. By default there is tree displayed, you can change it:

    .. option:: -flatall

        there are displayed full file/folder details like ID (first parameter in line)

    .. option:: -flat

    .. note:: In some phones (like N6230) content of some folders (with more files) can be cut (only part of files will be displayed) for example on infrared connection. This is not Gammu issue, but phone firmware problem.

.. option:: getfilesystemstatus

    Display info filesystem status - number of
    bytes available, used or used by some specific content.

.. option:: getfolderlisting folderID

    Display files and folders available in folder with given folder ID. You can get ID's using getfilesystem -flatall.

    .. warning::

       Please note, that in some phones (like N6230) content of some folders
       (with more files) can be cut (only part of files will be displayed) for
       example on infrared connection. This is not Gammu issue, but phone
       firmware problem.

.. option:: getrootfolders

    Display info about drives available in phone/memory card.

.. option:: sendfile name

    Sends file to a phone. It's up to phone to decide where
    to store this file and how to handle it (for example when you send vCard or
    vCalendar, most of phones will offer you to import it.

.. option:: setfileattrib folderID [-system] [-readonly] [-hidden] [-protected]


Logo and pictures commands
--------------------------

These options are mainly (there are few exceptions) for monochromatic logos and
images available in older phones. Recognized file formats: xpm (only saving),
2-colors bmp, nlm, nsl, ngg, nol, wbmp, gif (for Samsung).

In new models all bitmaps are saved in filesystem and should go into filesystem section

.. option:: copybitmap inputfile [outputfile [OPERATOR|PICTURE|STARTUP|CALLER]]

    Allow one to convert logos files to another. When give ONLY inputfile, output will
    be written to stdout using ASCII art. When give output file and format, in
    some file formats (like NLM) will be set indicator informing about logo type
    to given.

.. option:: getbitmap TYPE [type options]

    Reads bitmap from phone, following types are supported:

    .. option:: CALLER location [file]

        Get caller group logo from phone. Locations 1-5.

    .. option:: DEALER

        In some models it's possible to save dealer welcome note - text displayed
        during enabling phone, which can't be edited from phone menu.  Here you can
        get it.

    .. option:: OPERATOR [file]

        Get operator logo (picture displayed instead of operator name) from phone.

    .. option:: PICTURE location [file]

        Get Picture Image from phone.

    .. option:: STARTUP [file]

        Get static startup logo from phone. Allow one to save it in file.

    .. option:: TEXT

        Get startup text from phone.

.. option:: setbitmap TYPE [type options]

    Sets bitmap in phone, following types are supported:

    .. option:: CALLER location [file]

        Set caller logo.

    .. option:: COLOUROPERATOR [fileID [netcode]]

        Sets color operator logo in phone.

    .. option:: COLOURSTARTUP [fileID]

    .. option:: DEALER text

        Sets welcome message configured by dealer, which usually can not be changed in phone menus.

    .. option:: OPERATOR [file [netcode]]

        Set operator logo in phone. When won't give file and netcode, operator logo
        will be removed from phone. When will give only filename, operator logo will
        be displayed for your current GSM operator. When you give additionally network
        code, it will be displayed for this operator.

    .. option:: PICTURE file location [text]

        Sets picture image in phone.

    .. option:: STARTUP file|1|2|3

        Set startup logo in phone. It can be static (then you will have to give file
        name) or one of predefined animated (only some phones like Nokia 3310 or 3330
        supports it, use location 1, 2 or 3 for these).

    .. option:: TEXT text

        Sets startup text in phone.

    .. option:: WALLPAPER fileID

        Sets wallpaper in phone.

Ringtones commands
------------------

Ringtones are mostly supported only for older phones. For recent phones you
usually just upload them to some folder in phone filesystem.

There are recognized various file formats by options described below: rttl,
binary format created for Gammu, mid (saving), re (reading), ott, communicator,
ringtones format found in fkn.pl, wav (saving), ime/imy (saving), rng, mmf (for
Samsung).


.. option:: copyringtone source destination [RTTL|BINARY]

    Copy source ringtone to destination.

.. option:: getphoneringtone location [file]

    Get one of "default" ringtones and saves into file

.. option:: getringtone location [file]

    Get ringtone from phone in RTTL or BINARY format.

    Locations are numerated from 1.

.. option:: getringtoneslist

.. option:: playringtone file

    Play aproximation of ringtone over phone buzzer. File can be in RTTL or BINARY (Nokia DCT3) format.

.. option:: playsavedringtone number

    Play one of built-in ringtones. This option is available for DCT4 phones.
    For getting ringtones list use :option:`gammu getringtoneslist`.

.. option:: setringtone file [-location location] [-scale] [-name name]

    Set ringtone in phone. When don't give location, it will be written
    "with preview" (in phones supporting this feature like 61xx or 6210).

    .. option:: -scale

        Scale information will be added to each note of RTTL ringtone. It will
        avoid scale problems available during editing ringtone in composer
        from phone menu (for example, in Nokia 33xx).

    .. note::

        When use ``~`` char in ringtone name, in some phones (like 33xx) name
        will blink later in phone menus.


Calendar notes commands
-----------------------

In Nokia 3310, 3315 and 3330 these are named "Reminders" and have some limitations (depending on phone firmware version).

.. option:: deletecalendar start [stop]

    Deletes selected calendar entries in phone.

.. option:: getallcalendar

    Retrieves all calendar entries from phone.

.. option:: getcalendar start [stop]

    Retrieves selected calendar entries from phone.


To do list commands
-------------------

.. option:: deletetodo start [stop]

    Deletes selected todo entries in phone.

.. option:: getalltodo

    Retrieves all todo entries from phone.

.. option:: gettodo start [stop]

    Retrieves selected todo entries from phone.

Notes commands
--------------

.. option:: getallnotes

    Reads all notes from the phone.

    .. note::

        Not all phones supports this function, especially most Sony Ericsson
        phones even if they have notes inside phone.


Date, time and alarm commands
-----------------------------

.. option:: getalarm [start]

    Get alarm from phone, if no location is specified,
    1 is used.

.. option:: getdatetime

    Get date and time from phone

.. option:: setalarm hour minute

    Sets repeating alarm in phone on selected time.

.. option:: setdatetime [HH:MM[:SS]] [YYYY/MM/DD]

    Set date and time in phone to date and time set in computer. Please
    note, that this option doesn't show clock on phone screen. It only set
    date and time.

    .. note::

       You can make such synchronization each time, when will connect your
       phone and use Gammu. See :config:option:`SynchronizeTime` in :ref:`gammurc` for details.


Categories commands
-------------------

.. note::

    Categories are supported only on few phones (Alcatel).

.. option:: addcategory TODO|PHONEBOOK text

.. option:: getallcategory TODO|PHONEBOOK

.. option:: getcategory TODO|PHONEBOOK start [stop]

.. option:: listmemorycategory text|number

.. option:: listtodocategory text|number


Backing up and restoring commands
---------------------------------

.. option:: addnew file [-yes] [-memory ME|SM|..]

    Adds data written in file created using :option:`gammu backup` command. All things
    backed up :option:`gammu backup` can be restored (when made backup to Gammu text
    file).

    Please note that this adds all content of backup file to phone and
    does not care about current data in the phone (no duplicates are
    detected).

    Use -yes parameter to answer yes to all questions (you want to automatically
    restore all data).

    Use -memory parameter to force usage of defined memory type for storing entries
    regardless what backu format says.

.. option:: addsms folder file [-yes]

    Adds SMSes from file (format like :option:`gammu backupsms` uses) to
    selected folder in phone.

.. option:: backup file [-yes]

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

    * ``.lmb`` - Nokia backup, supports contacts, caller logos and startup logo.
    * ``.vcs`` - vCalendar, supports calendar and todo.
    * ``.vcf`` - vCard, supports contacts.
    * ``.ldif`` - LDAP import, supports contacts.
    * ``.ics`` - iCalendar, supports calendar and todo.
    * Any other extension is Gammu backup file and it supports all data mentioned above, see :ref:`gammu-backup` for more details.

    By default this command is interactive and asks which items tou want to backup.

    Use -yes for answering yes to all questions.

.. option:: backupsms file [-yes|-all]

    Stores all SMSes from phone to file into :ref:`gammu-smsbackup`.

    By default this command is interactive and asks which folders you want
    to backup and whether you want to remove messages from phone afterwards.

    Use -yes for answering yes to all questions (backup all messages and
    delete them from phone), or -all to just backup all folders while keeping
    messages in phone.

.. option:: restore file [-yes]

    .. warning::

        Please note that restoring deletes all current content in phone. If you
        want only to add entries to phone, use :option:`gammu addnew`.

    Restore settings written in file created using :option:`gammu backup` command.


    In some phones restoring calendar notes will not show error, but won't
    be done, when phone doesn't have set clock inside.

.. option:: restoresms file [-yes]

    .. warning::

       Please note that this overwrites existing messages in phone (if it supports it).

    Restores SMSes from file (format like :option:`gammu backupsms` uses) to
    selected folder in phone.

.. option:: savefile TYPE [type options]

    Converts between various file formats supported by Gammu, following types
    are supported:

    .. option:: BOOKMARK target.url file location

        Converts backup format supported by
        Gammu to vBookmark file.

    .. option:: CALENDAR target.vcs file location

        Allows one to convert between various backup formats which gammu
        supports for calendar events. The file type is guessed (for input file
        guess is based on extension and file content, for output solely on
        extension).

    .. option:: TODO target.vcs file location

        Allows one to convert between various backup formats which gammu
        supports for todo events. The file type is guessed (for input file
        guess is based on extension and file content, for output solely on
        extension).

    .. option:: VCARD10|VCARD21 target.vcf file SM|ME location

        Allows one to convert between various backup formats which gammu
        supports for phonebook events. The file type is guessed (for input file
        guess is based on extension and file content, for output solely on
        extension).

    .. seealso:: :option:`gammu convertbackup`

.. option:: convertbackup source.file output.file

    .. versionadded:: 1.28.94

    Converts backup between formats supported by Gammu. Unlike
    :option:`gammu savefile`, this does not give you any options what to
    convert, it simply takes converts all what can be saved into output file.

    .. seealso:: :option:`gammu savefile`


Nokia specific commands
-----------------------

.. option:: nokiaaddfile TYPE [type options]

    Uploads file to phone to specific location for the type:

    .. option:: APPLICATION|GAME file [-readonly] [-overwrite] [-overwriteall]

        Install the ``*.jar/*.jad`` file pair of a midlet in the application or
        game menu of the phone. You need to specify filename without the jar/jad
        suffix, both will be added automatically.

        .. option:: -overwrite

            Delete the application's .jad and .jar files before installing, but
            doesn't delete the application data.

        .. option:: -overwriteall

            Delete the application (same as :option:`-overwrite`) and all it's
            data.

        You can use :ref:`jadmaker` to generate a .jad file from a .jar file.

    .. option:: GALLERY|GALLERY2|CAMERA|TONES|TONES2|RECORDS|VIDEO|PLAYLIST|MEMORYCARD file [-name name] [-protected] [-readonly] [-system] [-hidden] [-newtime]

.. option:: nokiaaddplaylists

    Goes through phone memory and generated playlist for all music files found.

    To manually manage playlists:

    .. code-block:: sh

        gammu addfile a:\\predefplaylist filename.m3u

    Will add playlist filename.m3u

    .. code-block:: sh

        gammu getfilesystem

    Will get list of all files (including names of files with playlists)

    .. code-block:: sh

        gammu deletefiles a:\\predefplaylist\\filename.m3u

    Will delete playlist filename.m3u

    Format of m3u playlist is easy (standard mp3 playlist):

    First line is ``#EXTM3U``, next lines contain  names of files (``b:\file1.mp3``,
    ``b:\folder1\file2.mp3``, etc.). File needs t have ``\r\n`` terminated lines. So
    just run :program:`unix2dos` on the resulting file before uploading it your your phone.


.. option:: nokiacomposer file

    Show, how to enter RTTL ringtone in composer existing in many Nokia phones
    (and how should it look like).

.. option:: nokiadebug filename [[v11-22] [,v33-44]...]

.. option:: nokiadisplayoutput

.. option:: nokiadisplaytest number

.. option:: nokiagetadc

.. option:: nokiagetoperatorname

    6110.c phones have place for name for one GSM network (of course, with flashing it's
    possible to change all names, but Gammu is not flasher ;-)). You can get this
    name using this option.

.. option:: nokiagetpbkfeatures memorytype

.. option:: nokiagett9

    This option should display T9 dictionary content from
    DCT4 phones.

.. option:: nokiagetvoicerecord location

    Get voice record from location and save to WAV file. File is
    coded using GSM 6.10 codec (available for example in win32). Name
    of file is like name of voice record in phone.

    Created WAV files require GSM 6.10 codec to be played. In Win XP it's included
    by Microsoft. If you deleted it by accident in this operating system, make such
    steps:

    1. Control Panel
    2. Add hardware
    3. click Next
    4. select "Yes. I have already connected the hardware
    5. select "Add a new hardware device
    6. select "Install the hardware that I manually select from a list
    7. select "Sound, video and game controllers
    8. select "Audio codecs
    9.  select "windows\\system32" directory and file "mmdriver.inf
    10. if You will be asked for file msgsm32.acm, it should unpacked from Windows CD
    11. now You can be asked if want to install unsigned driver (YES), about select codec configuration (select what you want) and rebotting PC (make it)

.. option:: nokiamakecamerashoot

.. option:: nokianetmonitor test

    Takes output or set netmonitor for Nokia DCT3 phones.

    .. seealso::

        For more info about this option, please visit
        `Marcin's page <http://www.mwiacek.com/>`_ and read netmonitor manual
        there.

    .. note::

       test ``243`` enables all tests (after using command
       :command:`gammu nokianetmonitor 243` in some phones like 6210 or 9210 have to reboot
       them to see netmonitor menu)

.. option:: nokianetmonitor36

    Reset counters from netmonitor test 36 in Nokia DCT3 phones.

    .. seealso::

        For more info about this option, please visit
        `Marcin's page <http://www.mwiacek.com/>`_ and read netmonitor manual
        there.

.. option:: nokiasecuritycode

    Get/reset to "12345" security code

.. option:: nokiaselftests

    Perform tests for Nokia DCT3 phones.

    .. note::

       EEPROM test can show an error when your phone has an EEPROM in flash
       (like 82xx/7110/62xx/33xx). The clock test will show an error when the
       phone doesn?t have an internal battery for the clock (like 3xxx).

.. option:: nokiasetlights keypad|display|torch on|off

.. option:: nokiasetoperatorname [networkcode name]

.. option:: nokiasetphonemenus

    Enable all (?) possible menus for DCT3 Nokia phones:

    1. ALS (Alternative Line Service) option menu
    2. vibra menu for 3210
    3. 3315 features in 3310 5.45 and higher
    4. two additional games (React and Logic) for 3210 5.31 and higher
    5. WellMate menu for 6150
    6. NetMonitor

    and for DCT4:

    1. ALS (Alternative Line Service) option menu
    2. Bluetooth, WAP bookmarks and settings menu, ... (6310i)
    3. GPRS Always Online
    4. and others...

.. option:: nokiasetvibralevel level

    Set vibra power to "level" (given in percent)

.. option:: nokiatuneradio

.. option:: nokiavibratest


Siemens specific commands
-------------------------

.. option:: siemensnetmonact netmon_type

    Enables network monitor in Siemens phone. Currently known values for type
    are 1 for full and 2 for simple mode.

.. option:: siemensnetmonitor test

.. option:: siemenssatnetmon

Network commands
----------------

.. option:: getgprspoint start [stop]

.. option:: listnetworks [country]

    Show names/codes of GSM networks known for Gammu

.. option:: networkinfo

    Show information about network status from the phone.

.. option:: setautonetworklogin

WAP settings and bookmarks commands
-----------------------------------

.. option:: deletewapbookmark start [stop]

    Delete WAP bookmarks from phone.

    Locations are numerated from 1.

.. option:: getchatsettings start [stop]

.. option:: getsyncmlsettings start [stop]

.. option:: getwapbookmark start [stop]

    Get WAP bookmarks from phone.

    Locations are numerated from 1.

.. option:: getwapsettings start [stop]

    Get WAP settings from phone.

    Locations are numerated from 1.

MMS and MMS settings commands
-----------------------------

.. option:: getallmms [-save]

.. option:: geteachmms [-save]

.. option:: getmmsfolders

.. option:: getmmssettings start [stop]

.. option:: readmmsfile file [-save]


FM radio commands
-----------------

.. option:: getfmstation start [stop]

    Show info about FM stations in phone

Phone information commands
--------------------------

.. option:: battery

    Displays information about battery and power source.

.. option:: getdisplaystatus

.. option:: getlocation

    Gets network information from phone (same as networkinfo)
    and prints location (latitude and longitude) based on information from
    `OpenCellID <http://opencellid.org>`_.

.. option:: getsecuritystatus

    Show, if phone wait for security code (like PIN, PUK, etc.) or not

.. option:: identify

    Show the most important phone data.

.. option:: monitor [times]

    Retrieves phone status and writes it continuously to standard output. Press
    :kbd:`Ctrl+C` to interrupt this command.

    If no parameter is given, the program runs until interrupted, otherwise
    only given number of iterations is performed.

    This command outputs almost all information Gammu supports:

    * Number of contacts, calendar and todo entries, messages, calls, etc.
    * Signal strength.
    * Battery state.
    * Currently used network.
    * Notifications of incoming messages and calls.


Phone settings commands
-----------------------

.. option:: getcalendarsettings

    Displays calendar settings like first day of
    week or automatic deleting of old entries.

.. option:: getprofile start [stop]

.. option:: resetphonesettings PHONE|DEV|UIF|ALL|FACTORY

    .. warning:: This will delete user data, be careful.

    Reset phone settings.

    ``PHONE``
        Clear phone settings.

    ``DEV``
        Clear device settings.

    ``ALL``
        Clear user settings.

        * removes or set logos to default
        * set default phonebook and other menu settings
        * clear T9 words,
        * clear call register info
        * set default profiles settings
        * clear user ringtones

    ``UIF``
        Clear user settings and disables hidden menus.

        * changes like after ``ALL``
        * disables netmon and PPS (all "hidden" menus)

    ``FACTORY``
        Reset to factory defaults.

        * changes like after ``UIF``
        * clear date/time


Dumps decoding commands
-----------------------

.. note:: These commands are available only if Gammu was compiled with debugging options.

.. option:: decodebinarydump file [phonemodel]

    Decodes a dump made by Gammu with
    :config:option:`LogFormat` set to ``binary``.

.. option:: decodesniff MBUS2|IRDA file [phonemodel]

    Allows one to decode sniffs. See :ref:`Discovering protocol` for more details.

Other commands
--------------

.. option:: entersecuritycode PIN|PUK|PIN2|PUK2|PHONE|NETWORK code|- [newpin|-]

    Allow one to enter security code from PC. When code is ``-``, it is read from stdin.

    In case entering PUK, some phones require you to set new PIN as well.

.. option:: presskeysequence mMnNpPuUdD+-123456789*0#gGrR<>[]hHcCjJfFoOmMdD@

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

.. option:: reset SOFT|HARD

    Make phone reset:

    ``SOFT``
        without asking for PIN
    ``HARD``
        with asking for PIN

    .. note:: Some phones will ask for PIN even with ``SOFT`` option.
    .. warning:: Some phones will reset user data on ``HARD`` reset.

.. option:: screenshot filename

    Captures phone screenshot and saves it as filename. The extension is
    automatically appended to filename based on what data phone provides.

Batch mode commands
-------------------

.. option:: batch [file]

    Starts Gammu in a batch mode. In this mode you can issue
    several commands each on one line. Lines starting with # are treated as a
    comments.

    By default, commands are read from standard input, but you can optionally
    specify a file from where they would be read (special case ``-`` means standard
    input).

Configuration commands
----------------------

.. option:: searchphone [-debug]

    Attempts to search for a connected phone.

    .. warning::

       Please note that this can take a very long time, but in case you have
       no clue how to configure phone connection, this is a convenient way to
       find working setup for Gammu.

.. option:: install [-minimal]

    Installs applet for currently configured connection to the phone.

    You can configure search path for instllation files by
    :config:option:`DataPath`.

    The -minimal parameter forces installation of applet only without possible
    support libraries, this can be useful for updates.

Gammu information commands
--------------------------

.. option:: checkversion [STABLE]

    Checks whether there is newer Gammu version
    available online (if Gammu has been compiled with CURL). If you pass
    additional parameter ``STABLE``, only stable versions will be checked.

.. option:: features

    Print information about compiled in features.

.. option:: help [topic]

    Print help. By default general help is printed, but you can
    also specify a help category to get more detailed help on some topic.

.. option:: version

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
    Functionality has been moved. For example to :ref:`gammu-smsd`.

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
    Service is not running.
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
170
    Operation aborted.
171
    Installation data not found, please consult debug log and/or documentation for more details.
172
    Entry is read only.


Examples
++++++++

Configuration
-------------

To check it out, you need to have configuration file for gammu, see
:ref:`gammurc` for more details about it.

Sending messages
----------------

.. note::

    All messages bellow are sent to number 123456, replace it with proper destination.

Send text message up to standard 160 chars:

.. code-block:: sh

    echo "All your base are belong to us" | gammu sendsms TEXT 123456

or

.. code-block:: sh

    gammu sendsms TEXT 123456 -text "All your base are belong to us"

Send long text message:

.. code-block:: sh

    echo "All your base are belong to us" | gammu sendsms TEXT 123456 -len 400

or

.. code-block:: sh

    gammu sendsms TEXT 123456 -len 400 -text "All your base are belong to us"

or

.. code-block:: sh

    gammu sendsms EMS 123456 -text "All your base are belong to us"

Send some funky message with predefined sound and animation from 2 bitmaps:

.. code-block:: sh

    gammu sendsms EMS 123456 -text "Greetings" -defsound 1 -text "from Gammu -tone10 axelf.txt -animation 2 file1.bmp file2.bmp

Send protected message with ringtone:

.. code-block:: sh

    gammu sendsms EMS 123456 -protected 2 -variablebitmaplong ala.bmp -toneSElong axelf.txt -toneSE ring.txt


Retrieving USSD replies
-----------------------

For example for retrieving prepaid card status or retrieving various network info:

.. code-block:: sh

    gammu getussd '#555#'

Uploading files to Nokia
------------------------

Add Alien to applications in your phone (you need to have files Alien.JAD and Alien.JAR in current directory):

.. code-block:: sh

    gammu nokiaaddfile APPLICATION Alien

Add file.mid to ringtones folder:

.. code-block:: sh

    gammu nokiaaddfile TONES file.mid

Setting operator logo
---------------------

Set logo for network ``230 03`` (Vodafone CZ):

.. code-block:: sh

    gammu setbitmap OPERATOR ala.bmp "230 03"

.. _converting-formats:

Converting file formats
-----------------------

The formats conversion can done using :option:`gammu savefile` or
:option:`gammu convertbackup` commands.

Convert single entry (at position 260) from :ref:`gammu-backup` to vCalendar:

.. code-block:: sh

    gammu savefile CALENDAR output.vcs myCalendar.backup 260

Convert first phonebook entry from :ref:`gammu-backup` to vCard:

.. code-block:: sh

    gammu savefile VCARD21 output.vcf phone.backup ME 1

Convert all contacts from backup to vCard:

.. code-block:: sh

    gammu convertbackup phone.backup output.vcf


Reporting bugs
--------------

There are definitely many bugs, reporting to author is welcome. Please include
some useful information when sending bug reports (especially debug logs,
operating system, it's version and phone information are needed).

To generate debug log, enable it in :ref:`gammurc`:

.. code-block:: ini

    [gammu]
    YOUR CONNECTION SETTINGS
    logfile = /tmp/gammu.log
    logformat = textall

Alternatively you can specify logging on command line:

.. code-block:: sh

   gammu -d textall -f /tmp/gammu.log ...

With this settings, Gammu generates /tmp/gammu.log on each connection to
phone and stores dump of communication there. You can also find some
hints for improving support for your phone in this log.

See <http://wammu.eu/support/bugs/> for more information on reporting bugs.

Please report bugs to `Gammu bug tracker <http://bugs.cihar.com/>`_.
