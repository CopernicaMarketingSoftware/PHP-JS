/**
 *  array.cpp
 *
 *  Class that wraps around a PHP array to make it available
 *  in ecmascript. As in PHP, properties added to the array,
 *  both from ecmascript and PHP, will become visible on the
 *  other side.
 *
 *  @copyright 2015 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "array.h"
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
 *  @param  array   The array to count
 *  @return the number of numeric, sequential keys
 */
static uint32_t count(const Php::Array &array)
{
    // the variable to store count
    int64_t result = 0;

    // loop over all the properties
    for (auto &property : array)
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
 *  Retrieve a list of properties for enumeration
 *
 *  @param  info        callback info
 */
static void enumerator(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // create a local handle, so properties "fall out of scope" and retrieve the original object
    v8::HandleScope         scope(Isolate::get());
    Handle<Php::Array>      handle(info.Data());

    // create a new array to store all the properties
    v8::Local<v8::Array>    properties(v8::Array::New(Isolate::get()));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : *handle)
    {
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
    v8::HandleScope         scope(Isolate::get());
    Handle<Php::Array>      handle(info.Data());

    // check if we have an item at the requested offset
    if (handle->contains(index))
    {
        // retrieve the variable and store it as the result variable
        info.GetReturnValue().Set(value(handle->get(index)));
    }
    else
    {
        // in javascript, retrieving an unset array offset returns undefined
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
    v8::HandleScope         scope(Isolate::get());

    // retrieve handle to the original object and the property name
    Handle<Php::Array>      handle(info.Data());
    v8::String::Utf8Value   name(property);

    // check if the property exists
    if (handle->contains(*name, name.length()))
    {
        // retrieve the variable and store it as the result variable
        info.GetReturnValue().Set(value(handle->get(*name, name.length())));
    }
    else if (std::strcmp(*name, "length") == 0)
    {
        // return the count from this array
        info.GetReturnValue().Set(count(*handle));
    }
    else
    {
        // in javascript, retrieving an unset array offset returns undefined
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
    // retrieve handle to the original object
    Handle<Php::Array>  handle(info.Data());

    // store the property inside the array
    handle->set(index, value(input));
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
    // retrieve handle to the original object and convert the requested property
    Handle<Php::Array>      handle(info.Data());
    v8::String::Utf8Value   name(property);

    // store the property inside the array
    handle->set(*name, name.length(), value(input));
}

/**
 *  Constructor
 *
 *  @param  array   The array to wrap
 */
Array::Array(Php::Array array) :
    _template(v8::ObjectTemplate::New())
{
    // register the property handlers
    _template->SetNamedPropertyHandler(getter, setter, nullptr, nullptr, enumerator, Handle<Php::Array>(array));
    _template->SetIndexedPropertyHandler(getter, setter, nullptr, nullptr, nullptr, Handle<Php::Array>(array));
}

/**
 *  Retrieve the ecmascript object handle
 *  that can be assigned directly to v8
 *
 *  @return v8::Local<v8::Value>
 */
Array::operator v8::Local<v8::Value> ()
{
    // create a new object based on the template
    return _template->NewInstance();
}

/**
 *  End namespace
 */
}
