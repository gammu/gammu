Installation
============

Currently python-gammu is distributed together with Gammu_, so all you need
to get it is to build Gammu with Python support (it should be automatically
detected if you have development environment installed for Python).

Gammu uses CMake_ to generate build environment (for example Makefiles for
UNIX, Visual Studio projects, Eclipse projects, etc.) which you can later use
for building. You can use ``-DBUILD_PYTHON=/path/to/python`` to define path to
another Python interpreter to use than default one available in the system.

Alternatively you can use standard distutils, for which :file:`setup.py` is placed in
`python` subdirectory.

.. _Gammu: http://wammu.eu/gammu/
.. _CMake: http://www.cmake.org/
