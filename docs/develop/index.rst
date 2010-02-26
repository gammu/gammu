Welcome to Gammu's documentation!
=================================

`Gammu library`_ and `python-gammu`_ provide you way to communicate with
your cell phone using standard API. This documentation explains both
Python and C version of the API.

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
    :maxdepth: 2

    source
    python/index
    c/index
    install

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. _python-gammu: http://wammu.eu/python-gammu/
.. _Gammu library: http://wammu.eu/libgammu/
