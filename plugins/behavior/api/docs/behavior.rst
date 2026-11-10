Behavior
=========

.. plugin:: Behavior


Lifetime Mangement
~~~~~~~~~~~~~~~~~~

The Behavior system is an asynchronous execution model that is supposed to complement the asynchronous task model of the engine. Behaviors are always cancelable and there is no guarantees on which thread the work executes. Even more, sub tasks within a behavior should always communicate their results in the current thread to prevent unnecessary context switches. This has the consequence that Behavior code can even be invoked on time sensitive threads like render or audio. Therefore, Behavior implementations are required not to perform immediate expensive work on call. This allows immediate propagation to the next actual piece of work to be executed on the current thread. Calculation heavy Behaviors need to ensure they get queued to an appropriate Thread themselves. This is an implementation detail for each Behavior implementation itself. This approach should work very well with the design of systems, e.g. Raycasts. It does make sense to batch Raycasts and evaluate them while traversing the Scene hierarchy anyways. 

*Note*: it may be necessary due to performance issues to enforce more strict rules on where Behavior code may execute, but it is best to not make any assumptions on where any part of the Behavior will be running at any point in time.

Another property of the sender based Behavior execution model is the fire-and-forget characteristic. When submitting a Behavior to a Lifetime, it immediately gets started and from that moment on might be canceled from any other thread. Therefore, most functions related to queueing Behaviors give no feedback on success or failure at all. Lifetimes are managed in call order, so if an objects Lifetime may be restarted, it is necessary to ensure correct thread-safe ordering of calls. Otherwise, Behaviors might end up on an unintended Lifetime.