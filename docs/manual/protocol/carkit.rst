Nokia 6210/6310, CARC91, PC Experiment
======================================

Author:
    Jens Bennfors

Company
    AB Indevia

Date:
    2002-04-09

Introduction
-------------

The purpose of this experiment is to gain understanding about how Nokias commands for handsfree works in a way that can be of use in the construction of Com.n.sense. The means available is a Nokia 6210, a Nokia 6310, a HFU-2 CARC91 and a PC with a LabVIEW program installed.

Setup
-----

I have connected the phone to a Nokia original handsfree (CARC91). I then use the PC for listening to the data communication between the phone and CARC91. I also send the frames directly from the PC to the phone.

Nokia 6210
----------

Phone connected to PC
+++++++++++++++++++++

Initiation
~~~~~~~~~~
1F0004 D0 0001 04 00CE
    Power up from PC
1F0004 D0 0001 04 01CF
    Power up from PC
1F0400 D0 0001 05 10DF
    Power up from phone
1F0004 79 0005 0201 0164 00 0203
    Enable carkit mode from PC
1F0004 79 0005 0201 0164 00 0302
    Enable carkit mode from PC
1F0400 7F 0367
    Ack from phone
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 044F
      HFU-2 Version
1F0400 7F 0460
    Ack from phone
1F0400 78 0004 0102 0801 117C
    Status 0x08, 0x01 from phone
1F0400 DA 0002 0002 12D3
    Type => 0xDA, data => 0x00, 0x02
1F0004 79 0005 0201 0164 00 0504
    Enable carkit mode from PC
1F0004 79 0005 0201 0164 00 0607
    Enable carkit mode from PC
1F0400 7F 0662
    Ack from phone
1F0004 78 0003 0201 0307 67
    Status confirm from PC
1F0004 78 0003 0201 0308 68
    Status confirm from PC
1F0400 7F 086C
    Ack from phone

The phone enters the profile ”handsfree” when the frame carkit enable is sent. It sends out an unknown status frame 0x08, 0x01.

Incoming call
~~~~~~~~~~~~~

1F0400 78 0004 0102 0701 197B
    Status 0x07, 0x01 from phone
1F0400 78 0004 0102 0E03 1A73
    Status 0x0E, 0x03 from phone

Status type 0x07 with status 0x01 means mute external audio equipment. Status type 0x0E with status 0x03 means audio amplifier on.

Connected
~~~~~~~~~

The phone doesn’t send out anything when a call has been set up.

Initiation with connected phone
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1F0004 D0 0001 04 00CE
    Power up from PC
1F0400 D0 0001 05 1BD4
    Power up from phone
1F0004 79 0005 0201 0164 0001 00
    Enable carkit mode from PC
1F0400 7F 0165
    Ack from phone
1F0400 78 0004 0102 0E03 1C75
    Status 0x0E, 0x03 from phone
1F0400 78 0004 0102 0701 1D7F
    Status 0x07, 0x01 from phone
1F0004 79 0012 0201 0206 00 5620 3037 2E30 300A 4846 5532 00 0249
    HFU-2 Version from PC
1F0400 7F 0266
    Ack from phone
1F0400 78 0004 0102 0801 1E73
    Status 0x08, 0x01 from phone
1F0004 79 0005 0201 0164 0003 02
    Enable carkit mode from PC
1F0400 7F 0367
    Ack from phone
1F0400 78 0004 0102 0E03 1F76
    Status 0x0E, 0x03 from phone
1F0400 78 0004 0102 0701 2042
    Status 0x07, 0x01 from phone
1F0004 78 0003 0201 03 0464
    Status confirm from PC
1F0400 7F 0460
    Ack from phone

Disconnected
~~~~~~~~~~~~

1F04 0078 0004 0102 0700 2142
    Status 0x07, 0x00

Incoming SMS
~~~~~~~~~~~~

FCF0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0F0 F0
    Initiation of bit length from phone
1F0400 78 0004 0102 0E03 254C
    Status 0x0E, 0x03 from phone
F0F0F0F0
    Initiation of bit length from phone

Phone connected to CARC91
+++++++++++++++++++++++++

Initiation
~~~~~~~~~~

1F0004 D0 0001 04 00CE
    Power up from HFU-2
1F0400 D0 0001 05 02CD
    Power up from phone
1F0004 79 0005 0201 0164 00 0100
    Enable carkit mode from HFU-2
1F0400 7F 0165
    Ack from phone
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 0249
    HFU-2 Version
1F0400 7F 0266
    Ack from phone
1F0400 78 0004 0102 0801 036E
    Status 0x08, 0x01
1F0004 79 0005 0201 0164 00 0302
    Enable carkit mode from HFU-2
1F0400 7F 0367
    Ack from phone
1F0400 78 0004 0102 0801 036E
    Status 0x08, 0x01
1F0004 7F 0367
    Ack from HFU-2
1F0400 DA 0002 0002 04C5
    Status type => 0xDA, data => 0x00, 0x02
1F0004 7F 0460
    Ack from HFU-2
1F0400 78 0004 0102 0E03 056C
    Status 0x0E, 0x03
1F0004 7F 0561
    Ack from HFU-2
1F0004 78 0003 0201 03 0464
    Status confirm from HFU-2
1F0400 7F 0460
    Ack from phone
1F0400 78 0004 0102 0E00 066C
    Status 0x0E, 0x00
1F0004 7F 0662
    Ack from HFU-2
1F0004 78 0003 0201 03 0565
    Status confirm from HFU-2
1F0400 7F 0561
    Ack from phone

Incoming call
~~~~~~~~~~~~~

1F0400 78 0004 0102 0701 1173
    Status 0x07, 0x01
1F0004 7F 1175
    Ack from HFU-2
1F0400 78 0004 0102 0E03 127B
    Status 0x0E, 0x03
1F0004 7F 1276
    Ack from HFU-2
1F0004 78 0003 0201 03 0868
    Status confirm from HFU-2
1F0400 7F 086C
    Ack from phone

Connected
~~~~~~~~~

The phone doesn’t send out anything when a call has been set up.

Initiation with connected phone
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1F0004 D0 0001 04 00CE
    Power up from HFU-2
1F0400 D0 0001 05 1AD5
    Power up from phone
1F0004 79 0005 0201 0164 00 0100
    Enable carkit mode from HFU-2
1F0400 7F 0165
    Ack from phone
1F0400 78 0004 0102 0E03 1B72
    Status 0x0E, 0x03
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 0249
    HFU-2 Version
1F0400 7F 0266
    Ack from phone
1F0004 79 0005 0201 0164 00 0302
    Enable carkit mode from HFU-2
1F0400 7F 0367
    Ack from phone
1F0400 78 0004 0102 0E03 1B72
    Status 0x0E, 0x03
1F0004 7F 1B7F
    Ack from HFU-2
1F0400 78 0004 0102 0801 1C71
    Status 0x08, 0x01
1F0004 78 0003 0201 03 0464
    Status confirm from HFU-2
1F0400 7F 0460
    Ack from phone
1F0400 78 0004 0102 0801 1C71
    Status 0x08, 0x01
1F0004 7F 1C78
    Ack from HFU-2
1F0400 78 0004 0102 0E03 1D74
    Status 0x0E, 0x03
1F0004 7F 1D79
    Ack from HFU-2
1F0400 78 0004 0102 0701 1E7C
    Status 0x07, 0x01
1F0004 78 0003 0201 03 0565
    Status confirm from HFU-2
1F0400 7F 0561
    Ack from phone
1F0400 78 0004 0102 0701 1E7C
    Status 0x07, 0x01
1F0004 7F 1E7A
    Ack from HFU-2
1F0400 78 0004 0102 0701 1F7D
    Status 0x07, 0x01
1F0004 7F 1F7B
    Ack from phone
1F0400 DA 0002 0002 20E1
    Typ => 0xDA, data => 0x00. 0x02
1F0004 7F 2044
    Ack from HFU-2

Disconnected
~~~~~~~~~~~~

1F0400 78 0004 0102 0700 1774
    Status 0x07, 0x00
1F0004 7F 1773
    Ack from HFU-2
1F0400 78 0004 0102 0E00 1872
    Status 0x0E, 0x00
1F0004 7F 187C
    Ack from HFU-2
1F0004 78 0003 0201 03 0B6B
    Status confirm from HFU-2
1F0400 7F 0B6F
    Ack from phone

Incoming SMS
~~~~~~~~~~~~

1F0400 78 0004 0102 0E03 076E
    Status 0x0E, 0x03
1F0004 7F 0763
    Ack from HFU-2
1F0004 78 0003 0201 03 0666
    Status confirm from HFU-2
1F0400 7F 0662
    Ack from phone
1F0400 78 0004 0102 0E00 0862
    Status 0x0E, 0x00
1F0004 7F 086C
    Ack from HFU-2
1F0004 78 0003 0201 03 0767
    Status confirm from HFU-2
1F0400 7F 0763
    Ack from phone

Button pushed
~~~~~~~~~~~~~

1F0400 78 0004 0102 0E03 0960
    Status 0x0E, 0x03
1F0004 7F 096D
    Ack from HFU-2
1F0004 78 0003 0201 03 0868
    Status confirm from HFU-2
1F0400 7F 086C
    Ack from phone
1F0400 78 0004 0102 0E00 0A60
    Status 0x0E, 0x00
1F0004 7F 0A6E
    Ack from HFU-2
1F0004 78 0003 0201 03 0969
    Status confirm from HFU-2
1F0400 7F 096D
    Ack from phone

Nokia 6310
----------

Phone connected to PC
+++++++++++++++++++++

Initiation
~~~~~~~~~~

1F0004 D0 0001 04 02CC
    Power up from PC
1F0400 D0 0001 05 0DC2
    Power up from phone
1F0004 79 0005 0201 0164 00 0C0D
    Enable carkit mode from PC
1F0400 7F 0C68
    Ack from phone
1F0400 78 0004 0128 0B00 0E4B
    Status 0x0B, 0x00 from phone
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 0D46
    HFU-2 version from PC
1F0400 7F 0E6A
    Ack from phone
1F0400 DA 0004 0028 0000 0FE2
    ?
1F0004 79 0005 0201 0164 00 1716
    Enable carkit mode from PC
1F0400 7F 1773
    Ack from phone
1F0400 78 0004 0128 0B00 1055
    Status 0x0B, 0x00 from phone
1F0004 78 0003 0201 03 1878
    Status confirm from PC
1F0400 7F 1A7E
    Ack from phone

An unknown status frame (0x0B) is sent by the phone.

Incoming call
~~~~~~~~~~~~~

1F0400 78 0004 0128 0701 0D45
    Status 0x07, 0x01 from phone
1F0400 78 0004 0128 0E01 0F4E
    Status 0x0E, 0x01 from phone
1F0400 78 0004 0128 0A00 1054
    Status 0x0A, 0x00 from phone
1F0400 78 0004 0128 0901 1157
    Status 0x09, 0x01 from phone

Byte 8 in the status frames is some kind of ID number. 0x28 is the ID for 6310. Status 0x0A, 0x09 is unknown.

Connected
~~~~~~~~~

The phone doesn’t send out anything when a call has been set up. This might be because the profile “handsfree” is lost when ack isn’t sent.

Initiation with connected phone
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 1C57
    HFU-2 version from PC
1F0400 7F 1C78
    Ack from phone
1F0400 78 0004 0128 0E02 1A58
    Status 0x0E, 0x02
1F0400 78 0004 0128 0A00 1B5F
    Status 0x0A, 0x00
1F0400 78 0004 0128 0900 1C5B
    Status 0x09, 0x00
1F0400 78 0004 0128 0701 1D55
    Status 0x07, 0x01
1F0004 D0 0001 04 00CE
    Power up from HFU-2
1F0400 D0 0001 05 74BB
    Power up from phone
1F0004 79 0005 0201 0164 00 0100
    Enable carkit mode from HFU-2
1F0400 7F 0165
    Ack from phone
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 0249
    HFU-2 Version
1F0400 7F 0266
    Ack from phone
1F0400 78 0004 0128 0E01 7534
    Status 0x0E, 0x01
1F0004 79 0005 0201 0164 00 0302
    Enable carkit mode from HFU-2
1F0400 7F 0367
    Ack from phone
1F0400 78 0004 0128 0E01 7534
    Status 0x0E, 0x01
1F0004 7F 7511
    Ack from HFU-2
1F0400 78 0004 0128 0A01 7633
    Status 0x0A, 0x01
1F0004 7F 7612
    Ack from HFU-2
1F0400 78 0004 0128 0901 7731
    Status 0x09, 0x01
1F0004 7F 7713
    Ack from HFU-2
1F0400 78 0004 0128 0701 7830
    Status 0x07, 0x01
1F0004 7F 781C
    Ack from HFU-2
1F0400 78 0004 0128 0E01 7938
    Status 0x0E, 0x01
1F0004 7F 791D
    Ack from HFU-2
1F0004 78 0003 2801 03 044E
    Status confirm from HFU-2
1F0400 7F 0460
    Ack from phone
1F0400 DA 0004 0028 0000 7A97
    Type => 0xDA, data => 0x0028, 0x0000
1F0004 7F 7A1E
    Ack from HFU-2
1F0400 78 0004 0128 0E01 7B3A
    Status 0x0E, 0x01
1F0004 7F 7B1F
    Ack from HFU-2
1F0400 78 0004 0128 0A00 7C38
    Status 0x0A, 0x00
1F0004 78 0003 2801 03 054F
    Status confirm from HFU-2
1F0400 7F 0561
    Ack from phone
1F0400 78 0004 0128 0A00 7C38
    Status 0x0A, 0x00
1F0004 7F 7C18
    Ack from HFU-2
1F0400 78 0004 0128 0700 7D34
    Status 0x07, 0x00
1F0004 7F 7D19
    Ack from HFU-2
1F0400 78 0004 0128 0E00 7E3E
    Status 0x0E, 0x00
1F0004 7F 7E1A
    Ack from HFU-2
1F0004 78 0003 2801 03 064C
    Status confirm from HFU-2
1F0400 7F 0662
    Ack from phone

Disconnected
~~~~~~~~~~~~

No response. Probably because phone has lost the profile “handsfree”.

Incoming SMS
~~~~~~~~~~~~

1F0400 78 0004 0128 0E01 0849
    Status 0x0E, 0x01
1F0400 78 0004 0128 0A00 094D
    Status 0x0A, 0x00
1F0400 78 0004 0128 0901 0A4C
    Status 0x09, 0x01

Phone connected to CARC91
+++++++++++++++++++++++++

Initiation
~~~~~~~~~~

1F0004 D0 0001 04 00CE
    Power up from HFU-2
1F0400 D0 0001 05 2DE2
    Power up from phone
1F0004 79 0005 0201 0164 00 0100
    Enable carkit mode from HFU-2
1F0400 7F 0165
    Ack from phone
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 0249
    HFU version from HFU-2
1F0400 7F 0266
    Ack from phone
1F0004 79 0005 0201 0164 00 0302
    Enable carkit mode from HFU-2
1F0400 7F 0367
    Ack from phone
1F0400 78 0004 0128 0E00 2E6E
    Status 0x0E, 0x00
1F0004 7F 2E4A
    Ack from HFU-2
1F0004 78 0003 2801 03 044E
    Status confirm from HFU-2
1F0400 7F 0460
    Ack from phone
1F0400 DA 0004 0028 0000 2FC2
    ?
1F0004 7F 2F4B
    Ack from HFU-2

Incoming call
~~~~~~~~~~~~~

1F0400 78 0004 0128 0701 3078
    Status 0x07, 0x01
1F0004 7F 3054
    Ack from HFU-2
1F0400 78 0004 0128 0701 3179
    Status 0x07, 0x01
1F0004 7F 3155
    Ack from HFU-2
1F0400 78 0004 0128 0E01 3273
    Status 0x0E, 0x01
1F0004 7F 3256
    Ack from HFU-2
1F0400 78 0004 0128 0A00 3377
    Status 0x0A, 0x00
1F0004 78 0003 2801 03 054F
    Status confirm from HFU-2
1F0400 7F 0561
    Ack from phone
1F0400 78 0004 0128 0A00 3377
    Status 0x0A, 0x00
1F0004 7F 33 57
    Ack from HFU-2
1F0400 78 0004 0128 0901 3472
    Status 0x09, 0x01
1F0004 7F 3450
    Ack from HFU-2

Connected
~~~~~~~~~

1F0400 78 0004 0128 0E01 3574
    Status 0x0E, 0x01
1F0004 7F 3551
    Ack from HFU-2
1F0400 78 0004 0128 0A01 3673
    Status 0x0A, 0x01
1F0004 78 0003 2801 03 064C
    Status confirm from HFU-2
1F0400 7F 0662
    Ack from phone
1F0400 78 0004 0128 0A01 3673
    Status 0x0A, 0x01
1F0004 7F 3652
    Ack from HFU-2
1F0400 78 0004 0128 0A00 3773
    Status 0x0A, 0x00
1F0004 7F 3753
    Ack from HFU-2
1F0400 78 0004 0128 0900 387F
    Status 0x09, 0x00
1F0004 7F 385C
    Ack from HFU-2
1F0400 78 0004 0128 0A01 397C
    Status 0x0A, 0x01
1F0004 7F 395D
    Ack from HFU-2
1F0400 78 0004 0128 0901 3A7C
    Status 0x09, 0x01
1F0004 7F 3A5E
    Ack from HFU-2

Initiation with connected phone
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1F0004 D0 0001 04 00CE
    Power up from HFU-2
1F0400 D0 0001 05 5996
    Power up from phone
1F0004 79 0005 0201 0164 00 0100
    Enable carkit mode from HFU-2
1F0400 7F 0165
    Ack from phone
1F0004 79 0012 0201 0206 0056 2030 372E 3030 0A48 4655 3200 0249
    HFU-2 Version
1F0400 7F 0266
    Ack from phone
1F0400 78 0004 0128 0E01 5A1B
    Status 0x0E, 0x01
1F0004 79 0005 0201 0164 00 0302
    Enable carkit mode from HFU-2
1F0400 7F 0367
    Ack from phone
1F0400 78 0004 0128 0E01 5A1B
    Status 0x0E, 0x01
1F0004 7F 5A3E
    Ack from HFU-2
1F0400 78 0004 0128 0A01 5B1E
    Status 0x0A, 0x01
1F0004 7F 5B3F
    Ack from HFU-2
1F0400 78 0004 0128 0901 5C1A
    Status 0x09, 0x01
1F0004 7F 5C38
    Ack from HFU-2
1F0400 78 0004 0128 0701 5D15
    Status 0x07, 0x01
1F0004 7F 5D39
    Ack from HFU-2
1F0004 78 0003 2801 0305 4F
    Status confirm from HFU-2
1F0400 7F 0561
    Ack from phone
1F0400 DA 0004 0028 0000 5EB3
    ?
1F0004 7F 5E3A
    Ack from HFU-2

Disconnected
~~~~~~~~~~~~

1F0400 78 0004 0128 0E01 3B7A
    Status 0x0E, 0x01
1F0004 7F 3B5F
    Ack from HFU-2
1F0400 78 0004 0128 0A00 3C78
    Status 0x0A, 0x00
1F0004 78 0003 2801 03 074D
    Status confirm from HFU-2
1F0400 7F 0763
    Ack from phone
1F0400 78 0004 0128 0A00 3C78
    Status 0x0A, 0x00
1F0004 7F 3C58
    Ack from HFU-2
1F0400 78 0004 0128 0700 3D74
    Status 0x07, 0x00
1F0004 7F 3D59
    Ack from HFU-2
1F0400 78 0004 0128 0E00 3E7E
    Status 0x0E, 0x00
1F0004 7F 3E5A
    Ack from HFU-2
1F0004 78 0003 2801 0308 42
    Status confirm from HFU-2
1F0400 7F 086C
    Ack from phone

Incoming SMS
~~~~~~~~~~~~

1F0400 78 0004 0128 0E01 6627
    Status 0x0E, 0x01
1F0004 7F 6602
    Ack from HFU-2
1F0004 78 0003 2801 03 064C
    Status confirm from HFU-2
1F0400 7F 0662
    Ack from phone
1F0400 78 0004 0128 0E00 6727
    Status 0x0E, 0x00
1F0004 7F 6703
    Ack from HFU-2
1F0004 78 0003 2801 03 074D
    Status confirm from HFU-2
1F0400 7F 0763
    Ack from phone

Button pushed
~~~~~~~~~~~~~

1F0400 78 0004 0128 0E01 0948
    Status 0x0E, 0x01
1F0004 7F 096D
    Ack from HFU-2
1F0004 78 0003 2801 03 064C
    Status confirm from HFU-2
1F0400 7F 0662
    Ack from phone
1F0400 78 0004 0128 0E00 0A4A
    Status 0x0E, 0x00
1F0004 7F 0A6E
    Ack from HFU-2
1F0004 78 0003 2801 03 074D
    Status confirm from HFU-2
1F0400 7F 0763
    Ack from phone

Result
------

Important things to consider when designing a program for Com.n.sense that is to work with 6310.

* 6310 sends out status 0x0E, 0x01 when speaker should be enabled
* HFU-2 version has to be sent in order for 6310 to switch to profile ”Handsfree”.
* Status 0x0A might say weather the phone is ringing or connected. Only 6310 send this status.
* Status confirm should be sent when status 0x0E is received.
