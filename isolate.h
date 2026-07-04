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
    static v8::Isolate::CreateParams _params;

    /**
     *  The underlying isolate
     *  @var    v8::Isolate*
     */
    static v8::Isolate *_isolate;

    /**
     *  Templates for wrapping objects
     *  @var std::vector
     */
    static std::vector<Template> _templates;
    
    /**
     *  Total number of instances
     *  @var size_t
     */
    static size_t _instances;
    
public:
    /**
     *  Constructor that is called every time a "core" is created that needs an isolate
     *  @paran  context
     */
    Isolate(Core *core)
    {
        // do we already have an isolate
        if (_instances++ != 0) return;
        
        // we need an allocator
        _params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        
        // construct the isolate
        _isolate = v8::Isolate::New(_params);
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
        // was this the last reference
        if (--_instances != 0) return;
        
        // remove the templates first before we dispose the isolate
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
            if (!prototype.matches(object)) continue;
            
            // we can apply this prototype
            return prototype;
        }
        
        // we need a new template
        _templates.emplace_back(_isolate, object);
        
        // use it
        return _templates.back();
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
