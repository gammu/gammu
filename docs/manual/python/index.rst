.. _python:

python-gammu API
================

A taste of python-gammu
-----------------------

Python-gammu allows you to easily access the phone. Following code will connect
to phone based on your Gammu configuration (usually stored in
:file:`~/.gammurc`) and gets network information from it::

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

.. toctree::
    :maxdepth: 3

    examples

API documentation
-----------------

.. toctree::
    :maxdepth: 3

    gammu
    smsd
    data
    worker
    exceptions
    objects

