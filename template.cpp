/**
 *  Template.cpp
 * 
 *  Implementation file for the Template class.
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "template.h"
#include "core.h"
#include "scope.h"
#include "linker.h"
#include "fromphp.h"
#include "php_variable.h"
#include "fromiterator.h"
#include "php_array.h"
#include "exception.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  Watched out: the passed in PHP-value is only used to decide which handlers to install,
 *  but the template can after that be used for multiple PHP variables with a similar
 *  signature (see also Template::apply())
 *  @param  isolate
 *  @param  object
 */
Template::Template(v8::Isolate *isolate, const Php::Value &value) : 
    _isolate(isolate),
    _realarray(value.isArray()),
    _arrayaccess(value.instanceOf("ArrayAccess")),
    _callable(value.isObject() && Php::call("method_exists", value, "__invoke"))
{
    // get the template as local object
    v8::Local<v8::ObjectTemplate> tpl(v8::ObjectTemplate::New(isolate));
    
    // register the property handlers for objects and arrays
    tpl->SetHandler(v8::NamedPropertyHandlerConfiguration(
        &Template::getProperty,                                   // get access to a property         
        &Template::setProperty,                                   // assign a property
        nullptr,                                                  // query to check which properties exist
        nullptr,                                                  // remove a property
        &Template::enumerateProperties                            // enumerate over an object
    ));

    // for ArrayAccess objects we also configure callbacks to get access to properties by their ID
    if (_arrayaccess || _realarray) tpl->SetHandler(v8::IndexedPropertyHandlerConfiguration(
        &Template::getIndex,                                      // get access to an index
        &Template::setIndex,                                      // assign a property by index
        nullptr,                                                  // query to check which properties exist
        nullptr,                                                  // remove a property
        _realarray ? nullptr : &Template::enumerateIndexes        // enumerate over an object based on the index  // @todo I wonder if this is correct! ArrayAccess does not necessarily imply indexed access
    ));

    // when object is callable, we need to install a callback too
    if (_callable) tpl->SetCallAsFunctionHandler(&Template::call);
    
    // make sure handler is preserved
    _template.Reset(isolate, tpl);
}

/**
 *  Destructor
 */
Template::~Template()
{
    // forget handle
    _template.Reset();
}

/**
 *  Is this template useful for a certain object? Does it have the features that are expected?
 *  @param  value
 *  @return bool
 */
bool Template::matches(const Php::Value &value) const
{
    // check whetner the object has certain features
    if (_realarray != value.isArray()) return false;
    if (_arrayaccess != value.instanceOf("ArrayAccess")) return false;
    if (_callable != (value.isObject() && Php::call("method_exists", value, "__invoke"))) return false;
    
    // seems all properties are supported
    return true;
}

/**
 *  Apply the template on a PHP variable, to turn it into a JS object
 *  @param  value
 *  @return v8::Local<v8::Object>
 */
v8::Local<v8::Value> Template::apply(const Php::Value &value) const
{
    // we need the template locally
    v8::Local<v8::ObjectTemplate> tpl(_template.Get(_isolate));
    
    // construct a new instance
    auto result = tpl->NewInstance(_isolate->GetCurrentContext());
    
    // if not valid
    if (result.IsEmpty()) return v8::Undefined(_isolate);

    // the actual ojbect
    auto object = result.ToLocalChecked();
    
    // arrays cannot be weak-referenced, so we won't link them
    if (!value.isObject()) return object;

    // object to link the two objects together
    Linker linker(_isolate, object);
    
    // attach the objects
    linker.attach(value);

    // get the object
    return result.ToLocalChecked();
}

/**
 *  Retrieve a property or function from the object
 *  @param  property    the property to retrieve
 *  @param  info        callback info
 */
v8::Intercepted Template::getProperty(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // if the retrieved property is a symbol
    if (property->IsSymbol()) return getSymbol(property.As<v8::Symbol>(), info);
    
    // symbol-objects are a specific case that we do not expect to ever see in real life, but we can handle it as symbol like this
    if (property->IsSymbolObject()) return getSymbol(v8::Local<v8::Symbol>::Cast(property.As<v8::SymbolObject>()->ValueOf()), info);
    
    // we need the isolate a couple of times
    auto *isolate = info.GetIsolate();
    
    // handle-scope
    Scope scope(isolate);
    
    // the object that is being accessed
    Php::Value object = Linker(isolate, info.This()).value();
    
    // we expect an object or array now
    if (!object.isObject() && !object.isArray()) return v8::Intercepted::kNo;
    
    // convert to a utf8value to get the actual c-string
    v8::Local<v8::String> prop = property.As<v8::String>();

    // for use as c_str    
    v8::String::Utf8Value name(isolate, prop);
    
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
     
    // avoid exceptions
    try
    {
        // does the method exist, is it callable or is it a property
        bool method_exists  = object.isObject() && Php::call("method_exists", object, *name);
        bool is_callable    = object.isCallable(*name);
        bool contains       = object.contains(*name, name.length());
        
        // does a property exist by the given name and is it not defined as a method?
        if (contains && !method_exists)
        {
            // get the object property value
            FromPhp value(isolate, object.get(*name, name.length()));
            
            // convert it to a javascript handle and return it
            info.GetReturnValue().Set(value);

            // handled
            return v8::Intercepted::kYes;
        }
        // is it a countable object we want the length off?
        else if (std::strcmp(*name, "length") == 0 && (object.instanceOf("Countable") || object.isArray()))
        {
            // return the count from this object
            info.GetReturnValue().Set(FromPhp(isolate, Php::call("count", object)));

            // handled
            return v8::Intercepted::kYes;
        }
        else if (object.instanceOf("ArrayAccess") && object.call("offsetExists", *name))
        {
            // get the object property value
            FromPhp value(isolate, object.call("offsetGet", *name));

            // use the array access to retrieve the property
            info.GetReturnValue().Set(value);

            // handled
            return v8::Intercepted::kYes;
        }
        else if (object.isCallable("__toString") && (std::strcmp(*name, "valueOf") == 0 || std::strcmp(*name, "toString") == 0))
        {
            // handle the to-string conversion
            return getString(info);
        }
        else if (is_callable)
        {
            // we goong to pass the "this" and method name via data
            v8::Local<v8::Array> data = v8::Array::New(isolate, 2);
            
            // store the method name and this-pointer in the data
            data->Set(scope, 0, info.This()).Check();
            data->Set(scope, 1, prop).Check();
            
            // create a new function object (with the data holding "this" and the name)
            auto func = v8::Function::New(scope, &Template::method, data).ToLocalChecked();
            
            // create the function to be called
            info.GetReturnValue().Set(func);

            // handled
            return v8::Intercepted::kYes;
        }
        else
        {
            // not handled
            return v8::Intercepted::kNo;
        }
    }
    catch (const Php::Exception &exception)
    {
        // pass on
        isolate->ThrowException(Exception(isolate, exception));

        // handled
        return v8::Intercepted::kYes;
    }
}

/**
 *  Retrieve a property or function from the object
 *  @param  symbol      the symbol to retrieve
 *  @param  info        callback info
 */
v8::Intercepted Template::getSymbol(v8::Local<v8::Symbol> symbol, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // we need the isolate
    auto *isolate = info.GetIsolate();
    
    // create a handlescope
    Scope scope(isolate);

    // to-string conversions
    if (symbol->Equals(scope, v8::Symbol::GetToStringTag(isolate)).FromMaybe(false)) return getString(info);

    // to-primitive conversions are also treated as to-strings
    if (symbol->Equals(scope, v8::Symbol::GetToPrimitive(isolate)).FromMaybe(false)) return getString(info);
    
    // to-iterator conversions
    if (symbol->Equals(scope, v8::Symbol::GetIterator(isolate)).FromMaybe(false)) return getIterator(info);
    
    // not handled
    return v8::Intercepted::kNo;
}

/**
 *  Convert to a string
 *  @param  info        callback info
 */
v8::Intercepted Template::getString(const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // we need the isolate
    auto *isolate = info.GetIsolate();

    // catch exceptions
    try
    {
        // the object that is being accessed
        Php::Value object = Linker(isolate, info.This()).value();
        
        // only when underlying object can be converted to strings
        if (!object.isObject() || !object.isCallable("__toString")) return v8::Intercepted::kNo;
        
        // set the return-value
        info.GetReturnValue().Set(FromPhp(isolate, object.call("__toString")));
    }
    catch (const Php::Exception &exception)
    {
        // pass the exception on to javascript userspace
        isolate->ThrowException(Exception(isolate, exception));
    }
        
    // handled
    return v8::Intercepted::kYes;
}

/**
 *  Convert to an iterator
 *  @param  info        callback info
 */
v8::Intercepted Template::getIterator(const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // we need the isolate
    auto *isolate = info.GetIsolate();
    
    // create a handlescope
    Scope scope(isolate);

    // the object that is being accessed
    Php::Value object = Linker(isolate, info.This()).value();
    
    // if it is not iterable
    if (!object.instanceOf("Traversable") && !object.isArray()) return v8::Intercepted::kNo;
            
    // an iterator should be a function
    auto func = v8::Function::New(scope, [](const v8::FunctionCallbackInfo<v8::Value>& info) {

        // we need the isolate
        auto *isolate = info.GetIsolate();
        
        // create a handlescope
        Scope scope(isolate);
        
        // avoid exceptions
        try
        {
            // get original php object
            Php::Value object = Linker(isolate, info.This()).value();
            
            // the retval that needs updating
            auto retval = info.GetReturnValue();
            
            // if the object is already traversable
            if (object.instanceOf("Iterator")) retval.Set(FromIterator(isolate, object).value());
            
            // of a can indirectly retrieve the iterator?
            else if (object.instanceOf("IteratorAggregate")) retval.Set(FromIterator(isolate, object.call("getIterator")).value());
            
            // arrays themselves can be iterated
            else if (object.isArray()) retval.Set(FromIterator(isolate, Php::Object("ArrayIterator", object)).value());
            
            // this should not happen
            else retval.Set(FromIterator(isolate, Php::Object("EmptyIterator")).value());
        }
        catch (const Php::Exception &exception)
        {
            // pass the exception on to javascript userspace
            isolate->ThrowException(Exception(isolate, exception));
        }

    }).ToLocalChecked();

    // use the array access to retrieve the property
    info.GetReturnValue().Set(func);
    
    // this has been handled by us
    return v8::Intercepted::kYes;
}

/**
 *  Retrieve a property or function from the object
 *  @param  index       The index to find the property
 *  @param  info        callback info
 *  @return v8::Intercepted
 */
v8::Intercepted Template::getIndex(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // some variables
    auto *isolate = info.GetIsolate();

    // handle scope
    Scope scope(isolate);
    
    // avoid exceptions
    try
    {
        // the object that is being accessed
        Php::Value object = Linker(isolate, info.This()).value();
        
        // is the underlying variable an array?
        if (object.isArray() && object.contains(index))
        {
            // make the call
            FromPhp value(isolate, object.get(static_cast<int64_t>(index)));

            // set the result
            info.GetReturnValue().Set(v8::Global<v8::Value>(isolate, value));

            // call was handled
            return v8::Intercepted::kYes;
        }
        
        // is the underlying variable an ArrayAccess with this property?
        if (object.isObject() && object.call("offsetExists", static_cast<int64_t>(index)))
        {
            // make the call
            FromPhp value(isolate, object.call("offsetGet", static_cast<int64_t>(index)));

            // set the result
            info.GetReturnValue().Set(v8::Global<v8::Value>(isolate, value));
            
            // call was handled
            return v8::Intercepted::kYes;
        }
    
        // call was not handled
        return v8::Intercepted::kNo;
    }
    catch (const Php::Exception &exception)
    {
        // pass the exception on to javascript userspace
        isolate->ThrowException(Exception(isolate, exception));
    
        // call was handled
        return v8::Intercepted::kYes;
    }
}

/**
 *  Set a property or function on the object
 *  @param  property    the property to update
 *  @param  input       the new property value
 *  @param  info        callback info
 */
v8::Intercepted Template::setProperty(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info)
{
    // some variables we need a couple of times
    auto isolate = info.GetIsolate();
    
    // handle scope
    Scope scope(isolate);
    
    // We are calling into PHP space so we need to catch all exceptions
    try
    {
        // the object that is being accessed
        Php::Value object = Linker(isolate, info.This()).value();

        // if the underlying variable is an array
        if (object.isArray()) object.set(PhpVariable(isolate, input), PhpVariable(isolate, input));

        // make the call
        else object.call("offsetSet", PhpVariable(isolate, property), PhpVariable(isolate, input));
    }
    catch (const Php::Exception &exception)
    {
        // pass the exception on to javascript userspace
        isolate->ThrowException(Exception(isolate, exception));
    }

    // call was handled
    return v8::Intercepted::kYes;
}

/**
 *  Set a property or function on the object
 *  @param  index       The index to update
 *  @param  input       the new property value
 *  @param  info        callback info
 */
v8::Intercepted Template::setIndex(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info)
{
    // some variables we need a couple of times
    auto isolate = info.GetIsolate();
    
    // handle scope
    Scope scope(isolate);

    // We are calling into PHP space so we need to catch all exceptions
    try
    {
        // the object that is being accessed
        Php::Value object = Linker(info.GetIsolate(), info.This()).value();

        // the variable to set
        PhpVariable value(isolate, input);
        
        // if the underlying variable is an array
        if (object.isArray()) object.set(static_cast<int64_t>(index), value);
        
        // make the call
        else object.call("offsetSet", static_cast<int64_t>(index), value);
    }
    catch (const Php::Exception &exception)
    {
        // pass the exception on to javascript userspace
        isolate->ThrowException(Exception(isolate, exception));
    }
    
    // call was handled
    return v8::Intercepted::kYes;
}

/**
 *  Retrieve a list of string properties for enumeration
 *  @param  info        callback info
 */
void Template::enumerateProperties(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // some variables we need a couple of times
    auto isolate = info.GetIsolate();
    
    // handle scope
    Scope scope(isolate);
    
    // the object that is being accessed
    Php::Value object = Linker(isolate, info.This()).value();

    // create a new array to store all the properties
    v8::Local<v8::Array> properties(v8::Array::New(isolate));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : object)
    {
        // we only care about string indices
        if (!property.first.isString()) continue;

        // add the property to the list
        auto result = properties->Set(scope, index++, FromPhp(isolate, property.first));
        
        // leap out on error
        if (!result.IsJust() || !result.FromJust()) return;
    }

    // set the value as the 'return' parameter
    info.GetReturnValue().Set(properties);
}

/**
 *  Retrieve a list of integer properties for enumeration
 *  @param  info        callback info
 */
void Template::enumerateIndexes(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // some variables we need a couple of times
    auto isolate = info.GetIsolate();
    
    // handle scope
    Scope scope(isolate);
    
    // the object that is being accessed
    Php::Value object = Linker(isolate, info.This()).value();

    // create a new array to store all the properties
    v8::Local<v8::Array> properties(v8::Array::New(isolate));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : object)
    {
        // we only care about integer indices
        if (!property.first.isNumeric()) continue;

        // add the property to the list
        auto result = properties->Set(scope, index++, FromPhp(isolate, property.first));
        
        // leap out on error
        if (!result.IsJust() || !result.FromJust()) return;
    }

    // set the value as the 'return' parameter
    info.GetReturnValue().Set(properties);
}

/**
 *  A function is called that happens to be a method
 *  @param  into        callback info
 */
void Template::method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    // we need the isolate
    auto *isolate = info.GetIsolate();
    
    // we might need a scope
    Scope scope(isolate);
    
    // avoid exceptions
    try
    {
        // the data is an array
        v8::Local<v8::Array> data(info.Data().As<v8::Array>());
        
        // the "this" object is stored in the data
        auto self = data->Get(scope, 0).As<v8::Object>().ToLocalChecked();
        auto prop = data->Get(scope, 1).As<v8::String>().ToLocalChecked();
        
        // the object that is being accessed
        Php::Value object = Linker(isolate, self).value();

        // the callable
        Php::Array callable({object, PhpVariable(isolate, prop)});

        // call the function
        auto result = Php::call("call_user_func_array", callable, PhpArray(info));

        // store return value
        info.GetReturnValue().Set(FromPhp(isolate, result));
    }
    catch (const Php::Exception &exception)
    {
        // pass the exception on to javascript userspace
        isolate->ThrowException(Exception(isolate, exception));
    }
}

/**
 *  The object is called as if it was a function
 *  @param  into        callback info
 */
void Template::call(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    // we need the isolate
    auto *isolate = info.GetIsolate();
    
    // create handle-scope
    Scope scope(isolate);
    
    // avoid exceptions
    try
    {
        // the object that is being accessed
        Php::Value object = Linker(isolate, info.This()).value();

        // call the function
        auto result = Php::call("call_user_func_array", object, PhpArray(info));

        // store return value
        info.GetReturnValue().Set(FromPhp(isolate, result));
    }
    catch (const Php::Exception &exception)
    {
        // pass the exception on to javascript userspace
        isolate->ThrowException(Exception(isolate, exception));
    }
}

/**
 *  End of namespace
 */
}

