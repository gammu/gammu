.. python-gammu documentation master file, created by sphinx-quickstart on Tue Mar 10 18:14:17 2009.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to python-gammu's documentation!
========================================

python-gammu provides Python interface to `Gammu library`_. It provides access
to most of it's functionality mapped to object model.

A taste of python-gammu
-----------------------

Following code will connect to phone based on your Gammu configuration
(usually stored in :file:`~/.gammurc`) and gets network information from it::

    import gammu
    import sys

    # Create state machine object
    sm = gammu.StateMachine()

    # Read ~/.gammurc
    sm.ReadConfig()

    # Connect to phone
    sm.Init()

    # Reads network information from phone
    netinfo = sm.GetNetworkInfo()

    # Print information
    print 'Network name: %s' % netinfo['NetworkName']
    print 'Network code: %s' % netinfo['NetworkCode']
    print 'LAC: %s' % netinfo['LAC']
    print 'CID: %s' % netinfo['CID']


Contents
--------

.. toctree::
    :maxdepth: 3

    api
    install

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. _Gammu library: http://wammu.eu/libgammu/
