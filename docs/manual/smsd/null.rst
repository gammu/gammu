.. _gammu-smsd-null:

Null Backend
============

Description
-----------

NULL backend does not store data at all. It could be useful in case you
don't want to store messages at all and you want to process then in
:config:option:`RunOnReceive` handler.

Configuration
-------------

The configuration file then can look like:

.. code-block:: ini

    [smsd]
    Service = null
    RunOnReceive = /usr/local/bin/process-sms

.. seealso:: :ref:`gammu-smsdrc`

