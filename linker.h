/**
 *  Linker.h
 * 
 *  Class that can be used to link a javascript-object with a PHP-object,
 *  so that if the javascript is returned multiple times to PHP space,
 *  that we always return the same PHP object. 
 * 
 *  It also makes sure that once the javascript object is destructed,
 *  the associated PHP object is destructed too.
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
#include <v8.h>
#include <phpcpp.h>

/**
 *  Begin of namespace
 */
namespace JS {
    
/**
 *  Forward declarations
 */
class Link;

/**
 *  Class definition
 */
class Linker
{
private:
    /**
     *  The active isolate
     *  @var v8::Isolate
     */
    v8::Isolate *_isolate;

    /**
     *  The private symbol for lookups of the external pointer
     *  @var v8::Local<v8::Private>
     */
    v8::Local<v8::Private> _key;

    /**
     *  The underlying object
     *  @var v8::Local<v8::Object>
     */
    v8::Local<v8::Object> _object;
    
    /**
     *  Helper function to get access to the pointer to the Php::Value
     *  @return Link
     */
    Link *pointer() const;

    /**
     *  Constructor
     *  @param  isolate     the active isolate
     *  @param  key         the private symbol that is associated storing external pointers
     *  @param  object      the javascript object to be linked
     */
    Linker(v8::Isolate *isolate, const v8::Global<v8::Private> &key, const v8::Local<v8::Object> &object);

public:
    /**
     *  Constructor
     *  @param  isolate     the active isolate
     *  @param  object      the javascript object to be linked
     */
    Linker(v8::Isolate *isolate, const v8::Local<v8::Object> &object);
    
    /**
     *  No copying
     *  @param  that
     */
    Linker(const Linker &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Linker() = default;
    
    /**
     *  Is the linker associated with a PHP object?
     *  @return bool
     */
    bool valid() const;
    
    /**
     *  Associate the object with a PHP variable
     *  @param  value
     *  @param  weak
     *  @return Php::Value
     */
    const Php::Value &attach(const Php::Value &value, bool weak = false);
    
    /**
     *  Detach the PHP object from the javascript object
     */
    void detach();
    
    /**
     *  Expose the object in PHP space
     *  @return Php::Value
     */
    Php::Value value() const;
};
    
/**
 *  End of namespace
 */
}

