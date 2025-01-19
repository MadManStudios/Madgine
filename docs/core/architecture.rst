\page Architecture Architecture Overview


Architecture Overview
==================

Folder hierarchy
~~~~~~~~~~~~~~~~~

- cmake: cmake utility scripts
- docs: documentation (doxygen)
- launcher: executable/entry-point setup for all supported platforms
- plugins: set of plugins grouped by functionality
- samples: small working examples of using the framework
- shared: middleware & feature libraries (note: not plugins)
- src: core libraries
- test: tests for core libraries


Core Libraries
~~~~~~~~~~~~~~~

The framework is strongly plugin-oriented. It contains a set of core libraries that provide the necessary generic functionality needed in a working Madgine-Application:
- \subpage Generic : header-only library. Mostly template code, including container traits
- \subpage Interfaces : hardware/platform abstractions
- \subpage Meta : reflection and serialization/synchronization library
- \subpage Modules : threading and plugin functionality

.. toctree::
   generic
   meta
   modules
   interfaces


This keeps the minimal footprint of the framework low and allows customizations to as much parts of the code as possible.

![libraries](img/libraries.svg)


Source folder names & namespaces
~~~~~~~~~~~~~~~~~~~~~~~~

Within the source directory of each project the subfolder hierarchy mirrors the namespace hierarchy used in code. This is purposefully orthogonal to the separation of libraries, meaning that a library can define multiple symbols in different namespaces, while a single namespace can contain symbols from different libraries/plugins. For example, the Interfaces define the OSWindow within the Window namespace; the Client plugin adds a more high-level implementation of a MainWindow to the same namespace.

.. toctree::
   /launcher/docs/launcher