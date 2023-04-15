# TaskSchedulerRevamped
A scheduler written in c++ for educational purposes only.
Tasks may be scheduled once or as a repeated action.
Upon start, the tasks are executed in by a thread, which
checks for the next task using conditional variables/observing
the taskqueue (priority_queue). If a task is determined to
be ready, it is executed in a separate thread.
