:mod:`gammu` -- Mobile phone access
===================================

.. module:: gammu
    :synopsis: Provides access to mobile phones.

This module wraps all python-gammu functionality. 

:class:`gammu.StateMachine`
---------------------------

.. class:: StateMachine
   :module: gammu

   StateMachine(Locale)
   
   StateMachine object, that is used for communication with phone.
   
   param Locale: What locales to use for gammu error messages, default is 'auto' which does autodetection according to user locales
   type Locale: string
   
   
   .. attribute:: StateMachine.AddCalendar
      :module: gammu
   
      AddCalendar(Value)
      
      Adds calendar entry.
      
      @param Value: Calendar entry data
      @type Value: hash
      @return: Location of newly created entry
      @rtype: int
      
   
   .. attribute:: StateMachine.AddCategory
      :module: gammu
   
      AddCategory(Type, Name)
      
      Adds category to phone.
      
      @param Type: Type of category to read, one of 'ToDo', 'Phonebook'
      @type Type: string
      @param Name: Category name
      @type Name: string
      @return: Location of created category
      @rtype: int
      
   
   .. attribute:: StateMachine.AddFilePart
      :module: gammu
   
      AddFilePart(File)
      
      Adds file part to filesystem.
      
      @param File: File data
      @type File: hash
      @return: File data for subsequent calls (Finished indicates transfer has been completed)
      @rtype: hash
      
   
   .. attribute:: StateMachine.AddFolder
      :module: gammu
   
      AddFolder(ParentFolderID, Name)
      
      Adds folder to filesystem.
      
      @param ParentFolderID: Folder where to create subfolder
      @type ParentFolderID: string
      @param Name: New folder name
      @type Name: string
      @return: New folder ID.
      @rtype: string
      
   
   .. attribute:: StateMachine.AddMemory
      :module: gammu
   
      AddMemory(Value)
      
      Adds memory (phonebooks or calls) entry.
      
      @param Value: Memory entry
      @type Value: hash
      @return: Location of created entry
      @rtype: int
      
   
   .. attribute:: StateMachine.AddSMS
      :module: gammu
   
      AddSMS(Value)
      
      Adds SMS to specified folder.
      @param Value: SMS data
      @type Value: hash
      @return: Tuple for location and folder.
      @rtype: tuple
      
   
   .. attribute:: StateMachine.AddSMSFolder
      :module: gammu
   
      AddSMSFolder(Name)
      
      Creates SMS folder.
      
      @param Name: Name of new folder
      @type Name: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.AddToDo
      :module: gammu
   
      AddToDo(Value)
      
      Adds ToDo in phone.
      
      @param Value: ToDo data
      @type Value: hash
      @return: Location of created entry
      @rtype: int
      
   
   .. attribute:: StateMachine.AnswerCall
      :module: gammu
   
      AnswerCall(ID, All)
      
      Accept current incoming call.
      @param ID: ID of call
      @type ID: integer
      @param All: Answer all calls?
      @type All: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.CancelCall
      :module: gammu
   
      CancelCall(ID, All)
      
      Deny current incoming call.
      @param ID: ID of call
      @type ID: integer
      @param All: Cancel all calls?
      @type All: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.ConferenceCall
      :module: gammu
   
      ConferenceCall(ID)
      
      Initiates conference call.
      @param ID: ID of call
      @type ID: integer
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteAllCalendar
      :module: gammu
   
      DeleteAllCalendar()
      
      Deletes all calendar entries.
      
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteAllMemory
      :module: gammu
   
      DeleteAllMemory(Type)
      
      Deletes all memory (phonebooks or calls) entries of specified type.
      
      @param Type: Memory type, one of 'ME', 'SM', 'ON', 'DC', 'RC', 'MC', 'MT', 'FD', 'VM'
      @type Type: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteAllToDo
      :module: gammu
   
      DeleteAllToDo()
      
      Deletes all todo entries in phone.
      
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteCalendar
      :module: gammu
   
      DeleteCalendar(Location)
      
      Deletes calendar entry.
      
      @param Location: Calendar entry to delete
      @type Location: int
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteFile
      :module: gammu
   
      DeleteFile(FileID)
      
      Deletes file from filesystem.
      
      @param FileID: File to delete
      @type FileID: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteFolder
      :module: gammu
   
      DeleteFolder(FolderID)
      
      Deletes folder on filesystem.
      
      @param FolderID: Folder to delete
      @type FolderID: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteMemory
      :module: gammu
   
      DeleteMemory(Type, Location)
      
      Deletes memory (phonebooks or calls) entry.
      
      @param Type: Memory type, one of 'ME', 'SM', 'ON', 'DC', 'RC', 'MC', 'MT', 'FD', 'VM'
      @type Type: string
      @param Location: Location of entry to delete
      @type Location: int
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteSMS
      :module: gammu
   
      DeleteSMS(Folder, Location)
      
      Deletes SMS.
      
      @param Folder: Folder where to read entry (0 is emulated flat memory)
      @type Folder: int
      @param Location: Location of entry to delete
      @type Location: int
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteSMSFolder
      :module: gammu
   
      DeleteSMSFolder(ID)
      
      Deletes SMS folder.
      
      @param ID: Index of folder to delete
      @type ID: int
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DeleteToDo
      :module: gammu
   
      DeleteToDo(Location)
      
      Deletes ToDo entry in phone.
      @param Location: Location of entry to delete
      @type Location: int
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DialService
      :module: gammu
   
      DialService(Number)
      
      Dials number and starts voice call.
      @param Number: Number to dial
      @type Number: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.DialVoice
      :module: gammu
   
      DialVoice(Number, ShowNumber)
      
      Dials number and starts voice call.
      @param Number: Number to dial
      @type Number: string
      @param ShowNumber: Identifies whether to enable CLIR (None = keep default phone settings). Default is None
      @type ShowNumber: boolean or None
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.EnterSecurityCode
      :module: gammu
   
      EnterSecurityCode(Type, Code)
      
      Entres security code.
      @param Type: What code to enter, one of 'PIN', 'PUK', 'PIN2', 'PUK2', 'Phone'.
      @type Type: string
      @param Code: Code value
      @type Code: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.GetAlarm
      :module: gammu
   
      GetAlarm(Location)
      
      Reads alarm set in phone.
      
      @param Location: Which alarm to read. Many phone support only one alarm. Default is 1.
      @type Location: int
      @return: Alarm hash
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetBatteryCharge
      :module: gammu
   
      GetBatteryCharge()
      
      Gets information about battery charge and phone charging state.
      
      @return: Hash containing information about battery state (BatteryPercent and ChargeState)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetCalendar
      :module: gammu
   
      GetCalendar(Location)
      
      Retrieves calendar entry.
      
      @param Location: Calendar entry to read
      @type Location: int
      @return: Hash with calendar values
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetCalendarStatus
      :module: gammu
   
      GetCalendarStatus()
      
      Retrieves calendar status (number of used entries).
      
      @return: Hash with calendar status (Used)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetCategory
      :module: gammu
   
      GetCategory(Type, Location)
      
      Reads category from phone.
      
      @param Type: Type of category to read, one of 'ToDo', 'Phonebook'
      @type Type: string
      @param Location: Location of category to read
      @type Location: int
      @return: Category name as string
      @rtype: string
      
   
   .. attribute:: StateMachine.GetCategoryStatus
      :module: gammu
   
      GetCategoryStatus(Type)
      
      Reads category status (number of used entries) from phone.
      @param Type: Type of category to read, one of 'ToDo', 'Phonebook'
      @type Type: string
      @return: Hash containing information about category status (Used)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetConfig
      :module: gammu
   
      GetConfig(Section)
      
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
      
      @param Section: Index of config section to read. Defaults to 0.
      @type Section: int
      @return: Hash containing configuration
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetDateTime
      :module: gammu
   
      GetDateTime()
      
      Reads date and time from phone.
      
      @return: Date and time from phone as datetime.datetime object.
      @rtype: datetime.datetime
      
   
   .. attribute:: StateMachine.GetDisplayStatus
      :module: gammu
   
      GetDisplayStatus()
      
      Acquired display status.
      @return: List of indicators displayed on display
      @rtype: list
      
   
   .. attribute:: StateMachine.GetFilePart
      :module: gammu
   
      GetFilePart(File)
      
      Gets file part from filesystem.
      
      @param File: File data
      @type File: hash
      @return: File data for subsequent calls (Finished indicates transfer has been completed)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetFileSystemStatus
      :module: gammu
   
      GetFileSystemStatus()
      
      Acquires filesystem status.
      
      @return: Hash containing filesystem status (Used and Free)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetFirmware
      :module: gammu
   
      GetFirmware()
      
      Reads firmware information from phone.
      
      @return: Tuple from version, date and numeric version.
      @rtype: tuple
      
   
   .. attribute:: StateMachine.GetFolderListing
      :module: gammu
   
      GetFolderListing(Folder, Start)
      
      Gets next filename from filesystem folder.
      
      @param Folder: Folder to list
      @type Folder: string
      @param Start: Whether we're starting listing. Defaults to False.
      @type Start: boolean
      @return: File data as hash
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetHardware
      :module: gammu
   
      GetHardware()
      
      Gets hardware information about device.
      
      @return: Hardware information as string.
      @rtype: string
      
   
   .. attribute:: StateMachine.GetIMEI
      :module: gammu
   
      GetIMEI()
      
      Reads IMEI/serial number from phone.
      
      @return: IMEI of phone as string.
      @rtype: string
      
   
   .. attribute:: StateMachine.GetLocale
      :module: gammu
   
      GetLocale()
      
      Gets locale information from phone.
      
      @return: Hash of locale settings. L{SetLocale} lists them all.
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetManufactureMonth
      :module: gammu
   
      GetManufactureMonth()
      
      Gets month when device was manufactured.
      
      @return: Month of manufacture as string.
      @rtype: string
      
   
   .. attribute:: StateMachine.GetManufacturer
      :module: gammu
   
      GetManufacturer()
      
      Reads manufacturer from phone.
      
      @return: String with manufacturer name
      @rtype: string
      
   
   .. attribute:: StateMachine.GetMemory
      :module: gammu
   
      GetMemory(Type, Location)
      
      Reads entry from memory (phonebooks or calls). Which entry shouldbe read is defined in entry.
      @param Type: Memory type, one of 'ME', 'SM', 'ON', 'DC', 'RC', 'MC', 'MT', 'FD', 'VM'
      @type Type: string
      @return: Memory entry as hash
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetMemoryStatus
      :module: gammu
   
      GetMemoryStatus(Type)
      
      Gets memory (phonebooks or calls) status (eg. number of used andfree entries).
      
      @param Type: Memory type, one of 'ME', 'SM', 'ON', 'DC', 'RC', 'MC', 'MT', 'FD', 'VM'
      @type Type: string
      @return: Hash with information about memory (Used and Free)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetModel
      :module: gammu
   
      GetModel()
      
      Reads model from phone.
      
      @return: Tuple containing gammu identification and real model returned by phone.
      @rtype: tuple
      
   
   .. attribute:: StateMachine.GetNetworkInfo
      :module: gammu
   
      GetNetworkInfo()
      
      Gets network information.
      
      @return: Hash with information about network (NetworkName, State, NetworkCode, CID and LAC)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetNextCalendar
      :module: gammu
   
      GetNextCalendar(Start, Location)
      
      Retrieves calendar entry. This is useful for continuous reading of all calendar entries.
      
      @param Start: Whether to start. This can not be used together with Location
      @type Start: boolean
      @param Location: Last read location. This can not be used together with Start
      @type Location: int
      @return: Hash with calendar values
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetNextFileFolder
      :module: gammu
   
      GetNextFileFolder(Start)
      
      Gets next filename from filesystem.
      
      @param Start: Whether we're starting listing. Defaults to False.
      @type Start: boolean
      @return: File data as hash
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetNextMemory
      :module: gammu
   
      GetNextMemory(Type, Start, Location)
      
      Reads entry from memory (phonebooks or calls). Which entry shouldbe read is defined in entry. This can be easily used for reading all entries.
      
      @param Type: Memory type, one of 'ME', 'SM', 'ON', 'DC', 'RC', 'MC', 'MT', 'FD', 'VM'
      @type Type: string
      @param Start: Whether to start. This can not be used together with Location
      @type Start: boolean
      @param Location: Last read location. This can not be used together with Start
      @type Location: int
      @return: Memory entry as hash
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetNextRootFolder
      :module: gammu
   
      GetNextRootFolder(Folder)
      
      Gets next root folder from filesystem. Start with empty folder name.
      
      @param Folder: Previous read fodlder. Start with empty folder name.
      @type Folder: string
      @return: Structure with folder information
      
   
   .. attribute:: StateMachine.GetNextSMS
      :module: gammu
   
      GetNextSMS(Folder, Start, Location)
      
      Reads next (or first if start set) SMS message. This might befaster for some phones than using L{GetSMS} for each message.
      
      @param Folder: Folder where to read entry (0 is emulated flat memory)
      @type Folder: int
      @param Start: Whether to start. This can not be used together with Location
      @type Start: boolean
      @param Location: Location last read entry. This can not be used together with Start
      @type Location: int
      @return: Hash with SMS data
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetNextToDo
      :module: gammu
   
      GetNextToDo(Start, Location)
      
      Reads ToDo from phone.
      
      @param Start: Whether to start. This can not be used together with Location
      @type Start: boolean
      @param Location: Last read location. This can not be used together with Start
      @type Location: int
      @return: Hash with ToDo values
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetOriginalIMEI
      :module: gammu
   
      GetOriginalIMEI()
      
      Gets original IMEI from phone.
      
      @return: Original IMEI of phone as string.
      @rtype: string
      
   
   .. attribute:: StateMachine.GetPPM
      :module: gammu
   
      GetPPM()
      
      Gets PPM (Post Programmable Memory) from phone.
      
      @return: PPM as string
      @rtype: string
      
   
   .. attribute:: StateMachine.GetProductCode
      :module: gammu
   
      GetProductCode()
      
      Gets product code of device.
      @return: Product code as string.
      @rtype: string
      
   
   .. attribute:: StateMachine.GetSIMIMSI
      :module: gammu
   
      GetSIMIMSI()
      
      Gets SIM IMSI from phone.
      
      @return: SIM IMSI as string
      @rtype: string
      
   
   .. attribute:: StateMachine.GetSMS
      :module: gammu
   
      GetSMS(Folder, Location)
      
      Reads SMS message.
      
      @param Folder: Folder where to read entry (0 is emulated flat memory)
      @type Folder: int
      @param Location: Location of entry to read
      @type Location: int
      @return: Hash with SMS data
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetSMSC
      :module: gammu
   
      GetSMSC(Location)
      
      Gets SMS Service Center number and SMS settings.
      
      @param Location: Location of entry to read. Defaults to 1
      @type Location: int
      @return: Hash with SMSC information
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetSMSFolders
      :module: gammu
   
      GetSMSFolders()
      
      Returns SMS folders information.
      
      @return: List of SMS folders.
      @rtype: list
      
   
   .. attribute:: StateMachine.GetSMSStatus
      :module: gammu
   
      GetSMSStatus()
      
      Gets information about SMS memory (read/unread/size of memory for both SIM and phone).
      
      @return: Hash with information about phone memory (SIMUnRead, SIMUsed, SIMSize, PhoneUnRead, PhoneUsed, PhoneSize and TemplatesUsed)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetSecurityStatus
      :module: gammu
   
      GetSecurityStatus()
      
      Queries whether some security code needs to be entered.
      
      @return: String indicating which code needs to be entered or None if none is needed
      @rtype: string
      
   
   .. attribute:: StateMachine.GetSignalQuality
      :module: gammu
   
      GetSignalQuality()
      
      Reads signal quality (strength and error rate).
      
      @return: Hash containing information about signal state (SignalStrength, SignalPercent and BitErrorRate)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetSpeedDial
      :module: gammu
   
      GetSpeedDial(Location)
      
      Gets speed dial.
      
      @param Location: Location of entry to read
      @type Location: int
      @return: Hash with speed dial (Location, MemoryLocation, MemoryNumberID, MemoryType)
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetToDo
      :module: gammu
   
      GetToDo(Location)
      
      Reads ToDo from phone.
      @param Location: Location of entry to read
      @type Location: int
      @return: Hash with ToDo values
      @rtype: hash
      
   
   .. attribute:: StateMachine.GetToDoStatus
      :module: gammu
   
      GetToDoStatus()
      
      Gets status of ToDos (count of used entries).
      
      @return: Hash of status (Used)
      @rtype: hash
      
   
   .. attribute:: StateMachine.HoldCall
      :module: gammu
   
      HoldCall(ID)
      
      Holds call.
      @param ID: ID of call
      @type ID: integer
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.Init
      :module: gammu
   
      Init(Replies)
      
      Initialises the connection with phone.
      
      @param Replies: Number of replies to wait for on each request. Defaults to 3.
      @type Replies: int
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.PressKey
      :module: gammu
   
      PressKey(Key, Press)
      
      Emulates key press.
      
      @param Key: What key to press
      @type Key: string
      @param Press: Whether to emulate press or release.
      @type Press: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.ReadConfig
      :module: gammu
   
      ReadConfig(Section, Configuration, Filename)
      
      Reads specified section of gammurc
      
      @param Section: Index of config section to read. Defaults to 0.
      @type Section: int
      @param Configuration: Index where config section will be stored. Defaults to Section.
      @type Configuration: int
      @param Filename: Path to configuration file (otherwise it is autodetected).
      @type Filename: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.ReadDevice
      :module: gammu
   
      ReadDevice(Wait)
      
      Reads data from device.
      
      @param Wait: Whether to wait, default is not to wait.
      @type Wait: boolean
      @return: Number of bytes read
      @rtype: int
      
   
   .. attribute:: StateMachine.Reset
      :module: gammu
   
      Reset(Hard)
      
      Performs phone reset.
      
      @param Hard: Whether to make hard reset
      @type Hard: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.ResetPhoneSettings
      :module: gammu
   
      ResetPhoneSettings(Type)
      
      Resets phone settings.
      
      @param Type: What to reset, one of 'PHONE', 'UIF', 'ALL', 'DEV', 'FACTORY'
      @type Type: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SendDTMF
      :module: gammu
   
      SendDTMF(Number)
      
      Sends DTMF (Dual Tone Multi Frequency) tone.
      @param Number: Number to dial
      @type Number: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SendFilePart
      :module: gammu
   
      SendFilePart(File)
      
      Sends file part to phone.
      
      @param File: File data
      @type File: hash
      @return: File data for subsequent calls (Finished indicates transfer has been completed)
      @rtype: hash
      
   
   .. attribute:: StateMachine.SendSMS
      :module: gammu
   
      SendSMS(Value)
      
      Sends SMS.
      
      @param Value: SMS data
      @type Value: hash
      @return: Message reference as integer
      @rtype: int
      
   
   .. attribute:: StateMachine.SendSavedSMS
      :module: gammu
   
      SendSavedSMS(Folder, Location)
      
      Sends SMS saved in phone.
      
      @param Folder: Folder where to read entry (0 is emulated flat memory)
      @type Folder: int
      @param Location: Location of entry to send
      @type Location: int
      @return: Message reference as integer
      @rtype: int
      
   
   .. attribute:: StateMachine.SetAlarm
      :module: gammu
   
      SetAlarm(DateTime, Location, Repeating, Text)
      
      Sets alarm in phone.
      @param DateTime: When should alarm happen.
      @type DateTime: datetime.datetime
      @param Location: Location of alarm to set. Defaults to 1.
      @type Location: int
      @param Repeating: Whether alarm should be repeating. Defaults to True.
      @type Repeating: boolean
      @param Text: Text to be displayed on alarm. Defaults to empty.
      @type Text: string
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetAutoNetworkLogin
      :module: gammu
   
      SetAutoNetworkLogin()
      
      Enables network auto login.
      
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetCalendar
      :module: gammu
   
      SetCalendar(Value)
      
      Sets calendar entry
      
      @param Value: Calendar entry data
      @type Value: hash
      @return: Location of set entry
      @rtype: int
      
   
   .. attribute:: StateMachine.SetConfig
      :module: gammu
   
      SetConfig(Section, Values)
      
      Sets specified config section.
      
      @param Section: Index of config section to modify
      @type Section: int
      @param Values: Config values, see L{GetConfig} for description of accepted
      @type Values: hash
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetDateTime
      :module: gammu
   
      SetDateTime(Date)
      
      Sets date and time in phone.
      
      @param Date: Date to set
      @type Date: datetime.datetime
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetDebugFile
      :module: gammu
   
      SetDebugFile(File, Global)
      
      Sets state machine debug file.
      
      @param File: File where to write debug stuff (as configured by L{SetDebugLevel}). Can be either None for no file, Python file object or filename.
      @type File: mixed
      @param Global: Whether to use global debug structure (overrides File)
      @type Global: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetDebugLevel
      :module: gammu
   
      SetDebugLevel(Level)
      
      Sets state machine debug level accorting to passed string. You need to configure output file using L{SetDebugFile} to activate it.
      
      @type Level: string
      @param Level: name of debug level to use, currently one of:
         - nothing
         - text
         - textall
         - binary
         - errors
         - textdate
         - textalldate
         - errorsdate
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetFileAttributes
      :module: gammu
   
      SetFileAttributes(Filename, ReadOnly, Protected, System, Hidden)
      
      Sets file attributes.
      
      @param Filename: File to modify
      @type Filename: string
      @param ReadOnly: Whether file is read only. Default to False.
      @type ReadOnly: boolean
      @param Protected: Whether file is protected. Default to False.
      @type Protected: boolean
      @param System: Whether file is system. Default to False.
      @type System: boolean
      @param Hidden: Whether file is hidden. Default to False.
      @type Hidden: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetIncomingCB
      :module: gammu
   
      SetIncomingCB(Enable)
      
      Gets network information from phone.
      @type Enable: boolean
      @param Enable: Whether to enable notifications, default is True
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetIncomingCall
      :module: gammu
   
      SetIncomingCall(Enable)
      
      Activates/deactivates noticing about incoming calls.
      @type Enable: boolean
      @param Enable: Whether to enable notifications, default is True
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetIncomingCallback
      :module: gammu
   
      SetIncomingCallback(Callback)
      
      Sets callback function which is called whenever any (enabled) incoming event appears. Please note that you have to enable each event type by calling SetIncoming* functions.
      
      @param Callback: callback function or None for disabling
      @type Callback: function, it will get three params: StateMachine object, event type and it's data in dictionary
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetIncomingSMS
      :module: gammu
   
      SetIncomingSMS(Enable)
      
      Enable/disable notification on incoming SMS.
      
      @type Enable: boolean
      @param Enable: Whether to enable notifications, default is True
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetIncomingUSSD
      :module: gammu
   
      SetIncomingUSSD(Enable)
      
      Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services).
      @type Enable: boolean
      @param Enable: Whether to enable notifications, default is True
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetLocale
      :module: gammu
   
      SetLocale(DateSeparator, DateFormat, AMPMTime)
      
      Sets locale of phone.
      
      @param DateSeparator: Date separator.
      @type DateSeparator: string
      @param DateFormat: Date format, one of 'DDMMYYYY', 'MMDDYYYY', 'YYYYMMDD'
      @type DateFormat: string
      @param AMPMTime: Whether to use AM/PM time.
      @type AMPMTime: boolean
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetMemory
      :module: gammu
   
      SetMemory(Value)
      
      Sets memory (phonebooks or calls) entry.
      
      @param Value: Memory entry
      @type Value: hash
      @return: Location of created entry
      @rtype: int
      
   
   .. attribute:: StateMachine.SetSMS
      :module: gammu
   
      SetSMS(Value)
      
      Sets SMS.
      
      @param Value: SMS data
      @type Value: hash
      @return: Tuple for location and folder.
      @rtype: tuple
      
   
   .. attribute:: StateMachine.SetSMSC
      :module: gammu
   
      SetSMSC(Value)
      
      Sets SMS Service Center number and SMS settings.
      
      @param Value: SMSC information
      @type Value: hash
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetSpeedDial
      :module: gammu
   
      SetSpeedDial(Value)
      
      Sets speed dial.
      
      @param Value: Speed dial data, see L{GetSpeedDial} for listing.
      @type Value: hash
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SetToDo
      :module: gammu
   
      SetToDo(Value)
      
      Sets ToDo in phone.
      
      @param Value: ToDo data
      @type Value: hash
      @return: Location of created entry
      @rtype: int
      
   
   .. attribute:: StateMachine.SplitCall
      :module: gammu
   
      SplitCall(ID)
      
      Splits call.
      @param ID: ID of call
      @type ID: integer
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.SwitchCall
      :module: gammu
   
      SwitchCall(ID, Next)
      
      Switches call.
      @param ID: ID of call
      @type ID: integer
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.Terminate
      :module: gammu
   
      Terminate()
      
      Terminates the connection with phone.
      
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.TransferCall
      :module: gammu
   
      TransferCall(ID, Next)
      
      Transfers call.
      @param ID: ID of call
      @type ID: integer
      @return: None
      @rtype: None
      
   
   .. attribute:: StateMachine.UnholdCall
      :module: gammu
   
      UnholdCall(ID)
      
      Unholds call.
      @param ID: ID of call
      @type ID: integer
      @return: None
      @rtype: None
      


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

    :param File: File where to write debug stuff (as configured by L{SetDebugLevel}). Can be either None for no file, Python file object or filename.
    :type File: mixed
    :return: None
    :rtype: None

.. function:: SetDebugLevel(Level)

    Sets global debug level accorting to passed string. You need to configure output file using L{SetDebugFile} to activate it.

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
    :param Messages: List of messages to link
    :param EMS: Whether to detect ems, defauls to True
    :return: List of linked messages
    :rtype: list

.. function:: DecodeSMS(Messages, EMS)

    Decodes multi part SMS message.

    :param Messages: Nessages to decode
    :type Messages: list
    :param EMS: Whether to use EMS, defalt to True
    :type EMS: boolean
    :return: Multi part message information
    :rtype: hash

.. function:: EncodeSMS(MessageInfo)

    Encodes multi part SMS message.

    :param MessageInfo: Description of message
    :type MessageInfo: dict
    :return: Dictionary with raw message
    :rtype: dict

.. function:: DecodePDU(Data, SMSC = False)

    Parses PDU packet.

    :param Data: PDU data, need to be binary not hex encoded
    :type Data: string
    :param SMSC: Whether PDU includes SMSC.
    :type SMSC: boolean
    :return: Message data
    :rtype: dict

.. function:: EncodePDU(SMS, Layout = Submit)

    Creates PDU packet.

    :param SMS: SMS dictionary
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
    :return: Memory entry
    :rtype: hash

.. function:: EncodeVCARD(Entry)

    Encodes memory entry to a vCard.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vCard
    :rtype: string

.. function:: DecodeVCS(Text)

    Decodes todo/calendar entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Calendar or todo entry (whatever one was included in string)
    :rtype: hash

.. function:: DecodeICS(Text)

    Decodes todo/calendar entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Calendar or todo entry (whatever one was included in string)
    :rtype: hash

.. function:: EncodeVCALENDAR(Entry)

    Encodes memory entry to a vCalendar.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vCalendar
    :rtype: string

.. function:: EncodeICALENDAR(Entry)

    Encodes memory entry to a iCalendar.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with iCalendar
    :rtype: string

.. function:: EncodeVTODO(Entry)

    Encodes memory entry to a vTodo.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vTodo
    :rtype: string

.. function:: EncodeITODO(Entry)

    Encodes memory entry to a iTodo.

    :param Entry: Memory entry
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
    :param Format: One of 'ott', 'mid', 'rng', 'imy', 'wav', 'rttl'
    :type Format: string
    :return: None
    :rtype: None

.. function:: SaveBackup(Filename, Backup, Format)

    Saves backup into file.

    :param Filename: Name of file to read backup from
    :type Filename: string
    :param Backup: Backup data, see L{ReadBackup} for description
    :type Backup: dict
    :param Format: File format to use (default is AutoUnicode)
    :type Format: string (Auto, AutoUnicode, LMB, VCalendar, VCard, LDIF, ICS, Gammu, GammuUnicode)
    :return: None
    :rtype: None

.. function:: ReadBackup(Filename, Format)

    Reads backup into file.

    :param Filename: Name of file where backup is stored
    :type Filename: string
    :param Format: File format to use (default is AutoUnicode)
    :type Format: string (Auto, AutoUnicode, LMB, VCalendar, VCard, LDIF, ICS, Gammu, GammuUnicode)
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
