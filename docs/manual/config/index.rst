.. _gammurc:

Gammu Configuration File
========================

Synopsis
--------

On Linux, MacOS X, BSD and other Unix-like systems, the config file is
searched in following order:

1. ``$XDG_CONFIG_HOME/gammu/config``
2. ``~/.config/gammu/config``
3. ``~/.gammurc``
4. ``/etc/gammurc``

On Microsoft Windows:

1. ``$PROFILE\Application Data\gammurc``
2. ``.\gammurc``

Description
-----------

Gammu requires configuration to be able to properly talk to your phone.
:ref:`gammu` reads configuration from a config file. It's location is determined
on runtime, see above for search paths.

You can use :ref:`gammu-config` or :ref:`gammu-detect` to generate
configuration file or start from :ref:`Fully documented example`.

For hints about configuring your phone, you can check Gammu Phone
Database <http://wammu.eu/phones/> to see what user users
experienced.

This file use ini file syntax, see :ref:`ini`.

Configuration file for gammu can contain several sections - ``[gammu]``, ``[gammu1]``,
``[gammuN]``, ... Each section configures one connection setup and in default mode
gammu tries all of them in numerical order. You can also specify which
configuration section to use by giving it's number (:config:section:`[gammu]` has number 0) as a
parameter to :ref:`gammu` and it will then use only this section.

.. config:section:: [gammu]

This section is read by default unless you specify other on command line.

Device connection parameters
++++++++++++++++++++++++++++

.. config:option:: Connection

    Protocol which will be used to talk to your phone.

    For Nokia cables you want to use one of following:

    ``fbus``
        serial FBUS connection
    ``dlr3``
        DLR-3 and compatible cables
    ``dku2``
        DKU-2 and compatible cables
    ``dku5``
        DKU-5 and compatible cables
    ``mbus``
        serial MBUS connection

    If you use some non original cable, you might need to append ``-nodtr`` (eg. for
    ARK3116 based cables) or ``-nopower``, but Gammu should be able to detect this
    automatically.

    For non-Nokia phones connected using cable you generally want:

    ``at``
        generic AT commands based connection

    You can optionally specify speed of the connection, eg. ``at19200``, but it is
    not needed for modern USB cables.

    For IrDA connections use one of following:

    ``irdaphonet``
        Phonet connection for Nokia phones.
    ``irdaat``
        AT commands connection for most of phones (this is not supported on Linux).
    ``irdaobex``
        OBEX (IrMC or file transfer) connection for most of phones.
    ``irdagnapbus``
        GNapplet based connection for Symbian phones, see :ref:`gnapplet`.

    For Bluetooth connection use one of following:

    ``bluephonet``
        Phonet connection for Nokia phones.
    ``blueat``
        AT commands connection for most of phones.
    ``blueobex``
        OBEX (IrMC or file transfer) connection for most of phones.
    ``bluerfgnapbus``
        GNapplet based connection for Symbian phones, see :ref:`gnapplet`.
    ``blues60``
        Connection to Series60 applet in S60 phones, see :ref:`s60`.

        .. versionadded:: 1.29.90

    .. seealso:: :ref:`faq-config`

.. config:option:: Device

    .. versionadded:: 1.27.95

    Device node or address of phone. It depends on used connection.

    For **cables** or emulated serial ports, you enter device name (for example
    ``/dev/ttyS0``, ``/dev/ttyACM0``, ``/dev/ircomm0``, ``/dev/rfcomm0`` on Linux,
    ``/dev/cuad0`` on FreeBSD or ``COM1:`` on Windows). The special exception are
    DKU-2 and DKU-5 cables on Windows, where the device is automatically detected
    from driver information and this parameters is ignored.

    For **USB** connections (currently only fbususb and dku2 on Linux), you can
    specify to which USB device Gammu should connect. You can either provide
    vendor/product IDs or device address on USB::

        Device = 0x1234:0x5678  # Match device by vendor and product id
        Device = 0x1234:-1      # Match device by vendor id
        Device = 1.10           # Match device by usb bus and device address
        Device = 10             # Match device by usb device address
        Device = serial:123456  # Match device by serial string

    .. note::

        On Linux systems, you might lack permissions for some device nodes.
        You might need to be member of some group (eg. :samp:`plugdev` or
        :samp:`dialout`) or or add special udev rules to enable you access
        these devices as non-root.

        For Nokia phones you can put follofing file (also available in sources
        as :file:`contrib/udev/45-nokiadku2.rules`) as
        :file:`/etc/udev/rules.d/45-nokiadku2.rules`:

        .. literalinclude:: ../../../contrib/udev/45-nokiadku2.rules
           :language: sh

    In case your USB device appears as the serial port in the system (eg.
    ``/dev/ttyACM0`` on Linux or ``COM5:`` on Windows), just use same setup as
    with serial port.

    For **Bluetooth** connection you have to enter Bluetooth address of your phone
    (you can list Bluetooth devices in range on Linux using :command:`hcitool scan`
    command). Optionally you can also force Gammu to use specified channel by
    including channel number after slash.

    Before using Gammu, your device should be paired with computer or you should
    have set up automatic pairing.

    For **IrDA** connections, this parameters is not used at all.

    If IrDA does not work on Linux, you might need to bring up the interface and
    enable discovery (you need to run these commands as root):

    .. code-block:: sh

        ip l s dev irda0 up          # Enables irda0 device
        sysctl net.irda.discovery=1  # Enables device discovery on IrDA

    .. note::

        Native IrDA is not supported on Linux, you need to setup virtual
        serial port for it (eg. ``/dev/ircomm0``) and use it same way as cable.
        This can be usually achieved by loading modules ``ircomm-tty`` and ``irtty-sir``:

        .. code-block:: sh

            modprobe ircomm-tty
            modprobe irtty-sir

    .. seealso:: :ref:`faq-config`

.. config:option:: Port

    .. deprecated:: 1.27.95

    Alias for :config:option:`Device`, kept for backward compatibility.

.. config:option:: Model

    Do not use this parameter unless really needed! The only use case for this is
    when Gammu does not know your phone and misdetects it's features.

    The only special case for using model is to force special type of OBEX
    connection instead of letting Gammu try the best suited for selected
    operation:

    ``obexfs``
        force using of file browsing service (file system support)
    ``obexirmc``
        force using of IrMC service (contacts, calendar and notes support)
    ``obexnone``
        none service chosen, this has only limited use for sending file (:option:`gammu sendfile` command)
    ``mobex``
        m-obex service for Samsung phones

.. config:option:: Use_Locking

    On Posix systems, you might want to lock serial device when it is being used
    using UUCP-style lock files. Enabling this option (setting to yes) will make
    Gammu honor these locks and create it on startup. On most distributions you
    need additional privileges to use locking (eg. you need to be member of uucp
    group).

    This option has no meaning on Windows.

Connection options
++++++++++++++++++

.. config:option:: SynchronizeTime

    If you want to set time from computer to phone during starting connection.

.. config:option:: StartInfo

    This option allow to set, that you want (setting ``yes``) to see message on the
    phone screen or phone should enable light for a moment during starting
    connection. Phone will not beep during starting connection with this
    option. This works only with some Nokia phones.


Debugging options
+++++++++++++++++

.. config:option:: LogFile

    Path to file where information about communication will be stored.

    .. note::

        For most debug levels (excluding ``errors``) the log file is overwritten on
        each execution.

.. config:option:: LogFormat

    Determines what all will be logged to :config:option:`LogFile`. Possible values are:

    ``nothing``
        no debug level
    ``text``
        transmission dump in text format
    ``textall``
        all possible info in text format
    ``textalldate``
        all possible info in text format, with time stamp
    ``errors``
        errors in text format
    ``errorsdate``
        errors in text format, with time stamp
    ``binary``
        transmission dump in binary format

    For debugging use either ``textalldate`` or ``textall``, it contains all
    needed information to diagnose problems.

.. config:option:: Features

    Custom features for phone. This can be used as override when values coded
    in ``common/gsmphones.c`` are bad or missing. Consult
    ``include/gammu-info.h`` for possible values (all :c:type:`GSM_Feature` values
    without leading ``F_`` prefix). Please report correct values to Gammu
    authors.

Locales and character set options
+++++++++++++++++++++++++++++++++

.. config:option:: GammuCoding

    Forces using specified codepage (for example ``1250`` will force CP-1250 or
    ``utf8`` for UTF-8). This should not be needed, Gammu detects it according
    to your locales.

.. config:option:: GammuLoc

    Path to directory with localisation files (the directory should contain
    ``LANG/LC_MESSAGES/gammu.mo``). If gammu is properly installed it should find
    these files automatically.

Other options
+++++++++++++

.. config:option:: DataPath

    Additional path where to search for data files. The default path is
    configured on build time (and defaults to ``/usr/share/data/gammu`` on Unix
    systems). Currently it is used only for searching files to upload to phone
    using :option:`gammu install`.

Examples
--------

There is more complete example available in Gammu documentation, see :ref:`gammu`.

Connection examples
+++++++++++++++++++

Gammu configuration for Nokia phone using DLR-3 cable:

.. code-block:: ini

    [gammu]
    device = /dev/ttyACM0
    connection = dlr3

Gammu configuration for Sony-Ericsson phone (or any other AT compatible
phone) connected using USB cable:

.. code-block:: ini

    [gammu]
    device = /dev/ttyACM0
    connection = at

Gammu configuration for Sony-Ericsson (or any other AT compatible
phone) connected using bluetooth:

.. code-block:: ini

    [gammu]
    device = B0:0B:00:00:FA:CE
    connection = blueat

Gammu configuration for phone which needs to manually adjust Bluetooth channel to use channel 42:

.. code-block:: ini

    [gammu]
    device = B0:0B:00:00:FA:CE/42
    connection = blueat

Working with multiple phones
++++++++++++++++++++++++++++

Gammu can be configured for multiple phones (however only one connection
is used at one time, you can choose which one to use with :option:`gammu -s`
parameter). Configuration for phones on three serial ports would look
like following:

.. code-block:: ini

    [gammu]
    device = /dev/ttyS0
    connection = at

    [gammmu1]
    device = /dev/ttyS1
    connection = at

    [gammmu2]
    device = /dev/ttyS2
    connection = at

.. _Fully documented example:

Fully documented example
++++++++++++++++++++++++

You can find this sample file as :file:`docs/config/gammurc` in Gammu sources.

.. literalinclude:: ../../../docs/config/gammurc
   :language: ini

