:mod:`gammu.smsd` -- SMSD access
===================================

.. module:: gammu.smsd
    :synopsis: Provides access to Gammu SMSD functions.

:class:`SMSD`
-------------

.. class:: SMSD(Config)

    SMSD main class, that is used for communication with phone.

    You don't need to run the SMS daemon itself to control or ask it for
    status, this can be also done on separately running instances. All you
    need to do for this is to give same configuration file as that instance is
    using.

    :param Config: Path to SMSD configuration file.
    :type Config: string

    .. method:: MainLoop(MaxFailures)

        Runs SMS daemon main loop.

        Please note that this will run until some serious error occurs or until
        terminated by :meth:`Shutdown`.

        :param MaxFailures: After how many init failures SMSD ends. Defaults to 0, what means never.
        :type MaxFailures: int
        :return: None
        :rtype: None


    .. method:: Shutdown()

        Signals SMS daemon to stop.

        :return: None
        :rtype: None

    .. method:: GetStatus()

        Returns SMSD status.

        The following values are set in resulting dictionary:

        Client

            Client software name.

        PhoneID

            PhoneID which can be used for multiple SMSD setup.

        IMEI

            IMEI of currently connected phone.

        Sent

            Number of sent messages.

        Received

            Number of received messages.

        Failed

            Number of failed messages.

        BatterPercent

            Last battery state as reported by connected phone.

        NetworkSignal

            Last signal level as reported by connected phone.

        :return: Dict with status values
        :rtype: dict

    .. method:: InjectSMS(Message)

        Injects SMS message into outgoing messages queue in SMSD.

        :param Message: Message to inject (can be multipart)
        :type Message: list of :ref:`sms_obj`
        :return: ID of inserted message
        :rtype: string
