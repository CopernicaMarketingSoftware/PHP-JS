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
     *  The associated context (this is a weak-pointer because the context is our parent,
     *  holding a reference to *us* so we do not want to keep it in use
     *  @var std::weak_ptr
     */
    std::weak_ptr<Context> _context;

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
    bool _arrayaccess = false;
    bool _callable = false;


private:
    /**
     *  Retrieve a property or function from the object
     *  @param  property    The name to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
    static v8::Intercepted getPropertyCB(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Retrieve a property or function from the object
     *  @param  index       The index to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
    static v8::Intercepted getIndexCB(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Set a property or function on the object
     *  @param  property    the property to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
    static v8::Intercepted setPropertyCB(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Set a property or function on the object
     *  @param  index       The index to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
    static v8::Intercepted setIndexCB(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Retrieve a list of string properties for enumeration
     *  @param  info        callback info
     */
    static void enumeratePropertiesCB(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  Retrieve a list of integer properties for enumeration
     *  @param  info        callback info
     */
    static void enumerateIndexesCB(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  The object is called as if it was a function
     *  @param  into        callback info
     */
    static void callCB(const v8::FunctionCallbackInfo<v8::Value>& info);

    /**
     *  Retrieve a property or function from the object
     *  @param  property    The name to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
    v8::Intercepted getProperty(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Retrieve a property or function from the object
     *  @param  index       The index to find the property
     *  @param  info        callback info
     *  @return v8::Intercepted
     */
    v8::Intercepted getIndex(unsigned index, const v8::PropertyCallbackInfo<v8::Value> &info);

    /**
     *  Set a property or function on the object
     *  @param  property    the property to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
    v8::Intercepted setProperty(v8::Local<v8::Name> property, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Set a property or function on the object
     *  @param  index       The index to update
     *  @param  input       the new property value
     *  @param  info        callback info
     */
    v8::Intercepted setIndex(unsigned index, v8::Local<v8::Value> input, const v8::PropertyCallbackInfo<void>& info);

    /**
     *  Retrieve a list of string properties for enumeration
     *  @param  info        callback info
     */
    void enumerateProperties(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  Retrieve a list of integer properties for enumeration
     *  @param  info        callback info
     */
    void enumerateIndexes(const v8::PropertyCallbackInfo<v8::Array> &info);
    
    /**
     *  The object is called as if it was a function
     *  @param  into        callback info
     */
    void call(const v8::FunctionCallbackInfo<v8::Value>& info);



public:
    /**
     *  Constructor
     *  @param  context
     *  @param  value
     */
    ObjectTemplate(const std::shared_ptr<Context> &context, const Php::Value &value);
    
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
