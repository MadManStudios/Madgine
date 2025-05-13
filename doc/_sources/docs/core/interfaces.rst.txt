\page Interfaces Interfaces

Interfaces
==========

The engine is supported on multiple different platforms, which each come with multiple APIs that are accessed in multiple different places. To improve code quality all platform dependent APIs are abstracted in the Interfaces library. This way no platform dependent code has to be in any other library/plugin (except for rendering). The following abstractions are implemented:

- Shared library loading
- Filesystem access
- TCP/UDP sockets
- Logging
- OS window management
- Input management
- Debug utilities
- Thread naming

The library is using an extended version of the header/source file structure used all throughout the engine. All platform-dependent code is stored separately in source files of the form `<header-name>_<platform>.cpp` (e.g. `dlapi_unix.cpp`).