/**
 *  context.h
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
#include "newplatform.h"
#include "isolate.h"
//#include <v8.h>
//#include "stack.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Context : public std::enable_shared_from_this<Context>
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
     *  The context
     *  @var    Stack<V8::Context>
     */
    //Stack<v8::Context> _context;

    /**
     *  List of external pointers to track
     *  @var    std::set<External>
     */
    //std::set<External*> _externals;
    
    
    /**
     *  Run the event loop for this isolate
     *  This will resolve promises, expire timers, etc
     *  @return ???
     */
    void run();
    
public:
    /**
     *  Constructor
     */
    Context();

    /**
     *  No copying allowed
     *  @param  that    the object we cannot copy
     */
    Context(const Context &that) = delete;

    /**
     *  Destructor
     */
    virtual ~Context();

    /**
     *  Release the object / it is no longer directly referenced from PHP space
     *  Note that we may still have objects in PHP space that are indirectly referenced
     */
    void release();

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
     *  Expose the context
     *  Watch out: a handling-scope must be passed to prove that is exists
     *  @param  scope
     *  @return v8::Global<v8::Context>
     */
    v8::Local<v8::Context> context(const v8::HandleScope &scope) { return _context.Get(_isolate); }

    /**
     *  Retrieve the currently active context
     *
     *  @return The current context, or a nullptr
     */
    //static Context *current();

    /**
     *  Track a new external object
     *
     *  @var    External*
     */
    //void track(External *external);

    /**
     *  Unregister an external object
     *
     *  @var    external    The external object we no longer to track
     */
    //void untrack(External *external);

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
