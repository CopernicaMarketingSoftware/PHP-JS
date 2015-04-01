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
    Handle<Php::Array>  handle(info.Data());

    // an array to hold all the arguments
    Php::Array arguments;

    // add all the arguments
    for (int i = 0; i < info.Length(); ++i) arguments.set(i, value(info[i]));

    // now execute the function
    // @todo 
    //      this could throw an exception, should be caught and 
    //      turned into a javascript exception
    Php::Value result(Php::call("call_user_func_array", handle, arguments));

    // cast the value and set it as return parameter
    info.GetReturnValue().Set(value(result));
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

    // does the object we are retrieving from have a property with that name?
    if (handle->contains(*name, name.length()))
    {
        // retrieve the value, convert it to a javascript handle and return it
        info.GetReturnValue().Set(value(handle->get(*name, name.length())));
    }
    else if (handle->isCallable(*name))
    {
        // create a new PHP array that will contain the object and the method to call
        Php::Array callable;

        // add the object and the method to call
        callable.set(0, *handle);
        callable.set(1, Php::Value(*name, name.length()));

        // create the function to be called
        info.GetReturnValue().Set(v8::FunctionTemplate::New(isolate(), callback, Handle<Php::Array>(std::move(callable)))->GetFunction());
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
    // register the property handlers
    _template->SetNamedPropertyHandler(getter, setter, nullptr, nullptr, nullptr, Handle<Php::Object>(object));
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
