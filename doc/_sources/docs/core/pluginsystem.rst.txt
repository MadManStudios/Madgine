\page Pluginsystem Plugins & Unique Components



Plugins & Unique Components
===========================

Plugins are one of the core features that the whole framework is built around. Unique Components are a concept that is integrated with the classic notion of plugins to extend their functionality even more and making them easier to use.

## Plugins

Plugins are implemented traditionally using shared libraries. Changes in the plugin selection require a restart of the engine. The implementation contains a dependency tracker that is integrated with the CMake-System. Loading a plugin will automatically trigger a load of all dependency plugins that are linked to said plugin in CMake. Furthermore it is possible to group plugins in sections and mark them with any combination of `exclusive` and/or `atleastOne`. Those invariants will then be respected if other plugins of the same section are tried to be loaded. Looking up symbols of plugins by name is also supported but discouraged in favor of Unique Components (see below).

### Plugin Definition Files

When providing a `MADGINE_CONFIGURATION` folder (see /ref Build) a `plugins.ini` file can be put in that folder to determine which plugins should be enabled. If it is present, all dynamic plugin loading code will be excluded from the build and instead the provided configuration will be statically applied. Due to its static linking nature this option currently is only supported with the CMake option [`BUILD_SHARED_LIBS`][] being disabled. The easiest way to modify the plugin selection and any plugin configuration files is by using the \subpage PluginManagerManual Tool.

## Unique Components

Unique Components are a way to avoid using possibly mangled name lookup on shared libraries in a plugin-based system. They allow code to be registered with the system by just writing it **without editing any setup code** somewhere in the engine. When exporting a plugin definition file, the plugin manager also generates a source file that contains the registered components for that configuration, removing most of the overhead introduced by the system in a static build. 

### How it works

First of all a type of Unique Component has to be defined. This is done by defining a base class and registering this base class to the system. (e.g. the SceneManager registers SceneComponentBase to the system). By implementing a class derived from this base in a different plugin it will be registered to the Collector of the respective plugin. When the plugin is loaded, the derived class will be automatically added to a list if all classes inheriting the base class. This way you have at all times an up-to-date list of implementations of your Unique Component type. See \subpage FirstUniqueComponent for more details.

### How to use it

The library comes with two kinds of accessors to use the registered types: Containers and Selectors. Containers will contain one instance of each registered type, while Selectors allow to pick one of them.

### Disclaimer

The implementation of this feature is highly compiler and linker specific and relies on behavior not defined by the standard. If you encounter a bug or try to use it on a new platform, please [file a bug][] (following this [guidelines][]).

[Overview]: overview.md
[Modules]: modules.md
[file a bug]: https://github.com/MadManRises/Madgine/issues
[guidelines]: ../.github/ISSUE_TEMPLATE/bug_report.md

[`BUILD_SHARED_LIBS`]: https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html
