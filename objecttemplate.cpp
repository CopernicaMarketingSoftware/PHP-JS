/**
 *  ObjectTemplate.cpp
 * 
 *  Implementation file for the ObjectTemplate class.
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "objecttemplate.h"
#include "context.h"
#include "scope.h"
#include "linker.h"
#include "fromphp.h"
#include "tophp.h"
#include "iterator.h"


#include <iostream>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Find the highest existing numerical index in the object
 *  @param  object  The object to count
 *  @return the number of numeric, sequential keys
 * 
 * 
 *  @todo it is silly to call this on objects that implement Countable!
 */
static uint32_t findMax(const Php::Value &object)
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
 *  Constructor
 *  @param  isolate
 *  @param  object
 */
ObjectTemplate::ObjectTemplate(v8::Isolate *isolate, const Php::Value &value) : 
    _isolate(isolate),
    _realarray(value.isArray()),
    _arrayaccess(value.instanceOf("ArrayAccess")),
    _callable(value.instanceOf("Callable"))
{
    // @todo do we need a scope here?
    
    // get the template as local object
    v8::Local<v8::ObjectTemplate> tpl(v8::ObjectTemplate::New(_isolate));
    
    // pointer to ourselves
    auto self = v8::External::New(_isolate, this);
    
    std::cout << "set-named-prooerty-handles" << std::endl;
    
    // register the property handlers for objects and arrays
    tpl->SetHandler(v8::NamedPropertyHandlerConfiguration(
        &ObjectTemplate::getPropertyCB,                                 // get access to a property         
        &ObjectTemplate::setPropertyCB,                                 // assign a property
        nullptr,                                                        // query to check which properties exist
        nullptr,                                                        // remove a property
        &ObjectTemplate::enumeratePropertiesCB,                         // enumerate over an object
        self                                                            // self-reference
    ));

    if (_arrayaccess || _realarray) std::cout << "set-index-prooerty-handles" << std::endl;
    
    // for ArrayAccess objects we also configure callbacks to get access to properties by their ID
    if (_arrayaccess || _realarray) tpl->SetHandler(v8::IndexedPropertyHandlerConfiguration(
        &ObjectTemplate::getIndexCB,                                    // get access to an index
        &ObjectTemplate::setIndexCB,                                    // assign a property by index
        nullptr,                                                        // query to check which properties exist
        nullptr,                                                        // remove a property
        _realarray ? nullptr : &ObjectTemplate::enumerateIndexesCB,     // enumerate over an object based on the index  // @todo I wonder if this is correct! ArrayAccess does not necessarily imply indexed access
        self                                                            // self-reference
    ));

    if (_callable) std::cout << "set call-as-function" << std::endl;
    
    // when object is callable, we need to install a callback too
    if (_callable) tpl->SetCallAsFunctionHandler(&ObjectTemplate::callCB, self);
    
    // make sure handler is preserved
    _template.Reset(_isolate, tpl);
}

/**
 *  Destructor
 */
ObjectTemplate::~ObjectTemplate()
{
    // forget handle
    _template.Reset();
}

/**
 *  Is this template useful for a certain object
 *  @param  value
 *  @return bool
 */
bool ObjectTemplate::matches(const Php::Value &value) const
{
    // check whetner the object has certain features
    if (_realarray != value.isArray()) return false;
    if (_arrayaccess != value.instanceOf("ArrayAccess")) return false;
    if (_callable != value.isCallable()) return false;
    
    // seems all properties are supported
    return true;
}

/**
 *  Apply the template on a PHP variable, to turn it into a JS object
 *  @param  value
 *  @return v8::Local<v8::Object>
 */
v8::Local<v8::Value> ObjectTemplate::apply(const Php::Value &value) const
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
    // @todo or would it be more effective to wrap in an ArrayObject?
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
v8::Intercepted ObjectTemplate::getPropertyCB(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());
    
    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    return self->getProperty(property, info);
}

/**
 *  Retrieve a property or function from the object
 *  @param  index       The index to find the property
 *  @param  info        callback info
 *  @return v8::Intercepted
 */
v8::Intercepted ObjectTemplate::getIndexCB(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());

    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    return self->getIndex(index, info);
}    

/**
 *  Set a property or function on the object
 *  @param  property    the property to update
 *  @param  input       the new property value
 *  @param  info        callback info
 */
v8::Intercepted ObjectTemplate::setPropertyCB(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());

    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    return self->setProperty(property, input, info);
}

/**
 *  Set a property or function on the object
 *  @param  index       The index to update
 *  @param  input       the new property value
 *  @param  info        callback info
 */
v8::Intercepted ObjectTemplate::setIndexCB(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());

    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    return self->setIndex(index, input, info);
}

/**
 *  Retrieve a list of string properties for enumeration
 *  @param  info        callback info
 */
void ObjectTemplate::enumeratePropertiesCB(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());

    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    self->enumerateProperties(info);
}

/**
 *  Retrieve a list of integer properties for enumeration
 *  @param  info        callback info
 */
void ObjectTemplate::enumerateIndexesCB(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());

    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    self->enumerateIndexes(info);
}

/**
 *  The object is called as if it was a function
 *  @param  into        callback info
 */
void ObjectTemplate::callCB(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    // we need a scope before we access the data
    Scope scope(info.GetIsolate());

    // get self-pointer
    ObjectTemplate *self = static_cast<ObjectTemplate *>(info.Data().As<v8::External>()->Value());
    
    // pass on to the non-static method
    self->call(info);
}

/**
 *  Retrieve a property or function from the object
 *  @param  property    the property to retrieve
 *  @param  info        callback info
 */
v8::Intercepted ObjectTemplate::getProperty(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    // @todo check implementation for objects
    

    std::cout << "ObjectTemplate::getProperty " << std::endl;
    
    
    Scope scope(info.GetIsolate());
    
    if (property->IsSymbol()) 
    {
        std::cout << "is-symbol!" << std::endl;

        v8::Local<v8::Symbol> s = property.As<v8::Symbol>();
        
        v8::Local<v8::Value> sv = s->Description(_isolate);

        v8::String::Utf8Value desc(_isolate, s->Description(_isolate));
        std::cout << "Symbol description: " << *desc << std::endl;
        
        if (s->Equals(scope, v8::Symbol::GetIterator(_isolate)).FromMaybe(false))
        {
            std::cout << "It's an interator" << std::endl;

            // the object that is being accessed
            Php::Value object = Linker(_isolate, info.This()).value();
            
            // create an iterator
            Iterator iter(_isolate, object);

            // use the array access to retrieve the property
            info.GetReturnValue().Set(v8::Global<v8::Value>(_isolate, iter.value()));
            
            // this has been handled by us
            return v8::Intercepted::kYes;
            
            
        }
        // this is not handled by us
        return v8::Intercepted::kNo;
    }
    
    // @todo do we need an implementation here?
    if (property->IsSymbolObject()) std::cout << "is-symbol object!" << std::endl;
    
    
    // the object that is being accessed
    Php::Value object = Linker(_isolate, info.This()).value();
    
    // convert to a utf8value to get the actual c-string
    v8::String::Utf8Value name(_isolate, property.As<v8::String>());

    const char *cstr = *name;
    
    if (cstr == nullptr) std::cout << "nullptr" << std::endl;

    std::cout << "ObjectTemplate::getProperty " << *name << std::endl;

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
    bool method_exists  = object.isObject() && Php::call("method_exists", object, *name);
//    bool is_callable    = object.isCallable(*name);
    bool contains       = object.contains(*name, name.length());
    
    // does a property exist by the given name and is it not defined as a method?
    if (contains && !method_exists)
    {
        // get the object property value
        FromPhp value(_isolate, object.get(*name, name.length()));
        
        // convert it to a javascript handle and return it
        info.GetReturnValue().Set(v8::Global<v8::Value>(_isolate, value.local()));
    }
    // is it a countable object we want the length off?
    else if (std::strcmp(*name, "length") == 0 && object.instanceOf("Countable"))
    {
        // return the count from this object
        info.GetReturnValue().Set(findMax(object));
    }
    else if (object.instanceOf("ArrayAccess") && object.call("offsetExists", *name))
    {
        // get the object property value
        FromPhp value(_isolate, object.call("offsetGet", *name));

        // use the array access to retrieve the property
        info.GetReturnValue().Set(v8::Global<v8::Value>(_isolate, value.local()));
    }
// @todo implementation
//    else if (object.isCallable("__toString") && (std::strcmp(*name, "valueOf") == 0 || std::strcmp(*name, "toString") == 0))
//    {
//        // create an array with the object and the __toString method to invoke
//        Php::Array callable({ object, Php::Value{ "__toString", 10 } });
//
//        // create the function to be called
//        info.GetReturnValue().Set(v8::FunctionTemplate::New(_isolate, callback, Handle(std::move(callable)))->GetFunction());
//    }
//    else if (is_callable)
//    {
//        // create an array with the object and the method to be called
//        Php::Array callable({ object, Php::Value{ *name, name.length() } });
//
//        // create the function to be called
//        info.GetReturnValue().Set(v8::FunctionTemplate::New(Isolate::get(), callback, Handle(std::move(callable)))->GetFunction());
//    }
//    else
//    {
//        // in javascript, retrieving an unset object property returns undefined
//        info.GetReturnValue().SetUndefined();
//    }

    // @todo should we handle this in all cases?
    return v8::Intercepted::kYes;
}

/**
 *  Retrieve a property or function from the object
 *  @param  index       The index to find the property
 *  @param  info        callback info
 *  @return v8::Intercepted
 */
v8::Intercepted ObjectTemplate::getIndex(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    std::cout << "ObjectTemplate::getIndex" << std::endl;

    // the object that is being accessed
    Php::Value object = Linker(_isolate, info.This()).value();
    
    // is the underlying variable an array?
    if (object.isArray() && object.contains(index))
    {
        // make the call
        FromPhp value(_isolate, object.get(static_cast<int64_t>(index)));

        // set the result
        info.GetReturnValue().Set(v8::Global<v8::Value>(_isolate, value.local()));

        // call was handled
        return v8::Intercepted::kYes;
    }
    
    // is the underlying variable an ArrayAccess with this property?
    if (object.isObject() && object.call("offsetExists", static_cast<int64_t>(index)))
    {
        // make the call
        FromPhp value(_isolate, object.call("offsetGet", static_cast<int64_t>(index)));

        // set the result
        info.GetReturnValue().Set(v8::Global<v8::Value>(_isolate, value.local()));
        
        // call was handled
        return v8::Intercepted::kYes;
    }
    
    // call was not handled
    // @todo this is a change of behavior, originally we returned undefined
    return v8::Intercepted::kNo;
}

/**
 *  Set a property or function on the object
 *  @param  property    the property to update
 *  @param  input       the new property value
 *  @param  info        callback info
 */
v8::Intercepted ObjectTemplate::setProperty(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info)
{
    std::cout << "ObjectTemplate::SetProperty" << std::endl;

    // @todo check implementation for arrays
    
    // the object that is being accessed
    Php::Value object = Linker(_isolate, info.This()).value();

    // We are calling into PHP space so we need to catch all exceptions
    // @todo why do we not have such try/catch blocks in other calls?
    try
    {
        // make the call
        object.call("offsetSet", ToPhp(_isolate, property), ToPhp(_isolate, input));
    }
    catch (const Php::Exception& exception)
    {
        // construct the error message
        auto message = v8::String::NewFromUtf8(_isolate, exception.what());
        
        // @todo check if error message is valid
        
        // pass the exception on to javascript userspace
        _isolate->ThrowException(v8::Exception::Error(message.ToLocalChecked()));
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
v8::Intercepted ObjectTemplate::setIndex(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info)
{
    std::cout << "ObjectTemplate::setIndex" << std::endl;

    // the object that is being accessed
    Php::Value object = Linker(_isolate, info.This()).value();

    // We are calling into PHP space so we need to catch all exceptions
    // @todo why do we not have such try/catch blocks in other calls?
    try
    {
        // the variable to set
        ToPhp value(_isolate, input);
        
        // if the underlying variable is an array
        if (object.isArray()) object.set(static_cast<int64_t>(index), value);
        
        // make the call
        else object.call("offsetSet", static_cast<int64_t>(index), value);
    }
    catch (const Php::Exception& exception)
    {
        // construct the error message
        auto message = v8::String::NewFromUtf8(_isolate, exception.what());
        
        // @todo check if error message is valid
        
        // pass the exception on to javascript userspace
        _isolate->ThrowException(v8::Exception::Error(message.ToLocalChecked()));
    }
    
    // call was handled
    return v8::Intercepted::kYes;
}

/**
 *  Retrieve a list of string properties for enumeration
 *  @param  info        callback info
 */
void ObjectTemplate::enumerateProperties(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    std::cout << "ObjectTemplate::enumerate-properties" << std::endl;


    // @todo check implementation for arrays!
    
    
    // the object that is being accessed
    Php::Value object = Linker(_isolate, info.This()).value();

    // create a new array to store all the properties
    v8::Local<v8::Array> properties(v8::Array::New(_isolate));

    // there is no 'push' method on v8::Array, so we simply have
    // to 'Set' the property with the correct index, declared here
    uint32_t index = 0;

    // iterate over the properties in the object
    for (auto &property : object)
    {
        // we only care about string indices
        if (!property.first.isString()) continue;

        // add the property to the list
        auto result = properties->Set(_isolate->GetCurrentContext(), index++, FromPhp(_isolate, property.first));
        
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
void ObjectTemplate::enumerateIndexes(const v8::PropertyCallbackInfo<v8::Array> &info)
{
    std::cout << "ObjectTemplate::enumerate-indexes" << std::endl;
    
    // @todo implementation
}

/**
 *  The object is called as if it was a function
 *  @param  into        callback info
 */
void ObjectTemplate::call(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    std::cout << "ObjectTemplate::call" << std::endl;


    // @todo implementation
}

/**
 *  End of namespace
 */
}

