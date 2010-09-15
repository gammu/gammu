State Machine
=============

The state machine is core of libGammu operations. It gets the data from the
phone and dispatches them through protocol layer to phone drivers.

To see how it operates, following figure shows example of what happens when
:c:func:`GSM_GetModel` is called from the program:

.. graphviz::

   digraph statemachine {
        "GSM_GetModel" -> "Phone.Functions.GetModel";
        "Phone.Functions.GetModel" -> "GSM_WaitFor";
        "GSM_WaitFor" -> "Protocol.Functions.WriteMessage";
        "Protocol.Functions.WriteMessage" -> "GSM_WaitForOnce";
        "GSM_WaitForOnce" -> "GSM_WaitFor" [label="Retries"];
        "GSM_WaitForOnce" -> "GSM_ReadDevice";
        "GSM_ReadDevice" -> "GSM_WaitForOnce" [label="Wair for complete request"];
        "GSM_ReadDevice" -> "Device.Functions.ReadDevice";
        "Device.Functions.ReadDevice" -> "GSM_ReadDevice" [label="Wait for data"];
        "Device.Functions.ReadDevice" -> "Protocol.Functions.StateMachine";
        "Protocol.Functions.StateMachine" -> "Phone.Functions.DispatchMessage"
        "Phone.Functions.DispatchMessage" -> "GSM_DispatchMessage";
        "GSM_DispatchMessage" -> "Phone.Functions.ReplyFunctions[]()";
        "Phone.Functions.ReplyFunctions[]()" -> "Phone.Functions.GetModel";
        "Phone.Functions.GetModel" -> "GSM_GetModel";
   }

