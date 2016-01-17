Localization
============

Localization uses `Gettext`_ po files for both program translations and the
documentation. The documentation translation is managed using `po4a`_.

Using Translation
-----------------

You can set locales you want to use by specifying LANG or LC_* environment
variables (on Linux you usually don't care about this, on Windows just export
e.g. ``LANG=cs_CZ``).

Improving Translation
---------------------

If you want to improve existing translation, please visit
`translation server`_. For adding new one, you need to
contact `Michal Čihař`_ and then you will be able to edit it on
former mentioned URL.

You can also go ahead with traditional way of creating/updating po files
in locale/ folder and then sending updated ones to bug tracker.

Translation Areas
-----------------

There are several po files to translate:

``libgammu.po``
    Messages used in the Gammu library (see :ref:`libgammu`).
``gammu.po``
    Messages used by command line utilities (mostly :ref:`gammu`).
``docs.po``
    Basic documentation shipped within package (eg. :file:`README.rst` and
    :file:`INSTALL` files).


.. _translation server: http://hosted.weblate.org/projects/gammu/
.. _Michal Čihař: mailto:michal@cihar.com
.. _Gettext: http://en.wikipedia.org/wiki/GNU_gettext\
.. _po4a: http://po4a.alioth.debian.org/
