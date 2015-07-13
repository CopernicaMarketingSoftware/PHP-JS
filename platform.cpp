/**
 *  platform.cpp
 *
 *  A "platform" as needed by the v8 engine, implementing
 *  functionality required for, amongst other things,
 *  garbage collection.
 *
 *  @copyright 2015 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "platform.h"
#include <time.h>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Constructor
 */
Platform::Platform() :
    _running(true),
    _worker(&Platform::run, this)
{}

/**
 *  Destructor
 */
Platform::~Platform()
{
    // we are no longer running
    _running = false;

    // signal the thread in case it is waiting for input
    _condition.notify_one();

    // and wait for the thread to stop
    _worker.join();
}

/**
 *  Execute some work, this is called
 *  by the worker thread to keep going
 */
void Platform::run()
{
    // keep going indefinitely
    while (true)
    {
        // lock the work queue
        std::unique_lock<std::mutex> lock(_mutex);

        // wait for new work to arrive
        while (_tasks.empty() && _running) _condition.wait(lock);

        // still no tasks? we must be done
        if (!_running) return;

        // retrieve the new task
        auto *task = _tasks.front();

        // remove it from the queue
        _tasks.pop_front();

        // unlock the queue again
        lock.unlock();

        // execute the task
        task->Run();

        // and clean it up
        delete task;
    }
}

/**
 *  Schedule a task to be executed on a background thread.
 *
 *  The platform takes responsibility for the task, and will
 *  free it when the task has finished executing.
 *
 *  @param  task    The task to execute
 *  @param  time    The expected run time
 */
void Platform::CallOnBackgroundThread(v8::Task *task, ExpectedRuntime time)
{
    // lock the mutex
    _mutex.lock();

    // add the task to the list
    _tasks.push_back(task);

    // unlock the mutex again
    _mutex.unlock();

    // signal the worker that work is ready
    _condition.notify_one();
}

/**
 *  Schedule a task to be executed on a foreground thread.
 *
 *  @param  isolate The isolate the task belongs to
 *  @param  task    The task to execute
 */
void Platform::CallOnForegroundThread(v8::Isolate *isolate, v8::Task *task)
{
    /**
     *  We are _required_ to implement this function, as it is a pure-virtual
     *  in the v8::Platform class. It is, however, never called, unless one
     *  calls the PumpMessageLoop method, in which case it would not work
     *  either, since that function blindly casts it to a DefaultPlatform
     *  object and executes the PumpMessageLoop on this object.
     *
     *  Besides the fact that the cast would fail, and the function call
     *  thus ends in a segfault, there is no way to fix this even without
     *  this weird casting, since the PumpMessageLoop is not a virtual
     *  function.
     *
     *  Because of this I hereby declare this function to be utterly
     *  useless. It can never be called, but we can't compile without.
     */
}

/**
 *  Retrieve the monotonically increasing time. The starting point
 *  is not relevant, but it must return at least millisecond-precision
 *
 *  @return time in seconds since an unspecified time
 */
double Platform::MonotonicallyIncreasingTime()
{
    // the result structure
    struct timespec time;

    // retrieve the current time
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);

    // convert the result to a double with millisecond precision
    return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_nsec) / 1000000000;
}

/**
 *  End namespace
 */
}
