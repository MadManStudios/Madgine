Build Instructions
==================

This article describes all the neccessary steps to compile your first application with the Madgine.


## Dependencies

The Madgine is built using CMake. It can be downloaded [here](https://cmake.org/download/). 
All source and library dependencies are included in the repository itself.
A compiler supporting C++20 is necessary.


## Setup The Madgine

Download the source code from [GitHub](https://github.com/MadManRises/Madgine).

There are two ways to use and compile the engine. 

### Setup Build From The Repository

The first one being to build the project using CMake from the top-level folder of the engine. This is useful if you intend to modify the engine code itself. No further CMake scripts are needed. 

### Setup Build From An External Project

The second method is to create a separate CMake project and reference the engine code from there. This allows you to separate your own work from the engine code, but keeps the benefit of building the engine source code together with your game/plugin code, allowing for better whole program optimization. All the necessary utility scripts for this method are provided. To setup the engine this way, create your own CMakeLists.txt for your game/plugin and add the following:
```cmake
include(<root folder of local madgine repository>/cmake/MadgineSDK.cmake)
```
To create the executable add the following line to the end of your CmakeLists.txt:
```cmake
create_launcher(<name>)
```

## Generating Project Files

Generate the build files using CMake for your preferred compiler/IDE or open the folder in IDEs that have direct CMake integration. 

## Build Configurations

The default build settings lead to a project with the following properties:
- Tools are included
- Plugins are built as dynamic libraries
- Plugin selection can be changed at any time

Once a project needs to be destributed it is likely that some of those properties need to be changed. To do so the Madgine provides a configuration system. Build configurations are folders, that contain all configuration files for a specific build type. The type of files is not restricted and some plugins define their own rules and configuration options. The most common configuration file types are:
- `.ini` files will be converted into CMake variables `<filename without extension>_<ini group>_<ini field name>`
- `.cmake` files will be executed at CMake startup
- `.list` files contain a list of paths to files that will be included in the packaged build

To provide a configuration to CMake, set `MADGINE_CONFIGURATION` to the path of the desired configuration. Most plugins provide a built-in tool that allows to modify configurations through a UI.


\addtogroup Howtos
\ref Build

