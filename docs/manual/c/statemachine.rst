State machine
=============

.. c:function:: GSM_Error GSM_InitConnection_Log(GSM_StateMachine * s, int ReplyNum, GSM_Log_Function log_function, void *user_data)

    :param s: State machine data
    :param ReplyNum: Number of replies to await (usually 3).
    :param log_function: Logging function, see :c:type:`GSM_SetDebugFunction`.
    :param user_data: User data for logging function, see :c:type:`GSM_SetDebugFunction`.
    :return: Error code

    Initiates connection with custom logging callback.

.. c:function:: GSM_Error GSM_InitConnection(GSM_StateMachine * s, int ReplyNum)

    :param s: State machine data
    :param ReplyNum: Number of replies to await (usually 1). Higher value makes sense only on unreliable links.
    :return: Error code

    Initiates connection.


.. c:function:: GSM_Error GSM_TerminateConnection(GSM_StateMachine * s)

    :param s: State machine data
    :return: Error code

    Terminates connection.

.. c:function:: GSM_Error GSM_AbortOperation(GSM_StateMachine * s)

    :param s: State machine data
    :return: Error code

    Aborts current operation.
 
    This is thread safe call to abort any existing operations with the
    phone.

.. c:function: GSM_Error GSM_Install(GSM_StateMachine *s, const char *ExtraPath)

    :param s: State machine data.
    :param ExtraPath: Extra path where to search for installation data.
    :return: Result of operation.

    Installs applet required for configured connection to the phone.

.. c:type:: GSM_StateMachine

    Private structure holding information about phone connection. Should
    be allocated by :c:func:`GSM_AllocStateMachine` and freed by
    :c:func:`GSM_FreeStateMachine`.

.. c:type:: GSM_ConnectionType

     Connection types definitions.

.. c:type:: GSM_Config

    Configuration of state machine.

    .. c:member:: char Model[50]

        Model from config file.

.. c:type:: GSM_Log_Function

    :param text: Text to be printed, \n will be also sent (as a separate message).
    :param data: Arbitrary logger data, as passed to :c:func:`GSM_InitConnection_Log`.
    :returns: void

    Type of callback function for logging.

.. doxygenfunction:: GSM_ReadDevice
.. doxygenfunction:: GSM_IsConnected
.. doxygenfunction:: GSM_FindGammuRC
.. doxygenfunction:: GSM_ReadConfig
.. doxygenfunction:: GSM_GetConfig
.. doxygenfunction:: GSM_GetConfigNum
.. doxygenfunction:: GSM_SetConfigNum
.. doxygenfunction:: GSM_AllocStateMachine
.. doxygenfunction:: GSM_FreeStateMachine
.. doxygenfunction:: GSM_GetUsedConnection
.. doxygenstruct:: GSM_Config

