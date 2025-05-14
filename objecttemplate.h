/**
 *  ObjectTemplate.h
 * 
 *  The template for regular PHP objects that are exposed to JS space.
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include <v8.h>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Forward declarations
 */
class Context;

/**
 *  Class definition
 */
class ObjectTemplate
{
protected:
    /**
     *  The isolate to which it belongs
     *  @var v8::Isolate
     */
    v8::Isolate *_isolate;

    /**
     *  Handle to the template object
     *  @var v8::Global<ObjectTemplate>
     */
    v8::Global<v8::ObjectTemplate> _template;

    /**
     *  We want to remember which features have been enabled for this template
     *  @var bool
     */
    bool _realarray = false;
    bool _arrayaccess = false;
    bool _callable = false;


private:
    /**
     *  Retrieve a property or function from the object
     *  @param  property    The name to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
//    static v8::Intercepted getPropertyCB(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Retrieve a property or function from the object
     *  @param  index       The index to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
//    static v8::Intercepted getIndexCB(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Set a property or function on the object
     *  @param  property    the property to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
//    static v8::Intercepted setPropertyCB(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Set a property or function on the object
     *  @param  index       The index to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
//    static v8::Intercepted setIndexCB(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Retrieve a list of string properties for enumeration
     *  @param  info        callback info
     */
//    static void enumeratePropertiesCB(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  Retrieve a list of integer properties for enumeration
     *  @param  info        callback info
     */
//    static void enumerateIndexesCB(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  The object is called as if it was a function
     *  @param  into        callback info
     */
//    static void callCB(const v8::FunctionCallbackInfo<v8::Value>& info);

    /**
     *  Retrieve a property or function from the object
     *  @param  property    The name to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
    static v8::Intercepted getProperty(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Retrieve a symbol from the object
     *  @param  symbol      the symbol to retrieve
     *  @param  info        callback info
     */
    static v8::Intercepted getSymbol(v8::Local<v8::Symbol> symbol, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Convert to a string
     *  @param  info        callback info
     */
    static v8::Intercepted getString(const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Convert to an iterator
     *  @param  info        callback info
     */
    static v8::Intercepted getIterator(const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Retrieve a property or function from the object
     *  @param  index       The index to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
    static v8::Intercepted getIndex(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Set a property or function on the object
     *  @param  property    the property to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
    static v8::Intercepted setProperty(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Set a property or function on the object
     *  @param  index       The index to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
    static v8::Intercepted setIndex(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Retrieve a list of string properties for enumeration
     *  @param  info        callback info
     */
    static void enumerateProperties(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  Retrieve a list of integer properties for enumeration
     *  @param  info        callback info
     */
    static void enumerateIndexes(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  The object is called as if it was a function
     *  @param  into        callback info
     */
    static void call(const v8::FunctionCallbackInfo<v8::Value>& info);



public:
    /**
     *  Constructor
     *  @param  isolate
     *  @param  value
     */
    ObjectTemplate(v8::Isolate *isolate, const Php::Value &value);
    
    /**
     *  Destructor
     */
    virtual ~ObjectTemplate();

    /**
     *  Is this template useful for a certain object
     *  @param  value
     *  @return bool
     */
    bool matches(const Php::Value &value) const;

    /**
     *  Apply the template on a PHP variable, to turn it into a JS object
     *  @param  value
     *  @return v8::Local<v8::Object>
     */
    v8::Local<v8::Value> apply(const Php::Value &value) const;
};
    
/**
 *  End of namespace
 */
}
