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
#include <v8.h>
#include "platform.h"
#include "isolate.h"
#include <atomic>
#include <time.h>
#include <mutex>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Mutex for safely creating the platform
 *  @var    std::mutex
 */
static std::mutex mutex;

/**
 *  The platform instance
 *  @var    std::atomic<Platform*>
 */
static std::atomic<Platform*> platform;

/**
 *  Include the dumps of the natives and snapshot blobs
 */
#include "natives_blob.h"
#include "snapshot_blob.h"

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
 *  Create a new platform if one does not exist yet.
 *
 *  This function is thread-safe.
 */
void Platform::create()
{
    // retrieve the current platform
    auto *result = platform.load(std::memory_order_relaxed);

    // create a memory fence for reading the variable
    std::atomic_thread_fence(std::memory_order_acquire);

    // is the platform initialized yet?
    if (result == nullptr)
    {
        // lock the platform
        std::lock_guard<std::mutex> lock(mutex);

        // check again whether the variable was initialized
        result = platform.load(std::memory_order_relaxed);

        // still not set, we need to create it now
        if (result == nullptr)
        {
            // create the platform
            result = new Platform;

            // initialize the ICU and v8 engine
            v8::V8::InitializeICU();

            // create a setup a StartupData object for the natives blob
            v8::StartupData natives;
            natives.data = (const char*) _tmp_natives_blob_bin;
            natives.raw_size = _tmp_natives_blob_bin_len;
            v8::V8::SetNativesDataBlob(&natives);

            // create a setup a StartupData object for the snapshot blob
            v8::StartupData snapshot;
            snapshot.data = (const char*) _tmp_snapshot_blob_bin;
            snapshot.raw_size = _tmp_snapshot_blob_bin_len;
            v8::V8::SetSnapshotDataBlob(&snapshot);

            // initialize the platform
            v8::V8::InitializePlatform(result);
            v8::V8::Initialize();

            // create a memory fence for storing the variable
            std::atomic_thread_fence(std::memory_order_release);

            // store the new platform
            platform.store(result, std::memory_order_relaxed);
        }
    }
}

/**
 *  Shutdown the platform
 *
 *  This function is thread-safe.
 */
void Platform::shutdown()
{
    /**
     *  You might wonder why we take care of neatly storing
     *  the nullptr in the platform as we are only calling
     *  this method from the destructor of the complete PHP
     *  engine and you would not expect to receive any more
     *  calls to come into the get() method.
     *
     *  When we are used from within apache2, a reload causes
     *  all PHP engines to be destructed, but we actually do
     *  stay loaded in memory. So we have to make sure that
     *  this variable is correctly set to a nullptr, so that
     *  the create() method creates a new instance.
     */

    // retrieve the current platform
    auto *result = platform.load(std::memory_order_relaxed);

    // create a memory fence for reading the variable
    std::atomic_thread_fence(std::memory_order_acquire);

    // is the platform initialized yet?
    if (result != nullptr)
    {
        // lock the platform
        std::lock_guard<std::mutex> lock(mutex);

        // check again whether the variable was initialized
        result = platform.load(std::memory_order_relaxed);

        // still not set, we need to create it now
        if (result != nullptr)
        {
            // create the platform
            delete result;

            // and shut down the engine (this also takes care of the ICU) and the platform
            v8::V8::Dispose();
            v8::V8::ShutdownPlatform();

            // create a memory fence for storing the variable
            std::atomic_thread_fence(std::memory_order_release);

            // store the new platform
            platform.store(nullptr, std::memory_order_relaxed);
        }
    }
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
 * Schedules a task to be invoked on a foreground thread wrt a specific
 * |isolate| after the given number of seconds |delay_in_seconds|.
 * Tasks posted for the same isolate should be execute in order of
 * scheduling. The definition of "foreground" is opaque to V8.
 */
void Platform::CallDelayedOnForegroundThread(v8::Isolate *isolate, v8::Task *task, double delay_in_seconds)
{
    // schedule this task to be executed in the isolate
    Isolate::scheduleTask(isolate, task, delay_in_seconds);
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
