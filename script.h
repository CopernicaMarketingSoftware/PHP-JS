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
     *  The compiled script, not bound to a context. This allows us to reset the context between calls
     *  @var v8::Global<v8::UnboundScript>
     */
    v8::Global<v8::UnboundScript> _script;

public:
    /**
     *  Constructor
     *  Although the theory is that a context is not needed to compile an unbound javascript, it turns
     *  out that the CompileUnboundScript does seem to crash without a current context, hence we pass a core
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
     *  @param  core
     *  @param  timeout
     *  @return Php::Value
     *  @throws Php::Exception
     */
    Php::Value execute(const std::shared_ptr<Core> &core, time_t timeout);
};

/**
 *  End namespace
 */
}
