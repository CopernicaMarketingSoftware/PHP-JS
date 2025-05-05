/**
 *  context.h
 *
 *  The main javascript class, used for assigning variables
 *  and executing javascript
 *
 *  @copyright 2015 - 2025 Copernica B.V.
 */

#if false

/**
 *  Dependencies
 */
#include "external.h"
#include "context.h"
#include "isolate.h"
#include "value.h"

#include <condition_variable>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Constructor
 */
Context::Context()
{
    // temporary handle, necessary to store the values before the are put in the unique persistent handle
    // note, since we want this variable to fall out of scope, we cannot use member initialization
    v8::HandleScope scope(Isolate::get());

    // now create the context
    _context = v8::Context::New(Isolate::get(), nullptr);

    // store a link to ourselves
    _context->SetAlignedPointerInEmbedderData(1, this);
}

/**
 *  Destructor
 */
Context::~Context()
{
    // destroy all externals
    for (auto *external : _externals) delete external;
}

/**
 *  Retrieve the currently active context
 *
 *  @return The current context, or a nullptr
 */
Context *Context::current()
{
    // retrieve the current context
    auto context = Isolate::get()->GetEnteredContext();

    // if no context is available we are out of options
    if (context.IsEmpty()) return nullptr;

    // retrieve the context from the handle
    return reinterpret_cast<Context*>(context->GetAlignedPointerFromEmbedderData(1));
}

/**
 *  Track a new external object
 *
 *  @var    External*
 */
void Context::track(External *external)
{
    // add to the list of tracked references
    _externals.insert(external);
}

/**
 *  Unregister an external object
 *
 *  @var    external    The external object we no longer to track
 */
void Context::untrack(External *external)
{
    // remove from the list of tracked references
    _externals.erase(external);
}

/**
 *  Assign a variable to the javascript context
 *
 *  @param  params  array of parameters:
 *                  -   string  name of property to assign  required
 *                  -   mixed   property value to assign    required
 *                  -   integer property attributes         optional
 *
 *  The property attributes can be one of the following values
 *
 *  - ReadOnly
 *  - DontEnum
 *  - DontDelete
 *
 *  If not specified, the property will be writable, enumerable and
 *  deletable.
 */
void Context::assign(Php::Parameters &params)
{
    // create a local "scope" and "enter" our context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      contextScope(_context);

    // retrieve the global object from the context
    v8::Local<v8::Object>   global(_context->Global());

    // the attribute for the newly assigned property
    v8::PropertyAttribute   attribute(v8::None);

    // if an attribute was given, assign it
    if (params.size() > 2)
    {
        // check the attribute that was selected
        switch ((int16_t)params[2])
        {
            case v8::None:          attribute = v8::None;       break;
            case v8::ReadOnly:      attribute = v8::ReadOnly;   break;
            case v8::DontDelete:    attribute = v8::DontDelete; break;
            case v8::DontEnum:      attribute = v8::DontEnum;   break;
        }
    }

    // and store the value
    global->ForceSet(value(params[0]), value(params[1]), attribute);
}

/**
 *  Parse a piece of javascript code
 *
 *  @param  params  array with one parameter: the code to execute
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value Context::evaluate(Php::Parameters &params)
{
    // retrieve the optional timeout variable
    int timeout = (params.size() >= 2 ? params[1].numericValue() : 0);

    // create a handle, so that all variables fall "out of scope"
    v8::HandleScope         scope(Isolate::get());

    // enter the compilation/execution scope
    v8::Context::Scope      contextScope(_context);

    // catch any errors that occur while either compiling or running the script
    v8::TryCatch            catcher;

    // compile the code into a script
    v8::Local<v8::String>   source(v8::String::NewFromUtf8(Isolate::get(), params[0]));
    v8::Local<v8::Script>   script(v8::Script::Compile(source));

    // we create a mutex and a condition_variable so we can use wait_until on
    // another thread which we can stop from our main thread. We use this to maybe abort
    // execution of javascript after a certain amount of time.
    bool busy = true;
    std::mutex mutex;
    std::condition_variable condition;

    // create a temporary thread which will mostly just sleep, but kill the script after a certain time period
    std::thread aborter;

    // only create this thread if our timeout is higher than 0
    if (timeout > 0) aborter = std::move(std::thread([this, &busy, &mutex, &condition, timeout]() {

        // time we want to terminate execution
        auto end = std::chrono::system_clock::now() + std::chrono::seconds(timeout);

        // has the execution timed out?
        std::cv_status status = std::cv_status::no_timeout;

        // obtain a lock around busy
        std::unique_lock<std::mutex> lock(mutex);

        // check to prevent a spurious wakeup from removing the timeout
        // additionally, the main thread might have finished before we even start
        while (busy && status != std::cv_status::timeout)
        {
            // we wait until some point in the future (this unlocks the lock until it returns)
            status = condition.wait_until(lock, end);
        }

        // unlock the lock
        lock.unlock();

        // in case we timeout we must terminate execution
        if (status != std::cv_status::timeout) return;

        // create a handle for the local variable that is created by dereferencing _context
        v8::HandleScope scope(Isolate::get());

        // terminate execution
        _context->GetIsolate()->TerminateExecution();
    }));

    // execute the script
    v8::Local<v8::Value>    result(script->Run());

    // obtain a lock around busy
    std::unique_lock<std::mutex> lock(mutex);

    // we are no longer busy
    busy = false;

    // unlock the lock
    lock.unlock();

    // notify the aborter thread
    condition.notify_one();

    // join our aborting thread
    if (aborter.joinable()) aborter.join();

    // did we catch an exception?
    if (catcher.HasCaught())
    {
        // if we have terminated we just throw a fixed error message as the catcher.Message()
        // method won't return anything useful (in fact it'll return nothing meaning we just segfault)
        if (catcher.HasTerminated()) throw Php::Exception("Execution timed out");

        // retrieve the message describing the problem
        v8::Local<v8::Message>  message(catcher.Message());
        v8::Local<v8::String>   description(message->Get());

        // convert the description to utf so we can dump it
        v8::String::Utf8Value   string(description);

        // pass this exception on to PHP userspace
        throw Php::Exception(std::string(*string, string.length()));
    }

    // return the result
    return value(result);
}

/**
 *  End namespace
 */
}


#endif
