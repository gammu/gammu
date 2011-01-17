Nokia protocols
===============

Document describing protocol used in Nokia phones.

The data provided is for information purposes only.
Some of the frames might be hazardous to your phone. Be careful!!!
We do not take any responsibility or liability for damages, etc.

Last update 23.06.2003

Assembled by
Balazs Nagy          <js@iksz.hu>
Alfred R. Nurnberger <arnu@flosys.com>
Hugh Blemings        <Hugh.Blemings@vsb.com.au>
Mike Bradley         <mike@trumpington.st>
Odinokov Serge       <serge@takas.lt>
Pavel Janik          <Pavel@Janik.cz>
Pawel Kot            <pkot@linuxnews.pl>
Marcin Wiacek        <Marcin@MWiacek.com>
Jens Bennfors        <jens.bennfors@ing.hj.se>
Michael Hund         <michael@drhund.de>
Jay Bertrand         <jay.bertrand@libertysurf.fr>
<arnu@venia.net>
Andrew Kozin
Pavel Machek         <pavel@ucw.cz>
Diego Betancor       <dbetancor@duocom.net>
... and other members of gnokii mailing list
and authors of some WWW pages.

.. note::

    this information isn't (and can't be) complete.  If you know anything
    about features not listed here or you noticed a bug in this list, please
    notify us via e-mail.  Thank you.

Frame format for MBUS version 1
-------------------------------

Request from Computer/Answer from Phone::

   { DestDEV, SrcDEV, FrameLength, MsgType, {block}, id, ChkSum }

       where DestDEV, SrcDEV:   0x00: phone
                                0xf8: PC (wakeup msg)
                                0xe4: PC (normal msg)
             FrameLength:       length of data frame. Maximal 0x78. Longer
                                frames are divided into smaller.
             MsgType:           see List
             {block}:           main frame
             id:                request identity number 1..n, incremented after
                                the request is accepted
             ChkSum:            XOR on frame's all numbers

Ack from Phone::

    { DestDEV, 0x00, FrameLength, MsgType, {block} , id, ChkSum }

       where DestDEV:           taken from original request packet
             FrameLength:       0x7f, when DestDEV = 0xe4
                                0x7e, when DestDEV = 0xf8
             MsgType:           see List. Present only, when DestDEV = 0xf8
             {block}:           main frame. Present only, when DestDEV = 0xf8
             id:                request identity number 1..?, corresponding
                                to the original request packet id
                                the request is accepted
             ChkSum:            XOR on frame's all numbers

Update: description above according to the
http://www.gadgets.demon.co.uk/nokia21xx/protocol.html.

Pavel Machek <pavel@ucw.cz> wrote:
  0x7e is actually registration acknowledge. Both have nothing to do
  with DestDEV, except that special device needs to be used for
  registration.

Ack from Computer::

    { 0x00, SrcDEV, 0x7f, id, ChkSum }

       where SrcDEV:            taken from response packet
             id:                request identity number 1..?, corresponding
                                to the response packet id
                                the request is accepted
             ChkSum:            XOR on frame's all numbers

Port settings:
     Speed 9600 bps, Bits 8, ParityOdd, Stop Bits 1, DTR and RTS logic 0

In the MBUS bus, the phone has only one connector for transmition and
reception.

Because of this characteristics of the phone connector, every time that the
PC writes into the phone it is writing as well into its own Rx. So every
time the PC sends info into the phone it finds that same information in its
own Rx buffers, like a mirror copy. This should be discarded.

The communications is made like an old cb radio, only one
talking at a time. Many transmission are made this way:

* <computer sends request>
* <phone sends ack>
* <phone sends response>
* <computer sends ack>

Some frames are sent from phone without asking for them

You have to implement collision protocol. IE. you should listen for
what you are transmitting, and if it does not come back, you have
collision.

You should wait for bus to be free for 3 miliseconds before normal
message, and for 2.5 miliseconds before acknowledge. You should wait
for acknowledge for 200 miliseconds, then retransmit.

Frame format for FBUS version 1
-------------------------------

All frames::

    { FrameID, FrameLength, MsgType, {block}, SeqNo, ChkSum }

         where FrameID:         0x01 Command frame from computer to Nokia
                                0x02 ??? - Data call frame from computer to Nokia - ???
                                0x03 Data call frame from Nokia to computer
                                0x04 Command frame from Nokia to computer
               FrameLength:     {block} + 2
               MsgType:         see List
               SeqNum:          Sequence number of command in case where direction is
                                from ME to computer, the sequence number is
                                counting from 0x30 to 0x37 and resetting back to 0x30.
                                When direction is from computer to ME,
                                sequence number counts from 0x08 to 0x0f and resets back to 0x08.
                                It may not be required to be this way.
                                Sequence numbers are used in acknowledging commands.
               ChkSum1:         CRC = 0;
                                for (i = 0; i < (2 + CMD_LEN); i++)
                                  CRC ^= frame[i];

Frame format for FBUS version 2/Direct IRDA
-------------------------------------------

All frames::

    { FrameID, DestDEV, SrcDEV, MsgType, 0x00, FrameLength, {block}, FramesToGo,
      SeqNo, PaddingByte?, ChkSum1, ChkSum2 }

         where FrameID:         0x1c: IR / FBUS
                                0x1e: Serial / FBUS
               DestDev, SrcDev: 0x00: mobile phone
                                0x0c: TE (FBUS) [eg. PC]
               MsgType:         see List
               FrameLength:     {block} + 2 (+ 1 if PaddingByte exists)
               FramesToGo:      0x01 means the last frame
               SeqNo:           [0xXY]
                                  X: 4: first block
                                     0: continuing block
                                     Y: sequence number
               PaddingByte:     0x00 if FrameLength would be an odd number
                                anyways it doesn't exists
               ChkSum1:         XOR on frame's odd numbers
               ChkSum2?:        XOR on frame's even numbers

Frame format for MBUS version 2
-------------------------------

Cable::

    { FrameID, DestDEV, SrcDEV, MsgType, FrameLengthLO, FrameLengthHI, {block},
      SeqNo, ChkSum }

         where FrameID:         0x1f: Serial / M2BUS
               DestDev, SrcDev: 0x00: mobile phone
                                0x1d: TE (M2BUS)
                                0x10: TE (M2BUS) (Service Software ?)
                                0x04: Carkit?
                                0x48: DLR3 cable?
                                0xF8: unknown target?
                                0xFF: global target?
               MsgType:         see List
               FrameLength:     {block}
               SeqNo:           sequence number
               ChkSum:          XOR on frame's all numbers

Please note that M2BUS has only one checksum: XOR on frame[FrameID..SeqNo]

Ack::

    { FrameID, DestDEV, SrcDEV, 0x7f, Id_SeqNo, ChkSum }
	
	 where Id_SeqNo:        Is the sequence number that you are
	                        acknowleging (from the other part).

Frame format for Infrared::

    { FrameID, DestDEV, SrcDEV, MsgType, FrameLengthLo, FrameLengthHi, {block}}

         where FrameID:         0x14
               DestDev, SrcDev: 0x00: mobile phone
                                0x0c: TE [eg. PC]
               MsgType:         see List
               FrameLength:     {block}

Frame format for Bluetooth::

    { FrameID, DestDEV, SrcDEV, MsgType, FrameLengthLo, FrameLengthHi, {block} }

         where FrameID:         0x19
               DestDev, SrcDev: 0x00: mobile phone
                                0x10: TE [eg. PC]
               MsgType:         see List
               FrameLength:     {block}

Frames list format::

    hex: Short description
       x msg desc                { ... }
	0xXX   -> one byte
	0xXXYY -> two bytes (== 0xXX, 0xYY)

         where hex:     message type
               x:       s=send (eg. to mobile), r=receive
               { ... }: data after 0x00, 0x01 header
               {+... }: raw data (without header)

Misc (about MBUS version 2)
---------------------------

0x4E commands
+++++++++++++

(sent from a 5160i TDMA / 6160i TDMA / 6185 CDMA or 7110 GSM
phone to the uC in the DLR-3 cable)

DLR-3 req:

1F 48 00 4E 00 02 01 XX SQ CS

  frame sent from the phone to the DLR-3 cable
  (after 15kOhm resistor detected betw. XMIC (3) and DGND (9).)
  DSR,DCD,CTS flow control data is coded into the 2nd databyte

  XX:

      * bit.0=/CTS
      * bit.1=/DCD
      * bit.2=CMD/DATA
      * bit.3=DSR
      * bit.4-7=0

0x78 / 0x79 commands
++++++++++++++++++++

(used by handsfree carkit) Works also on GSM phones (5110 / 6110 / etc)

These commands are used by the Nokia Carkits to switch the phone audio path to
XMiC and XEAR , turn the phone on/off according to the car ignition, and
control the PA loudspeaker amplifier in the carkit and the car radio mute
output which silences the car radio during a call

mute status tone:
  1F 04 00 78 00 04 01 02 0E 00 SQ CS
     status indication = disable carkit audio amplifier (no audio / no tone)
mute status tone:
  1F 04 00 78 00 04 01 02 0E 03 SQ CS
     status indication = enable carkit audio amplifier (audio / tone present)
mute status call:
  1F 04 00 78 00 04 01 02 07 00 SQ CS
     status indication = disable radio mute output (no call)
mute status call:
  1F 04 00 78 00 04 01 02 07 01 SQ CS
     status indication = enable radio mute output (call active)
enable ???:
  1F 04 00 78 00 04 01 02 08 01 SQ CS
      status indication = enable ???  sent to HFU-2 on power on
      byte 9 (07,08,0E) seems to be a pointer to a memory location,
      byte 10 is the data at this memeory location.
response from HFU:
  1F 00 04 78 00 03 02 01 03 SQ CS
     response message from HFU-2 (use unknown)
go HF and IGN on:
  1F 00 04 79 00 05 02 01 01 63 00 SQ CS
     enables carkit mode + turns phone on + req. mute status
go HF and IGN off:
  1F 00 04 79 00 05 02 01 01 61 00 SQ CS
     enables carkit mode + powers phone off (1 min delay) + req. mute status
ext. HS Offhk:
  1F 00 04 79 00 05 02 01 01 23 00 SQ CS
     enables carkit mode + external handset lifted (OFF-Hook)
ext. HS Onhk:
  1F 00 04 79 00 05 02 01 01 63 00 SQ CS
     enables carkit mode + external handset put back (ON-Hook)
     Ignition and Hook are coded into one byte

        * bit.0 = 0:on power on 1:when in operation
        * bit.1 = IGNITION STATUS
        * bit.2 = x  can be 1 or 0
        * bit.3 = 0
        * bit.4 = 0
        * bit.5 = 1
        * bit.6 = Hook (inverted)
        * bit.7 = 0
HFU-2 version:
  1F 00 04 79 00 12 02 01 02 06 00 56 20 30 36 2E 30 30 0A 48 46 55 32 00 SQ CS
for HFU-2:
  1F 04 00 DA 00 02 00 02 SQ CS
    function unknown - sent from Nokia phone to HFU-2mute output (call active )

0xD0 commands
+++++++++++++

init:
  1F 00 1D D0 00 01 04 SQ CS
    sent by the Service Software or HFU-2 on startup
init resp:
  1F 1D 00 D0 00 01 05 SQ CS
    response from phone to above frame
