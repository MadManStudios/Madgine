Resources
=========

.. plugin:: Resources


Resource-Types
~~~~~~~~~~~~~~

ResourceLoader
--------------

ResourceLoaders are the main facility that the Madgine uses to manage resource loading and storage. ResourceLoaders register file extensions and automatically discover any assets that match.
All loading is handled within the IO-Thread (unless explicitly overriden). Additional to discovered file Assets, manual assets may be registered through code.

Loaders provide a Handle type that is used to manage Resource references. Handles provide refcounted access to loaded resources data. As long as a Handle is held, the pointed to data will
remain valid. When a Handle is reset, it is possible an unload will be triggered. There exist two types of data loading, named and unnamed. When named resource is happening, the loader
first checks for any existing loaded data with that name. If found, it will just return a reference to the existing data. If not, new data will be allocated and loaded and stored under the
given name. Those named caches are immutable and immortal. If temporary data is desired (e.g. for procedurally generated content), unnamed loading can be used. It circumvents any caching.
Therefore, if such Handles should be shared, it has to be done manually.

InstanceLoader
--------------

InstanceLoaders do not manage any data themselves. Instead they provide a convenient way to discover resource files and parse them in the IO-Thread. When loading, the user needs to provide
a reference to an object of the Data type. It is the users responsibility to ensure the reference remains valid until loading is done. A typical usecase for InstanceLoaders is Scene loading.
