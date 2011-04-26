Adding support for new phone
============================

This document covers basic information on adding support for new phone into
Gammu. It will never cover all details, but will give you basic instructions.

Adding support for new AT commands
----------------------------------

The easiest situation is when all you need to support new device is to add
support for new AT commands. All the protocol infrastructure is there, you
only need to hook new code into right places.

The main code for AT driver is in :file:`libgammu/phone/at/atgen.c`. At the
bottom of the file, you can find two arrays, one defining driver interface
(:c:type:`GSM_Phone_Functions`) and second one defining callbacks (see
:ref:`reply-functions` for more detailed description. You will definitely need
to define callbacks for newly introduced commands, but the interface for
desired functionality might already exist.

Detecting whether command is supported
++++++++++++++++++++++++++++++++++++++

As Gammu is trying to support as much phones as possible, you should try to
make it automatically detect whether connected phone supports the command.
This can be done on first invocation of affected operation or on connecting to
phone. As we want to avoid lenghty connecting to phone, in most cases you
should probe for support on first attempt to use given functionality. The code
might look like following:

.. code-block:: c

    GSM_Error ATGEN_GetFoo(GSM_StateMachine *s) {
        GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

        if (Priv->Foo_XXXX == 0) {
            ATGEN_CheckXXXX(s);
        }

        if (Priv->Foo_XXXX == AT_AVAILABLE) {
            /* Perform reading */
        }

        /* Fail with error or fallback to other methods */
        return ERR_NOTSUPPORTED;
    }

    GSM_Error ATGEN_CheckXXXX(GSM_StateMachine *s) {
        GSM_Error 	error;
        GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

        smprintf(s, "Checking availability of XXXX\n");
        ATGEN_WaitForAutoLen(s, "AT+XXXX=?\r", 0x00, 4, ID_GetProtocol);
        if (error == ERR_NONE) {
            Priv->Foo_XXXX = AT_AVAILABLE;
        } else {
            Priv->Foo_XXXX = AT_NOTAVAILABLE;
        }
        return error;
    }


    GSM_Reply_Function ATGENReplyFunctions[] = {
    ...
    {ATGEN_GenericReply,	"AT+XXXX=?"		,0x00,0x00,ID_GetProtocol		 },
    ...

Alternatively (if detection is not possible), you can use features and phones
database (see :file:`libgammu/gsmphones.c`) or vendor based decision to use
some commands.

Invoking AT command
+++++++++++++++++++

The AT commands are invoked using :c:func:`GSM_WaitFor`, or a wrapper
:c:func:`ATGEN_WaitForAutoLen`, where you don't have to specify length for
text commands and automatically sets error variable.

Generally you need to construct buffer and then invoke it. For some simple
functions it is pretty straight forward:

.. code-block:: c

    GSM_Error ATGEN_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
    {
        GSM_Error error;

        GSM_ClearBatteryCharge(bat);
        s->Phone.Data.BatteryCharge = bat;
        smprintf(s, "Getting battery charge\n");
        ATGEN_WaitForAutoLen(s, "AT+CBC\r", 0x00, 4, ID_GetBatteryCharge);
        return error;
    }

As you can see, it is often required to store pointer to data store somewhere,
for most data types ``s->Phone.Data`` does contain the pointer to do that.

Parsing reply
+++++++++++++

For parsing reply, you should use :c:func:`ATGEN_ParseReply`, which should
be able to handle all encoding and parsing magic. You can grab lines from the
reply using :c:func:`GetLineString`.

The reply function needs to be hooked to the reply functions array, so that it
is invoked when reply is received from the phone.

Continuing in above example for getting battery status, the (simplified)
function would look like:

.. code-block:: c

    GSM_Error ATGEN_ReplyGetBatteryCharge(GSM_Protocol_Message *msg, GSM_StateMachine *s)
    {
        GSM_Error error;
        GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
        GSM_BatteryCharge *BatteryCharge = s->Phone.Data.BatteryCharge;
        int bcs = 0, bcl = 0;

        switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
            case AT_Reply_OK:
                smprintf(s, "Battery level received\n");
                error = ATGEN_ParseReply(s,
                    GetLineString(msg->Buffer, &Priv->Lines, 2),
                    "+CBC: @i, @i",
                    &bcs,
                    &bcl);

                BatteryCharge->BatteryPercent = bcl;

                switch (bcs) {
                    case 0:
                        BatteryCharge->ChargeState = GSM_BatteryPowered;
                        break;
                    case 1:
                        BatteryCharge->ChargeState = GSM_BatteryConnected;
                        break;
                    case 2:
                        BatteryCharge->ChargeState = GSM_BatteryCharging;
                        break;
                    default:
                        BatteryCharge->ChargeState = 0;
                        smprintf(s, "WARNING: Unknown battery state: %d\n", bcs);
                        break;
                }
                return ERR_NONE;
            case AT_Reply_Error:
                smprintf(s, "Can't get battery level\n");
                return ERR_NOTSUPPORTED;
            case AT_Reply_CMSError:
                smprintf(s, "Can't get battery level\n");
                return ATGEN_HandleCMSError(s);
            case AT_Reply_CMEError:
                return ATGEN_HandleCMEError(s);
            default:
                return ERR_UNKNOWNRESPONSE;
        }
    }


    GSM_Reply_Function ATGENReplyFunctions[] = {
    ...
    {ATGEN_ReplyGetBatteryCharge,	"AT+CBC"		,0x00,0x00,ID_GetBatteryCharge	 },
    ...

As you can see, all reply function first need to handle which error code did
they receive and return appropriate error if needed. Functions
:c:func:`ATGEN_HandleCMSError` and :c:func:`ATGEN_HandleCMEError` simplify
this, but you might need to customize it by handling some error codes manually
(eg. when phone returns error on empty location).

The rest of the function is just call to :c:func:`ATGEN_ParseReply` and
processing parsed data.
