.. _gnapplet:

Gnapplet Protocol
=================

.. note:: 

    The original applet has been created for
    <http://www.gnokii.org/>. Gammu currently uses slightly
    extended version which will be hopefully merged back.

Installation
------------

To communicate with the phone, you need to install the applet. There are few
options how to do it:

Installation using Gammu
++++++++++++++++++++++++

Gammu can transmit the applet to your phone automatically. Just configure it
for use of gnapplet connection and invoke :option:`gammu install`:

.. code-block:: sh

   gammu install

It will automatically transmit the applet to the phone. On some phones the
installation will start automatically, on some you need to find received files
in the inbox and install them manually from there.

Downloading from phone
++++++++++++++++++++++

Downloading files from the phone and installing them directly. You can
download all required files from http://dl.cihar.com/gammu/gnapplet/.

Manual Installation using Gammu
+++++++++++++++++++++++++++++++

If the above mentioned :option:`gammu install` does not work for you, for
example when you need to use different applet, you can still use Gammu to send
files to the phone using :option:`gammu sendfile`.

First you need to create ``~/.gammurc`` with following content:

.. code-block:: ini

    [gammu]
    connection = blueobex
    model = obexnone
    device = 5C:57:C8:XX:XX:XX # Address of the phone

And now you can send files to your phone:

.. code-block:: sh

    gammu sendfile gnapplet.sis
    gammu sendfile gnapplet.ini

Files should appear in inbox in your phone and you can install them from
there.
