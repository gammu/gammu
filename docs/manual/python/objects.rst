Objects
=======

For various (mostly historical) reasons, all objects you get from Gammu are
not real objects but rather a dictionaries. This has quite a big impact of
usability and will most likely change in the future.

All the objects basically map to C structures, so you might also refer to
:ref:`libgammu` chapter.

.. _sms_obj:

SMS Object
----------

Object describing single SMS message in a way GSM network handles is (140
bytes of data). You can construct it from :ref:`sms_info_obj` using
:meth:`gammu.EncodeSMS`.

Message dictionary can consist of following fields:

.. describe:: SMSC

   SMSC information, see :ref:`smsc_obj`.

.. describe:: Number

   Recipient number, needs to be set for sending.

.. describe:: Name

   Name of the message, does not make any effect on sending, some phones might
   store it.

.. describe:: UDH

   User defined headers for SMS, see :ref:`udh_obj`.

.. describe:: Text

   Message text

.. describe:: Folder

   Folder where the message is stored

.. describe:: Location

   Location where the message is stored

.. describe:: InboxFolder

   Indication whether folder is an inbox

.. describe:: DeliveryStatus

   Message delivery status, used only for received messages

.. describe:: ReplyViaSameSMSC

   Flag indicating whether reply using same SMSC is requested

.. describe:: Class

   Message class

.. describe:: MessageReference

   Message reference number, used mostly to identify delivery reports

.. describe:: ReplaceMessage

   Id of message which this message is supposed to replace

.. describe:: RejectDuplicates

   Whether to reject duplicates

.. describe:: Memory

   Memory where the message is stored

.. describe:: Type

   Message type, one of:

* ``Submit`` - message to be send
* ``Deliver`` - delivered message
* ``Status_Report`` - when creating new message this will create submit message
  with request for delivery report

.. describe:: Coding

   Message encoding, one of:

* ``Unicode_No_Compression`` - unicode message which can contain any chars,
  but can be only 70 chars long
* ``Unicode_Compression`` - not supported by Gammu and most phones
* ``Default_No_Compression`` - message with GSM alphabet only, up to 160 chars
  long
* ``Default_Compression`` - not supported by Gammu and most phones
* ``8bit`` - for binary messages

.. describe:: DateTime

   Timestamp when the message was received or sent.

   Please note that most phones do no record timestamp of sent messages.

.. describe:: SMSCDateTime

   Timestamp when the message was at SMSC.

.. describe:: State

   Message state, one of:

* ``Sent``
* ``UnSent``
* ``Read``
* ``UnRead``

Examples::

    # Simple message to send, using SMSC from phone
    SMS_1 = {
        'Number': '123465',
        'SMSC': {'Location': 1},
        'Text': 'Hello world!',
    }

    # Class 0 (on display) message using custom SMSC number
    SMS_2 = {
        'Number': '123465',
        'SMSC': {'Number': '+420987654321'},
        'Text': 'Hello world!',
        'Class': 0,
    }


.. _udh_obj:

UDH Object
----------

UDH dictionary can consist of following fields:

.. describe:: ID8bit

   8-bit ID of the message, not required

.. describe:: ID16bit

   16-bit ID of the message, not required

.. describe:: PartNumber

   Number of current part

.. describe:: AllParts

   Count of all message parts

.. describe:: Type

   UDH type, one of predefined strings:

* ``NoUDH``
* ``ConcatenatedMessages``
* ``ConcatenatedMessages16bit``
* ``DisableVoice``
* ``DisableFax``
* ``DisableEmail``
* ``EnableVoice``
* ``EnableFax``
* ``EnableEmail``
* ``VoidSMS``
* ``NokiaRingtone``
* ``NokiaRingtoneLong``
* ``NokiaOperatorLogoLong``
* ``NokiaCallerLogo``
* ``NokiaWAP``
* ``NokiaWAPLong``
* ``NokiaCalendarLong``
* ``NokiaProfileLong``
* ``NokiaPhonebookLong``
* ``UserUDH``

.. describe:: Text

   UDH content

Example:

.. code-block:: python

    UDH = {
        "ID8bit": 0xCD,
        "PartNumber": 1,
        "AllParts": 2,
        "Type": "ConcatenatedMessages",
    }

.. _smsc_obj:

SMSC Object
-----------

SMSC dictionary can consist of following fields:

.. describe:: Location

   Location where the SMSC is stored

.. describe:: Number

   SMSC number

.. describe:: Name

   Name of the SMSC configuration

.. describe:: DefaultNumber

   Default recipient number, ignored on most phones

.. describe:: Format

   Default message format, one of:

* ``Text``
* ``Pager``
* ``Fax``
* ``Email``

.. describe:: Validity

   Default message validity as a string

* ``NA`` - validity not available
* ``Max`` - maximum validity allowed by network
* ``nM``, ``nH``, ``nD``, ``nW`` - period defined in minutes, hours, days or
  weeks, eg. ``3W``

Example:

.. code-block:: python

    SMSC = {
        "Location": 1,
        "Number": "+420987654321",
        "Format": "Text",
        "Validity": "Max",
    }

.. _sms_info_obj:

SMS Info Object
---------------

Message info dictionary can consist of following fields:

.. describe:: Unicode

   Whether to use Unicode for the message.

.. describe:: ReplaceMessage

   Id of message which this message is supposed to replace

.. describe:: Unknown

   Boolean flag indicating there was some part which Gammu could not decode.

.. describe:: Class

   Message class

.. describe:: Entries

   Actual message data, see :ref:`sms_info_part_obj`.

Example:

.. code-block:: python

    SMSINFO = {
        "Class": 1,
        "Entries": [
            {"ID": "Text", "Buffer": "This is a "},
            {"ID": "Text", "Buffer": "message", "Italic": True},
            {"ID": "Text", "Buffer": " from "},
            {"ID": "Text", "Buffer": "Gammu", "Bold": True},
        ],
    }

.. _sms_info_part_obj:

SMS Info Part Object
--------------------

Message component can consist of following fields:

.. describe:: ID

   Identification of the part type:

* ``Text``
* ``ConcatenatedTextLong``  - Contacenated SMS, when longer than 1 SMS.
* ``ConcatenatedAutoTextLong`` - Contacenated SMS, auto Default/Unicode coding.
* ``ConcatenatedTextLong16bit``
* ``ConcatenatedAutoTextLong16bit``
* ``NokiaProfileLong`` -  Nokia profile = Name`` Ringtone`` ScreenSaver
* ``NokiaPictureImageLong`` -  Nokia Picture Image + (text)
* ``NokiaScreenSaverLong`` -  Nokia screen saver + (text)
* ``NokiaRingtone`` -  Nokia ringtone - old SM2.0 format`` 1 SMS
* ``NokiaRingtoneLong`` -  Nokia ringtone concatenated`` when very long
* ``NokiaOperatorLogo`` -  Nokia 72x14 operator logo`` 1 SMS
* ``NokiaOperatorLogoLong`` -  Nokia 72x14 op logo or 78x21 in 2 SMS
* ``NokiaCallerLogo`` -  Nokia 72x14 caller logo`` 1 SMS
* ``NokiaWAPBookmarkLong`` -  Nokia WAP bookmark in 1 or 2 SMS
* ``NokiaWAPSettingsLong`` -  Nokia WAP settings in 2 SMS
* ``NokiaMMSSettingsLong`` -  Nokia MMS settings in 2 SMS
* ``NokiaVCARD10Long`` -  Nokia VCARD 1.0 - only name and default number
* ``NokiaVCARD21Long`` -  Nokia VCARD 2.1 - all numbers + text
* ``NokiaVCALENDAR10Long`` -  Nokia VCALENDAR 1.0 - can be in few sms
* ``NokiaVTODOLong``
* ``VCARD10Long``
* ``VCARD21Long``
* ``DisableVoice``
* ``DisableFax``
* ``DisableEmail``
* ``EnableVoice``
* ``EnableFax``
* ``EnableEmail``
* ``VoidSMS``
* ``EMSSound10`` -  IMelody 1.0
* ``EMSSound12`` -  IMelody 1.2
* ``EMSSonyEricssonSound``  -  IMelody without header - SonyEricsson extension
* ``EMSSound10Long`` -  IMelody 1.0 with UPI.
* ``EMSSound12Long`` -  IMelody 1.2 with UPI.
* ``EMSSonyEricssonSoundLong`` -  IMelody without header with UPI.
* ``EMSPredefinedSound``
* ``EMSPredefinedAnimation``
* ``EMSAnimation``
* ``EMSFixedBitmap`` -  Fixed bitmap of size 16x16 or 32x32.
* ``EMSVariableBitmap``
* ``EMSVariableBitmapLong``
* ``MMSIndicatorLong`` - MMS message indicator.
* ``WAPIndicatorLong``
* ``AlcatelMonoBitmapLong`` - Variable bitmap with black and white colors
* ``AlcatelMonoAnimationLong`` -  Variable animation with black and white colors
* ``AlcatelSMSTemplateName``
* ``SiemensFile`` - Siemens OTA

.. describe:: Left

   Text formatting

.. describe:: Right

   Text formatting

.. describe:: Center

   Text formatting

.. describe:: Large

   Text formatting

.. describe:: Small

   Text formatting

.. describe:: Bold

   Text formatting

.. describe:: Italic

   Text formatting

.. describe:: Underlined

   Text formatting

.. describe:: Strikethrough

   Text formatting

.. describe:: Protected

   Whether message part should be protected (DRM)

.. describe:: Number

   Number to encode in message.

.. describe:: Ringtone

   Ringtone to encode in message.

.. describe:: Bitmap

   Bitmap to encode in message.

.. describe:: Bookmark

   Bookmark to encode in message.

.. describe:: Settings

   Settings to encode in message.

.. describe:: MMSIndicator

   MMS indication to encode in message.

.. describe:: Phonebook

   Phonebook entry to encode in message, see :ref:`pbk_obj`.

.. describe:: Calendar

   Calendar entry to encode in message, see :ref:`cal_obj`.

.. describe:: ToDo

   Todo entry to encode in message, see :ref:`todo_obj`.

.. describe:: File

   File to encode in message, see :ref:`file_obj`.

.. describe:: Buffer

   String to encode in message.

.. _todo_obj:

Todo Object
-----------

Todo entry is a dictionary consisting of following fields:

.. describe:: Location

   Location where the entry is stored

.. describe:: Type

   Type of entry, one of:

* ``REMINDER`` - Reminder or Date
* ``CALL`` - Call
* ``MEETING`` - Meeting
* ``BIRTHDAY`` - Birthday or Anniversary or Special Occasion
* ``MEMO`` - Memo or Miscellaneous
* ``TRAVEL`` - Travel
* ``VACATION`` - Vacation
* ``T_ATHL`` - Training - Athletism
* ``T_BALL`` - Training - Ball Games
* ``T_CYCL`` - Training - Cycling
* ``T_BUDO`` - Training - Budo
* ``T_DANC`` - Training - Dance
* ``T_EXTR`` - Training - Extreme Sports
* ``T_FOOT`` - Training - Football
* ``T_GOLF`` - Training - Golf
* ``T_GYM`` - Training - Gym
* ``T_HORS`` - Training - Horse Race
* ``T_HOCK`` - Training - Hockey
* ``T_RACE`` - Training - Races
* ``T_RUGB`` - Training - Rugby
* ``T_SAIL`` - Training - Sailing
* ``T_STRE`` - Training - Street Games
* ``T_SWIM`` - Training - Swimming
* ``T_TENN`` - Training - Tennis
* ``T_TRAV`` - Training - Travels
* ``T_WINT`` - Training - Winter Games
* ``ALARM`` - Alarm
* ``DAILY_ALARM`` - Alarm repeating each day.

.. describe:: Priority

   Entry priority, one of:

* ``High``
* ``Medium``
* ``Low``
* ``None``

.. describe:: Entries

   Actual entries, see :ref:`todo_entry_obj`

Example:

.. code-block:: python

    TODO = {
        "Type": "MEMO",
        "Entries": [
            {
                "Type": "END_DATETIME",
                "Value": datetime.datetime.now() + datetime.timedelta(days=1),
            },
            {"Type": "TEXT", "Value": "Buy some milk"},
        ],
    }

.. _todo_entry_obj:

Todo Entries Object
-------------------

.. describe:: Type

   Type of entry, one of:

* ``END_DATETIME`` - Due date (Date).
* ``COMPLETED`` - Whether is completed (Number).
* ``ALARM_DATETIME`` - When should alarm be fired (Date).
* ``SILENT_ALARM_DATETIME`` - When should silent alarm be fired (Date).
* ``TEXT`` - Text of to do (Text).
* ``DESCRIPTION`` - Description of to do (Text).
* ``LOCATION`` - Location of to do (Text).
* ``PRIVATE`` - Whether entry is private (Number).
* ``CATEGORY`` - Category of entry (Number).
* ``CONTACTID`` - Related contact ID (Number).
* ``PHONE`` - Number to call (Text).
* ``LUID`` - IrMC LUID which can be used for synchronisation (Text).
* ``LAST_MODIFIED`` - Date and time of last modification (Date).
* ``START_DATETIME`` - Start date (Date).

.. describe:: Value

   Actual value, corresponding type to Type field.

.. _cal_obj:

Calendar Object
---------------

Calendar entry is a dictionary consisting of following fields:

.. describe:: Location

   Location where the entry is stored

.. describe:: Type

   Type of entry, one of:

* ``REMINDER`` - Reminder or Date
* ``CALL`` - Call
* ``MEETING`` - Meeting
* ``BIRTHDAY`` - Birthday or Anniversary or Special Occasion
* ``MEMO`` - Memo or Miscellaneous
* ``TRAVEL`` - Travel
* ``VACATION`` - Vacation
* ``T_ATHL`` - Training - Athletism
* ``T_BALL`` - Training - Ball Games
* ``T_CYCL`` - Training - Cycling
* ``T_BUDO`` - Training - Budo
* ``T_DANC`` - Training - Dance
* ``T_EXTR`` - Training - Extreme Sports
* ``T_FOOT`` - Training - Football
* ``T_GOLF`` - Training - Golf
* ``T_GYM`` - Training - Gym
* ``T_HORS`` - Training - Horse Race
* ``T_HOCK`` - Training - Hockey
* ``T_RACE`` - Training - Races
* ``T_RUGB`` - Training - Rugby
* ``T_SAIL`` - Training - Sailing
* ``T_STRE`` - Training - Street Games
* ``T_SWIM`` - Training - Swimming
* ``T_TENN`` - Training - Tennis
* ``T_TRAV`` - Training - Travels
* ``T_WINT`` - Training - Winter Games
* ``ALARM`` - Alarm
* ``DAILY_ALARM`` - Alarm repeating each day.

.. describe:: Entries

   Actual entries, see :ref:`cal_entry_obj`

Example:

.. code-block:: python

    CAL = {
        "Type": "MEMO",
        "Entries": [
            {"Type": "START_DATETIME", "Value": datetime.datetime.now()},
            {
                "Type": "END_DATETIME",
                "Value": datetime.datetime.now() + datetime.timedelta(days=1),
            },
            {"Type": "LOCATION", "Value": "Home"},
            {"Type": "TEXT", "Value": "Relax for one day"},
        ],
    }

.. _cal_entry_obj:

Calendar Entries Object
-----------------------

.. describe:: Type

   Type of entry, one of:

* ``START_DATETIME`` - Date and time of event start.
* ``END_DATETIME`` - Date and time of event end.
* ``TONE_ALARM_DATETIME`` - Alarm date and time.
* ``SILENT_ALARM_DATETIME`` - Date and time of silent alarm.
* ``TEXT`` - Text.
* ``DESCRIPTION`` - Detailed description.
* ``LOCATION`` - Location.
* ``PHONE`` - Phone number.
* ``PRIVATE`` - Whether this entry is private.
* ``CONTACTID`` - Related contact id.
* ``REPEAT_DAYOFWEEK`` - Repeat each x'th day of week.
* ``REPEAT_DAY`` - Repeat each x'th day of month.
* ``REPEAT_DAYOFYEAR`` - Repeat each x'th day of year.
* ``REPEAT_WEEKOFMONTH`` - Repeat x'th week of month.
* ``REPEAT_MONTH`` - Repeat x'th month.
* ``REPEAT_FREQUENCY`` - Repeating frequency.
* ``REPEAT_STARTDATE`` - Repeating start.
* ``REPEAT_STOPDATE`` - Repeating end.
* ``REPEAT_COUNT`` - Number of repetitions.
* ``LUID`` - IrMC LUID which can be used for synchronisation.
* ``LAST_MODIFIED`` - Date and time of last modification.

.. describe:: Value

   Actual value, corresponding type to Type field.

.. _pbk_obj:

Phonebook Object
----------------

Phonebook entry is a dictionary consisting of following fields:

.. describe:: Location

   Location where the entry is stored

.. describe:: MemoryType

   Memory where the message is stored

.. describe:: Entries

   Actual entries, see :ref:`pbk_entry_obj`

Example:

.. code-block:: python

    PBK = {
        "Location": 1000,
        "MemoryType": "ME",
        "Entries": [
            {"Type": "Number_General", "Value": "+420123456789"},
            {"Type": "Text_Name", "Value": "Stojan Jakotyc"},
        ],
    }

.. _pbk_entry_obj:

Phonebook Entries Object
------------------------

.. describe:: Type

    Type of entry, one of:

    * ``Number_General`` - General number. (Text)
    * ``Number_Mobile`` - Mobile number. (Text)
    * ``Number_Fax`` - Fax number. (Text)
    * ``Number_Pager`` - Pager number. (Text)
    * ``Number_Other`` - Other number. (Text)
    * ``Text_Note`` - Note. (Text)
    * ``Text_Postal`` - Complete postal address. (Text)
    * ``Text_Email`` - Email. (Text)
    * ``Text_Email2`` - Second email. (Text)
    * ``Text_URL`` - URL (Text)
    * ``Date`` - Date and time of last call. (Date)
    * ``Caller_Group`` - Caller group. (Number)
    * ``Text_Name`` - Name (Text)
    * ``Text_LastName`` - Last name. (Text)
    * ``Text_FirstName`` - First name. (Text)
    * ``Text_Company`` - Company. (Text)
    * ``Text_JobTitle`` - Job title. (Text)
    * ``Category`` - Category. (Number, if -1 then text)
    * ``Private`` - Whether entry is private. (Number)
    * ``Text_StreetAddress`` - Street address. (Text)
    * ``Text_City`` - City. (Text)
    * ``Text_State`` - State. (Text)
    * ``Text_Zip`` - Zip code. (Text)
    * ``Text_Country`` - Country. (Text)
    * ``Text_Custom1`` - Custom information 1. (Text)
    * ``Text_Custom2`` - Custom information 2. (Text)
    * ``Text_Custom3`` - Custom information 3. (Text)
    * ``Text_Custom4`` - Custom information 4. (Text)
    * ``RingtoneID`` - Ringtone ID. (Number)
    * ``PictureID`` - Picture ID. (Number)
    * ``Text_UserID`` - User ID. (Text)
    * ``CallLength`` - Length of call (Number)
    * ``Text_LUID`` - LUID - Unique Identifier used for synchronisation (Text)
    * ``LastModified`` - Date of last modification (Date)
    * ``Text_NickName`` - Nick name (Text)
    * ``Text_FormalName`` - Formal name (Text)
    * ``Text_PictureName`` - Picture name (on phone filesystem). (Text)
    * ``PushToTalkID`` - Push-to-talk ID (Text)
    * ``Number_Messaging`` - Favorite messaging number. (Text)
    * ``Photo`` - Photo (Picture).
    * ``SecondName`` - Second name. (Text)
    * ``VOIP`` - VOIP address (Text).
    * ``SIP`` - SIP address (Text).
    * ``DTMF`` - DTMF (Text).
    * ``Video`` - Video number. (Text)
    * ``SWIS`` - See What I See address. (Text)
    * ``WVID`` - Wireless Village user ID. (Text)
    * ``NamePrefix`` - Name prefix (Text)
    * ``NameSuffix`` - Name suffix (Text)

.. describe:: Location

    Location for the field:

    * ``Unknown`` - not define
    * ``Home`` - home
    * ``Work`` - work

.. describe:: Value

   Actual value, corresponding type to Type field.

.. describe:: PictureType

   Type of picture which is stored in Value field (only for Picture fields).

.. _file_obj:

File Object
-----------

File is a dictionary consisting of following fields:

.. describe:: Used

   Number of bytes used by this file.

.. describe:: Name

   File name.

.. describe:: Folder

   Boolean value indicating whether this is a folder.

.. describe:: Level

   Depth of file on the filesystem.

.. describe:: Type

   File type, one of:

* ``Other``
* ``Java_JAR``
* ``Image_JPG``
* ``Image_BMP``
* ``Image_GIF``
* ``Image_PNG``
* ``Image_WBMP``
* ``Video_3GP``
* ``Sound_AMR``
* ``Sound_NRT`` - DCT4 binary format
* ``Sound_MIDI``
* ``MMS``

.. describe:: ID_FullName

   Full file name including path.

.. describe:: Buffer

   Content of the file.

.. describe:: Modified

   Timestamp of last change

.. describe:: Protected

   Boolean value indicating whether file is protected (DRM).

.. describe:: ReadOnly

   Boolean value indicating whether file is read only.

.. describe:: Hidden

   Boolean value indicating whether file is hidden.

.. describe:: System

   Boolean value indicating whether file is system.

.. describe:: Pos

   Current position of file upload

.. describe:: Finished

   Boolean value indicating completed file transfer.

Example:

.. code-block:: python

    FILE = {
        "ID_FullName": PATH,
        "Name": os.path.basename(PATH),
        "Buffer": data,
        "Protected": 0,
        "ReadOnly": 0,
        "Hidden": 0,
        "System": 0,
        "Folder": 0,
        "Level": 0,
        "Type": "Other",
        "Finished": 0,
        "Pos": 0,
    }

.. _divert-type:

Divert Type
-----------

The divert type can have one of following values:

* ``Busy`` - Divert when busy.
* ``NoAnswer`` - Divert when not answered.
* ``OutOfReach`` - Divert when phone off or no coverage.
* ``AllTypes`` - Divert all calls without ringing.

.. _divert-call:

Call Type
---------

The call type for diverts can have one of following values:

* ``Voice`` - Voice calls.
* ``Fax`` - Fax calls.
* ``Data`` - Data calls.
* ``All`` - All calls.

.. _divert-entry:

Call Divert Objects
-------------------

.. describe:: DivertType

    When to do the divert, see :ref:`divert-type`.

.. describe:: CallType

    What call types to divert, see :ref:`divert-call`.

.. describe:: Number

    Phone number where to divert.

.. describe:: Timeout

    Timeout after which the divert will happen.
