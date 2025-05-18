/**
 *  isolate.cpp
 *
 *  Simple getter for the (unique) v8 isolate. This
 *  method makes sure that a single isolate gets
 *  constructed and that the v8 engine is properly
 *  initialized once.
 *
 *  Note that this is explicitly not thread-safe,
 *  but it is fast. Since none of our other extensions
 *  are properly thread-safe, this is an acceptable
 *  limitation
 *
 *  @copyright 2015 Copernica B.V.
 */

#if false

/**
 *  Dependencies
 */
#include "platform.h"
#include "isolate.h"
#include <cstring>
#include <cstdlib>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  The isolate instance for this thread
 *  @var    std::unique_ptr<Isolate>
 */
static thread_local std::unique_ptr<Isolate> isolate;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
    virtual void* Allocate(size_t length) {
        void* data = AllocateUninitialized(length);
        return data == NULL ? data : memset(data, 0, length);
    }
    virtual void* AllocateUninitialized(size_t length)
    {
        return malloc(length);
    }
    virtual void Free(void* data, size_t)
    {
        free(data);
    }
};

static ArrayBufferAllocator allocator;

/**
 *  Constructor
 */
Isolate::Isolate()
{
    // create a platform
    Platform::create();

    // create our parameters
    v8::Isolate::CreateParams params;

    // set our custom allocator
    params.array_buffer_allocator = &allocator;

    // create the actual isolate
    _isolate = v8::Isolate::New(params);

    // associate ourselves with this isolate
    // so that we can find it back from the pointer
    _isolate->SetData(0, this);

    // and enter it
    _isolate->Enter();
}

/**
 *  Destructor
 */
Isolate::~Isolate()
{
    // run all tasks still awaiting executing
    runTasks();

    /**
     *  the tasks that are still there are scheduled
     *  somewhere in the future, we just delete these
     *  - this seems like a strange thing to do, but
     *  executing them now seems to throw v8 off guard
     *  and results in either a deadlock or some weird
     *  error message about garbage collection running
     *  in some old "space" or something equally weird
     *  and confusing.
     */
    _tasks.clear();

    // leave the isolate scope
    _isolate->Exit();

    // clean it up
    _isolate->Dispose();
}

/**
 *  Perform all waiting tasks for this isolate
 */
void Isolate::runTasks()
{
    // no tasks? then we're done fast!
    if (_tasks.empty()) return;

    // determine the current time
    auto now = std::chrono::system_clock::now();

    // loop over all the tasks
    for (auto iter = _tasks.begin(); iter != _tasks.end(); ++iter)
    {
        // is the execution time still in the future?
        if (now < iter->first)
        {
            // first task? then don't remove anything
            if (iter == _tasks.begin()) return;

            // remove from the beginning up until the task
            // since we already moved past the last task we
            // need to go back one so we don't remove a task
            // we have not actually executed yet
            _tasks.erase(_tasks.begin(), --iter);

            // tasks executed and removed
            return;
        }

        // execute the task
        iter->second->Run();
    }

    // we ran through all the tasks and executed all of them
    _tasks.clear();
}

/**
 *  Schedule a task to be executed
 *
 *  @param  isolate The isolate to execute the task under
 *  @param  task    The task to execute
 *  @param  delay   Number of seconds to wait before executing
 */
void Isolate::scheduleTask(v8::Isolate *isolate, v8::Task *task, double delay)
{
    // first retrieve the isolate to schedule it under
    auto *real = static_cast<Isolate*>(isolate->GetData(0));

    // determine the time at which the task should be executed
    auto expire = std::chrono::system_clock::now() + std::chrono::microseconds{ static_cast<int64_t>(delay * 1000000) };

    // schedule the task to be executed
    real->_tasks.emplace(std::make_pair(expire, std::unique_ptr<v8::Task>{ task }));
}

/**
 *  Get the isolate for this thread
 *
 *  @return The thread-local isolate instance
 */
v8::Isolate *Isolate::get()
{
    // do we still have to create the isolate?
    if (!isolate) isolate.reset(new Isolate);

    // execute tasks for this isolate
    isolate->runTasks();

    // return the isolate
    return *isolate;
}

/**
 *  Clean up the isolate - if any - for this thread
 */
void Isolate::destroy()
{
    // remove the isolate
    isolate.reset();
}

/**
 *  Cast to the underlying isolate
 *
 *  @return v8::Isolate*
 */
Isolate::operator v8::Isolate* () const
{
    // return member
    return _isolate;
}

/**
 *  End namespace
 */
}

#endif
