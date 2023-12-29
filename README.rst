String-Ext
==========

.. image:: https://www.github.com/RyugaXhypeR/string-ext/actions/workflows/c-cpp.yml/badge.svg
   :alt: Build Status
   :target: https://github.com/RyugaXhypeR/string-ext/actions

A wrapper around c-strings to provide high level string functions.


Installation
------------

On Unix, Linux and macOS:

.. code-block:: bash

   autoreconf --install
   ./configure
   make
   make install


Usage
-----

After installation, include the header file ``string_ext.h`` to use it.
Also, link the library file ``/usr/local/lib/libstringext.a`` while compiling

.. code-block:: bash

   cc <input-file.c> /usr/local/lib/libstringext.a
