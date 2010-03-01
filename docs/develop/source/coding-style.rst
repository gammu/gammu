Coding Style
============

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
