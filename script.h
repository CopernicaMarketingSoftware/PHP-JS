/**
 *  Script.h
 *
 *  Class that is used to parse and evaluate a script.
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
#include "core.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Script
{
private:
    /**
     *  Shared pointer to the actual core data
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

    /**
     *  The compiled script, not bound to a context. This allows us to reset the context between calls
     *  @var v8::Global<v8::UnboundScript>
     */
    v8::Global<v8::UnboundScript> _script;


public:
    /**
     *  Constructor
     *  @param  script
     *  @throws Php::Exception
     */
    Script(const char *script);

    /**
     *  Constructor
     *  @param  core
     *  @param  script
     *  @throws Php::Exception
     */
    Script(const std::shared_ptr<Core> &core, const char *script);

    /**
     *  No copying allowed
     *  @param  that    the object we cannot copy
     */
    Script(const Script &that) = delete;

    /**
     *  Destructor
     */
    virtual ~Script() = default;
    
    /**
     *  Execute the script
     *  @param  timeout
     *  @return Php::Value
     *  @throws Php::Exception
     */
    Php::Value execute(time_t timeout);
};

/**
 *  End namespace
 */
}
