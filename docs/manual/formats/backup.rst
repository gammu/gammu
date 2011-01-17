.. _gammu-backup:

Backup Format
=============

The backup format is text file encoded in either ASCII or UCS-2-BE encodings.

This file use ini file syntax, see :ref:`ini`.

Examples
--------

If you will backup settings to Gammu text file, it will be possible to edit
it. It's easy: many things in this file will be written double - once in Unicode,
once in ASCII. When you will remove Unicode version Gammu will use ASCII
on \fBrestore\fR (and you can easy edit ASCII text) and will convert it
according to your OS locale. When will be available Unicode version of text,
it will be used instead of ASCII (useful with Unicode phones - it isn't important,
what locale is set in computer and no conversion Unicode -> ASCII and ASCII ->
Unicode is done).

You can use any editor with regular expressions function to edit backup text
file. Examples of such editors can be `vim <http://www.vim.org/>`_ or
`TextPad <http://www.textpad.com/>`_ which both do support regular expressions.

Remove info about voice tags

Find::

    ^Entry\([0-9][0-9]\)VoiceTag = \(.*\)\n

Replace::

    <blank>

Change all numbers starting from +3620, +3630, +3660, +3670 to +3620

Find::

    Type = NumberGeneral\nEntry\([0-9][0-9]\)Text = "\+36\(20\|30\|60\|70\)\n

Replace::

    Type = NumberMobile\nEntry\1Text = "\+3620

Change phone numbers type to mobile for numbers starting from +3620, +3630,... and removing the corresponding TextUnicode line

Find::

    Type = NumberGeneral\nEntry\([0-9][0-9]\)Text = "\+36\([2367]0\)\([^\"]*\)"\nEntry\([0-9][0-9]\)TextUnicode = \([^\n]*\)\n

Replace::

    Type = NumberMobile\nEntry\1Text = "\+36\2\3"\n

.. seealso:: :ref:`converting-formats`
