/**
 *  PhpObject.cpp
 *
 *  Class that wraps around an ecmascript object and makes it available to PHP userspace.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2025 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "php_object.h"
#include "scope.h"
#include "fromphp.h"
#include "php_variable.h"
#include "php_iterator.h"
#include "php_exception.h"
#include "names.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Retrieve a property
 *  @param  name    Name of the property
 *  @return The requested property
 */
Php::Value PhpObject::__get(const Php::Value &name) const
{
    // scope for the call
    Scope scope(_core);
    
    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_core->isolate()).As<v8::Object>());
    
    // get the property value
    auto property = object->Get(scope, FromPhp(_core->isolate(), name));
    
    // if it does not exist, we fall back on the default behavior
    if (property.IsEmpty()) return Php::Base::__get(name);

    // convert the value to a PHP value
    return PhpVariable(_core->isolate(), property.ToLocalChecked());
}

/**
 *  Change a property
 *  @param  name        Name of the property
 *  @param  property    New value for the property
 */
void PhpObject::__set(const Php::Value &name, const Php::Value &property)
{
    // scope for the call
    Scope scope(_core);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_core->isolate()).As<v8::Object>());
    
    // convert the value to a ecmascript value and store it (we explicitly want to ignore the return-value)
    object->Set(scope, FromPhp(_core->isolate(), name), FromPhp(_core->isolate(), property)).Check();
}

/**
 *  Check if a property is set
 *  @param  name        Name of the property
 *  @return Is the property set
 */
bool PhpObject::__isset(const Php::Value &name)
{
    // scope for the call
    Scope scope(_core);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_core->isolate()).As<v8::Object>());

    // check if the object has the requested property
    auto result = object->Has(scope, FromPhp(_core->isolate(), name));
    
    // check for success
    return result.IsJust() && result.FromJust();
}

/**
 *  Call a function
 *  @param  name        Name of the function to call
 *  @param  params      The input parameters
 *  @return The result of the function call
 */
Php::Value PhpObject::__call(const char *name, Php::Parameters &params)
{
    // scope for the call
    Scope scope(_core);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_core->isolate()).As<v8::Object>());
    
    // construct the method name
    auto methodname = v8::String::NewFromUtf8(_core->isolate(), name);
    if (methodname.IsEmpty()) throw Php::Exception("invalid method name");
    
    // variables to store property
    v8::Local<v8::Value> property;
    
    // do the checks
    if (!object->Get(scope, methodname.ToLocalChecked()).ToLocal(&property)) throw Php::Exception("no such property");
    
    // it must be a function
    if (!property->IsFunction()) throw Php::Exception("not a method");

    // convert to a function
    v8::Local<v8::Function> method = property.As<v8::Function>();
    
    // we need the parameters
    std::vector<v8::Local<v8::Value>> args;
    
    // convert the parameters
    for (size_t i = 0; i < params.size(); ++i) 
    {
        // set a parameter
        args.push_back(FromPhp(_core->isolate(), params[i]));
    }
    
    // catch any errors that occur while either compiling or running the script
    v8::TryCatch catcher(_core->isolate());
    
    // the result
    auto result = method->Call(scope, object, args.size(), args.data());
    
    // no exception
    if (!catcher.HasCaught()) return result.IsEmpty() ? Php::Value(nullptr) : PhpVariable(_core->isolate(), result.ToLocalChecked());

    // pass this exception on to PHP userspace
    throw PhpException(_core->isolate(), catcher);
}

/**
 *  Cast to a string
 *  @return The result of the string conversion
 */
Php::Value PhpObject::__toString()
{
    // scope for the call
    Scope scope(_core);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_core->isolate()).As<v8::Object>());

    // convert to string and then to php
    auto result = object->ToString(scope);
    
    // if not set
    if (result.IsEmpty()) return nullptr;
    
    // convert to php space
    return PhpVariable(_core->isolate(), result.ToLocalChecked());
}

/**
 *  Retrieve the iterator
 *  @return The iterator
 */
Php::Iterator *PhpObject::getIterator()
{
    // scope for the call
    Scope scope(_core);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_core->isolate()).As<v8::Object>());

    // create a new iterator instance, cleaned up by PHP-CPP
    return new PhpIterator(this, _core, object);
}

/**
 *  End namespace
 */
}

