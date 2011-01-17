Phone Support FAQ
=================

Is my phone supported?
----------------------

Generally any phone capable of AT commands or IrMC should be supported. Also
any Nokia phone using Nokia OS should work. For Symbian please check separate
topic. You can check other user experiences in `Gammu Phones Database`_.

.. seealso:: :ref:`faq-symbian`, :ref:`gammurc`

Which phone is best for SMS gateway?
------------------------------------

Forget about using standard phones, they tend not to be reliable for long time
connection to PC. Best option are GSM (GPRS, UMTS) terminals/modems. The best
option seem to be Siemens modems (eg. ES75/MC35i/MC55i). Slightly cheaper,
while still good are modems made by Huawei (eg. E160/E220/E1750/...). You can
check other user experiences in `Gammu Phones Database`_.

.. _faq-symbian:

Are Symbian phones supported?
-----------------------------

Short answer: Not really.

Long answer: For older phones (Symbian 9.0 and older), you can install gnapplet
to phone and access data through it. However gnapplet has not yet been ported
to newer versions, so you have no chance with recent phones. You can try using
`Series60-Remote`_, which works pretty well with S60 phones. Another option is
using something what supports SyncML to retrieve contacts or calendar from your
phone, for example `OpenSync`_ or `syncEvolution`_.

.. _Gammu Phones Database: http://wammu.eu/phones/
.. _Series60-Remote: http://series60-remote.sourceforge.net/
.. _OpenSync: http://www.opensync.org/
.. _syncEvolution: http://syncevolution.org/
