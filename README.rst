String-Ext
==========

.. image:: https://www.github.com/RyugaXhypeR/string-ext/actions/workflows/c-cpp.yml/badge.svg
   :alt: Build Status
   :target: https://github.com/RyugaXhypeR/string-ext/actions

A wrapper around c-strings to provide high level string functions.


Installation
------------

Install the package from latest releases or clone the repository and cd into its directory.

On Unix, Linux and macOS:

.. code-block:: bash

   autoreconf --install
   ./configure
   make
   make install


Usage
-----

After installation, include the header file ``string_ext.h`` to use it.

Also, link the library file ``libstringext.a`` while compiling

.. code-block:: bash

   cc <input-file.c> -l:libstringext.a
