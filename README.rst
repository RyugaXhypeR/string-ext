String-Ext
==========

A wrapper around c-strings to provide high level string functions.


Installation
------------

.. code-block:: bash

    git clone https://github.com/RyugaXhypeR/string
    cd string
    make install

Usage
-----

Include the header file ``string_ext.h`` after installation.

While compiling, pass in the ``/usr/local/lib/string_ext.a`` shared library file like so::
    
    cc <input-file.c> /usr/local/lib/string_ext.a
