/**
 *  ToPhp.cpp
 * 
 *  Implementation file to convert a v8/javascript variable into its
 *  PHP counterpart
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "php_variable.h"
#include "php_object.h"
#include "php_function.h"
#include "core.h"
#include "linker.h"
#include "php_array.h"
#include "names.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  isolate
 *  @param  input
 */
PhpVariable::PhpVariable(v8::Isolate *isolate, const v8::Local<v8::Value> &input)
{
    // if we received an invalid input we simply keep empty PHP value
    if (input.IsEmpty() || input->IsNull() || input->IsUndefined()) return;

    // check for boolean true/false or a javascript "Boolean" instance
    if (input->IsBoolean() || input->IsBooleanObject()) 
    {
        // convert the value to a boolean
        v8::Local<v8::Boolean> value(v8::Local<v8::Boolean>::Cast(input));

        // expose the value
        _value = value->Value();
    }
    else if (input->IsInt32())
    {
        // convert the value to an int32
        v8::Local<v8::Int32> value(v8::Local<v8::Int32>::Cast(input));

        // expose the value
        _value = value->Value();
    }
    else if (input->IsNumber() || input->IsNumberObject())
    {
        // convert the value to a number
        v8::Local<v8::Number> value(v8::Local<v8::Number>::Cast(input));
        
        // expose the value
        _value = value->Value();
    }
    else if (input->IsString())
    {
        // convert input to a string
        v8::Local<v8::String> value(v8::Local<v8::String>::Cast(input));

        // convert to 
        v8::String::Utf8Value utf8(isolate, value);
        
        // expose to the php value
        _value = Php::Value(*utf8, utf8.length());
    }
    else if (input->IsStringObject())
    {
        // convert input to a string
        v8::Local<v8::StringObject> value(v8::Local<v8::StringObject>::Cast(input));

        // convert to 
        v8::String::Utf8Value utf8(isolate, value);
        
        // expose to the php value
        _value = Php::Value(*utf8, utf8.length());
    }
    else if (input->IsRegExp())
    {
        // convert input to a regexp
        v8::Local<v8::RegExp> value(v8::Local<v8::RegExp>::Cast(input));

        // convert to 
        v8::String::Utf8Value utf8(isolate, value);
        
        // expose to the php value
        _value = Php::Value(*utf8, utf8.length());
    }
    else if (input->IsFunction())
    {
        // retrieve the function
        auto func = input.As<v8::Function>();

        // use a linker to check if the object was already associated with a php::value
        Linker linker(isolate, func);
        
        // if already linked
        if (linker.valid()) _value = linker.value();

        // otherwise we associate the object now
        else _value = linker.attach(Php::Object(Names::Function, new PhpFunction(isolate, func)));
    }
    else if (input->IsArray())
    {
        // convert input to a string
        v8::Local<v8::Array> value(v8::Local<v8::Array>::Cast(input));

        // we have a helper class for filling arrays
        _value = PhpArray(isolate, value);
    }
    else if (input->IsObject())
    {
        // retrieve the object
        auto object = input.As<v8::Object>();

        // use a linker to check if the object was already associated with a php::value
        Linker linker(isolate, object);
        
        // if already linked
        if (linker.valid()) _value = linker.value();

        // otherwise we associate the object now
        else _value = linker.attach(Php::Object(Names::Object, new PhpObject(isolate, object)));
    }
}

/**
 *  End of namespace
 */
}


