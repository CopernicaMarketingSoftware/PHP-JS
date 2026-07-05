/**
 *  PhpScript.h
 *
 *  Class that is exposed to javascript and that can be used to parse
 *  a script, and evaluate it multiple times. It can either be constructed
 *  using "$script = new JS\Script($sourcecode)" or via 
 *  "$script = JS\Context::compile($source)".
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
     *  Shared pointer to the actual core data
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

    /**
     *  The actual script
     *  @var std::optional<Script>
     */
    std::optional<Script> _script;

public:
    /**
     *  Constructor
     *  This one is typically used for "new JS\Script(...)" calls
     */
    PhpScript() : _core(std::make_shared<Core>()) {}

    /**
     *  Constructor
     *  This one is typically used for JS\Context->compile(...) calls
     *  @param  core
     *  @param  source
     *  @throws Php::Exception
     */
    PhpScript(const std::shared_ptr<Core> &core, const char *source) : _core(core)
    {
        // construct the script right now
       _script.emplace(core, source);
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
        _script.emplace(_core, params[0]);
    }
    
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
    Php::Value assign(Php::Parameters &params)
    {
        // pass on
        _core->assign(params[0], params[1], params.size() > 2 ? params[2] : Php::Value(v8::None));
        
        // allow chaining
        return this;
    }
    
    /**
     *  Reset the context of the screen to the initial state
     *  @param  params  array with one optional parameter: the new root object
     *  @return Php::Value
     */
    Php::Value reset(Php::Parameters &params)
    {
        // install the new core
        if (params.size() == 0) _core = std::make_shared<Core>();
        
        // start with the root object
        else _core = std::make_shared<Core>(params[0]);

        // allow chaining
        return this;
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
        return _script->execute(_core, params.size() == 0 ? Php::Value(0) : params[0]);
    }
    
    /**
     *  Alias for execute
     *  @param  params  array with one parameter: the code to execute
     *  @return Php::Value
     *  @throws Php::Exception
     */
    Php::Value __invoke(Php::Parameters &params)
    {
        // pass on
        return _script->execute(_core, params.size() == 0 ? Php::Value(0) : params[0]);
    }
};

/**
 *  End namespace
 */
}
