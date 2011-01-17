Project Documentation
=====================

The documentation for Gammu consists of two major parts - The Gammu Manual,
which you are currently reading and comments in the sources, which are partly
included in this manual as well.

The Gammu Manual
----------------

This manual is in written in rst format and built using
`Sphinx <http://sphinx.pocoo.org/>`_ with
`breathe <https://github.com/michaeljones/breathe>`_ extension. As Gammu uses
latest version of this extension, it is shipped within Gammu sources, but if
you use git, you need to check out it separately as git submodule (you should
update it as well any time you update Gammu git):

.. code-block:: sh

    git submodule update --init

To generate the documentation there are various ``manual-*`` targets for make.
You can build HTML, PDF, PS, HTMLHELP and Latex versions of it:

.. code-block:: sh

    # Generates HTML version of manual in docs/manual/html
    make manual-html

    # Generates PS version of manual in docs/manual/latex/gammu.ps
    make manual-ps

    # Generates PDF version of manual in docs/manual/latex/gammu.pdf
    make manual-pdf

    # Generates HTML version of manual in docs/manual/htmlhelp
    make manual-htmlhelp

    # Generates HTML version of manual in docs/manual/latex
    make manual-latex

Man pages
---------

The man pages for all commands are generated using Sphinx as well:

.. code-block:: sh

    # Generates HTML version of manual in docs/manual/man
    make manual-man

However man pages are stored in Git as well, so you should update generated
copy on each change:

.. code-block:: sh

    # Updates generated man pages in Git
    make update-man

Code comments
-------------

The code comments in C code should be parseable by Doxygen, what is more or
less standard way to document C code.
