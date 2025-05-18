/**
 *  Core.h
 *
 *  The main javascript class, used for assigning variables
 *  and executing javascript
 *
 *  @copyright 2015 - 2025 Copernica B.V.
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include "platform.h"
#include "isolate.h"
#include "template.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Core : public std::enable_shared_from_this<Core>
{
private:
    /**
     *  Pointer to the full javascript v8 platform
     *  @var Platform
     */
    Platform *_platform;

    /**
     *  The Isolate that manages the v8 environment (this is a bit like the "window"
     *  platform in a browser, a fully separated environment)
     *  @var Isolate
     */
    Isolate _isolate;
    
    /**
     *  The context in which variables are stored
     *  @var v8::Global<v8::Context>
     */
    v8::Global<v8::Context> _context;
    
    /**
     *  The private symbol that we use for associating php objects to js objects
     *  @var v8::Global<v8::Private>
     */
    v8::Global<v8::Private> _symbol;

    /**
     *  Templates for wrapping objects
     *  @var std::vector
     */
    std::vector<std::unique_ptr<Template>> _templates;

public:
    /**
     *  Constructor
     */
    Core();

    /**
     *  No copying allowed
     *  @param  that    the object we cannot copy
     */
    Core(const Core &that) = delete;

    /**
     *  Destructor
     */
    virtual ~Core() = default;

    /**
     *  Given an isolate, it is possible to upgrade to the full context
     *  @return std::shared_ptr
     */
    static std::shared_ptr<Core> upgrade(v8::Isolate *isolate)
    {
        return Isolate::core(isolate)->shared_from_this();
    }

    /**
     *  The isolate of this context
     *  @return Isolate
     */
    v8::Isolate *isolate() { return _isolate; }
    
    /** 
     *  The symbol used for linkin php and js objects to each other
     *  @return v8::Global<v8::Private>
     */
    const v8::Global<v8::Private> &symbol() const { return _symbol; }
    
    /**
     *  Wrap a certain PHP object into a javascript object
     *  @param  object      MUST be an object!
     *  @return v8::Local<v8::Value>
     */
    v8::Local<v8::Value> wrap(const Php::Value &object);
    
    /**
     *  Expose the context
     *  Watch out: a handling-scope must be passed to prove that is exists
     *  @param  scope
     *  @return v8::Local<v8::Context>
     */
    v8::Local<v8::Context> context(const v8::HandleScope &scope) { return _context.Get(_isolate); }

    /**
     *  Assign a variable to the javascript context
     *  @param  name        name of property to assign  required
     *  @param  value       value to be assigned
     *  @param  attribytes  property attributes
     *  @return Php::Value
     */
    Php::Value assign(const Php::Value &name, const Php::Value &value, const Php::Value &attributes);

    /**
     *  Parse a piece of javascript code
     *  @param  code        the code to execute
     *  @param  timeout     possible timeout in seconds
     *  @return Php::Value
     *  @throws Php::Exception
     */
    Php::Value evaluate(const Php::Value &code, const Php::Value &timeout);
};

/**
 *  End namespace
 */
}
