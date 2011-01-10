Coding Style
============

Please follow coding style when touching Gammu code. We know that there are
still some parts which really do not follow it and fixes to that are also
welcome.

The coding style is quite similar to what Linux kernel uses, the only major
differences are requested block braces and switch indentation.

1. Use indentation, tab is tab and is 8 chars wide.
2. Try to avoid long lines (though there is currently no hard limit on line
   length).
3. Braces are placed according to K&R:
    .. code-block:: c

        int function(int x)
        {
            body of function
        }

        do {
            body of do-loop
        } while (condition);

        if (x == y) {
            ..
        } else if (x > y) {
            ...
        } else {
            ...
        }
4. All blocks should have braces, even if the statements are one liners:
    .. code-block:: c

        if (a == 2) {
            foo();
        }
5. There should be no spaces after function names, but there should be space after do/while/if/... statements:
    .. code-block:: c

        while (TRUE) {
            do_something(work, FALSE);
        }
6. Each operand should have spaces around, no spaces after opening parenthesis or before closing parenthesis:
    .. code-block:: c

        if ((i + 1) == ((j + 2) / 5)) {
            return *bar;
        }
7. Generally all enums start from 1, not from 0. 0 is used for not set value.

You can use :program:`admin/Gindent` to adjust coding style of your file to
match our coding style.
