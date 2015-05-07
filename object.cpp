/**
 *  object.h
 *
 *  Class that wraps around a PHP object to make it available in
 *  ecmascript. As in PHP, properties added to the object, both
 *  from ecmascript and PHP, will become visible on the other
 *  side.
 *
 *  @copyright 2015 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "object.h"
#include "handle.h"
#include "value.h"
#include <iostream>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Callback function to be used when invoking
 *  member functions defined from the PHP side
 *
 *  @param  info    callback information
 */
static void callback(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    // create a local handle, so properties "fall out of scope"
    v8::HandleScope     scope(isolate());

    // retrieve handle to the original object
    Handle<Php::Value>  handle(info.Data());

    // an array to hold all the arguments
    Php::Array arguments;

    // add all the arguments
    for (int i = 0; i < info.Length(); ++i) arguments.set(i, value(info[i]));

    // catch any exceptions the PHP code might throw
    try
    {
        // now execute the function
        Php::Value result(Php::call("call_user_func_array", handle, arguments));

        // cast the value and set it as return parameter
        info.GetReturnValue().Set(value(result));
    }
    catch (const Php::Exception& exception)
    {
        // pass the exception on to javascript userspace
        isolate()->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate(), exception.what())));
    }
}

/**
 *  Retrieve a list of properties for enumeration
 *
 *  @param  info        callback info
 */
static void enumerator(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(isolate());
    Handle<Php::Object>     handle(info.Data());

    // create a new array to store all the properties
    v8::Local<v8::Array>    properties(v8::Array::New(isolate()));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : *handle)
    {
        // are we dealing with a string here? just set it directly
        if (property.first.isString()) properties->Set(index++, v8::String::NewFromUtf8(isolate(), property.first));

        // if not, we have to clone it to a string and then retrieve the value
        else properties->Set(index++, v8::String::NewFromUtf8(isolate(), property.first.clone(Php::Type::String)));
    }

    // set the value as the 'return' parameter
    info.GetReturnValue().Set(properties);
}

/**
 *  Retrieve a property or function from the object
 *
 *  @param  property    the property to retrieve
 *  @param  info        callback info
 */
static void getter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // create a local handle, so properties "fall out of scope"
    v8::HandleScope         scope(isolate());

    // retrieve handle to the original object and the property name
    Handle<Php::Object>     handle(info.Data());
    v8::String::Utf8Value   name(property);

    /**
     *  This is where it gets a little weird.
     *
     *  PHP has the concept of "magic functions", these are sort
     *  of like operator overloading in C++, only much, much
     *  clumsier.
     *
     *  The issue we have to work around is that, although it is
     *  possible to write an __isset method to define which properties
     *  are possible to retrieve with __get, there is no such sister
     *  function for __call, which means that as soon as the __call
     *  function is implemented, every function suddenly becomes
     *  callable. This is a problem, because we would be creating
     *  function objects to return to v8 instead of retrieving the
     *  properties as expected.
     *
     *  Therefore we are using a three-step test. First we see if the
     *  method exists, and is callable. This filters out __call, so if
     *  this fails, we check to see if a property exists (or can be
     *  retrieved with __get). If that fails, we try again if it is
     *  callable (then it will be a __call for sure!)
     */

    // does the method exist, is it callable or is it a property
    bool method_exists  = Php::call("method_exists", *handle, *name);
    bool is_callable    = handle->isCallable(*name);
    bool contains       = handle->contains(*name, name.length());

    // can we call this as a function?
    if (is_callable && (!contains || method_exists))
    {
        // create a new PHP array that will contain the object and the method to call
        Php::Value callable(Php::Type::Array);

        // add the object and the method to call
        callable.set(0, *handle);
        callable.set(1, Php::Value(*name, name.length()));

        // create the function to be called
        info.GetReturnValue().Set(v8::FunctionTemplate::New(isolate(), callback, Handle<Php::Value>(std::move(callable)))->GetFunction());
    }
    // does the object we are retrieving from have a property with that name?
    else if (contains)
    {
        // retrieve the value, convert it to a javascript handle and return it
        info.GetReturnValue().Set(value(handle->get(*name, name.length())));
    }
    else
    {
        // in javascript, retrieving an unset object property returns undefined
        info.GetReturnValue().SetUndefined();
    }
}

/**
 *  Set a property or function on the object
 *
 *  @param  property    the property to update
 *  @param  input       the new property value
 *  @param  info        callback info
 */
static void setter(v8::Local<v8::String> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    // retrieve handle to the original object
    Handle<Php::Object> handle(info.Data());

    // convert the requested property into a utf8 value (which is castable to a const char *)
    v8::String::Utf8Value   name(property);

    // store the property inside the object
    handle->set(*name, name.length(), value(input));
}

/**
 *  Constructor
 *
 *  @param  object  The object to wrap
 */
Object::Object(Php::Object object) :
    _template(v8::ObjectTemplate::New())
{
    // if the object can be invoked as a function, we register the callback
    if (object.isCallable()) _template->SetCallAsFunctionHandler(callback, Handle<Php::Value>(object));

    // register the property handlers
    _template->SetNamedPropertyHandler(getter, setter, nullptr, nullptr, enumerator, Handle<Php::Object>(object));
}

/**
 *  Retrieve the ecmascript object handle
 *  that can be assigned directly to v8
 *
 *  @return v8::Local<v8::Value>
 */
Object::operator v8::Local<v8::Value> ()
{
    // create a new object based on the template
    return _template->NewInstance();
}

/**
 *  End namespace
 */
}
