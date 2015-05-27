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
#include <cstring>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Find the highest existing numerical index in the object
 *
 *  @param  object  The object to count
 *  @return the number of numeric, sequential keys
 */
static uint32_t count(const Php::Object &object)
{
    // the variable to store count
    int64_t result = 0;

    // loop over all the properties
    for (auto &property : object)
    {
        // is it numeric and greater than what we've seen before?
        if (property.first.isNumeric() && property.first >= result)
        {
            // store it
            result = property.first;

            // add another one
            ++result;
        }
    }

    // return the number of keys in the object
    return static_cast<uint32_t>(result);
}

/**
 *  Callback function to be used when invoking
 *  member functions defined from the PHP side
 *
 *  @param  info    callback information
 */
static void callback(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    // create a local handle, so properties "fall out of scope" and retrieve a handle to the original object
    v8::HandleScope     scope(isolate());
    Handle<Php::Object> handle(info.Data());

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
 *  Retrieve a list of numeric properties for enumeration
 *
 *  @param  info        callback info
 */
static void indexed_enumerator(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(isolate());
    Handle<Php::Object>     handle(info.Holder()->GetInternalField(0));

    // create a new array to store all the properties
    v8::Local<v8::Array>    properties(v8::Array::New(isolate()));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : *handle)
    {
        // we only care about numeric indices
        if (!property.first.isNumeric()) continue;

        // add the property to the list
        properties->Set(index++, value(property.first));
    }

    // set the value as the 'return' parameter
    info.GetReturnValue().Set(properties);
}

/**
 *  Retrieve a list of string properties for enumeration
 *
 *  @param  info        callback info
 */
static void named_enumerator(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(isolate());
    Handle<Php::Object>     handle(info.Holder()->GetInternalField(0));

    // create a new array to store all the properties
    v8::Local<v8::Array>    properties(v8::Array::New(isolate()));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : *handle)
    {
        // we only care about string indices
        if (!property.first.isString()) continue;

        // add the property to the list
        properties->Set(index++, value(property.first));
    }

    // set the value as the 'return' parameter
    info.GetReturnValue().Set(properties);
}

/**
 *  Retrieve a property or function from the object
 *
 *  @param  index       The index to find the property
 *  @param  info        callback info
 */
static void getter(uint32_t index, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(isolate());
    Handle<Php::Object>     handle(info.Holder()->GetInternalField(0));

    // check if we have an item at the requested offset
    if (handle->call("offsetExists", static_cast<int64_t>(index)))
    {
        // retrieve the variable and store it as the result variable
        info.GetReturnValue().Set(value(handle->call("offsetGet", static_cast<int64_t>(index))));
    }
    else
    {
        // in javascript, retrieving an unset object property returns undefined
        info.GetReturnValue().SetUndefined();
    }
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
    Handle<Php::Object>     handle(info.Holder()->GetInternalField(0));
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

    // does a property exist by the given name and is it not defined as a method?
    if (contains && !method_exists)
    {
        // retrieve the value, convert it to a javascript handle and return it
        info.GetReturnValue().Set(value(handle->get(*name, name.length())));
    }
    // is it a countable object we want the length off?
    else if (std::strcmp(*name, "length") == 0 && handle->instanceOf("Countable"))
    {
        // return the count from this object
        info.GetReturnValue().Set(count(*handle));
    }
    else if (handle->instanceOf("ArrayAccess") && handle->call("offsetExists", *name))
    {
        // use the array access to retrieve the property
        info.GetReturnValue().Set(value(handle->call("offsetGet", *name)));
    }
    else if (handle->isCallable("__toString") && (std::strcmp(*name, "valueOf") == 0 || std::strcmp(*name, "toString") == 0))
    {
        // create an array with the object and the __toString method to invoke
        Php::Array callable({ *handle, Php::Value{ "__toString", 10 } });

        // create the function to be called
        info.GetReturnValue().Set(v8::FunctionTemplate::New(isolate(), callback, Handle<Php::Value>(std::move(callable)))->GetFunction());
    }
    else if (is_callable)
    {
        // create an array with the object and the method to be called
        Php::Array callable({ *handle, Php::Value{ *name, name.length() } });

        // create the function to be called
        info.GetReturnValue().Set(v8::FunctionTemplate::New(isolate(), callback, Handle<Php::Value>(std::move(callable)))->GetFunction());
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
 *  @param  index       the index to find the property
 *  @param  input       the new property value
 *  @param  info        callback info
 */
static void setter(uint32_t index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(isolate());
    Handle<Php::Object>     handle(info.Holder()->GetInternalField(0));

    // store the property inside the object
    handle->call("offsetSet", static_cast<int64_t>(index), value(input));
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
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(isolate());
    Handle<Php::Object>     handle(info.Holder()->GetInternalField(0));
    v8::String::Utf8Value   name(property);

    // if the object is not implementing ArrayAccess or has the given property as a member
    // then we set it directly, otherwise we use the offsetSet method to use it as an array
    if (!handle->instanceOf("ArrayAccess") || handle->contains(*name, name.length()))
    {
        // store the property inside the object
        handle->set(*name, name.length(), value(input));
    }
    else
    {
        // set it as an array offset
        handle->call("offsetSet", Php::Value{ *name, name.length() }, value(input));
    }
}

/**
 *  Constructor
 *
 *  @param  object  The object to wrap
 */
Object::Object(Php::Object object) :
    _template(v8::ObjectTemplate::New()),
    _object(object)
{
    // TODO: check whether it saves memory and time to re-use object templates

    // reserve space to store the handle to the object as an external reference
    _template->SetInternalFieldCount(1);

    // if the object can be invoked as a function, we register the callback
    if (object.isCallable()) _template->SetCallAsFunctionHandler(callback, Handle<Php::Value>(object));

    // register the property handlers
    _template->SetNamedPropertyHandler(getter, setter, nullptr, nullptr, named_enumerator);

    // if the object implements the ArrayAccess interface, we should also set the indexed property handler
    if (object.instanceOf("ArrayAccess")) _template->SetIndexedPropertyHandler(getter, setter, nullptr, nullptr, indexed_enumerator);
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
    auto instance = _template->NewInstance();

    // attach the object to the instance
    instance->SetInternalField(0, Handle<Php::Value>(_object));

    // return the instance
    return instance;
}

/**
 *  End namespace
 */
}
