Contributing
============

We welcome contribution in any area, if you don't have developer skills, you
can always contribute to :doc:`localization` or just `donate us money`_. In case you are
interested in fixing some code, please read :doc:`../internal/index` to
understand structure of Gammu code. We also maintain list of
`wanted skills <http://wammu.eu/contribute/wanted/>`_ where you can find in
which areas we currently mostly lack manpower.

Sending patches
---------------

As we use `Git <http://git-scm.com/>`_ for development, the preferred way to
get patches is in form which can be directly applied to Git. So start with
cloning our Git repository::

    git clone git://github.com/gammu/gammu.git gammu

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

If for whatever reason you don't want to use Git, you can also manually create
patches using :program:`diff`. Also we can handle if you send us just the file
you have changed with reference where did you take it.

To manually create patch you can use following steps:

1. Copy source with Gammu, you start from, into ``gammu`` directory.
2. Copy source with Gammu, you want to modify, into ``work`` directory.
3. Make your changes in ``work`` directory.
4. Go into parent directory, where ``gammu`` and ``work`` directories are
   placed.
5. Call ``diff -rup -X .git gammu work > patchfile``.
6. Send ``patchfile`` to us (you can use bug tracker or mailing list).

.. _donate us money: http://wammu.eu/donate/
