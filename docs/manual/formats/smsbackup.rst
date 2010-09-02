SMS Backup Format
=================

The SMS backup format is text file encoded in current encoding of platform
where Gammu is running. The syntax is standard INI file, with both ``;`` and
``#`` used for comments.

The file consists of unlimited number of sections, each is for one message and
it's name is formatted like ``SMSBackup001``. The numbering of messages must
be consistent - Gammu stops to read the file on first skipped number.
