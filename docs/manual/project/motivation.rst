Motivation to fork Gnokii
=========================

.. note::

    Please note that this is original list of differences written by
    Marcin when forking Gnokii, so it represents state of the code in
    that time.

1. Unicode used almost everywhere. In MyGnokii and Gnokii with modern
    phones (they return everything in Unicode) things are converted from
    Unicode and again to Unicode in other places. No more unnecessary
    conversions.
2. Almost everything is structural. In Gnokii some things are declared
    in files, not in "main" phone structure. It can make some problems, when
    will try to support two phones on two serial ports in one application.
3. in Gammu you can make support for some things without adding source
    to "main" phone modules. Very good idea for things, which are available
    only for few models and for all other will be UNIMPLEMENTED. It includes
    also some obsolete functions - why should we compile RLP source, when
    all new better phones have modems built in ?
4. Gnokii/MyGnokii has to have some compatibility with previously written source. In Gammu some solutions are reimplemented and done easier.
5. no more reimplementing C libraries in source - see snprintf in gnokii.
6. more OS supported.
7. better sharing source. Less source = smaller application easier to debug.
8. better user friendly interface
9. no more 2 years rewriting source...
10. it's easier to see, what frames are implemented, what not (in phone modules they're put line after line).
11. better compatiblity with ANSI C = no warnings in MS VC 6
12. all locations for user start from 0 (in Gnokii some from 0, some from 1)
13. some things like SMS can be accessed few ways
14. when possible, there are used "constant" locations. I will explain
    on the example:

    1. save two calendar notes in any Nokia 61xx phone. Call them "reminder" and "call" notes. Reminder will be returned by phone of 1'st location, Call on 2'nd.
    2. Now Reminder will be deleted (for example, from phone keypad).  Call will be moved from 2'nd to 1'st.
    3. When will read calendar notes again, have to read all notes again
       because of changed locations (let's say, we won't read Call note
       again. It will have location 2 in PC. Now you will write new note
       into phone (for keypad) and it will save in under location 2. When
       will try to save Call not with location 2, it will overwrite new
       saved note !).

    This is not good. When for example delete one entry from phonebook,
    other locations "stays" on their places. These are "constant" locations.

    With "constans" locations, when delete one location from PC, don't have
    to read full memory from phone.

etc. etc.

Of course, some of these things can be in the future in gnokii too...
