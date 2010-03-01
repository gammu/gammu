Contributing
============

We welcome contribution in any area, if you don't have developer skills, you
can always contribute to :doc:`../source/localization`. In case you are
interested in fixing some code, please read :doc:`../source/index` to
understand structure of Gammu code. We also maintain list of 
`wanted skills <http://wammu.eu/contribute/wanted/>`_ where you can find in
which areas we currently mostly lack manpower.

Sending patches
---------------

As we use `Git <http://git-scm.com/>`_ for development, the preferred way to
get patches is in form which can be directly applied to Git. So start with
cloning our Git repository::

    git clone git://gitorious.org/gammu/mainline.git gammu

Once you have done that, do some fixes and commit them (see 
`Git tutorial <http://www.kernel.org/pub/software/scm/git/docs/gittutorial.html>`_
for information how to work with Git). Once you're satisfied with your
results, you can send the patches (all changes you've made so far) to us::

    git send-email --to=gammu-users@lists.sourceforge.net origin

Please note that `mailing list <https://lists.sourceforge.net/lists/listinfo/gammu-users>`_
requires you to subscribe before posting. This is anyway good idea in case you
want to contribute. However if you don't want to do that, just send the mails
directly to one of authors::

    git send-email --to=michal@cihar.com origin


Creating patches
----------------

I will describe, how to make patch using diff. If you can't make it,
simply send me info about changes in plain text email or attached ASCII
file.

1. copy source with Gammu, you start from, into "gammu" dir
2. copy source with Gammu, you want to modify, into "work" dir
3. make changes in "work"
4. go into subdirectory with "gammu" and "work"
5. make "diff -urx CVS gammu work > patchfile"
6. send "patchfile" to me ;-)
