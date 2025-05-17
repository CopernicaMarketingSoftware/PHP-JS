/**
 *  PhpScript.h
 *
 *  Class that is exposed to javascript and that can be used to parse
 *  a script, and evaluate it multiple times. It can either be constructed
 *  using "$script = new JS\Script($sourcecode)" or via 
 *  "$script = JS\Context::compile($source)".
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
#include "script.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class PhpScript : public Php::Base
{
private:
    /**
     *  The actual script
     *  @var std::optional<Script>
     */
    std::optional<Script> _script;

public:
    /**
     *  Constructor
     */
    PhpScript() = default;

    /**
     *  Constructor
     *  @param  core
     *  @param  source
     *  @throws Php::Exception
     */
    PhpScript(const std::shared_ptr<Core> &core, const char *script)
    {
        // construct right away
        _script.emplace(script);
    }

    /**
     *  No copying allowed
     *  @param  that    the object we cannot copy
     */
    PhpScript(const PhpScript &that) = delete;

    /**
     *  Destructor
     */
    virtual ~PhpScript() = default;
    
    /**
     *  Constructor
     *  @param  params
     */
    void __construct(Php::Parameters &params)
    {
        // construct
        _script.emplace(params[0]);
    }

    /**
     *  Execute script
     *  @param  params  array with one parameter: the code to execute
     *  @return Php::Value
     *  @throws Php::Exception
     */
    Php::Value execute(Php::Parameters &params)
    {
        // pass on
        return _script->execute(params.size() == 0 ? 0 : params[0]);
    }
};

/**
 *  End namespace
 */
}
