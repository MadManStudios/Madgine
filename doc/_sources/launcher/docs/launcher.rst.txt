Launcher
========


Core Plugins
~~~~~~~~~~~~

The engine comes with a set of plugins that build the base architecture of the Madgine. Each plugin makes use of the UniqueComponent and defines at least one UniqueComponent type that can be used to extend the respective feature set. The launcher project creates all the top-level objects of each plugin, which in turn instantiate their UniqueComponentContainers. This also means that using the launcher will result in having dependencies to all base plugins. The engine uses the Model-View-Controller pattern, where every component is represented by a different plugin (see figure).

.. graphviz::
   digraph example {
      Base [ label="Base\n(Model)"];
      Client [ label="Client\n(View)"];
      UI [ label="UI\n(Controller)"];
      UI -> Base;
      UI -> Client;
   }

The responsibilities of the plugins are as follows:
- \subpage Base : Contains the Application class which holds a list of GlobalAPIComponents, which are all implementations of globally accessible features of the engine. (e.g. SceneManager)
- \subpage Client : Contains the rendering interface and the MainWindow, which also holds a list of MainWindowComponents, which are all implementations of additional input/render layers to the engine. (e.g. ImGui)
- \subpage UI : It contains a few utilities to connect events from Base and Client with each other. It induces the direct dependency to the Widgets plugin. The UIManager holds two lists of different handler component types. A GuiHandler is mostly used on top-level-widgets to be able to show/hide the widget and access all its children. A GameHandler is usually used on SceneWindows and it contains additional code for game interaction, like dragging, frame updates, etc.
- \subpage Root : TODO
