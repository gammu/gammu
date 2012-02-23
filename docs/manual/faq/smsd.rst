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

.. _Kalkun: http://kalkun.sourceforge.net/
.. _MySQL: http://www.mysql.com/
.. _PostgreSQL: http://www.postgresql.org/
.. _libdbi: http://libdbi.sourceforge.net/
.. _Gammu Phones Database: http://wammu.eu/phones/
