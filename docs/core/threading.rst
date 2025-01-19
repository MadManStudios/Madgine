\page Threading Threading

Threads & Asynchronous Execution
================================

The Madgine comes with its own full fletched Threading model. It introduces TaskQueues, which can be used to split your tasks into semantic groups. A scheduler will then assign threads to TaskQueues. In addition to TaskQueues the system adds so called Workgroups. A Workgroup builds a group of TaskQueues and Threads. While the communication of the threads within one Workgroup is not restricted, the communication between threads of multiple Workgroups should be minimized. The Figure illustrates the semantics of all related objects.

![workgroups](img/workgroups.svg)

## Threads

Threads are managed by Workgroups and created using `std::async`. Workgroup itself does not implement thread pools, which can be implemented in custom schedulers. 

## Tasks

Tasks in the Madgine are implemented using C++20 coroutines. After the task was created it has to be assigned to a TaskQueue, which returns a TaskHandle. A TaskHandle is a responsibility object. Deleting a TaskHandle that is holding a coroutine is an error, which also means that tasks cannot be aborted mid execution. The only way to reset a TaskHandle is to call it, which will resume the coroutine up to its next suspension point. At this point the responsibility to continue the task is moved into the coroutine and has to be passed on to the next Awaiter object. To ensure that a coroutine is not leaked it is recommended to implement all variants of `await_suspend` as `await_suspend(Engine::Threading::TaskHandle)`. That way the responsibility to continue the task is always carried on in a safe way.

## TaskQueues

TaskQueues provide many different ways to configure task execution. TaskHandles can be queued in different ways:
- direct queueing puts the task into a FIFO queue that will be accessed with highest priority on the next fetch
- timed queueing puts the task into the same FIFO queue, but the task will be skipped on fetch until the specified time point has passed
- repeated tasks can be registered on an interval and are returned by fetch if nothing can be fetched from the queue and the interval has expired.

Additionally, setup/shutdown steps can be specified. Those are executed before any other tasks or when the TaskQueue has been flagged to shutdown.

## Workgroups

Workgroups form a semantic group of TaksQueues and Threads. With the standard launcher you will only have one single Workgroup. Workgroups only become important in Server applications, which might need to simulate multiple game instances at the same time. Workgroups can be used to separate those instances, which do not need to communicate with each other. Workgroups also provide a storage similar to thread-local storage. Workgroup-local variables are an alternative to global variables, when multiple Workgroups need to see different values of a variable. In the default setting with just one Workgroup a workgroup-local variable is equivalent to a global variable.

## Scheduler

The last statement in the entry point of a new Workgroup (which is the `main` function for the first Workgroup) should be `return <scheduler-type>{}.run();`. The scheduler will take over the thread it was called from and spawn additional threads as necessary. It will return an integer value, where 0 indicates proper execution and any non-zero value indicates an error. This way the value can be directly returned from `main`. The default scheduler allocates one thread for every TaskQueue.