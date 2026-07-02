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
#include "template.h"

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
     *  Templates for wrapping objects
     *  @var std::vector
     */
    std::vector<std::unique_ptr<Template>> _templates;

    /**
     *  Indexes for storing pointers
     *  @var    int
     */
    static const int CORE_INDEX = 1;
    static const int ISOLATE_INDEX = 1;

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
        
        // store a pointer to the isolate and core
        _isolate->SetData(ISOLATE_INDEX, this);
        _isolate->SetData(CORE_INDEX, core);
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
        // remove the templates first
        _templates.clear();
        
        // free up the isolate
        _isolate->Dispose();
        
        // free up allocator
        delete _params.array_buffer_allocator;
    }

    /**
     *  Look for an appropriate template
     *  @param  object
     *  @return Template
     */
    const Template &prototype(const Php::Value &object)
    {
        // check the prototypes that we have
        for (const auto &prototype : _templates)
        {
            // is this one compatible with the object
            if (!prototype->matches(object)) continue;
            
            // we can apply this prototype
            return *prototype;
        }
        
        // we need a new template
        _templates.emplace_back(new Template(_isolate, object));
        
        // use it
        return *_templates.back();
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
