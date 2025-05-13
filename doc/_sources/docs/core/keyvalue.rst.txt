\page Keyvalue Keyvalue

KeyValue
========

KeyValue is the name of the Madgines reflection system. It provides additional type information that can be used to enable embedded scripting languages or tools to interact with the engines internal state. The two core elements of this system are the ValueType class, which allows to store almost anything in a type aware structure, and MetaTables, which are used to describe the attributes of a class. Almost all parts of the engine are implementing their MetaTables. 

## ValueType

The strongly typed nature of C++ is a big selling point of the language, however it makes the design of a reflection system rather complicated. It presents the task to support all potential types in a fixed API. Scripting languages on the other hand use weakly typed variables, that can accept any value to store. To make C++ compatible with scripting languages and a generic reflection API KeyValue presents the ValueType class. It is a fully type aware structure that is still able to accept almost any C++ value you could come up with. This design reduces the necessary steps for embedding a new scripting language into the system to simply providing conversions from ValueType to the scripting type system and vice versa.

The ValueType consists of one `std::variant` that can store one of the following types:
- empty type (default value)
- fundamental types (`int`, `float`, ...)
- strings
- math types (`Vector3`, `Matrix3`, ...)
- functions
- enums
- associative/non-associative ranges of types in this list
- implementations of the ObjectInstance interface (main use is to access scripting types from C++)
- owning/non-owning instances of types with MetaTable

\note The size of a variant is determined by the size of it's biggest potential type. Even though ValueTypes are not recommended to be used to store values persistently it is still a type that is used a lot within the engine. Therefore, it is space optimised in the following way: All variant types are required to take at most the size of two pointers in memory. Adding in the memory necessary for storing the currently held index this makes the size of a ValueType 24 bytes on 64-bit systems. Types that exceed that limitation (e.g. `Matrix4`) are implemented using heap allocation. (see implementation)

## MetaTables

The ValueType is able to represent most primitive objects in C++. To complement that MetaTables provide a way to specify the primitives that a compound type is composed of. They are implemented as descriptors (see /ref Meta) and do not require any modification to the class itself. Whenever a ValueType is assigned a value that is not recognized as anything else, it will assume that the type comes with it's one MetaTable. If none is provided, a linker error will tell for which type a MetaTable is missing. The ValueType will store a pointer to the object alongside a pointer to it's MetaTable. That way the type of an object is always accessible without storing it within the object itself. The MetaTable itself stores some information about the type - like name or base class - and a list of all accessible properties of that type. Properties can be of any type that a ValueType can hold. MetaTables are defined in the `.cpp` file of a type by convetion. An example table could look like this:

```
METATABLE_BEGIN(My::Namespace::Unit)
MEMBER(Name)
READONLY_PROPERTY(Owner, owner)
PROPERTY(Task, getCurrentTask, setCurrentTask)
FUNCTION(takeDamage, damage)
METATABLE_END(My::Namespace::Unit)
```

MetaTables are defined within a `BEGIN`/`END` pair. The definition shall not be put inside of a namespace. The most commonly used entries of a MetaTable are shown in the example:
- `MEMBER(<name>)` adds the property `<name>` to the table. Reads and Writes are accessing the C++ variable `<name>`.
- `READONLY_PROPERTY(<name>, <getter>)` adds the read-only property `<name>` to the table. Reads are accessing the C++ identifier `<getter>`, which can be a member or a member function. If it is a member function, its return value is used as the value of the property on read.
- `PROPERTY(<name>, <getter>, <setter>)` adds the property `<name>` to the table. Reads behave the same as for `READONLY_PROPERTY`. Writes are accessing the C++ identifierr `<setter>`, which can be a member or a member function. If it is a member function, the value to be written is passed as the only argument to the function.
- `FUNCTION(<name>[, <arg-name>...])` adds the read-only property `<name>` to the table. Reads return a BoundApiFunction, which holds a pointer to the function `<name>` and a pointer to the object for which the property was accessed. The `<arg-name>` list is necessary for proper diagnostics.
