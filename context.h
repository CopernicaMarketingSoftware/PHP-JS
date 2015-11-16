/**
 *  context.h
 *
 *  The main javascript class, used for assigning variables
 *  and executing javascript
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
#include <phpcpp.h>
#include <v8.h>
#include "stack.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Forward declarations
 */
class External;

/**
 *  Class definition
 */
class Context : public Php::Base
{
private:
    /**
     *  The context
     *  @var    Stack<V8::Context>
     */
    Stack<v8::Context> _context;

    /**
     *  List of external pointers to track
     *  @var    std::set<External>
     */
    std::set<External*> _externals;
public:
    /**
     *  Constructor
     */
    Context();

    /**
     *  No copying allowed
     *
     *  @param  that    the object we cannot copy
     */
    Context(const Context &that) = delete;

    /**
     *  Move constructor
     *
     *  @param  that    object to move
     */
    Context(Context&&that) = default;

    /**
     *  Destructor
     */
    virtual ~Context();

    /**
     *  Retrieve the currently active context
     *
     *  @return The current context, or a nullptr
     */
    static Context *current();

    /**
     *  Track a new external object
     *
     *  @var    External*
     */
    void track(External *external);

    /**
     *  Unregister an external object
     *
     *  @var    external    The external object we no longer to track
     */
    void untrack(External *external);

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
    void assign(Php::Parameters &params);

    /**
     *  Parse a piece of javascript code
     *
     *  @param  params  array with one parameter: the code to execute
     *  @return Php::Value
     */
    Php::Value evaluate(Php::Parameters &params);
};

/**
 *  End namespace
 */
}
