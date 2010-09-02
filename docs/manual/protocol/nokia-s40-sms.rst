Nokia S40 filesystem SMS format
===============================

This text is work in progress and does not claim to be correct or accurate. It
is solely based on Gammu dumps received from users. Analysed by Michal Cihar
<michal@cihar.com>.

File structure
--------------

- 176 bytes header
    - at offset 7 is length of PDU data
    - at offset 94 is stored remote number in unicode
    - rest is not known
- PDU data (without SMSC)
    - here can be sometimes also some failure block, which is not known yet
- structured data header: 0x01 0x00 <LEN>, where <LEN> is length of rest
- structured blocks:

Block: <TYPE = byte> <LENGTH = word> <DATA ...>

Blocks
------

0x01
  Unknown \x00 / \x01 (maybe received / sent)
0x02
  SMSC number, ASCII
0x03
  Text, unicode
0x04
  Sender, unicode
0x05
  Recipient, unicode
0x06
  Unknown \x00\x00\x00\x00
0x07
  Unknown \x00
0x08
  Unknown \x02 / \x00
0x09
  Unknown \x00\x00\x00\x00
0x0a
  Unknown \x00
0x0b
  Unknown \x00
0x0c
  Unknown, several values (maybe message reference per number)
0x0d
  Unknown \x00\x00
0x0e
  Unknown \x00\x00
0x0f
  Unknown \x00\x00


0x22
  Unknown \x00
0x23
  Unknown \x00\x00\x00\x00
0x24
  Unknown \x00
0x26
  Unknown \x00
0x27
  Unknown \x00
0x2a
  Unknown \x00
0x2b
  some text (Sender?), unicode


To test:
    - multiple recipients sms
