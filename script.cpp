/**
 *  Script.cpp
 * 
 *  Implementation file for the Script class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "script.h"
#include "timeout.h"
#include "scope.h"
#include "php_exception.h"
#include "php_variable.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  script
 *  @throws Php::Exception
 */
Script::Script(const char *script) :
    Script(std::make_shared<Core>(), script) {}

/**
 *  Constructor
 *  @param  core
 *  @param  script
 *  @throws Php::Exception
 */
Script::Script(const std::shared_ptr<Core> &core, const char *source) : _core(core)
{
    // create a scope
    Scope scope(_core);

    // we need the isolate
    auto *isolate = _core->isolate();

    // catch any errors that occur while either compiling or running the script
    v8::TryCatch catcher(isolate);
    
    // compile the code into a script
    auto script = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();

    // get the compiled program
    auto compiled = v8::Script::Compile(scope, script);
    
    // check for success
    if (!compiled.IsEmpty()) _script.Reset(isolate, compiled.ToLocalChecked());

    // report error
    else throw PhpException(isolate, catcher);
}

/**
 *  Execute the script
 *  @param  timeout
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value Script::execute(time_t timeout)
{
    // create a scope
    Scope scope(_core);

    // we need the isolate
    auto *isolate = _core->isolate();
    
    // install a timeout
    Timeout timer(isolate, timeout);

    // for catching errors
    v8::TryCatch catcher(isolate);

    // Run the script to get the result.
    auto result = _script.Get(isolate)->Run(scope);

    // if no exception occured we're done
    if (!catcher.HasCaught()) return result.IsEmpty() ? Php::Value(nullptr) : PhpVariable(isolate, result.ToLocalChecked());

    // if we have terminated we just throw a fixed error message as the catcher.Message()
    // method won't return anything useful (in fact it'll return nothing meaning we just segfault)
    if (catcher.HasTerminated()) throw Php::Exception("Execution timed out");

    // pass this exception on to PHP userspace
    throw PhpException(isolate, catcher);
}

/**
 *  End of namespace
 */
}
