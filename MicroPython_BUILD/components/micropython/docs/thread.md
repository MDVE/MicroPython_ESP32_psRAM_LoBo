# **_thread** Module

Python functions can be run in thread. It means the function execution runs in ESP32 task separate from the main MicroPython thread and apears as running in *background*.

Maximum number of threads that can be started is defined via **menuconfig**. As each thread needs some memory (for stack), creating large amount of threads can couse *stack overflow*.

Thread function is usualy defined as infinite loop.

Threads can be **created**, **paused**, **resumed** and **stopped**.

Threads can comunicate with each other or the main thread (repl) using **notifications** and **messages**.


| Method  | Notes |
| - | - |
| _thread.start_new_thread("th_name", th_func, args[, kwargs]) | Start a new thread and return its identifier. The thread executes the function with the argument list args (which must be a tuple). The optional kwargs argument specifies a dictionary of keyword arguments. |
| _thread.stack_size([size]) | Return the thread stack size (in bytes) used when creating new threads. The optional size argument specifies the stack size to be used for subsequently created threads. The maximum stack size used by the thread can be checked with *_thread.list()* |
| _thread.allowsuspend(allow) | The thread can be suspended if *allow* is True. When created, thread does not allow suspension until explicitly set by this function. **The method must be called from the thread function**. |
| _thread.suspend(th_id) | Suspend the execution of the thread function. |
| _thread.resume(th_id) | Resume the execution of the thread function previously suspended with *_thread.suspend()*. |
| _thread.kill(th_id) | Terminate the thread, free all allocated memory. |
| _thread.getThreadName(th_id) | Get the name of the thread whose id is *th_id*. |
| _thread.getSelfName() | Get the name of the thread executing this method. |
| _thread.getReplID() | Get the thread id of the main (repl) thread. Can be uset to send notifications/messages to the main thread. |
| _thread.getnotification() | Check if any notification was sent to the thread executing this method. Returns integer >0 if there was pending notification or 0 if not |
| _thread.getmsg() | Check if any message was sent to the thread executing this method. Returns 3-items tuple containing message type, sender thread id and message itself (integer or string). |
| _thread.notify(th_id, value) | Send notification to the thread with id *th_id*. *value* is integer >0. |
| _thread.sendmsg(th_id, msg) | Send message to the thread with id *th_id*. *msg* can be integer or string. |
| _thread.replAcceptMsg([flag]) | Return True if the main thread (repl) is allowed to accept messages. If executed from the main thread, optional *flag* (True|False) argument can be given to allow/dissallow accepting messages in the main thread |
| _thread.list([print]) | Print the status of all created threads. If the optional *print* argument is set to *False*, returns the tuple with created threads information. Thread info tuple has *th_id*, *type*, *name*, *suspended state*, *stack size* and *max stack used* items |

