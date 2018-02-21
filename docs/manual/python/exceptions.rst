:mod:`gammu.exception` -- Gammu exception handling
==================================================

.. module:: gammu.exception
    :synopsis: Defines all exceptions used in Gammu.

.. exception:: gammu.GSMError

    Generic class as parent for all Gammu exceptions. This is never raised
    directly, but should be used to catch any Gammu related exception.

.. exception:: gammu.ERR_ABORTED

    Exception corresponding to gammu error ERR_ABORTED.
    Verbose error description: Operation aborted.

.. exception:: gammu.ERR_BADFEATURE

    Exception corresponding to gammu error ERR_BADFEATURE.
    Verbose error description: Bad feature string in configuration.

.. exception:: gammu.ERR_BUG

    Exception corresponding to gammu error ERR_BUG.
    Verbose error description: Nobody is perfect, some bug appeared in protocol implementation. Please contact authors.

.. exception:: gammu.ERR_BUSY

    Exception corresponding to gammu error ERR_BUSY.
    Verbose error description: Command rejected because device was busy. Wait and restart.

.. exception:: gammu.ERR_CANCELED

    Exception corresponding to gammu error ERR_CANCELED.
    Verbose error description: Transfer was canceled by phone, maybe you pressed cancel on phone.

.. exception:: gammu.ERR_CANTOPENFILE

    Exception corresponding to gammu error ERR_CANTOPENFILE.
    Verbose error description: Can not open specified file.

.. exception:: gammu.ERR_CORRUPTED

    Exception corresponding to gammu error ERR_CORRUPTED.
    Verbose error description: Corrupted data returned by phone.

.. exception:: gammu.ERR_COULDNT_CONNECT

    Exception corresponding to gammu error ERR_COULDNT_CONNECT.
    Verbose error description: Could not connect to the server.

.. exception:: gammu.ERR_COULDNT_RESOLVE

    Exception corresponding to gammu error ERR_COULDNT_RESOLVE.
    Verbose error description: Could not resolve the host name.

.. exception:: gammu.ERR_DATACONVERTED

    Exception corresponding to gammu error ERR_DATACONVERTED.
    Verbose error description: Data were converted.

.. exception:: gammu.ERR_DEVICEBUSY

    Exception corresponding to gammu error ERR_DEVICEBUSY.
    Verbose error description: Error opening device, it is already opened by other application.

.. exception:: gammu.ERR_DEVICECHANGESPEEDERROR

    Exception corresponding to gammu error ERR_DEVICECHANGESPEEDERROR.
    Verbose error description: Error setting device speed. Maybe speed not supported.

.. exception:: gammu.ERR_DEVICEDTRRTSERROR

    Exception corresponding to gammu error ERR_DEVICEDTRRTSERROR.
    Verbose error description: Error setting device DTR or RTS.

.. exception:: gammu.ERR_DEVICELOCKED

    Exception corresponding to gammu error ERR_DEVICELOCKED.
    Verbose error description: Error opening device, it is locked.

.. exception:: gammu.ERR_DEVICENODRIVER

    Exception corresponding to gammu error ERR_DEVICENODRIVER.
    Verbose error description: Error opening device. No required driver in operating system.

.. exception:: gammu.ERR_DEVICENOPERMISSION

    Exception corresponding to gammu error ERR_DEVICENOPERMISSION.
    Verbose error description: Error opening device, you don't have permissions.

.. exception:: gammu.ERR_DEVICENOTEXIST

    Exception corresponding to gammu error ERR_DEVICENOTEXIST.
    Verbose error description: Error opening device, it doesn't exist.

.. exception:: gammu.ERR_DEVICENOTWORK

    Exception corresponding to gammu error ERR_DEVICENOTWORK.
    Verbose error description: Error opening device. Some hardware not connected/wrongly configured.

.. exception:: gammu.ERR_DEVICEOPENERROR

    Exception corresponding to gammu error ERR_DEVICEOPENERROR.
    Verbose error description: Error opening device. Unknown, busy or no permissions.

.. exception:: gammu.ERR_DEVICEPARITYERROR

    Exception corresponding to gammu error ERR_DEVICEPARITYERROR.
    Verbose error description: Can't set parity on the device.

.. exception:: gammu.ERR_DEVICEREADERROR

    Exception corresponding to gammu error ERR_DEVICEREADERROR.
    Verbose error description: Error during reading from the device.

.. exception:: gammu.ERR_DEVICEWRITEERROR

    Exception corresponding to gammu error ERR_DEVICEWRITEERROR.
    Verbose error description: Error writing to the device.

.. exception:: gammu.ERR_DISABLED

    Exception corresponding to gammu error ERR_DISABLED.
    Verbose error description: Desired functionality has been disabled on compile time.

.. exception:: gammu.ERR_EMPTY

    Exception corresponding to gammu error ERR_EMPTY.
    Verbose error description: Empty entry.

.. exception:: gammu.ERR_EMPTYSMSC

    Exception corresponding to gammu error ERR_EMPTYSMSC.
    Verbose error description: No SMSC number given. Provide it manually or use the one configured in phone.

.. exception:: gammu.ERR_FILEALREADYEXIST

    Exception corresponding to gammu error ERR_FILEALREADYEXIST.
    Verbose error description: File with specified name already exists.

.. exception:: gammu.ERR_FILENOTEXIST

    Exception corresponding to gammu error ERR_FILENOTEXIST.
    Verbose error description: File with specified name doesn't exist.

.. exception:: gammu.ERR_FILENOTSUPPORTED

    Exception corresponding to gammu error ERR_FILENOTSUPPORTED.
    Verbose error description: File format not supported by Gammu.

.. exception:: gammu.ERR_FOLDERNOTEMPTY

    Exception corresponding to gammu error ERR_FOLDERNOTEMPTY.
    Verbose error description: Folder must be empty.

.. exception:: gammu.ERR_FOLDERPART

    Exception corresponding to gammu error ERR_FOLDERPART.
    Verbose error description: Only part of folder has been listed.

.. exception:: gammu.ERR_FRAMENOTREQUESTED

    Exception corresponding to gammu error ERR_FRAMENOTREQUESTED.
    Verbose error description: Frame not requested right now. See <https://wammu.eu/support/bugs/> for information how to report it.

.. exception:: gammu.ERR_FULL

    Exception corresponding to gammu error ERR_FULL.
    Verbose error description: Memory full.

.. exception:: gammu.ERR_GETTING_SMSC

    Exception corresponding to gammu error ERR_GETTING_SMSC.
    Verbose error description: Failed to get SMSC number from phone.

.. exception:: gammu.ERR_GNAPPLETWRONG

    Exception corresponding to gammu error ERR_GNAPPLETWRONG.
    Verbose error description: Wrong GNAPPLET version in phone. Use version from currently used Gammu.

.. exception:: gammu.ERR_INSIDEPHONEMENU

    Exception corresponding to gammu error ERR_INSIDEPHONEMENU.
    Verbose error description: You're inside phone menu (maybe editing?). Leave it and try again.

.. exception:: gammu.ERR_INSTALL_NOT_FOUND

    Exception corresponding to gammu error ERR_INSTALL_NOT_FOUND.
    Verbose error description: Installation data not found, please consult debug log and/or documentation for more details.

.. exception:: gammu.ERR_INVALIDDATA

    Exception corresponding to gammu error ERR_INVALIDDATA.
    Verbose error description: Invalid data given to phone.

.. exception:: gammu.ERR_INVALIDDATETIME

    Exception corresponding to gammu error ERR_INVALIDDATETIME.
    Verbose error description: Invalid date or time specified.

.. exception:: gammu.ERR_INVALIDLOCATION

    Exception corresponding to gammu error ERR_INVALIDLOCATION.
    Verbose error description: Invalid location. Maybe too high?

.. exception:: gammu.ERR_MEMORY

    Exception corresponding to gammu error ERR_MEMORY.
    Verbose error description: Phone memory error, maybe it is read only.

.. exception:: gammu.ERR_MOREMEMORY

    Exception corresponding to gammu error ERR_MOREMEMORY.
    Verbose error description: More memory required...

.. exception:: gammu.ERR_NEEDANOTHERANSWER

    Exception corresponding to gammu error ERR_NEEDANOTHERANSWER.
    Verbose error description: Phone module need to send another answer frame.

.. exception:: gammu.ERR_NETWORK_ERROR

    Exception corresponding to gammu error ERR_NETWORK_ERROR.
    Verbose error description: Network error.

.. exception:: gammu.ERR_NONE

    Exception corresponding to gammu error ERR_NONE.
    Verbose error description: No error.

.. exception:: gammu.ERR_NONE_SECTION

    Exception corresponding to gammu error ERR_NONE_SECTION.
    Verbose error description: No such section exists.

.. exception:: gammu.ERR_NOSERVICE

    Exception corresponding to gammu error ERR_NOSERVICE.
    Verbose error description: Service configuration is missing.

.. exception:: gammu.ERR_NOSIM

    Exception corresponding to gammu error ERR_NOSIM.
    Verbose error description: Can not access SIM card.

.. exception:: gammu.ERR_NOTCONNECTED

    Exception corresponding to gammu error ERR_NOTCONNECTED.
    Verbose error description: Phone is not connected.

.. exception:: gammu.ERR_NOTIMPLEMENTED

    Exception corresponding to gammu error ERR_NOTIMPLEMENTED.
    Verbose error description: Functionality not implemented. You are welcome to help authors with it.

.. exception:: gammu.ERR_NOTRUNNING

    Exception corresponding to gammu error ERR_NOTRUNNING.
    Verbose error description: Service is not running.

.. exception:: gammu.ERR_NOTSUPPORTED

    Exception corresponding to gammu error ERR_NOTSUPPORTED.
    Verbose error description: Function not supported by phone.

.. exception:: gammu.ERR_OTHERCONNECTIONREQUIRED

    Exception corresponding to gammu error ERR_OTHERCONNECTIONREQUIRED.
    Verbose error description: Current connection type doesn't support called function.

.. exception:: gammu.ERR_PERMISSION

    Exception corresponding to gammu error ERR_PERMISSION.
    Verbose error description: Operation not allowed by phone.

.. exception:: gammu.ERR_PHONEOFF

    Exception corresponding to gammu error ERR_PHONEOFF.
    Verbose error description: Phone is disabled and connected to charger.

.. exception:: gammu.ERR_PHONE_INTERNAL

    Exception corresponding to gammu error ERR_PHONE_INTERNAL.
    Verbose error description: Internal phone error.

.. exception:: gammu.ERR_READ_ONLY

    Exception corresponding to gammu error ERR_READ_ONLY.
    Verbose error description: Entry is read only.

.. exception:: gammu.ERR_SECURITYERROR

    Exception corresponding to gammu error ERR_SECURITYERROR.
    Verbose error description: Security error. Maybe no PIN?

.. exception:: gammu.ERR_SHOULDBEFILE

    Exception corresponding to gammu error ERR_SHOULDBEFILE.
    Verbose error description: You have to give file name and not folder name.

.. exception:: gammu.ERR_SHOULDBEFOLDER

    Exception corresponding to gammu error ERR_SHOULDBEFOLDER.
    Verbose error description: You have to give folder name and not file name.

.. exception:: gammu.ERR_SOURCENOTAVAILABLE

    Exception corresponding to gammu error ERR_SOURCENOTAVAILABLE.
    Verbose error description: Some functions not available for your system (disabled in config or not implemented).

.. exception:: gammu.ERR_SPECIFYCHANNEL

    Exception corresponding to gammu error ERR_SPECIFYCHANNEL.
    Verbose error description: Bluetooth configuration requires channel option.

.. exception:: gammu.ERR_TIMEOUT

    Exception corresponding to gammu error ERR_TIMEOUT.
    Verbose error description: No response in specified timeout. Probably phone not connected.

.. exception:: gammu.ERR_UNCONFIGURED

    Exception corresponding to gammu error ERR_UNCONFIGURED.
    Verbose error description: Gammu is not configured.

.. exception:: gammu.ERR_UNKNOWN

    Exception corresponding to gammu error ERR_UNKNOWN.
    Verbose error description: Unknown error.

.. exception:: gammu.ERR_UNKNOWNCONNECTIONTYPESTRING

    Exception corresponding to gammu error ERR_UNKNOWNCONNECTIONTYPESTRING.
    Verbose error description: Unknown connection type string. Check config file.

.. exception:: gammu.ERR_UNKNOWNFRAME

    Exception corresponding to gammu error ERR_UNKNOWNFRAME.
    Verbose error description: Unknown frame. See <https://wammu.eu/support/bugs/> for information how to report it.

.. exception:: gammu.ERR_UNKNOWNMODELSTRING

    Exception corresponding to gammu error ERR_UNKNOWNMODELSTRING.
    Verbose error description: Unknown model type string. Check config file.

.. exception:: gammu.ERR_UNKNOWNRESPONSE

    Exception corresponding to gammu error ERR_UNKNOWNRESPONSE.
    Verbose error description: Unknown response from phone. See <https://wammu.eu/support/bugs/> for information how to report it.

.. exception:: gammu.ERR_USING_DEFAULTS

    Exception corresponding to gammu error ERR_USING_DEFAULTS.
    Verbose error description: Using default values.

.. exception:: gammu.ERR_WORKINPROGRESS

    Exception corresponding to gammu error ERR_WORKINPROGRESS.
    Verbose error description: Function is currently being implemented. If you want to help, please contact authors.

.. exception:: gammu.ERR_WRITING_FILE

    Exception corresponding to gammu error ERR_WRITING_FILE.
    Verbose error description: Error writing file to disk.

.. exception:: gammu.ERR_WRONGCRC

    Exception corresponding to gammu error ERR_WRONGCRC.
    Verbose error description: CRC error.

.. exception:: gammu.ERR_WRONGFOLDER

    Exception corresponding to gammu error ERR_WRONGFOLDER.
    Verbose error description: Wrong folder used.
