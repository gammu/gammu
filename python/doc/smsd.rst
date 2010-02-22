:mod:`gammu.smsd` -- SMSD access
===================================

.. module:: gammu.smsd
    :synopsis: Provides access to Gammu SMSD functions.

:class:`gammu.smsd.SMSD`
------------------------

.. class:: gammu.smsd.SMSD(Config)
    
    SMSD main class, that is used for communication with phone.

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
        
        This works also for external SMSD daemon instances, the instance is
        identified by configuration file passed when creating SMSD class.

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
        
        This works also for external SMSD daemon instances, the instance is
        identified by configuration file passed when creating SMSD class.

        :param Message: Nessage to inject (can be multipart)
        :type Message: list
        :return: ID of inserted message
        :rtype: string
