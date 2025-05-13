/**
 *  isolate.h
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

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
//#include <v8-platform.h>
//#include <chrono>
#include <v8.h>
//#include <map>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Forward declarations
 */
class Context;

/**
 *  Private class
 */
class Isolate final
{
private:
    /**
     *  The create-params
     *  v8::Isolate::CreateParams
     */
    v8::Isolate::CreateParams _params;

    /**
     *  The underlying isolate
     *  @var    v8::Isolate*
     */
    v8::Isolate *_isolate;

    /**
     *  Indexes for storing pointers
     *  @var    int
     */
    static const int ISOLATE_INDEX = 0;
    static const int CONTEXT_INDEX = 1;


    /**
     *  List of tasks to execute
     *  @var    std::multimap<std::chrono::system_clock::time_point, std::unique_ptr<v8::Task>>
     */
//    std::multimap<std::chrono::system_clock::time_point, std::unique_ptr<v8::Task>> _tasks;

    /**
     *  Perform all waiting tasks for this isolate
     */
//    void runTasks();

public:
    /**
     *  Constructor
     *  A context-pointer has to be passed to the isolate, to make the Context::get() method work
     *  @paran  context
     */
    Isolate(Context *context)
    {
        // we need an allocator
        _params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        
        // construct the isolate
        _isolate = v8::Isolate::New(_params);
        
        // store a pointer to the context
        _isolate->SetData(CONTEXT_INDEX, context);
        
        // store a pointer to the isolate
        _isolate->SetData(ISOLATE_INDEX, this);
    }
    
    /**
     *  No copying
     *  @param  that
     */
    Isolate(const Isolate &that) = delete;

    /**
     *  Destructor
     */
    virtual ~Isolate()
    {
        // free up the isolate
        _isolate->Dispose();
        
        // free up allocator
        delete _params.array_buffer_allocator;
    }

    /**
     *  Get access to the isolate
     *  @param  isolate
     *  @return Isolate
     */
    static Isolate *update(v8::Isolate *isolate)
    {
        // is stored in a data field
        return static_cast<Isolate *>(isolate->GetData(ISOLATE_INDEX));
    }

    /**
     *  Get access to the context
     *  @param  isolate
     *  @return Context
     */
    static Context *context(v8::Isolate *isolate)
    {
        // is stored in a data field
        return static_cast<Context *>(isolate->GetData(CONTEXT_INDEX));
    }

    /**
     *  Schedule a task to be executed
     *
     *  @param  isolate The isolate to schedule the task under
     *  @param  task    The task to execute
     *  @param  delay   Number of seconds to wait before executing
     */
    //static void scheduleTask(v8::Isolate *isolate, v8::Task *task, double delay);

    /**
     *  Get the isolate for this thread
     *
     *  @return The thread-local isolate instance
     */
    //static v8::Isolate *get();

    /**
     *  Clean up the isolate - if any - for this thread
     */
    //static void destroy();

    /**
     *  Cast to the underlying isolate
     *  @return v8::Isolate*
     */
    operator v8::Isolate* () const
    {
        // expose underlying pointer
        return _isolate;
    }
};

/**
 *  End namespace
 */
}
