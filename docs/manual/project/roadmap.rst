Roadmap for Gammu
=================

There are some major issues which should be addressed in Gammu soon. This list
is not sorted at all, but includes bad design decisions made in Gammu past
which we would like to fix.

Locations handling
------------------

One problem is locations handling, because current scheme (using numbers)
really does not match majority of current phones and it should be converted to
using path based locations for messages, phonebook, calendar, etc.

Unicode strings
---------------

The another major obstacle which is all around Gammu code is own
implementation of unicode (UCS-2-BE) strings. This code should be dropped and
wchar_t used instead.

Hardcoded length for strings
----------------------------

Most of the strings have hardcoded length limits. This limitation should be
removed and strings allocated on the fly.

Unsigned char mess
------------------

In many cases ``unsigned char`` is used without any reason.

Extendability of libGammu
-------------------------

Current way of adding protocol specific functionality from applications using
libGammu is broken. Actually only application using this is Gammu utility.
This option should be either completely removed or rewritten from scratch not
to be dependant on libGammu internals.

Built time configuration
------------------------

Avoid heavy usage of gsmstate.h header and move the
``#ifdef...#define...#endif`` blocks to ``gammu-config.h``. Or rather cleanup
them and have only single define for single compile time option.

Config file handling
--------------------

Drop multiple configurations handling in libGammu, it should provide just API
to read some section from Gammurc and possible fall-back logic should be in
application.

AT module
---------

There should be simpler way to generate AT command with proper escaping and
charset conversion of fields. Something like reverse ATGEN_ParseReply.
