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
 *  @copyright 2015 - 2026 Copernica B.V.
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <v8.h>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Forward declarations
 */
class Core;

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
    static const int CORE_INDEX = 1;

public:
    /**
     *  Constructor
     *  A core-pointer has to be passed to the isolate, to make the Core::upgrade() method work
     *  @paran  context
     */
    Isolate(Core *core)
    {
        // we need an allocator
        _params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        
        // construct the isolate
        _isolate = v8::Isolate::New(_params);
        
        // store a pointer to the core
        _isolate->SetData(CORE_INDEX, core);
        
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
     *  Get the key for associating js and php objects
     *  @return v8::Local<v8::Private>
     */
    v8::Local<v8::Private> symbol()
    {
        return v8::Private::ForApi(_isolate, v8::String::NewFromUtf8Literal(_isolate, "php-js.object"));
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
     *  Get access to the core
     *  @param  isolate
     *  @return Core
     */
    static Core *core(v8::Isolate *isolate)
    {
        // is stored in a data field
        return static_cast<Core *>(isolate->GetData(CORE_INDEX));
    }

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
