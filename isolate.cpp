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

/**
 *  Dependencies
 */
#include "isolate.h"
#include "platform.h"
#include <cstring>
#include <cstdlib>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Private class
 */
class Isolate final
{
private:
    /**
     *  The "platform" we run on
     *  @var    Platform
     */
    Platform _platform;

    /**
     *  The underlying isolate
     *  @var    v8::Isolate*
     */
    v8::Isolate *_isolate;
public:
    /**
     *  Constructor
     */
    Isolate()
    {
        // initialize the ICU and v8 engine
        v8::V8::InitializeICU();
        v8::V8::InitializePlatform(&_platform);
        v8::V8::Initialize();

        // create the actual isolate
        _isolate = v8::Isolate::New();

        // and enter it
        _isolate->Enter();
    }

    /**
     *  Destructor
     */
    ~Isolate()
    {
        // leave the isolate scope
        _isolate->Exit();

        // clean it up
        _isolate->Dispose();

        // and shut down the engine (this also takes care of the ICU)
        v8::V8::Dispose();

        // finally shut down the platform
        v8::V8::ShutdownPlatform();
    }

    /**
     *  Cast to the underlying isolate
     *
     *  @return v8::Isolate*
     */
    operator v8::Isolate* () const
    {
        // return member
        return _isolate;
    }
};

/**
 *  Retrieve the isolate
 *
 *  @return v8::Isolate*
 */
v8::Isolate* isolate()
{
    // the one and only isolate in this thread
    static thread_local Isolate isolate;

    // cast it to the right type
    return isolate;
}

/**
 *  End namespace
 */
}
