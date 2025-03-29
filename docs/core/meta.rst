\page Meta Meta

Meta Information
================

The Madgine contains many features that need to know all properties of the type they are working on given only the base type (e.g. Script embedding, Serialization). There exist multiple techniques to implement that in C++:
* Use virtual dispatch
* Use templates
* Use type descriptors

Virtual dispatch always increases the object size in memory. Depending on the size and count of object instances this cost can be significant, also it might take away the ability to plainly copy the data of a POD-type for Serialization. Templates on the other hand are very compile- & link-time expensive. Furthermore, it forces every implementation to be included in all user code, making this solution also unfeasible.

This leaves type descriptors as the only acceptable solution. In the framework they are implemented as constant blocks of data that contain all information. Those blocks are defined - mostly using macros - at the beginning of the corresponding source file of an object. This allows generic code to work properly given only a pointer to that description. Due to its nature this pointer can also be used as a unique identifier of a type.

@remark It can be confusing to find code describing a type outside of its class definition. Please see the practice to add this kind of information at the beginning of the source file outside of any namespaces as a coding guideline.