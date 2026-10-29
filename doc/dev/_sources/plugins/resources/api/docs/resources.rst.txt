Resources
=========

.. plugin:: Resources


Resource-Types
~~~~~~~~~~~~~~

ResourceLoader
--------------

ResourceLoaders are the main facility that the Madgine uses to manage resource loading and storage. ResourceLoaders register file extensions and automatically discover any assets that match.
All loading is handled within the IO-Thread (unless explicitly overriden). Additional to discovered file Assets, manual assets may be registered through code.

Loaders provide two types to manage Resource references.

* Handle: Handles provide refcounted access to a named resources data. As long as a Handle is held, the pointed to data will remain valid. When a Handle is reset, it is possible an
unload will be triggered.
* Ptr: Ptrs act as a unique_ptr to unnamed resources. They are mostly used for procedurally generated resources. Ptrs themselves cannot be shared. Instead the user is expected to store the
Ptr somewhere and provide direct access to the underlying data. Loading will still be done on the IO-Thread. As soon as the Ptr is destroyed the resource will be unloaded. It can not be
loaded again.

InstanceLoader
--------------

InstanceLoaders do not manage any data themselves. Instead they provide a convenient way to discover resource files and parse them in the IO-Thread. When loading, the user needs to provide
a reference to an object of the Data type. It is the users responsibility to ensure the reference remains valid until loading is done. A typical usecase for InstanceLoaders is Scene loading.
