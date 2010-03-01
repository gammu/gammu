Localization
============

Localization uses Gettext. You can set locales you want to use by
specifying LANG or LC_* environment variables (on Linux you usually
don't care about this, on Windows just export e.g. ``LANG=cs_CZ``).

If you want to improve existing translation, please visit
`translation server`_. For adding new one, you need to
contact `Michal Čihař`_ and then you will be able to edit it on
former mentioned URL.

You can also go ahead with traditional way of creating/updating po files
in locale/ folder and then sending updated ones to bug tracker.


.. _translation server: http://l10n.cihar.com/projects/gammu/
.. _Michal Čihař: mailto:michal@cihar.com
