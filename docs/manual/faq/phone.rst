Phone Support FAQ
=================

.. _faq-phones:

Is my phone supported?
----------------------

Generally any phone capable of AT commands or IrMC should be supported. Also
any Nokia phone using Nokia OS should work. For Symbian please check separate
topic. You can check other user experiences in `Gammu Phones Database`_.

For information how to configure your phone, see :ref:`faq-config`.

.. seealso:: :ref:`faq-symbian`, :ref:`faq-android`, :ref:`faq-blackberry`, :ref:`faq-iphone`, :ref:`gammurc`

Which phone is best for SMS gateway?
------------------------------------

Forget about using standard phones, they tend not to be reliable for long time
connection to PC. Best option are GSM (GPRS, UMTS) terminals/modems. The best
option seem to be Siemens modems (eg. ES75/MC35i/MC55i). Slightly cheaper,
while still good are modems made by Huawei (eg. E160/E220/E1750/...). You can
check other user experiences in `Gammu Phones Database`_.

.. _faq-nokia:

Are Nokia phones supported?
---------------------------

It depends on used operating systems Series40 and older phones should work 
(see :ref:`faq-config-nokia` for information how to configure them), Symbian 
based phones are covered in separate topic, check :ref:`faq-symbian`.

.. _faq-symbian:

Are Symbian phones supported?
-----------------------------

You need to install applet to the phone to allow Gammu talk to it. For older
phones (Symbian 9.0 and older), install gnapplet (see :ref:`gnapplet`). Newer
phones can use Python based applet called Series60-remote (see :ref:`s60`).
This option is supported since Gammu 1.29.90.

.. seealso:: :ref:`faq-config-symbian`

.. _faq-android:

Are Andriod phones supported?
-----------------------------

Unfortunately no at the moment. Any help in this area is welcome.

.. _faq-blackberry:

Are Blackberry phones supported?
--------------------------------

Unfortunately no at the moment. Any help in this area is welcome.

.. _faq-iphone:

Are iPhone phones supported?
----------------------------

Unfortunately no at the moment. Any help in this area is welcome.

What are free alternatives to Gammu?
------------------------------------

It depends on your phone. For Nokia or AT based phones, you can try `Gnokii`_,
but Gammu should be superior in most cases. For Symbian phone you can try using
`Series60-Remote`_, which works pretty well with S60 phones.

If you are looking for synchronisation, try using something what supports
SyncML to retrieve contacts or calendar from your phone, for example
`OpenSync`_ or `syncEvolution`_.

.. _Gammu Phones Database: http://wammu.eu/phones/
.. _Series60-Remote: http://series60-remote.sourceforge.net/
.. _OpenSync: http://www.opensync.org/
.. _syncEvolution: http://syncevolution.org/
.. _Gnokii: http://gnokii.org/
