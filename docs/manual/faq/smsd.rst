SMSD FAQ
========

Which databases does SMSD support?
----------------------------------

SMSD natively supports `MySQL`_ and `PostgreSQL`_. However it has also support for
`libdbi`_, which provides access to wide range of database engines (eg. SQLite, MS
SQL Server, Sybase, Firebird,...). Unfortunately `libdbi`_ currently does not work
natively on Microsoft Windows, so you can use it only on Unix platforms.

Since version 1.29.92, SMSD can also connect to any ODBC data source, so you
should be able to connect to virtually any database engine using this
standard.

.. seealso:: :ref:`gammu-smsd-sql`

Is there some user interface for SMSD?
--------------------------------------

Yes. You can use some of example interfaces distributed with gammu in ``contrib``
directory. Or there is full featured separate interface written in PHP called
`Kalkun`_.

.. _faq-smsd-phone:

Which phone is best for SMSD gateway?
-------------------------------------

Standard phones usually do not perform good when used long term as a modem. So
it's always better to choose some GSM (GPRS, EDGE, UMTS) terminals/modems, which
are designed to be used long for term in connection with computer.

The best option seem to be Siemens modems (eg. ES75/MC35i/MC55i). Slightly
cheaper, while still good are modems made by Huawei (eg. E160/E220/E1750/...).
We have heard also positive experiences with cheap modems from various Chinese
resellers like DealExtreme or Alibaba.

.. seealso:: You can check other user experiences in `Gammu Phones Database`_.

The RunOnReceive script fails, how to fix that?
-----------------------------------------------

There can be various reasons why the script you've supplied as
:config:option:`RunOnReceive` has failed. You can usually find more information
in the debug log (see :ref:`reporting-bugs-smsd`). For example it can look like
following:

.. code-block:: log

    gammu-smsd[9886]: Starting run on receive: ../received.sh
    gammu-smsd[9875]: Process failed with exit status 2
    gammu-smsd[9875]: Subprocess output: ../received.sh: 7: ../received.sh: Syntax error: end of file unexpected (expecting "then")

From here it's quite easy to diagnose it's a syntax error in the script causing
troubles.

.. note::

    If process output is missing from your debug log, you're using older
    version, which didn't support this. Please upgrade to version newer than
    1.36.4.

.. seealso:: :ref:`gammu-smsd-run`, :config:option:`RunOnReceive`

Why received delivery reports are not matched to sent messages?
---------------------------------------------------------------

This can occasionally happen and can have several reasons.

* If reports are arriving late, you can adjust
  :config:option:`DeliveryReportDelay`.
* If reports are coming from different SMSC than you're using for sending, set
  :config:option:`SkipSMSCNumber`.
* If SMSD is unable to match sent message with delivery report, it might be due
  to missing international prefix in one of the numbers. Generally the best
  approach is to always send messages to international number (eg. use
  ``+32485xxxxxx`` instead of ``0485xxxxxx``).

.. note::

    If using Gammu 1.36.3 or newer, whenever first two cases happen, you will
    see hint to adjust the configuration in the log.

.. _Kalkun: http://kalkun.sourceforge.net/
.. _MySQL: https://www.mysql.com/
.. _PostgreSQL: https://www.postgresql.org/
.. _libdbi: http://libdbi.sourceforge.net/
.. _Gammu Phones Database: https://wammu.eu/phones/
