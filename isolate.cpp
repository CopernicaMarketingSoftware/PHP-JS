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

/**
 *  Constructor
 */
Isolate::Isolate()
{
    // create a platform
    Platform::create();

    // create the actual isolate
    _isolate = v8::Isolate::New();

    // and enter it
    _isolate->Enter();
}

/**
 *  Destructor
 */
Isolate::~Isolate()
{
    // leave the isolate scope
    _isolate->Exit();

    // clean it up
    _isolate->Dispose();
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
