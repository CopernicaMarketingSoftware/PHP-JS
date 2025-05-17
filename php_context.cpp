/**
 *  PhpContext.cpp
 * 
 *  Implementation file for the PhpContext class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "php_context.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 */
PhpContext::PhpContext() : _core(std::make_shared<Core>()) {}

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
    return _core->assign(params[0], params[1], params[2]);
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
 *  End of namespace
 */
}

