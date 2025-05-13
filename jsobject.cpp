/**
 *  jsobject.cpp
 *
 *  Class that wraps around an ecmascript object and makes it available to PHP userspace.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2025 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "jsobject.h"
#include "scope.h"
#include "fromphp.h"
#include "tophp.h"
#include "jsiterator.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  isolate     The isolate
 *  @param  object      The ecmascript object
 */
JSObject::JSObject(v8::Isolate *isolate, const v8::Local<v8::Object> &object) :
    _context(Context::upgrade(isolate)),
    _object(isolate, object) {}

/**
 *  Destructor
 */
JSObject::~JSObject()
{
    // forget the associated javascript object
    _object.Reset();
}

/**
 *  Retrieve a property
 *  @param  name    Name of the property
 *  @return The requested property
 */
Php::Value JSObject::__get(const Php::Value &name) const
{
    // scope for the call
    Scope scope(_context);
    
    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_context->isolate()));
    
    // get the property value
    auto property = object->Get(scope, FromPhp(_context->isolate(), name));
    
    // if it does not exist, we fall back on the default behavior
    // @todo consider better error handling
    if (property.IsEmpty()) return Php::Base::__get(name);

    // convert the value to a PHP value
    return ToPhp(_context->isolate(), property.ToLocalChecked());
}

/**
 *  Change a property
 *  @param  name        Name of the property
 *  @param  property    New value for the property
 */
void JSObject::__set(const Php::Value &name, const Php::Value &property)
{
    // scope for the call
    Scope scope(_context);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_context->isolate()));
    
    // convert the value to a ecmascript value and store it (we explicitly want to ignore the return-value)
    __attribute__((unused)) auto result = object->Set(scope, FromPhp(_context->isolate(), name), FromPhp(_context->isolate(), property));
}

/**
 *  Check if a property is set
 *  @param  name        Name of the property
 *  @return Is the property set
 */
bool JSObject::__isset(const Php::Value &name)
{
    // scope for the call
    Scope scope(_context);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_context->isolate()));

    // check if the object has the requested property
    auto result = object->Has(scope, FromPhp(_context->isolate(), name));
    
    // check for success
    return result.IsJust() && result.FromJust();
}

/**
 *  Call a function
 *  @param  name        Name of the function to call
 *  @param  params      The input parameters
 *  @return The result of the function call
 */
Php::Value JSObject::__call(const char *name, Php::Parameters &params)
{
    // scope for the call
    Scope scope(_context);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_context->isolate()));
    
    // construct the method name
    auto methodname = v8::String::NewFromUtf8(_context->isolate(), name);
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
        args.push_back(FromPhp(_context->isolate(), params[i]));
    }
        
    // the result
    auto result = method->Call(scope, object, args.size(), args.data());
    
    // on success
    if (!result.IsEmpty()) return ToPhp(_context->isolate(), result.ToLocalChecked());
    
    // a failure took place
    // @todo should we be capturing exceptions?
    // @todo possibly report this
    
    // done
    return nullptr;
    
    
    
//    
//    // create a handle scope, so variables "fall out of scope", "enter" the context and retrieve the value
//    v8::HandleScope                     scope(Isolate::get());
//    v8::Context::Scope                  context(_object->CreationContext());
//    v8::Local<v8::Function>             function(_object->Get(value(Php::Value(name))).As<v8::Function>());
//    std::vector<v8::Local<v8::Value>>   input;
//
//    // check whether the value actually exists
//    if (function.IsEmpty())             throw Php::Exception(std::string{ "No such method: " } + name);
//
//    // reserve space for the input values
//    input.reserve(params.size());
//
//    // fill all the elements
//    for (auto &param : params) input.push_back(value(param));
//
//    // execute the function and return the result
//    return value(function->Call(static_cast<v8::Local<v8::Object>>(_object), input.size(), input.data()));
}

/**
 *  Cast to a string
 *  @return The result of the string conversion
 */
Php::Value JSObject::__toString()
{
    // scope for the call
    Scope scope(_context);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_context->isolate()));

    // convert to string and then to php
    auto result = object->ToString(scope);
    
    // if not set
    if (result.IsEmpty()) return nullptr;
    
    // convert to php space
    return ToPhp(_context->isolate(), result.ToLocalChecked());
}

/**
 *  Retrieve the iterator
 *  @return The iterator
 */
Php::Iterator *JSObject::getIterator()
{
    // scope for the call
    Scope scope(_context);

    // get the object in a local variable
    v8::Local<v8::Object> object(_object.Get(_context->isolate()));

    // create a new iterator instance, cleaned up by PHP-CPP
    return new JSIterator(this, _context, object);
}

/**
 *  End namespace
 */
}

