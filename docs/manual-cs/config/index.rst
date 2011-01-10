.. _gammurc:

Gammu Configuration File
========================

Syntaxe
-------

Na Linuxu, MacOS X, BSD a dalších systéme Unixového typu je soubor
vyhledáván v následujícím pořadí:

1. ``$XDG_CONFIG_HOME/gammu/config``
2. ``~/.config/gammu/config``
3. ``~/.gammurc``
4. ``/etc/gammurc``

Na Microsoft Windows:

1. ``$PROFILE\Application Data\gammurc``
2. ``.\gammurc``

Description
-----------

Gammu requires configuration to be able to properly talk to your phone.
:ref:`gammu` reads configuration from a config file. It's location is determined
je vyhledáno při spuštění, vyhledávací cesty jsou uvedeny výše.

You can use :ref:`gammu-config` or :ref:`gammu-detect` to generate
configuration file or start from :ref:`Fully documented example`.

Nějaké rady jak nastavit telefon můžete nalézt v Databázi telefonů Gammu
<http://wammu.eu/phones/>, kam uživatelé zapisují svoje zkušenosti.

This file use ini file syntax, see :ref:`ini`.

Konfigurační soubor pro gammu může obsahovat několik sekcí - ``[gammu]``,
``[gammu1]``, ``[gammuN]``, ... Každá sekce definuje jedno připojení a ve
výchozím režime gammu postupně zkouší všechny podle čísel. Můžete také zadat
parametrem pro :ref:`gammu`, které číslo sekce (:config:section:`[gammu]` je
číslo 0) má použít a pak se použije jen tato sekce.

.. config:section:: [gammu]

This section is read by default unless you specify other on command line.

Parametry připojení
+++++++++++++++++++++

.. config:option:: Připojení

    Protokol, který se použije pro komunikaci s telefonem.

    Pro telefony Nokia připojené kabelem budete chtít použít jedno z
    následujících:

    ``fbus``           
        sériové připojení protokolem FBUS
    ``dlr3``           
        DLR-3 a kompatibilní kabely
    ``dku2``           
        DKU-2 a kompatibilní kabely
    ``dku5``           
        DKU-5 a kompatibilní kabely
    ``mbus``           
        sériové připojení protokolem MBUS

    Pokud nepoužíváte originální kabel, možná budete potřebovat připojit
    ``-nodtr`` (například pro kabely používající ARK3116) nebo
    ``-nopower``. Toto nastavení použijte jen v případě, že Gammu správně
    parametery nezjistí automaticky.

    Pro ostatní telefony připojené kabelem budete chtít použít jedno z
    následujících:

    ``at``             
        obecné připojení používající AT příkazy

    Můžete volitelně zadat rychlost připojení, například ``at19200``, ale toto
    není nutné pro USB kabely.

    Pro připojení pomocí IrDA použijte jednu z následujících možností:

    ``irdaphonet``     
        protokol Phonet používaný telefony Nokia
    ``irdaat``         
        AT příkazy pro připojení k většině telefonů (toto není podporované na Linuxu).
    ``irdaobex``       
        připojení protokolem OBEX (IrMC nebo přenos souborů), použitelné pro většinu telefonů.
    ``irdagnapbus``    
        připojení k GNappletu běžícím na telefonech se Symbianem.

    Pro připojení pomocí Bluetooth použijte jednu z následujících možností:

    ``bluephonet``     
        protokol Phonet používaný telefony Nokia
    ``blueat``         
        AT příkazy pro připojení k většině telefonů.
    ``blueobex``       
        připojení protokolem OBEX (IrMC nebo přenos souborů), použitelné pro většinu telefonů.
    ``bluerfgnapbus``  
        připojení k GNappletu běžícím na telefonech se Symbianem.

.. config:option:: Zařízení

    .. versionadded::     1.27.95

    Zařízení kde je přístupný telefon nebo adresa telefonu, záleží na typu
    připojení.

    Pro **kabely** nebo emulovaný sériový port zadejte jméno zařízení (například
    ``/dev/ttyS0``, ``/dev/ttyACM0``, ``/dev/ircomm0``, ``/dev/rfcomm0`` na
    Linuxu, ``/dev/cuad0`` na FreeBSD nebo ``COM1:`` na Windows). Speciální
    výjjímkou jsou DKU-2 a DKU-5 kabely na Windows, kde je zařízení automaticky
    zjištěno z informací ovladače a tento parametr je ignorován.

    For **USB** connections (currently only fbususb and dku2 on Linux), you can
    specify to which USB device Gammu should connect. You can either provide
    vendor/product IDs or device address on USB::

        Device = 0x1234:0x5678  # Vyhledávání podle výrobce a produktu
        Device = 0x1234:-1      # Vyhledávání podle výrobce
        Device = 1.10           # Vyhledávání podle čísla sběrnice a adresy zařízení
        Device = 10             # Vyhledávání podle adresy zařízení
        Device = serial:123456  # Vyhledávání podle sériového čísla

    .. note::

        On Linux systems, you might lack permissions for some device nodes.  You
        might need to be member of some group (eg. :samp:`plugdev` or
        :samp:`dialout`) or or add special udev rules to enable you access these
        devices as non-root.

        For Nokia phones you can put follofing file (also available in sources as
        :file:`contrib/udev/45-nokiadku2.rules`) as
        :file:`/etc/udev/rules.d/45-nokiadku2.rules`:

        .. literalinclude::         ../../../contrib/udev/45-nokiadku2.rules
           :language: sh

    Při připojení přes **Bluetooth** musíte zadat Bluetooth adresu vašeho
    telefonu (na Linuxu můžete zjistit seznam zařízení v dosahu pomocí příkazu
    :command:`hcitool scan`). Volitelně můžete Gammu přinutit používat jiný
    kanál zadáním jeho čísla za lomítko.

    Před použitím Gammu, by vaše zařízení mělo být spárováno s počítačem, nebo
    byste měli mít zapnuto automatické párování.

    Pro připojení přes **IrDA** se tento parametr vůbec nepoužije.

    Pokud vám IrDA na Linuxu nefunguje, možná potřebujete zapnout rozhraní a
    zapnout objevování zařízení (tyto příkazy musíte spouštět jako root):

    .. code-block::     sh

        ip l s dev irda0 up sysctl net.irda.discovery=1

    .. note::

        Native IrDA is not supported on Linux, you need to setup virtual serial port
        for it (eg. ``/dev/ircomm0``) and use it same way as cable.  This can be
        usually achieved by loading module ``ircomm-tty``.

.. config:option:: Port

    .. deprecated::     1.27.95

    Stejné jako :config:option:`Device`, zachováno kvůli zpětné kompatibilitě.

.. config:option:: Model

    Nepoužívejte tento parametr, pokud to není opravdu nutné! Jediná situace,
    kdy může být dobré ho použít, je, když Gammu nezná váš telefon a špatně
    detekuje jeho vlastnosti.

    Speciální případ použití parametru Model je vynucení typu připojení přes
    OBEX namísto ponechání rozhodnutí na Gammu:

    ``obexfs``
        vynutí použití služby na procházení souborů (podpora přístupu k souborům)
    ``obexirmc``
        vynutí použití služby IrMC (kontakty, kalendář a poznámky)
    ``obexnone``
        nebube vybrána žádná služba, toto má jen omezené použití pro posílání
        souborů (příkaz :option:`gammu sendfile`)
    ``mobex``
        služba m-obex pro telefony Samsung

.. config:option:: Use_Locking

    Na Posixových systémech můžete chtít použít zamykání zařízení pomocí UUCP
    zámků. Zapnutím této volby (nastavením na yes) bude Gammu dodržovat tyto
    zámky a vytvářet je při připojování. Na většině distribucí pro vytvoření
    zámku potřebujete dodatečná oprávnění (například být členem skupiny uucp).

    Tento parametr nemá na Windows žádný význam.

Parametry připojení
+++++++++++++++++++++

.. config:option:: SynchronizeTime

    Určí, zda chcete nastavit čas v telefonu při připojení.

.. config:option:: StartInfo

    Tato volba umožní nastavit (při nastavení na ``yes``) zobrazení zprávy na
    displeji telefonu nebo rozsvítit jeho podsvícení při připojení. Telefon
    nebude během připojení vydávat žádné zvuky. Tato volba funguje jen s
    některými telefony Nokia.


Ladicí parametry
+++++++++++++++++

.. config:option:: LogFile

    Cesta k souboru, kde budou uloženy informace o komunikaci s telefonem.

    .. note::

        For most debug levels (excluding ``errors``) the log file is overwritten on
        each execution.

.. config:option:: LogFormat

    Určuje co vše se bude logovat do souboru :config:option:`LogFile`. Možné
    hodnoty jsou:

    ``nothing``     
        žádné ladicí zprávy
    ``text``        
        základní informace o přenosu v textové podobě
    ``textall``     
        prodrobné informace o přenosu v textové podobě
    ``textalldate`` 
        prodrobné informace o přenosu v textové podobě, s časovou značkou
    ``errors``      
        chyby v textovém formátu
    ``errorsdate``  
        chyby v textovém formátu, s časovou značkou
    ``binary``      
        obsah komunikace v binárním formátu

    Pro ladění použijte buď ``textalldate`` nebo ``textall``, tím budou v logu
    obsaženy všechny informace potřebné pro diagnostiku problému.

.. config:option:: Features

    Vlastní parametry telefonu. Toto může být použito pro přepsání parametrů
    zadaných v ``common/gsmphones.c``, které jsou chybné. Pro seznam hodnot se
    můžete podívat to ``include/gammu-info.h`` (všechny hodnot
    :c:type:`GSM_Feature` bez prefixu ``F_``). Prosím nahlašte správné hodnoty
    autorům Gammu.

Locales and character set options
+++++++++++++++++++++++++++++++++

.. config:option:: GammuCoding

    Vynutí použití zadaného kódování (například ``1250`` vynutí CP-1250 nebo
    ``utf8`` pro UTF-8). Tato volba by obvykle neměla být potřeba, Gammu
    kódování zjištuje podle nastavených locales.

.. config:option:: GammuLoc

    Cesta k adresáři s lokalizačními soubory (adresář by měl obsahovat
    ``LANG/LC_MESSAGES/gammu.mo``). Pokud je gammu správně instalované, mělo
    bypřeklady najít automaticky.

Examples
--------

Podrobnější příklady jsou dostupné v dokumentaci Gammu.

Příklady připojení
++++++++++++++++++++++

Konfigurace Gammu pro telefon Nokia připojený kabelem DLR-3:

.. code-block:: ini

    [gammu] device = /dev/ttyACM0 connection = dlr3

Konfigurace Gammu pro telefon Sony-Ericsson (nebo jiný používající AT
příkazy) připojený pomocí USB kabelu:

.. code-block:: ini

    [gammu] device = /dev/ttyACM0 connection = at

Konfigurace Gammu pro telefon Sony-Ericsson (nebo jiný používající AT
příkazy) připojený pomocí Bluetooth:

.. code-block:: ini

    [gammu] device = B0:0B:00:00:FA:CE connection = blueat

Nastavení Gammu pro telefon, ke kterému se musí připojovan na Bluetooth
kanálu 42:

.. code-block:: ini

    [gammu] device = B0:0B:00:00:FA:CE/42 connection = blueat

Práce s více telefony
+++++++++++++++++++++++

Gammu může být nastaveno pro práci s více telefony najedou (ale jen jedno
připojení v jeden okamžik, to zvolíte parametrem :option:`gammu
-s`). Nastavení telefonů na třech sériových portech by vypadalo následovně:

.. code-block:: ini

    [gammu] device = /dev/ttyS0 connection = at

    [gammmu1] device = /dev/ttyS1 connection = at

    [gammmu2] device = /dev/ttyS2 connection = at

.. _Plně dokumentovaný příklad:

Plně dokumentovaný příklad
++++++++++++++++++++++++++++++

You can find this sample file as :file:`docs/config/gammurc` in Gammu
sources.

.. literalinclude:: ../../../docs/config/gammurc
   :language: ini

