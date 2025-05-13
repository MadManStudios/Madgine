\page Logging Logging

Logging
=======

Logging is an integral tool for development and debugging.

## Message Types

Logging messages are split into five severity groups:
- Debugging `LOG_DEBUG`: Very detailed information (can cause performance decrease, when enabled)
- Info `LOG`: General information/notifications
- Warning `LOG_WARNING`: Potential issues
- Error `LOG_ERROR`: Something went wrong
- Fatal `LOG_FATAL`: The Engine is unable to recover from this error, often followed immediately by a crash

## Building Log Messages

To log an event from C++ use one of the Logging macros, depending on the severity of the event. The macro takes a sequence of message elements combined with the `<<` operator as argument, e.g. 
```
LOG_WARNING("The value of my_value is " << my_value);
```
All fundamental types can be logged this way. If you want to be able to log custom types this way, a freestanding `operator<<`, that takes an `std::ostream&` as first parameter and your type as second parameter, needs to be provided. (See TODO)

## Default Log

By default the engine sets up a standard log on startup. It posts all messages to `stdout`. It is possible to globally overwrite the target log to direct the messages to a different place. Alternatively, individual messages can be written to a custom log backend instead. This can help to group log messages.

## Log Listener

Additionally, log listeners can be attached to logs to be notified of any messages. The \subpage LogViewer is one of those listeners. It stores all received messages and provides a filterable and sortable graphical representation of all messages within the Engine.
