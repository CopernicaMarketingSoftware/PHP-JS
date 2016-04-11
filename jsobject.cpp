/**
 *  jsobject.cpp
 *
 *  Class that wraps around an ecmascript object
 *  and makes it available to PHP userspace.
 *
 *  @copyright 2015 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "jsobject.h"
#include "value.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Constructor
 *
 *  @param  base    The base that PHP-CPP insists on
 *  @param  object  The object to iterate
 */
JSObject::Iterator::Iterator(Php::Base *base, const Stack<v8::Object> &object) :
    Php::Iterator(base),
    _object(object),
    _position(0)
{
    // create a handle scope, so variables "fall out of scope" and "enter" the context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(object->CreationContext());

    // assign variables, this would normally be done inside
    // the initializer list, but that way we can't create a
    // HandleScope first and v8 refuses to work without one
    _keys   = _object->GetPropertyNames();
    _size   = _keys->Length();
}

/**
 *  Is the iterator still valid?
 *
 *  @return is an element present at the current offset
 */
bool JSObject::Iterator::valid()
{
    // we should not be out of bounds
    return _position < _size;
}

/**
 *  Retrieve the current value
 *
 *  @return value at current offset
 */
Php::Value JSObject::Iterator::current()
{
    // create a handle scope, so variables "fall out of scope" and "enter" the context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(_object->CreationContext());

    // retrieve the current key, the value and convert it
    return value(_object->Get(_keys->Get(_position)));
}

/**
 *  Retrieve the current key
 *
 *  @return the current key
 */
Php::Value JSObject::Iterator::key()
{
    // create a handle scope, so variables "fall out of scope" and "enter" the context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(_object->CreationContext());

    // retrieve the current key and convert it
    return value(_keys->Get(_position));
}

/**
 *  Move ahead to the next item
 */
void JSObject::Iterator::next()
{
    // move to the next position
    ++_position;
}

/**
 *  Start over at the beginning
 */
void JSObject::Iterator::rewind()
{
    // move back to the beginning
    _position = 0;
}

/**
 *  Constructor
 *
 *  @param  object  The ecmascript object
 */
JSObject::JSObject(v8::Handle<v8::Object> object) :
    _object(object)
{}

/**
 *  Retrieve a property
 *
 *  @param  name    Name of the property
 *  @return The requested property
 */
Php::Value JSObject::__get(const Php::Value &name) const
{
    // create a handle scope, so variables "fall out of scope", "enter" the context and retrieve the property
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(_object->CreationContext());
    v8::Local<v8::Value>    property(_object->Get(value(name)));

    // if it does not exist, we fall back on the default behavior
    if (property.IsEmpty()) return Php::Base::__get(name);

    // convert the value to a PHP value
    return value(property);
}

/**
 *  Change a property
 *
 *  @param  name        Name of the property
 *  @param  property    New value for the property
 */
void JSObject::__set(const Php::Value &name, const Php::Value &property)
{
    // create a handle scope, so variables "fall out of scope" and "enter" the context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(_object->CreationContext());

    // convert the value to a ecmascript value and store it
    _object->Set(value(name), value(property));
}

/**
 *  Check if a property is set
 *
 *  @param  name        Name of the property
 *  @return Is the property set
 */
bool JSObject::__isset(const Php::Value &name)
{
    // create a handle scope, so variables "fall out of scope" and "enter" the context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(_object->CreationContext());

    // check if the object has the requested property
    return _object->Has(value(name));
}

/**
 *  Call a function
 *
 *  @param  name        Name of the function to call
 *  @param  params      The input parameters
 *  @return The result of the function call
 */
Php::Value JSObject::__call(const char *name, Php::Parameters &params)
{
    // create a handle scope, so variables "fall out of scope", "enter" the context and retrieve the value
    v8::HandleScope                     scope(Isolate::get());
    v8::Context::Scope                  context(_object->CreationContext());
    v8::Local<v8::Function>             function(_object->Get(value(Php::Value(name))).As<v8::Function>());
    std::vector<v8::Local<v8::Value>>   input;

    // check whether the value actually exists
    if (function.IsEmpty())             throw Php::Exception(std::string{ "No such method: " } + name);

    // reserve space for the input values
    input.reserve(params.size());

    // fill all the elements
    for (auto &param : params) input.push_back(value(param));

    // execute the function and return the result
    return value(function->Call(static_cast<v8::Local<v8::Object>>(_object), input.size(), input.data()));
}

/**
 *  Cast to a string
 *
 *  @return The result of the string conversion
 */
Php::Value JSObject::__toString()
{
    // create a handle scope, so variables "fall out of scope" and "enter" the context
    v8::HandleScope         scope(Isolate::get());
    v8::Context::Scope      context(_object->CreationContext());

    // convert to string and then to php
    return value(_object->ToString());
}

/**
 *  Retrieve the iterator
 *
 *  @return The iterator
 */
Php::Iterator *JSObject::getIterator()
{
    // create a new iterator instance
    // it is cleaned up by PHP-CPP
    return new Iterator(this, _object);
}

/**
 *  Retrieve the original ecmascript value
 *
 *  @return original ecmascript value
 */
v8::Local<v8::Object> JSObject::object() const
{
    // the stack has the original object
    return _object;
}

/**
 *  End namespace
 */
}
