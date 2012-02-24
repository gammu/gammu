:mod:`gammu` -- Mobile phone access
===================================

.. module:: gammu
    :synopsis: Provides access to mobile phones.

This module wraps all python-gammu functionality.

:class:`gammu.StateMachine`
---------------------------

.. class:: StateMachine(Locale)

   StateMachine object, that is used for communication with phone.

   :param Locale: What locales to use for gammu error messages, default is ``auto`` which does autodetection according to user locales
   :type Locale: string


   .. method:: AddCalendar(Value)

      Adds calendar entry.

      :param Value: Calendar entry data, see :ref:`cal_obj`
      :type Value: dict
      :return: Location of newly created entry
      :rtype: int


   .. method:: AddCategory(Type, Name)

      Adds category to phone.

      :param Type: Type of category to read, one of ``ToDo``, ``Phonebook``
      :type Type: string
      :param Name: Category name
      :type Name: string
      :return: Location of created category
      :rtype: int


   .. method:: AddFilePart(File)

      Adds file part to filesystem.

      :param File: File data, see :ref:`file_obj`
      :type File: dict
      :return: File data for subsequent calls (Finished indicates transfer has been completed)
      :rtype: dict


   .. method:: AddFolder(ParentFolderID, Name)

      Adds folder to filesystem.

      :param ParentFolderID: Folder where to create subfolder
      :type ParentFolderID: string
      :param Name: New folder name
      :type Name: string
      :return: New folder ID.
      :rtype: string


   .. method:: AddMemory(Value)

      Adds memory (phonebooks or calls) entry.

      :param Value: Memory entry, see :ref:`pbk_obj`
      :type Value: dict
      :return: Location of created entry
      :rtype: int


   .. method:: AddSMS(Value)

      Adds SMS to specified folder.

      :param Value: SMS data, see :ref:`sms_obj`
      :type Value: dict
      :return: Tuple for location and folder.
      :rtype: tuple


   .. method:: AddSMSFolder(Name)

      Creates SMS folder.

      :param Name: Name of new folder
      :type Name: string
      :return: None
      :rtype: None


   .. method:: AddToDo(Value)

      Adds ToDo in phone.

      :param Value: ToDo data, see :ref:`todo_obj`
      :type Value: dict
      :return: Location of created entry
      :rtype: int


   .. method:: AnswerCall(ID, All)

      Accept current incoming call.

      :param ID: ID of call
      :type ID: integer
      :param All: Answer all calls?
      :type All: boolean
      :return: None
      :rtype: None

   .. method:: CancelAllDiverts()

      .. versionadded:: 1.31.90

      Cancels all call diverts.

      :return: None
      :rtype: None


   .. method:: CancelCall(ID, All)

      Deny current incoming call.

      :param ID: ID of call
      :type ID: integer
      :param All: Cancel all calls?
      :type All: boolean
      :return: None
      :rtype: None


   .. method:: ConferenceCall(ID)

      Initiates conference call.

      :param ID: ID of call
      :type ID: integer
      :return: None
      :rtype: None


   .. method:: DeleteAllCalendar()

      Deletes all calendar entries.

      :return: None
      :rtype: None


   .. method:: DeleteAllMemory(Type)

      Deletes all memory (phonebooks or calls) entries of specified type.

      :param Type: Memory type, one of ``ME``, ``SM``, ``ON``, ``DC``, ``RC``, ``MC``, ``MT``, ``FD``, ``VM``
      :type Type: string
      :return: None
      :rtype: None


   .. method:: DeleteAllToDo()

      Deletes all todo entries in phone.

      :return: None
      :rtype: None


   .. method:: DeleteCalendar(Location)

      Deletes calendar entry.

      :param Location: Calendar entry to delete
      :type Location: int
      :return: None
      :rtype: None


   .. method:: DeleteFile(FileID)

      Deletes file from filesystem.

      :param FileID: File to delete
      :type FileID: string
      :return: None
      :rtype: None


   .. method:: DeleteFolder(FolderID)

      Deletes folder on filesystem.

      :param FolderID: Folder to delete
      :type FolderID: string
      :return: None
      :rtype: None


   .. method:: DeleteMemory(Type, Location)

      Deletes memory (phonebooks or calls) entry.

      :param Type: Memory type, one of ``ME``, ``SM``, ``ON``, ``DC``, ``RC``, ``MC``, ``MT``, ``FD``, ``VM``
      :type Type: string
      :param Location: Location of entry to delete
      :type Location: int
      :return: None
      :rtype: None


   .. method:: DeleteSMS(Folder, Location)

      Deletes SMS.

      :param Folder: Folder where to read entry (0 is emulated flat memory)
      :type Folder: int
      :param Location: Location of entry to delete
      :type Location: int
      :return: None
      :rtype: None

      .. note::

            In most cases you want to use Folder=0 as in this mode it will accept locations
            as GetNextSMS returns them.


   .. method:: DeleteSMSFolder(ID)

      Deletes SMS folder.

      :param ID: Index of folder to delete
      :type ID: int
      :return: None
      :rtype: None


   .. method:: DeleteToDo(Location)

      Deletes ToDo entry in phone.

      :param Location: Location of entry to delete
      :type Location: int
      :return: None
      :rtype: None


   .. method:: DialService(Number)

      Dials number and starts voice call.

      :param Number: Number to dial
      :type Number: string
      :return: None
      :rtype: None


   .. method:: DialVoice(Number, ShowNumber)

      Dials number and starts voice call.

      :param Number: Number to dial
      :type Number: string
      :param ShowNumber: Identifies whether to enable CLIR (None = keep default phone settings). Default is None
      :type ShowNumber: boolean or None
      :return: None
      :rtype: None


   .. method:: EnterSecurityCode(Type, Code, NewPIN)

      Entres security code.

      :param Type: What code to enter, one of ``PIN``, ``PUK``, ``PIN2``, ``PUK2``, ``Phone``.
      :type Type: string
      :param Code: Code value
      :type Code: string
      :param NewPIN: New PIN value in case entering PUK
      :type NewPIN: string
      :return: None
      :rtype: None


   .. method:: GetAlarm(Location)

      Reads alarm set in phone.

      :param Location: Which alarm to read. Many phone support only one alarm. Default is 1.
      :type Location: int
      :return: Alarm dict
      :rtype: dict


   .. method:: GetBatteryCharge()

      Gets information about battery charge and phone charging state.

      :return: Dictionary containing information about battery state (BatteryPercent and ChargeState)
      :rtype: dict


   .. method:: GetCalendar(Location)

      Retrieves calendar entry.

      :param Location: Calendar entry to read
      :type Location: int
      :return: Dictionary with calendar values, see :ref:`cal_obj`
      :rtype: dict


   .. method:: GetCalendarStatus()

      Retrieves calendar status (number of used entries).

      :return: Dictionary with calendar status (Used)
      :rtype: dict


   .. method:: GetCallDivert(Divert = 'AllTypes', Type = 'All')

      .. versionadded:: 1.31.90

      Gets call diverts.

      :param Divert: When to do the divert.
      :type Divert: :ref:`divert-type`
      :param Type: What call types to divert.
      :type Type: :ref:`divert-call`
      :return: List of call diverts.
      :rtype: :ref:`divert-entry`

   .. method:: GetCategory(Type, Location)

      Reads category from phone.

      :param Type: Type of category to read, one of ``ToDo``, ``Phonebook``
      :type Type: string
      :param Location: Location of category to read
      :type Location: int
      :return: Category name as string
      :rtype: string


   .. method:: GetCategoryStatus(Type)

      Reads category status (number of used entries) from phone.

      :param Type: Type of category to read, one of ``ToDo``, ``Phonebook``
      :type Type: string
      :return: Dictionary containing information about category status (Used)
      :rtype: dict


   .. method:: GetConfig(Section)

      Gets specified config section. Configuration consists of all params which can be defined in gammurc config file:
       - Model
       - DebugLevel
       - Device
       - Connection
       - SyncTime
       - LockDevice
       - DebugFile
       - StartInfo
       - UseGlobalDebugFile

      :param Section: Index of config section to read. Defaults to 0.
      :type Section: int
      :return: Dictionary containing configuration
      :rtype: dict


   .. method:: GetDateTime()

      Reads date and time from phone.

      :return: Date and time from phone as datetime.datetime object.
      :rtype: datetime.datetime


   .. method:: GetDisplayStatus()

      Acquired display status.
      :return: List of indicators displayed on display
      :rtype: list


   .. method:: GetFilePart(File)

      Gets file part from filesystem.

      :param File: File data, see :ref:`file_obj`
      :type File: dict
      :return: File data for subsequent calls (Finished indicates transfer has been completed), see :ref:`file_obj`
      :rtype: dict


   .. method:: GetFileSystemStatus()

      Acquires filesystem status.

      :return: Dictionary containing filesystem status (Used and Free)
      :rtype: dict


   .. method:: GetFirmware()

      Reads firmware information from phone.

      :return: Tuple from version, date and numeric version.
      :rtype: tuple


   .. method:: GetFolderListing(Folder, Start)

      Gets next filename from filesystem folder.

      :param Folder: Folder to list
      :type Folder: string
      :param Start: Whether we're starting listing. Defaults to False.
      :type Start: boolean
      :return: File data as dict, see :ref:`file_obj`
      :rtype: dict


   .. method:: GetHardware()

      Gets hardware information about device.

      :return: Hardware information as string.
      :rtype: string


   .. method:: GetIMEI()

      Reads IMEI/serial number from phone.

      :return: IMEI of phone as string.
      :rtype: string


   .. method:: GetLocale()

      Gets locale information from phone.

      :return: Dictionary of locale settings. :meth:`SetLocale` lists them all.
      :rtype: dict


   .. method:: GetManufactureMonth()

      Gets month when device was manufactured.

      :return: Month of manufacture as string.
      :rtype: string


   .. method:: GetManufacturer()

      Reads manufacturer from phone.

      :return: String with manufacturer name
      :rtype: string


   .. method:: GetMemory(Type, Location)

      Reads entry from memory (phonebooks or calls). Which entry shouldbe read is defined in entry.

      :param Type: Memory type, one of ``ME``, ``SM``, ``ON``, ``DC``, ``RC``, ``MC``, ``MT``, ``FD``, ``VM``
      :type Type: string
      :return: Memory entry as dict, see :ref:`pbk_obj`
      :rtype: dict


   .. method:: GetMemoryStatus(Type)

      Gets memory (phonebooks or calls) status (eg. number of used andfree entries).

      :param Type: Memory type, one of ``ME``, ``SM``, ``ON``, ``DC``, ``RC``, ``MC``, ``MT``, ``FD``, ``VM``
      :type Type: string
      :return: Dictionary with information about memory (Used and Free)
      :rtype: dict


   .. method:: GetModel()

      Reads model from phone.

      :return: Tuple containing gammu identification and real model returned by phone.
      :rtype: tuple


   .. method:: GetNetworkInfo()

      Gets network information.

      :return: Dictionary with information about network (NetworkName, State, NetworkCode, CID and LAC)
      :rtype: dict


   .. method:: GetNextCalendar(Start, Location)

      Retrieves calendar entry. This is useful for continuous reading of all calendar entries.

      :param Start: Whether to start. This can not be used together with Location
      :type Start: boolean
      :param Location: Last read location. This can not be used together with Start
      :type Location: int
      :return: Dictionary with calendar values, see :ref:`cal_obj`
      :rtype: dict


   .. method:: GetNextFileFolder(Start)

      Gets next filename from filesystem.

      :param Start: Whether we're starting listing. Defaults to False.
      :type Start: boolean
      :return: File data as dict, see :ref:`file_obj`
      :rtype: dict


   .. method:: GetNextMemory(Type, Start, Location)

      Reads entry from memory (phonebooks or calls). Which entry shouldbe read is defined in entry. This can be easily used for reading all entries.

      :param Type: Memory type, one of ``ME``, ``SM``, ``ON``, ``DC``, ``RC``, ``MC``, ``MT``, ``FD``, ``VM``
      :type Type: string
      :param Start: Whether to start. This can not be used together with Location
      :type Start: boolean
      :param Location: Last read location. This can not be used together with Start
      :type Location: int
      :return: Memory entry as dict, see :ref:`pbk_obj`
      :rtype: dict


   .. method:: GetNextRootFolder(Folder)

      Gets next root folder from filesystem. Start with empty folder name.

      :param Folder: Previous read fodlder. Start with empty folder name.
      :type Folder: string
      :return: Structure with folder information


   .. method:: GetNextSMS(Folder, Start, Location)

      Reads next (or first if start set) SMS message. This might befaster for some phones than using :meth:`GetSMS` for each message.

      :param Folder: Folder where to read entry (0 is emulated flat memory)
      :type Folder: int
      :param Start: Whether to start. This can not be used together with Location
      :type Start: boolean
      :param Location: Location last read entry. This can not be used together with Start
      :type Location: int
      :return: Dictionary with SMS data, see :ref:`sms_obj`
      :rtype: dict


   .. method:: GetNextToDo(Start, Location)

      Reads ToDo from phone.

      :param Start: Whether to start. This can not be used together with Location
      :type Start: boolean
      :param Location: Last read location. This can not be used together with Start
      :type Location: int
      :return: Dictionary with ToDo values, see :ref:`todo_obj`
      :rtype: dict


   .. method:: GetOriginalIMEI()

      Gets original IMEI from phone.

      :return: Original IMEI of phone as string.
      :rtype: string


   .. method:: GetPPM()

      Gets PPM (Post Programmable Memory) from phone.

      :return: PPM as string
      :rtype: string


   .. method:: GetProductCode()

      Gets product code of device.
      :return: Product code as string.
      :rtype: string


   .. method:: GetSIMIMSI()

      Gets SIM IMSI from phone.

      :return: SIM IMSI as string
      :rtype: string


   .. method:: GetSMS(Folder, Location)

      Reads SMS message.

      :param Folder: Folder where to read entry (0 is emulated flat memory)
      :type Folder: int
      :param Location: Location of entry to read
      :type Location: int
      :return: Dictionary with SMS data, see :ref:`sms_obj`
      :rtype: dict


   .. method:: GetSMSC(Location)

      Gets SMS Service Center number and SMS settings.

      :param Location: Location of entry to read. Defaults to 1
      :type Location: int
      :return: Dictionary with SMSC information, see :ref:`smsc_obj`
      :rtype: dict


   .. method:: GetSMSFolders()

      Returns SMS folders information.

      :return: List of SMS folders.
      :rtype: list


   .. method:: GetSMSStatus()

      Gets information about SMS memory (read/unread/size of memory for both SIM and phone).

      :return: Dictionary with information about phone memory (SIMUnRead, SIMUsed, SIMSize, PhoneUnRead, PhoneUsed, PhoneSize and TemplatesUsed)
      :rtype: dict


   .. method:: GetSecurityStatus()

      Queries whether some security code needs to be entered.

      :return: String indicating which code needs to be entered or None if none is needed
      :rtype: string


   .. method:: GetSignalQuality()

      Reads signal quality (strength and error rate).

      :return: Dictionary containing information about signal state (SignalStrength, SignalPercent and BitErrorRate)
      :rtype: dict


   .. method:: GetSpeedDial(Location)

      Gets speed dial.

      :param Location: Location of entry to read
      :type Location: int
      :return: Dictionary with speed dial (Location, MemoryLocation, MemoryNumberID, MemoryType)
      :rtype: dict


   .. method:: GetToDo(Location)

      Reads ToDo from phone.

      :param Location: Location of entry to read
      :type Location: int
      :return: Dictionary with ToDo values, see :ref:`todo_obj`
      :rtype: dict


   .. method:: GetToDoStatus()

      Gets status of ToDos (count of used entries).

      :return: Dictionary of status (Used)
      :rtype: dict


   .. method:: HoldCall(ID)

      Holds call.

      :param ID: ID of call
      :type ID: integer
      :return: None
      :rtype: None


   .. method:: Init(Replies)

      Initialises the connection with phone.

      :param Replies: Number of replies to wait for on each request. Defaults to 1. Higher value makes sense only on unreliable links.
      :type Replies: int
      :return: None
      :rtype: None


   .. method:: PressKey(Key, Press)

      Emulates key press.

      :param Key: What key to press
      :type Key: string
      :param Press: Whether to emulate press or release.
      :type Press: boolean
      :return: None
      :rtype: None


   .. method:: ReadConfig(Section, Configuration, Filename)

      Reads specified section of gammurc

      :param Section: Index of config section to read. Defaults to 0.
      :type Section: int
      :param Configuration: Index where config section will be stored. Defaults to Section.
      :type Configuration: int
      :param Filename: Path to configuration file (otherwise it is autodetected).
      :type Filename: string
      :return: None
      :rtype: None


   .. method:: ReadDevice(Wait)

      Reads data from device.

      :param Wait: Whether to wait, default is not to wait.
      :type Wait: boolean
      :return: Number of bytes read
      :rtype: int


   .. method:: Reset(Hard)

      Performs phone reset.

      :param Hard: Whether to make hard reset
      :type Hard: boolean
      :return: None
      :rtype: None


   .. method:: ResetPhoneSettings(Type)

      Resets phone settings.

      :param Type: What to reset, one of ``PHONE``, ``UIF``, ``ALL``, ``DEV``, ``FACTORY``
      :type Type: string
      :return: None
      :rtype: None


   .. method:: SendDTMF(Number)

      Sends DTMF (Dual Tone Multi Frequency) tone.

      :param Number: Number to dial
      :type Number: string
      :return: None
      :rtype: None


   .. method:: SendFilePart(File)

      Sends file part to phone.

      :param File: File data, see :ref:`file_obj`
      :type File: dict
      :return: File data for subsequent calls (Finished indicates transfer has been completed), see :ref:`file_obj`
      :rtype: dict


   .. method:: SendSMS(Value)

      Sends SMS.

      :param Value: SMS data, see :ref:`sms_obj`
      :type Value: dict
      :return: Message reference as integer
      :rtype: int


   .. method:: SendSavedSMS(Folder, Location)

      Sends SMS saved in phone.

      :param Folder: Folder where to read entry (0 is emulated flat memory)
      :type Folder: int
      :param Location: Location of entry to send
      :type Location: int
      :return: Message reference as integer
      :rtype: int


   .. method:: SetAlarm(DateTime, Location, Repeating, Text)

      Sets alarm in phone.

      :param DateTime: When should alarm happen.
      :type DateTime: datetime.datetime
      :param Location: Location of alarm to set. Defaults to 1.
      :type Location: int
      :param Repeating: Whether alarm should be repeating. Defaults to True.
      :type Repeating: boolean
      :param Text: Text to be displayed on alarm. Defaults to empty.
      :type Text: string
      :return: None
      :rtype: None


   .. method:: SetAutoNetworkLogin()

      Enables network auto login.

      :return: None
      :rtype: None


   .. method:: SetCalendar(Value)

      Sets calendar entry

      :param Value: Calendar entry data, see :ref:`cal_obj`
      :type Value: dict
      :return: Location of set entry
      :rtype: int


   .. method:: SetConfig(Section, Values)

      Sets specified config section.

      :param Section: Index of config section to modify
      :type Section: int
      :param Values: Config values, see :meth:`GetConfig` for description of accepted
      :type Values: dict
      :return: None
      :rtype: None

   .. method:: SetCallDivert(Divert, Type, Number, Timeout=0)

      .. versionadded:: 1.31.90

      Sets call divert.

      :param Divert: When to do the divert.
      :type Divert: :ref:`divert-type`
      :param Type: What call types to divert.
      :type Type: :ref:`divert-call`
      :param Number: Phone number where to divert.
      :type Number: string
      :param Timeout: Optional timeout when divert happens.
      :type Timeout: int
      :return: None
      :rtype: None

   .. method:: SetDateTime(Date)

      Sets date and time in phone.

      :param Date: Date to set
      :type Date: datetime.datetime
      :return: None
      :rtype: None


   .. method:: SetDebugFile(File, Global)

      Sets state machine debug file.

      :param File: File where to write debug stuff (as configured by :meth:`SetDebugLevel`). Can be either None for no file, Python file object or filename.
      :type File: mixed
      :param Global: Whether to use global debug structure (overrides File)
      :type Global: boolean
      :return: None
      :rtype: None


   .. method:: SetDebugLevel(Level)

      Sets state machine debug level accorting to passed string. You need to configure output file using :meth:`SetDebugFile` to activate it.

      :type Level: string
      :param Level: name of debug level to use, currently one of:
         - nothing
         - text
         - textall
         - binary
         - errors
         - textdate
         - textalldate
         - errorsdate
      :return: None
      :rtype: None


   .. method:: SetFileAttributes(Filename, ReadOnly, Protected, System, Hidden)

      Sets file attributes.

      :param Filename: File to modify
      :type Filename: string
      :param ReadOnly: Whether file is read only. Default to False.
      :type ReadOnly: boolean
      :param Protected: Whether file is protected. Default to False.
      :type Protected: boolean
      :param System: Whether file is system. Default to False.
      :type System: boolean
      :param Hidden: Whether file is hidden. Default to False.
      :type Hidden: boolean
      :return: None
      :rtype: None


   .. method:: SetIncomingCB(Enable)

      Gets network information from phone.

      :type Enable: boolean
      :param Enable: Whether to enable notifications, default is True
      :return: None
      :rtype: None


   .. method:: SetIncomingCall(Enable)

      Activates/deactivates noticing about incoming calls.

      :type Enable: boolean
      :param Enable: Whether to enable notifications, default is True
      :return: None
      :rtype: None


   .. method:: SetIncomingCallback(Callback)

      Sets callback function which is called whenever any (enabled) incoming event appears. Please note that you have to enable each event type by calling SetIncoming* functions.

      The callback function needs to accept three parameters: StateMachine object, event type and it's data in dictionary.

      :param Callback: callback function or None for disabling
      :type Callback: function
      :return: None
      :rtype: None


   .. method:: SetIncomingSMS(Enable)

      Enable/disable notification on incoming SMS.

      :type Enable: boolean
      :param Enable: Whether to enable notifications, default is True
      :return: None
      :rtype: None


   .. method:: SetIncomingUSSD(Enable)

      Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services).

      :type Enable: boolean
      :param Enable: Whether to enable notifications, default is True
      :return: None
      :rtype: None


   .. method:: SetLocale(DateSeparator, DateFormat, AMPMTime)

      Sets locale of phone.

      :param DateSeparator: Date separator.
      :type DateSeparator: string
      :param DateFormat: Date format, one of ``DDMMYYYY``, ``MMDDYYYY``, ``YYYYMMDD``
      :type DateFormat: string
      :param AMPMTime: Whether to use AM/PM time.
      :type AMPMTime: boolean
      :return: None
      :rtype: None


   .. method:: SetMemory(Value)

      Sets memory (phonebooks or calls) entry.

      :param Value: Memory entry, see :ref:`pbk_obj`
      :type Value: dict
      :return: Location of created entry
      :rtype: int


   .. method:: SetSMS(Value)

      Sets SMS.

      :param Value: SMS data, see :ref:`sms_obj`
      :type Value: dict
      :return: Tuple for location and folder.
      :rtype: tuple


   .. method:: SetSMSC(Value)

      Sets SMS Service Center number and SMS settings.

      :param Value: SMSC information, see :ref:`smsc_obj`
      :type Value: dict
      :return: None
      :rtype: None


   .. method:: SetSpeedDial(Value)

      Sets speed dial.

      :param Value: Speed dial data, see :meth:`GetSpeedDial` for listing.
      :type Value: dict
      :return: None
      :rtype: None


   .. method:: SetToDo(Value)

      Sets ToDo in phone.

      :param Value: ToDo data, see :ref:`todo_obj`
      :type Value: dict
      :return: Location of created entry
      :rtype: int


   .. method:: SplitCall(ID)

      Splits call.

      :param ID: ID of call
      :type ID: integer
      :return: None
      :rtype: None


   .. method:: SwitchCall(ID, Next)

      Switches call.

      :param ID: ID of call
      :type ID: integer
      :return: None
      :rtype: None


   .. method:: Terminate()

      Terminates the connection with phone.

      :return: None
      :rtype: None

   .. method:: Abort()

      Aborts current operation.

      :return: None
      :rtype: None

   .. method:: TransferCall(ID, Next)

      Transfers call.

      :param ID: ID of call
      :type ID: integer
      :return: None
      :rtype: None


   .. method:: UnholdCall(ID)

      Unholds call.

      :param ID: ID of call
      :type ID: integer
      :return: None
      :rtype: None



Generic functions
-----------------

.. function:: Version()

    Get version information.

    :return: Tuple of version information - Gammu runtime version, python-gammu version, build time Gammu version.
    :rtype: tuple

Debugging configuration
-----------------------

.. function:: SetDebugFile(File)

    Sets global debug file.

    :param File: File where to write debug stuff (as configured by :meth:`SetDebugLevel`). Can be either None for no file, Python file object or filename.
    :type File: mixed
    :return: None
    :rtype: None

.. function:: SetDebugLevel(Level)

    Sets global debug level accorting to passed string. You need to configure output file using :meth:`SetDebugFile` to activate it.

    :type Level: string
    :param Level: name of debug level to use, currently one of:

       * nothing
       * text
       * textall
       * binary
       * errors
       * textdate
       * textalldate
       * errorsdate
    :return: None
    :rtype: None

Message processing
------------------

.. function:: LinkSMS(Messages, EMS)

    Links multi part SMS messages.

    :type Messages: list
    :type EMS: boolean
    :param Messages: List of messages to link, see :ref:`sms_obj`
    :param EMS: Whether to detect ems, defauls to True
    :return: List of linked messages, see :ref:`sms_obj`
    :rtype: list

.. function:: SMSCounter(Text, UDH = "NoUDH", Coding = "Default")

    Calculates number of SMS and free chars in SMS.

    :type Text: string
    :param Text: Message text
    :type UDH: string
    :param UDH: Message UDH
    :type Coding: string
    :param Coding: Message coding (eg. Unicode or Default)
    :return: Number of messages and number of free chars
    :rtype: tuple

    .. versionadded:: 1.29.90

.. function:: DecodeSMS(Messages, EMS)

    Decodes multi part SMS message.

    :param Messages: Nessages to decode, see :ref:`sms_obj`
    :type Messages: list
    :param EMS: Whether to use EMS, defalt to True
    :type EMS: boolean
    :return: Multi part message information, see :ref:`sms_info_obj`
    :rtype: dict

.. function:: EncodeSMS(MessageInfo)

    Encodes multi part SMS message.

    :param MessageInfo: Description of message, see :ref:`sms_info_obj`
    :type MessageInfo: dict
    :return: List of dictionaries with raw message, see :ref:`sms_obj`
    :rtype: dict

.. function:: DecodePDU(Data, SMSC = False)

    Parses PDU packet.

    :param Data: PDU data, need to be binary not hex encoded
    :type Data: string
    :param SMSC: Whether PDU includes SMSC.
    :type SMSC: boolean
    :return: Message data, see :ref:`sms_obj`
    :rtype: dict

.. function:: EncodePDU(SMS, Layout = Submit)

    Creates PDU packet.

    :param SMS: SMS dictionary, see :ref:`sms_obj`
    :type SMS: dict
    :param Layout: Layout (one of Submit, Deliver, StatusReport), Submit is default
    :type Layout: string
    :return: Message data
    :rtype: string

    .. versionadded:: 1.27.93

Encoding and decoding entries
-----------------------------

.. function:: DecodeVCARD(Text)

    Decodes memory entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Memory entry, see :ref:`pbk_obj`
    :rtype: dict

.. function:: EncodeVCARD(Entry)

    Encodes memory entry to a vCard.

    :param Entry: Memory entry, see :ref:`pbk_obj`
    :type Entry: dict
    :return: String with vCard
    :rtype: string

.. function:: DecodeVCS(Text)

    Decodes todo/calendar entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Calendar or todo entry (whatever one was included in string), see :ref:`cal_obj`, :ref:`todo_obj`
    :rtype: dict

.. function:: DecodeICS(Text)

    Decodes todo/calendar entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Calendar or todo entry (whatever one was included in string), see :ref:`cal_obj`, :ref:`todo_obj`
    :rtype: dict

.. function:: EncodeVCALENDAR(Entry)

    Encodes calendar entry to a vCalendar.

    :param Entry: Calendar entry, see :ref:`cal_obj`
    :type Entry: dict
    :return: String with vCalendar
    :rtype: string

.. function:: EncodeICALENDAR(Entry)

    Encodes calendar entry to a iCalendar.

    :param Entry: Calendar entry, see :ref:`cal_obj`
    :type Entry: dict
    :return: String with iCalendar
    :rtype: string

.. function:: EncodeVTODO(Entry)

    Encodes todo entry to a vTodo.

    :param Entry: Todo entry, see :ref:`todo_obj`
    :type Entry: dict
    :return: String with vTodo
    :rtype: string

.. function:: EncodeITODO(Entry)

    Encodes todo entry to a iTodo.

    :param Entry: Todo entry, see :ref:`todo_obj`
    :type Entry: dict
    :return: String with vCard
    :rtype: string

Backup reading and writing
--------------------------

.. function:: SaveRingtone(Filename, Ringtone, Format)

    Saves ringtone into file.

    :param Filename: Name of file where ringote will be saved
    :type Filename: string
    :param Ringtone: Ringtone to save
    :type Ringtone: dict
    :param Format: One of ``ott``, ``mid``, ``rng``, ``imy``, ``wav``, ``rttl``
    :type Format: string
    :return: None
    :rtype: None

.. function:: SaveBackup(Filename, Backup, Format)

    Saves backup into file.

    :param Filename: Name of file to read backup from
    :type Filename: string
    :param Backup: Backup data, see :func:`ReadBackup` for description
    :type Backup: dict
    :param Format: File format to use (``Auto``, ``AutoUnicode``, ``LMB``, ``VCalendar``, ``VCard``, ``LDIF``, ``ICS``, ``Gammu``, ``GammuUnicode``, the default is ``AutoUnicode``)
    :type Format: string
    :return: None
    :rtype: None

.. function:: ReadBackup(Filename, Format)

    Reads backup into file.

    :param Filename: Name of file where backup is stored
    :type Filename: string
    :param Format: File format to use (``Auto``, ``AutoUnicode``, ``LMB``, ``VCalendar``, ``VCard``, ``LDIF``, ``ICS``, ``Gammu``, ``GammuUnicode``, the default is ``AutoUnicode``)
    :type Format: string
    :return: Dictionary of read entries, it contains following keys, each might be empty:

         * IMEI
         * Model
         * Creator
         * PhonePhonebook
         * SIMPhonebook
         * Calendar
         * ToDo
         * DateTime
    :rtype: dict

.. function:: SaveSMSBackup(Filename, Backup)

    Saves SMS backup into file.

    :param Filename: Name of file where to save SMS backup
    :type Filename: string
    :param Backup: List of messages to store
    :type Backup: list
    :return: None
    :rtype: None

.. function:: ReadSMSBackup(Filename)

    Reads SMS backup into file.

    :param Filename: Name of file where SMS backup is stored
    :type Filename: string
    :return: List of messages read from file
    :rtype: list

Various data
------------

.. data:: GSMNetworks

   Dictionary with GSM network codes.

.. data:: GSMCountries

   Dictionary with GSM country codes.
