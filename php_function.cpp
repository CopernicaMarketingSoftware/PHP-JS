/**
 *  PhpFunctiom.cpp
 * 
 *  Implementation file for the PhpFunction class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "php_function.h"
#include "php_variable.h"
#include "php_exception.h"
#include "fromphp.h"
#include "scope.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Method that is called when the function is invoked
 *  @param  params
 *  @return Php::Value
 */
Php::Value PhpFunction::__invoke(Php::Parameters &params)
{
    // scope for the call
    Scope scope(_core);

    // get the function in a local variable
    v8::Local<v8::Function> func(_object.Get(_core->isolate()).As<v8::Function>());
    
    // catch any errors that occur while either compiling or running the script
    v8::TryCatch catcher(_core->isolate());

    // create a new array with parameters
    std::vector<v8::Local<v8::Value>> array;
    array.reserve(params.size());

    // iterate over all the given parameters and add them to the arrau
    for (auto &param: params) array.push_back(FromPhp(_core->isolate(), param));

    // now we can actually call the function
    auto result = func->Call(_core->isolate(), scope, v8::Undefined(_core->isolate()), array.size(), array.data());

    // did we catch an exception?
    if (!catcher.HasCaught()) return result.IsEmpty() ? Php::Value(nullptr) : PhpVariable(_core->isolate(), result.ToLocalChecked());

    // throw the exception
    throw PhpException(_core->isolate(), catcher);
}

/**
 *  End of namespace
 */
}
