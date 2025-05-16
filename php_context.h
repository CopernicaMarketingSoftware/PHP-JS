/**
 *  PhpContext.h
 *
 *  The main javascript context class as it is exposed to PHP space
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
#include "core.h"

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
class PhpContext : public Php::Base
{
private:
    /**
     *  Shared pointer to the actual core data
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

public:
    /**
     *  Constructor
     */
    PhpContext();

    /**
     *  No copying allowed
     *  @param  that    the object we cannot copy
     */
    PhpContext(const PhpContext &that) = delete;

    /**
     *  Destructor
     */
    virtual ~PhpContext();

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
     * 
     *  @return Php::Value
     */
    Php::Value assign(Php::Parameters &params);

    /**
     *  Parse a piece of javascript code
     *  @param  params  array with one parameter: the code to execute
     *  @return Php::Value
     *  @throws Php::Exception
     */
    Php::Value evaluate(Php::Parameters &params);
};

/**
 *  End namespace
 */
}
