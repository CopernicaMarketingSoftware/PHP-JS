/**
 *  Script.cpp
 * 
 *  Implementation file for the Script class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 - 2026 Copernica BV
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
 *  Although the theory is that a context is not needed to compile an unbound javascript, it turns
 *  out that the CompileUnboundScript does seem to crash without a current context, hence we pass a core
 *  @param  core
 *  @param  source
 *  @throws Php::Exception
 */
Script::Script(const std::shared_ptr<Core> &core, const char *source)
{
    // enter scope
    Scope scope(core);

    // catch any errors that occur while either compiling or running the script
    v8::TryCatch catcher(core->isolate());
    
    // compile the code into a script
    auto script = v8::String::NewFromUtf8(core->isolate(), source).ToLocalChecked();

    // dont know what this does
    v8::ScriptCompiler::Source script_source(script);

    // compile the script, not yet bound to a context
    auto compiled = v8::ScriptCompiler::CompileUnboundScript(core->isolate(), &script_source);

    // check for success
    if (!compiled.IsEmpty()) _script.Reset(core->isolate(), compiled.ToLocalChecked());

    // report error
    else throw PhpException(core->isolate(), catcher);
}

/**
 *  Execute the script
 *  @param  core
 *  @param  timeout
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value Script::execute(const std::shared_ptr<Core> &core, time_t timeout)
{
    // create a scope
    Scope scope(core);

    // we need the isolate
    auto *isolate = core->isolate();
    
    // install a timeout
    Timeout timer(isolate, timeout);

    // for catching errors
    v8::TryCatch catcher(isolate);

    // bind the script to the current context
    v8::Local<v8::Script> script = _script.Get(isolate)->BindToCurrentContext();

    // Run the script to get the result.
    auto result = script->Run(scope);

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
