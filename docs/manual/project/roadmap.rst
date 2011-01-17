Roadmap for Gammu
=================

1.30.0
------

I will try to work on change with locations handling here, but as it is quite
intrusive, it might not be ready for this release.

Future
------

There are some major issues which should be addressed in Gammu soon. One
problem is locations handling, because current scheme (using numbers) really
does not match majority of current phones and it should be converted to using
path based locations for messages, phonebook, calendar, etc.

The another major obstacle which is all around Gammu code is own
implementation of unicode (UCS-2-BE) strings. This code should be dropped and
wchar_t used instead.

Avoid heavy usage of gsmstate.h header and move the
``#ifdef...#define...#endif`` blocks to ``gammu-config.h``.

Drop multiple configurations handling in libGammu, it should provide just API
to read some section from Gammurc and possible fall-back logic should be in
application.
