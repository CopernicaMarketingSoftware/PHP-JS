/**
 *  PhpContext.cpp
 * 
 *  Implementation file for the PhpContext class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 - 2026 Copernica BV
 */

/**
 *  Dependencies
 */
#include "php_context.h"
#include "php_script.h"
#include "names.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  params
 */
void PhpContext::__construct(Php::Parameters &params)
{
    // if no parameters were supplied, we stick with the default context
    if (params.size() == 0) _core = std::make_shared<Core>();
    
    // the root object was supplied, create a new core
    else _core = std::make_shared<Core>(params[0]);
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
Php::Value PhpContext::assign(Php::Parameters &params)
{
    // pass on
    _core->assign(params[0], params[1], params.size() > 2 ? params[2] : Php::Value(v8::None));
    
    // allow chaining
    return this;
}

/**
 *  Parse a piece of javascript code
 *
 *  @param  params  array with one parameter: the code to execute
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value PhpContext::evaluate(Php::Parameters &params)
{
    // pass on
    return _core->evaluate(params[0], params.size() > 1 ? params[1] : Php::Value(0));
}

/**
 *  Parse a piece of javascript code for multi-use, returns a JS\Script
 *  @param  params  array with one parameter: the code to execute
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value PhpContext::parse(Php::Parameters &params)
{
    // first parameter must the source
    Php::Value source = params[0];
    
    // construct a script (can throw)
    auto *script = new PhpScript(_core, source.clone(Php::Type::String).rawValue());
    
    // wrap in user space object
    return Php::Object(Names::Script, script);
}
    
/**
 *  End of namespace
 */
}

