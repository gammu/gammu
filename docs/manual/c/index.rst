.. _libgammu:

libGammu
========

The libGammu library exposes all Gammu functionality for various phones in
standard API. It can be used to do anything with your phone, however for
easier tasks you might prefer to use Python and :doc:`../python/index`.

If you intend to use libGammu in your application, all you should need
is to ``#include <gammu.h>`` and then use Gammu functions. You can check
docs/examples/ for some small example applications. You don't need real phone
for testing, use :ref:`dummy-driver` instead.

.. warning::

   The C API documentation is still more complete in Doxygen format
   available on Gammu website. This still lacks some parts due to
   problems in Doxygen - Sphinx bridge.

.. toctree::
    :maxdepth: 3

    hints
    examples
    api
    porting
