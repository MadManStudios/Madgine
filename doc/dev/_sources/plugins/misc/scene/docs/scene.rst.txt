Scene
=====

.. plugin:: Scene

The Scene plugin contains a comprehensive implementation of a Scene system, including a full Entity-Component-System (ECS). It is built upon the Base plugin and the SceneManager is implemented as a GlobalAPIComponent.

Conventions
~~~~~~~~~~~

The 3D world uses a left-handed coordinate system, with forward being z+, right being x+ and up being y+.


Behaviors
~~~~~~~~~

The Scene plugin integrates seamlessly with the Behavior system. Every Entity holds a lifetime, that is itself embedded in an asynchronous lifetime. Behaviors are the preferred way to assign
logic to entities for gameplay purposes. In special cases, where heavy calculations over a large number of Entity components is needed, Systems can be used instead. The plugin also makes
use and extends the Context feature within Behaviors. EntityContext and SceneContext parameters can be added to any Behavior argument list, defaulting to `= {}`. This way the function signature
effectively remains unchanged, but inside the function body, those parameters will have been filled in, providing access to the enclosing Entity and the SceneManager.


Locking
~~~~~~~

Since the current state of a Scene Manager can be accessed by multiple systems, it is important to make sure that no critical data races can occur. The implementation makes use of an orthogonal model.
There exists a single DataMutex, that is used to guard adding or removing entity components from their respective EntityComponentLists. Note that components may only be added during creation
and may only be removed during destruction of the Entity. Furthermore, EntityComponentLists are required to keep pointers to their elements stable on insert/remove of the container.
Transitively, this means that as long as the Entity is guaranteed to be still alive, accessing any of it's components is also safe.
Entities have their own mechanism to ensure safe access. createEntity() itself is embedding itself into the lifetime of
its enclosing SceneContainer. This is already taking care of ensuring the DataLock is held during creation/destruction. createEntity() does *not* require a held DataMutex. 
Behaviors of an Entity are guaranteed to finish before the lifetime of that Entity is allowed to finish. When accessing Entities from Behaviors, they always need to be passed as EntityPtr.
EntityPtr is a Binding and acts as weak pointer into the lifetime of said Entity. If accessed successfully, the lifetime of the Entity is extended at least until the access finished. If the access fails, 
it is up to the caller how to proceed. There exist two methods of accessing EntityPtrs (and Bindings in general):
- Calling access_binding(), which takes a lambda and extends the lifetime during synchronous execution of said lambda
- using co_await in Behavior-Coroutines. This binds the target entity to the current Behavior, as long as the object returned from the expression is kept alive. Whenever the coroutine is resumed,
  it tries to access and lock all currently bound entity lifetimes. If any of those fail, the Behavior is cancelled. This can be used to guarantee safe accesses into an Entity inside a Behavior
  without using any locks.

Data inside of EntityComponents is currently not protected against concurrent accesses. On most Desktop hardware there are general memory guarantees in play, that make this approach safe.
On some more specialised hardware this can become an issue, but in most cases it should still be ok. If issues are found regarding this, the respective fields could be made atomic and
accessed through a relaxed memory policy.