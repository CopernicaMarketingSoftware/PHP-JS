/**
 *  JsContext.cpp
 * 
 *  Implementation file for the JsContext class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "jscontext.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 */
JSContext::JSContext() : _context(std::make_shared<Context>()) {}

/**
 *  Destructor
 */
JSContext::~JSContext()
{
    // tell the underlying context that it is no longer used in PHP 
    // space via a JS\Context instance (although it may still be referenced
    // by object that have been returned to PHP space)
    _context->release();
}

/**
 *  Assign a variable to the javascript context
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
Php::Value JSContext::assign(Php::Parameters &params)
{
    // pass on
    return _context->assign(params[0], params[1], params[2]);
}

/**
 *  Parse a piece of javascript code
 *
 *  @param  params  array with one parameter: the code to execute
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value JSContext::evaluate(Php::Parameters &params)
{
    // pass on
    return _context->evaluate(params[0], params[1]);
}
    
/**
 *  End of namespace
 */
}

